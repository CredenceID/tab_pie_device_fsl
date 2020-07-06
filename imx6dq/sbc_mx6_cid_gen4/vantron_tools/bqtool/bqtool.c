/*
 * Android utility for programming and debugging TI's bq27xxx
 * family of gas gauges
 *
 * Commands supported:
 * -f : specify flash file
 * -d : i2c number
 *	bqtool -f <bqfs file|dffs file>
 *	Eg: bqtool -f /system/bin/bq27520_v302.bqfs
 *
 *	For details of flashstream file format(.bqfs/.dffs) see:
 *	http://www.ti.com.cn/cn/lit/an/slua541a/slua541a.pdf
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>
#include <stdbool.h>
#include "i2c-dev.h"

/* #define BQPROG_DEBUG */

#define CMD_MAX_DATA_SIZE	128
#define MAX_LINE_LEN		((CMD_MAX_DATA_SIZE + 4) * 3)
#define RETRY_LIMIT		    3
#define CMD_RETRY_DELAY		100 /* in ms */
#define I2C_BUS			    2
#define I2C_DEV			    "/dev/i2c-%d"
#define SYSFS_BQ_SUSPEND    "/sys/module/bq27520_gasgauger/parameters/poll_suspend"
#ifdef __GNUC__
#define __PACKED	__attribute__((packed))
#else
#error "Make sure structure cmd_t is packed"
#endif

typedef enum {
	CMD_INVALID = 0,
	CMD_R,	/* Read */
	CMD_W,	/* Write */
	CMD_C,	/* Compare */
	CMD_N,  /* Equal don't execuate*/
	CMD_X,	/* Delay */
} cmd_type_t;

/*
 * DO NOT change the order of fields - particularly reg
 * should be immediately followed by data
 */
typedef struct {
	cmd_type_t cmd_type;
	uint8_t addr;
	uint8_t reg;
	union {
		uint8_t bytes[CMD_MAX_DATA_SIZE + 1];
		uint16_t delay;
	} data;
	uint8_t data_len;
	uint32_t line_num;
} __PACKED cmd_t;

static uint32_t line_num;

static int i2c_bus = I2C_BUS;
/*
 * Print usage of all commands when cmd == NULL
 * Otherwise print the usage of the respective command
 * Currently we have only one commnd
 */
static void usage(const char *cmd)
{
	fprintf(stderr, "Usage: bqtool -d <i2c bus number> -f <bqfs-file|dffs-file>\n");
}

static void print_delay(cmd_t *cmd)
{
	fprintf(stdout, "delay: %d", cmd->data.delay);
}

static void print_buf(FILE *fd, uint8_t *buf, uint8_t len)
{
	uint8_t i;

	for (i = 0; i < len; i++)
		fprintf(fd, " %02x", buf[i]);
}

static void print_data(cmd_t *cmd)
{
	int i;

	fprintf(stdout, "data: ");
	print_buf(stdout, cmd->data.bytes, cmd->data_len);
}
static void print_addr_reg(cmd_t *cmd)
{
	fprintf(stdout, "addr: %02x ", cmd->addr);
	fprintf(stdout, "reg: %02x ", cmd->reg);
}

#ifdef BQPROG_DEBUG
static void print_cmd(cmd_t *cmd)
{
	switch (cmd->cmd_type) {
	case CMD_R:
		fprintf(stdout, "R: ");
		print_addr_reg(cmd);
		break;
	case CMD_W:
		fprintf(stdout, "W: ");
		print_addr_reg(cmd);
		print_data(cmd);
		break;
	case CMD_C:
		fprintf(stdout, "C: ");
		print_addr_reg(cmd);
		print_data(cmd);
		break;
	case CMD_N:
		fprintf(stdout, "N: ");
		print_addr_reg(cmd);
		print_data(cmd);
		break;
	case CMD_X:
		fprintf(stdout, "X: ");
		print_delay(cmd);
		break;
	default:
		fprintf(stdout, "Unknown: ");
		break;
	}
	fprintf(stdout, "\n");
}
#else
static void print_cmd(cmd_t *cmd)
{
}
#endif

