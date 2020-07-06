#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

static void usage(char *app)
{
	fprintf(stderr, "%s [pwr_on|pwr_off|disable_on|disable_off|rst_on|rst_off]\n", app);
}

int main(int argc, char **argv)
{

#define PWR_PATH "/sys/van/minipcie_pwr"
#define DIS_PATH "/sys/van/minipcie_dis"
#define RST_PATH "/sys/van/minipcie_rst"

	int fd;
	unsigned char val[2];

	if (argc < 2) {
		usage(argv[0]);
		return -1;
	}

	if (strcmp(argv[1], "pwr_on") == 0) {

		fd = open(PWR_PATH, O_RDWR);
		if (fd > 0) {
			val[0] = '1';
			val[1] = '\n';
			write(fd, val, sizeof(val));
			close(fd);
			fprintf(stderr, "MINIPCIE_PWR on\n");
		}
	} else if (strcmp(argv[1], "pwr_off") == 0) {

		fd = open(PWR_PATH, O_RDWR);
		if (fd > 0) {
			val[0] = '0';
			val[1] = '\n';
			write(fd, val, sizeof(val));
			close(fd);
			fprintf(stderr, "MINIPCIE_PWR off\n");
		}
	} else if (strcmp(argv[1], "disable_on") == 0) {

		fd = open(DIS_PATH, O_RDWR);
		if (fd > 0) {
			val[0] = '1';
			val[1] = '\n';
			write(fd, val, sizeof(val));
			close(fd);
			fprintf(stderr, "MINIPCIE_DISABLE pull\n");
		}
	} else if (strcmp(argv[1], "disable_off") == 0) {

		fd = open(DIS_PATH, O_RDWR);
		if (fd > 0) {
			val[0] = '0';
			val[1] = '\n';
			write(fd, val, sizeof(val));
			close(fd);
			fprintf(stderr, "MINIPCIE_DISABLE release\n");
		}
	} else if (strcmp(argv[1], "rst_on") == 0) {

		fd = open(RST_PATH, O_RDWR);
		if (fd > 0) {
			val[0] = '1';
			val[1] = '\n';
			write(fd, val, sizeof(val));
			close(fd);
			fprintf(stderr, "MINIPCIE_RST pull\n");
		}
	} else if (strcmp(argv[1], "rst_off") == 0) {

		fd = open(RST_PATH, O_RDWR);
		if (fd > 0) {
			val[0] = '0';
			val[1] = '\n';
			write(fd, val, sizeof(val));
			close(fd);
			fprintf(stderr, "MINIPCIE_RST release\n");
		}
	} else {
		usage(argv[0]);
		return -1;
	}

	return 0;
}
