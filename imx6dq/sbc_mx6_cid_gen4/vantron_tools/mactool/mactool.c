/*
*******************************
Author:Lamson.Luo
Date:2016.12.5
Version:rev 1.0.1
this program is used to set  mac addr for ethernet/bluetooth/wireless
-------------------------------
usage:
setaddr: mactool [password] [setnet/setbt/setwl] [macaddr(format xx:xx:xx:xx:xx:xx)]
getaddr: mactool [password] [getnet/getbt/getwl]
*******************************
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define ENET_MAC  "VantNET"
#define BT_MAC      "VantBLE"
#define WL_MAC     "VantWIR"
#define PASSWORD    "Vantron123"
/*stable addr is 0x55 but hw's designing may be changed 0x52*/
//#define EEPROM_PATH "/sys/class/i2c-dev/i2c-1/device/1-0052/eeprom"
#ifdef CONFIG_SPEC_FOR_5071_TMFP
#define EEPROM_PATH "/sys/class/i2c-dev/i2c-2/device/2-0055/eeprom"
#else
#define EEPROM_PATH "/sys/class/i2c-dev/i2c-1/device/1-0055/eeprom"
#endif

void macaddr_get(int,char *);
void macaddr_set(int , char *,int);

static void usage(void)
{
	printf("Mactool Usage:\n\t");	
	printf("mactool password getnet/getbt/getwl\n\t");
	printf("mactool password setnet/setbt/setwl  xx:xx:xx:xx:xx:xx\n\n");
}


int main(int argc, char **argv)
{
	int eeprom_fd;
	int bshift,nret,bparam;
	if (argc < 3 || (strcmp(argv[2], "getnet") &&strcmp(argv[2], "getbt") &&strcmp(argv[2], "getwl") && strcmp(argv[2], "setnet")&& strcmp(argv[2], "setbt")&& strcmp(argv[2], "setwl"))) {
		usage();
		exit(1);
	}
	if (strcmp(argv[1], PASSWORD)) {
		printf("Password error!\n");
		exit(2);
	}

	eeprom_fd = open(EEPROM_PATH, O_RDWR);
	if (eeprom_fd < 0) {
		printf("Open MAC EEPROM error!\n");
		exit(1);
	}

	if (!strcmp(argv[2], "getnet")) {
		bshift=0;
		nret=lseek(eeprom_fd,bshift,SEEK_SET);
		macaddr_get(eeprom_fd,argv[2]);
	}else if(!strcmp(argv[2], "getbt")) {
		bshift=16;
		nret=lseek(eeprom_fd,bshift,SEEK_SET);
		macaddr_get(eeprom_fd,argv[2]);
	}else if(!strcmp(argv[2], "getwl")) {
		bshift=32;
		nret=lseek(eeprom_fd,bshift,SEEK_SET);
		macaddr_get(eeprom_fd,argv[2]);
	} else if (!strcmp(argv[2], "setnet")) {
		bshift=0;
		bparam=0;
		if (argv[3] != NULL) {
			printf("Set Enet-Mac :\n");
			nret=lseek(eeprom_fd,bshift,SEEK_SET);
			macaddr_set(eeprom_fd, argv[3],bparam);
		} else {
			printf("Please input new MAC address!\n");
			usage();
		}
	}
	else if (!strcmp(argv[2], "setbt")) {		
		bshift=16;
		bparam=1;
		if (argv[3] != NULL) {
			printf("Set BT-Mac :\n");			
			nret=lseek(eeprom_fd,bshift,SEEK_SET);
			macaddr_set(eeprom_fd, argv[3],bparam);
		} else {
			printf("Please input new MAC address!\n");
			usage();
		}
	}	
	else if (!strcmp(argv[2], "setwl")) {		
		bshift=32;
		bparam=2;
		if (argv[3] != NULL) {
			printf("Set WL-Mac :\n");			
			nret=lseek(eeprom_fd,bshift,SEEK_SET);
			macaddr_set(eeprom_fd, argv[3],bparam);
		} else {
			printf("Please input new MAC address!\n");
			usage();
		}
	}
		
	close(eeprom_fd);
	return 0;
}


void macaddr_get(int fd,char *bp)
{
	int n , i, j;
	unsigned char macaddr[16] = {0};
	if ((n = read(fd, macaddr, 13)) < 0) {
		printf("%saddr error!\n",bp);
	} else {
		printf("Get Mac Addr OK!\n");
		printf("%saddr : %x",bp,macaddr[7]);
		for (i = 8; i < 13; i++) {
			printf(":%x", macaddr[i]);
		}
		printf("\n");
	}
}


int addr_is_valid(char *macadd)
{
	int i , cnt = 0;

	for (i = 0; i < strlen(macadd) && i < 17; i++) {
		if ((macadd[i] >= '0' && macadd[i] <= '9') || (macadd[i] >= 'A' && macadd[i] <= 'F') || (macadd[i] >= 'a' && macadd[i] <= 'f') || macadd[i] == ':') {
			cnt++;
		} else {
			return 0;
		}
	}

	return 1;
}

void macaddr_set(int fd, char *macadd,int bp)
{
	int cnt = 0,i;
	char buf[8][3] = {0};
	char *str;
	unsigned char addr_tmp[16] = {0};
	char addr_back[24] = "::::::::::::::::::::::";
	char * paddr = addr_back;

	memcpy(addr_back, macadd, 17);
	if (strchr(addr_back, 0) != NULL) {
		cnt = (int)(strchr(addr_back, 0) - addr_back);
		if (cnt <= 17)
			addr_back[cnt] = ':';
	}

	if (!addr_is_valid(macadd)) {
		printf("MACADDR format error!\n");
		return ;
	}
	printf("New Mac Addr[%d] : %s\n", bp,macadd);

	for (i = 0; i < 6; i++) {
		if (strchr(paddr, ':') != NULL) {
			cnt = (int)(strchr(paddr, ':') - paddr);
			memcpy(&buf[i][0], paddr, cnt);
			/* printf("Lamson.Luo--> %s\n", &buf[i][0]); */
			paddr = &paddr[cnt + 1];
		}
	}
	if (bp==0) {
	memcpy(addr_tmp, ENET_MAC, strlen(ENET_MAC));}
	if (bp==1) {
	memcpy(addr_tmp, BT_MAC, strlen(BT_MAC));}
	if (bp==2) {
	memcpy(addr_tmp, WL_MAC, strlen(WL_MAC));
	} 
	 
	for (i = 0; i < 6; i++) {
		addr_tmp[i + 7] = strtol(&buf[i][0], &str, 16);
		/* printf("Lamson.Luo--> 0x%x\n", tmp_mac[i + 7]); */
	}
	
	if ((cnt = write(fd, addr_tmp, 13)) < 13) {
		printf("Set MACADDR error!\n");
	} else {
		printf("Set MACADDR OK!\n");
	}
}