static int read_bq_poll_intvl(void)
{
	int poll_file = -1;
	int poll_intvl = -1;
	char buf[20]; 

	poll_file = open(SYSFS_BQ_SUSPEND, O_RDONLY);

	if ((poll_file >= 0) && read(poll_file, buf, 20))
		sscanf(buf, "%d", &poll_intvl);
	else
		fprintf(stderr, "Failed to read: %s\n", SYSFS_BQ_SUSPEND);

	if (poll_file >= 0)
		close(poll_file);

	return poll_intvl;
}

static bool write_bq_poll_intvl(int poll_intvl)
{
	int poll_file = -1;
	char buf[20];

	poll_file = open(SYSFS_BQ_SUSPEND, O_RDWR);

	if (poll_file >= 0) {
		sprintf(buf, "%d", poll_intvl);
		write(poll_file, buf, 20);
		close(poll_file);
	}

	if (poll_intvl == read_bq_poll_intvl())
		return true;
	else
		return false;
}

static bool i2c_rw(int i2c_file, cmd_t *cmd, int write)
{
	int ret;
	struct i2c_rdwr_ioctl_data i2c_data;
	char *op;
	/* msg[0] for write command and msg[1] for read command */
	struct i2c_msg msgs[2];

	/* Linux expects 7 bit address */
	msgs[0].addr = cmd->addr >> 1;
	/* reg is data too as far as I2C xfr is concerned */
	msgs[0].buf = (char *)&cmd->reg;
	msgs[0].flags = 0;

	if (write) {
		msgs[0].len = cmd->data_len + 1;
		i2c_data.nmsgs = 1;
		op = "write";
	} else {
		msgs[0].len = 1;

		/* read command */
		msgs[1].addr = cmd->addr >> 1;
		msgs[1].buf = (char *)cmd->data.bytes;
		msgs[1].flags = I2C_M_RD;
		msgs[1].len = cmd->data_len;

		i2c_data.nmsgs = 2;
		op = "read";
	}

	i2c_data.msgs = msgs;
	ret = ioctl(i2c_file, I2C_RDWR, &i2c_data);
	if (ret < 0) {
		fprintf(stderr, "I2C %s failed at line %d error = %d\n",
			op, cmd->line_num, ret);
		return false;
	}

	return true;
}

static bool do_exec_cmd(int i2c_file, cmd_t *cmd)
{
	uint8_t tmp_buf[CMD_MAX_DATA_SIZE];

	switch (cmd->cmd_type) {
	case CMD_R:
		return i2c_rw(i2c_file, cmd, 0);

	case CMD_W:
		return i2c_rw(i2c_file, cmd, 1);

	case CMD_C:
		memcpy(tmp_buf, cmd->data.bytes, cmd->data_len);
		if (!i2c_rw(i2c_file, cmd, 0))
			return false;
		if (memcmp(tmp_buf, cmd->data.bytes, cmd->data_len)) {
			fprintf(stderr, "\nCommand C failed at line %d:\n",
				cmd->line_num);
			fprintf(stderr, "Expected data:");
			print_buf(stderr, tmp_buf, cmd->data_len);
			fprintf(stderr, "\nReceived data:");
			print_buf(stderr, cmd->data.bytes, cmd->data_len);
			fprintf(stderr, "\n");
			return false;
		}
		return true;
	case CMD_N:
		memcpy(tmp_buf, cmd->data.bytes, cmd->data_len);
		if (!i2c_rw(i2c_file, cmd, 0))
			return false;
		if (memcmp(tmp_buf, cmd->data.bytes, cmd->data_len) == 0) {
			fprintf(stderr, "\nCommand C failed at line %d:\n",
				cmd->line_num);
			fprintf(stderr, "Expected data:");
			print_buf(stderr, tmp_buf, cmd->data_len);
			fprintf(stderr, "\nReceived data:");
			print_buf(stderr, cmd->data.bytes, cmd->data_len);
			fprintf(stderr, "\n");
			return false;
		}
		return true;

	case CMD_X:
		usleep(cmd->data.delay * 1000);
		return true;

	default:
		fprintf(stderr, "Unsupported command at line %d\n",
			cmd->line_num);
		return false;
	}
}

static bool execute_cmd(int i2c_file, cmd_t *cmd)
{
	int i = 1;
	bool ret;

	ret = do_exec_cmd(i2c_file, cmd);

	while (!ret && i < RETRY_LIMIT) {
		usleep(CMD_RETRY_DELAY * 1000);
		ret = do_exec_cmd(i2c_file, cmd);
		i++;
	}

	if (!ret) {
		fprintf(stderr, "Command execution failed at line %d"
			" addr - 0x%02x reg - 0x%02x, tried %d times\n",
			cmd->line_num, cmd->addr, cmd->reg, RETRY_LIMIT);
	}

	return ret;
}

