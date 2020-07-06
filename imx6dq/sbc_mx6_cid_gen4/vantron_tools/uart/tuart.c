/* 
 * Project : NCPP
 * Test routines for uart1
 *
 */

#include <stdio.h>    
#include <stdlib.h>  
#include <unistd.h>  
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>  
#include <termios.h>
#include <errno.h> 
#include <string.h>
#include <sys/select.h>
#include <sys/time.h>

#include <stdarg.h>
#include <signal.h>


#define      PRODUCT_NAME       "TEST_UART"
#define      PRODUCT_VERSION    "0.1"
#define      BUILD_DATE         "20080214"

#define  dbgprintf(fmt, arg1...) fprintf(stderr, fmt, ## arg1)

#define  PRINT_APP() do{dbgprintf("app name    : %s\n", PRODUCT_NAME); \
		dbgprintf("app version : %s\n", PRODUCT_VERSION);	\
		dbgprintf("build date  : %s\n", BUILD_DATE);		\
		dbgprintf("=======================================\n");}while(0)

int set_speed(int fd, int speed)
{
	int   status;
	struct termios   Opt;


	tcgetattr(fd, &Opt);
	tcflush(fd, TCIOFLUSH);
	if(speed==1)
	{
		cfsetispeed(&Opt, B9600);
		cfsetospeed(&Opt, B9600);
	}else if(speed==2)
	{
		cfsetispeed(&Opt, B19200);
		cfsetospeed(&Opt, B19200);
	}else if(speed==3)
	{	
		cfsetispeed(&Opt, B38400);
		cfsetospeed(&Opt, B38400);
	}else if(speed==4)
	{	
		cfsetispeed(&Opt, B115200);
		cfsetospeed(&Opt, B115200);
	}else if(speed==5)
	{	
		cfsetispeed(&Opt, B4800);
		cfsetospeed(&Opt, B4800);
	}else if(speed==6)
	{	
		cfsetispeed(&Opt, B2400);
		cfsetospeed(&Opt, B2400);
	}else if(speed==7)
	{	
		cfsetispeed(&Opt, B230400);
		cfsetospeed(&Opt, B230400);
	}else if(speed==8)
	{	
		cfsetispeed(&Opt, B460800);
		cfsetospeed(&Opt, B460800);
	}else if(speed==9)
	{	
		cfsetispeed(&Opt, B921600);
		cfsetospeed(&Opt, B921600);
	}else if(speed==10)
	{	
		cfsetispeed(&Opt, B2000000);
		cfsetospeed(&Opt, B2000000);
	}else if(speed==11)
	{	
		cfsetispeed(&Opt, B3000000);
		cfsetospeed(&Opt, B3000000);
	}else if(speed==12)
	{	
		cfsetispeed(&Opt, B4000000);
		cfsetospeed(&Opt, B4000000);
	}else
		return -1;
  
	status = tcsetattr(fd, TCSANOW, &Opt);
	if  (status != 0)
		return -1;
	else return 0;
}

int set_Parity(int fd,int databits,int stopbits,int parity)
{
	struct termios options;


	if( tcgetattr( fd,&options)  !=  0)
	{
		return(-1);
	}
	options.c_cflag &= ~CSIZE;
	switch (databits) 
	{
  	case 1:
  		options.c_cflag |= CS7;
  		break;
  	case 2:
		options.c_cflag |= CS8;
		break;
	default:
		return (-1);
	}
	switch (parity)
  	{
	case 1:
		options.c_cflag &= ~PARENB;   /* Clear parity enable */
		options.c_iflag &= ~INPCK;     /* disable parity checking */
		break;
	case 2:
		options.c_cflag |= (PARODD | PARENB); 
		options.c_iflag |= INPCK;             /* odd parity checking */
		break;
	case 3:
		options.c_cflag |= PARENB;     /* Enable parity */
		options.c_cflag &= ~PARODD;
		options.c_iflag |= INPCK;       /*enable parity checking */
		break;
	default:
		return (-1);
	}
	switch (stopbits)
  	{
  	case 1:
  		options.c_cflag &= ~CSTOPB;
		break;
	case 2:
		options.c_cflag |= CSTOPB;
		break;
	default:
		return (-1);
	}
	// default is xon/xoff in linux /dev/ttySx
	tcflush(fd,TCIFLUSH); /* Update the options and do it NOW */
	options.c_iflag&=~(IXON|IXOFF|IXANY);
	options.c_lflag&=~(ECHO);
	options.c_lflag&=~(ECHONL); 
	options.c_oflag&=~(OCRNL);
	options.c_oflag&=~(ONLCR);
	options.c_lflag&=~(ICANON);
	//  options.flag |= 0x80;
  
	options.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP
			     |INLCR|IGNCR|ICRNL|IXON);
	options.c_oflag &= ~OPOST;
	options.c_lflag &= ~(ECHO|ECHONL|ICANON|ISIG|IEXTEN);
	options.c_cflag &= ~(CSIZE|PARENB);
	options.c_cflag |= CS8;

	if (tcsetattr(fd,TCSANOW,&options) != 0)
  	{
		return (-1);
	}
	return (0);
}

void usage(char *app)
{
	PRINT_APP();
	dbgprintf("usage:\n");
	dbgprintf("  %s device [-h|boundrate] [loop|send|recv]\n", app);
	dbgprintf("for examples:\n");
	dbgprintf("  %s -h\n", app);
	dbgprintf("  %s /dev/ttyS1 115200 loop\n", app);
	dbgprintf("  %s /dev/ttyS1 115200 send\n", app);
	dbgprintf("  %s /dev/ttyS1 115200 recv\n", app);
	dbgprintf("baudrate:\n" \
		  "  2400/4800/9600/19200/38400/115200/230400 8N1\n");
}