static bool get_delay(uint16_t *delay)
{
	char *tok;
	uint32_t temp;

	tok = strtok(NULL, " ");
	if (!tok)
		return false; /*end of line or file */

	if (1 != sscanf(tok, "%u", &temp)) {
		fprintf(stderr, "Syntax error while parsing delay at line %d\n",
			line_num);
		return false; /* syntax error */
	}

	if (temp > UINT16_MAX) {
		fprintf(stderr, "Command X delay too high at line %d - %dms\n",
			line_num, temp);
		return false;
	}

	*delay = (uint16_t)temp;
	return true;

}

/*
 * Returns:
 *	 0: success
 *	 1: EOF
 *	-1: Parse error
 */
static int get_byte(uint8_t *byte)
{
	char *tok;
	unsigned int temp;

	tok = strtok(NULL, " \t\r\n");
	if (!tok)
		return 1; /*end of line or file */

	if ((strlen(tok) != 2) || (sscanf(tok, "%2x", &temp) != 1)) {
			fprintf(stderr, "Syntax error at line %d, token - %s,"
				" temp - %x\n", line_num, tok, temp);
			return -1; /* syntax error */
	}

	*byte = (uint8_t)temp;

	return 0;	/* success */
}

static bool get_addr_n_reg(cmd_t *cmd)
{
	if (get_byte(&cmd->addr))
		return false;

	if (get_byte(&cmd->reg))
		return false;

	return true;
}

static bool get_data_bytes(cmd_t *cmd)
{
	int ret, i = 0;
	cmd->data_len = 0;

	do {
		ret = get_byte(&cmd->data.bytes[i++]);
		if (ret == -1)
			return false;
	} while ((ret == 0) && (i <= CMD_MAX_DATA_SIZE));

	if (ret == 0) {
		fprintf(stderr, "More than allowed number of data bytes at"
			" line %d, data_len %d, i %d\n", cmd->line_num,
			cmd->data_len, i);
		return false;
	}

	cmd->data_len = i - 1;

	return true;
}

static bool get_line(FILE *bqfs_file, char **buffer)
{
	int c;
	int i = 0;
	bool ret = true;
	char *buf;

	buf = malloc(MAX_LINE_LEN);
	line_num++;

	while (1) {
		c = fgetc(bqfs_file);

		if (feof(bqfs_file)) {
#ifdef BQPROG_DEBUG
			fprintf(stdout, "EOF\n");
#endif
			break;
		} else if (ferror(bqfs_file)) {
			fprintf(stderr, "File read error\n");
			ret = false;
			break;
		}

		if (((c == '\r') || (c == '\n') || (c == '\t')
			|| (c == ' ')) && (i == 0)) {
			/*
			 * Skip leading white space, if any, at the beginning
			 * of the line because this interferes with strtok
			 */
			fprintf(stderr, "Leading whitespace at line %d\n",
				line_num);
			if (c == '\n')
				line_num++;
			continue;	/* blank line, let's continue */
		} else if (c == '\n') {
			/* We've reached end of line */
			break;
		}

		buf[i++] = c;

		if (i == MAX_LINE_LEN) {
			/*
			 * Re-allocate in case the line is longer than
			 * expected
			 */
			buf = realloc(buf, MAX_LINE_LEN * 2);
			fprintf(stderr, "Line %d longer than expected,"
				" reallocating..\n", line_num);
		} else if (i == MAX_LINE_LEN * 2) {
			/*
			 * The line is already twice the expected maximum length
			 * - maybe the bqfs/dffs needs to be fixed
			 */
			fprintf(stderr, "Line %d too long, abort parsing..\n",
				line_num);
			ret = false;
			break;
		}
	}

	*buffer = buf;
	buf[i] = '\0';

	if (i < 1)
		ret = false;

	return ret;
}

static bool get_cmd(FILE *bqfs_file, cmd_t *cmd)
{
	char *res;
	char *tok;
	char *buf = NULL, *sp;
	int ret;

retry:
	if (!get_line(bqfs_file, &buf))
		goto error;

	/* ignore space */
	sp = buf;
	while (sp && isspace(*sp)) 
		++sp;
	if (!sp || sp[0] == ';') { /* ignore comment */
		free(buf);
		goto retry;
	}

	cmd->line_num = line_num;
	tok = strtok(sp, ":");
	if (!tok || (strlen(tok) != 1)) {
		fprintf(stderr, "Error parsing command at line %d tok=%s"
			" buf=%s", line_num, tok, buf);
		goto error;
	}

	switch (tok[0]) {
	case 'R':
	case 'r':
		cmd->cmd_type = CMD_R;
		if (!get_addr_n_reg(cmd))
			goto error;
		break;
	case 'W':
	case 'w':
		cmd->cmd_type = CMD_W;
		if (!get_addr_n_reg(cmd))
			goto error;
		if (!get_data_bytes(cmd))
			goto error;
		break;
	case 'C':
	case 'c':
		cmd->cmd_type = CMD_C;
		if (!get_addr_n_reg(cmd))
			goto error;
		if (!get_data_bytes(cmd))
			goto error;
		break;
	case 'N':
	case 'n':
		cmd->cmd_type = CMD_N;
		if (!get_addr_n_reg(cmd))
			goto error;
		if (!get_data_bytes(cmd))
			goto error;
		break;
	case 'X':
	case 'x':
		cmd->cmd_type = CMD_X;
		if (!get_delay(&cmd->data.delay))
			goto error;
		break;
	default:
		fprintf(stderr, "No command or unexpected command at"
			" line %d tok=\"%s\" buf=\"%s\"",
			line_num, tok, buf);
		goto error;
	}
	print_cmd(cmd);
	free(buf);
	return true;

error:
	cmd->cmd_type = CMD_INVALID;
	free(buf);
	return false;
}

int bqfs_flash(char *fname)
{
	int poll_intvl = -1;
	int i2c_file = -1;
	int poll_file = -1;
	FILE *bqfs_file = NULL;
	char *line = NULL;
	cmd_t *cmd = NULL;
	int ret = 0;
	char buf[20], i2c_dev[256];

	bqfs_file = fopen(fname, "r");
	if (!bqfs_file) {
		usage("--flash");
		ret = -1;
		goto end;
	}

	poll_intvl = read_bq_poll_intvl();
	/* Turn off polling */
	if (!write_bq_poll_intvl(0)) {
		fprintf(stderr, "Failed to stop driver polling\n");
		ret = -1;
		goto end;
	}

	snprintf(i2c_dev, sizeof i2c_dev,  I2C_DEV, i2c_bus);
	i2c_file = open(i2c_dev, O_RDWR);
	if (i2c_file < 0) {
		fprintf(stderr, "Failed to open I2C device %s\n", i2c_dev);
		ret = -1;
		goto end;
	}

	cmd = malloc(sizeof(cmd_t));

	while (get_cmd(bqfs_file, cmd) && execute_cmd(i2c_file, cmd)) {
		fputc('.', stdout);
		fflush(stdout);
	}

	if (feof(bqfs_file)) {
		fprintf(stdout, "\n%s programmed successfully!\n", fname);
		ret = 0;
	} else {
		fprintf(stdout, "\nprogramming %s failed!!\n", fname);
		ret = -1;
	}

end:
	if (poll_intvl >= 0) {
		if (!write_bq_poll_intvl(poll_intvl))
			fprintf(stderr, "Failed to restore driver polling\n");
	}

	if (poll_file >= 0)
		close(poll_file);

	if (i2c_file >= 0)
		close(i2c_file);

	if (bqfs_file)
		fclose(bqfs_file);

	if (cmd)
		free(cmd);

	return ret;
}

int main(int argc, char **argv)
{
	int c;
	char *bqf = NULL;

	while ((c = getopt(argc, argv, "d:f:")) != -1) {
		switch (c) {
			case 'd':
				i2c_bus = atoi(optarg);
				break;
			case 'f':
				bqf = optarg;
				break;
			default:
				usage(argv[0]);
				return -1;
		}
	}
	argc -= optind;
	argv += optind;
	if (!bqf && argc > 0)
		bqf = argv[0];
	if (!bqf) {
		usage(argv[0]);
		return -1;
	}

	return bqfs_flash(bqf);
}