static int running=1;

void sig_exit(int signo) {
	
	running = 0;
}

int main(int argc, char **argv)
{
	int speed=9600, ret=0, fd=0;
	unsigned char  c_o=0, c_i=0;

	struct timeval timeout;
	fd_set readfds, writefds;

	if ( (argc < 4) || !strcmp(argv[1], "-h") ) {
		usage(argv[0]);
		exit(0);
	}

	fd = open(argv[1], O_RDWR);
	if (fd < 0) {
		dbgprintf("Can't open device\n");
		exit(1);
	}

#define CLOSE_EXIT() do{close(fd);exit(1);}while(0)

	speed = atoi(argv[2]);
	switch (speed) {

	case 9600:
		ret = set_speed(fd, 1);
		break;
	case 19200:
		ret = set_speed(fd, 2);
		break;
	case 38400:
		ret = set_speed(fd, 3);
		break;
	case 115200:
		ret = set_speed(fd, 4);
		break;
	case 4800:
		ret = set_speed(fd, 5);
		break;
	case 2400:
		ret = set_speed(fd, 6);
		break;
	case 230400:
		ret = set_speed(fd, 7);
		break;
	case 460800:
		ret = set_speed(fd, 8);
		break;
	case 921600:
		ret = set_speed(fd, 9);
		break;
	case 2000000:
		ret = set_speed(fd, 10);
		break;
	case 3000000:
		ret = set_speed(fd, 11);
		break;
	case 4000000:
		ret = set_speed(fd, 12);
		break;
	default:
		dbgprintf("Unknown boundrates\n");
		CLOSE_EXIT();
	}

	if ( ret < 0 ) {
		dbgprintf("Can't set baundrate : %d\n", speed);
		CLOSE_EXIT();
	}

	if ( (ret=set_Parity(fd, 2, 1, 1)) < 0) {
		dbgprintf("Can't set paras : 8 N 1\n");
		CLOSE_EXIT();
	}
	dbgprintf("Set UART1: %d, 8 N 1\n", speed);
	sleep(1);

	signal(SIGINT, sig_exit);
	
#define ERR_MSG(err) do{dbgprintf("\nTest Failed, Possible Reasons:\n"); \
		dbgprintf("  1. Device is not UART1\n");		\
		dbgprintf("  2. Rx and Tx of UART1 were not short circuit\n"); \
		dbgprintf("  Err message: %s\n", err);}while(0);

	if (!strcmp(argv[3], "loop")) {

		for (c_o = 0x00; c_o < 0xff; c_o++) {

			FD_ZERO(&readfds);
			FD_ZERO(&writefds);
			FD_SET(fd, &readfds);
			FD_SET(fd, &writefds);

			timeout.tv_sec  = 1;
			timeout.tv_usec = 0;
			if (select(fd+1, NULL, &writefds, NULL, &timeout) <=0) {
				ERR_MSG("Write Timeout");
				CLOSE_EXIT();
			}
			if (write(fd, &c_o, 1) < 1 ) {
				ERR_MSG("Write Error");
				CLOSE_EXIT();
			}

			timeout.tv_sec  = 1;
			timeout.tv_usec = 0;
			if ((ret=select(fd+1, &readfds, NULL, NULL, &timeout)) <=0) {
				ERR_MSG("Read Timeout\n");
				CLOSE_EXIT();
			}

			if (read(fd, &c_i, 1) < 1 || c_i != c_o) {
				printf("%x %x\n", c_i, c_o);
				ERR_MSG("Read Error");
				CLOSE_EXIT();
			}
			if ( !(c_o%10) )   dbgprintf("*");
			if ( c_o==0xff  ) {dbgprintf("\n"); break;}
		}
		dbgprintf("Loop test done\n");
	}

	else if (!strcmp(argv[3], "send")) {

		dbgprintf("Circularly send chars 'a' to 'z', type 'ctrl+c' to quit\n");

		while (running) {
			for (c_i = 'a'; c_i <= 'z'; c_i++) {
				if (!running)
					break;

				FD_ZERO(&writefds);
				/* FD_SET(fd, &writefds); */

				timeout.tv_sec  = 1;
				timeout.tv_usec = 0;
				/* if (select(fd+1, NULL, &writefds, NULL, &timeout) <=0) { */
				/* 	ERR_MSG("Write Timeout"); */
				/* 	CLOSE_EXIT(); */
				/* } */
				select(fd+1, NULL, NULL, NULL, &timeout);

				if (write(fd, &c_i, 1) < 1) {
					ERR_MSG("Write Error");
					CLOSE_EXIT();
				}
				usleep(10);
			}
		}

		dbgprintf("Sending test done\n");
	}

	else if (!strcmp(argv[3], "recv")) {

		dbgprintf("Keep on receiving data and print it in hex, type 'ctrl+c' to quit\n");

		while (running) {

			FD_ZERO(&readfds);
			FD_SET(fd, &readfds);

			timeout.tv_sec  = 1;
			timeout.tv_usec = 0;
			if ((ret=select(fd+1, &readfds, NULL, NULL, &timeout)) <=0)
				continue;

			if (read(fd, &c_i, 1) < 1) {
				ERR_MSG("Read Error");
				CLOSE_EXIT();
			}

			dbgprintf("%c", c_i);
		}

		dbgprintf("Receving test done\n");
	}
	else
		usage(argv[0]);

	close(fd);
	return 0;
}
