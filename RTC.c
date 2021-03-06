#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include "i2c.h"

#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

void Print_Help(void);
void Read_RTC(void);
void Write_RTC(void);
void Sync_RTC(void);

unsigned char BCDtoInt(unsigned char BCD);
unsigned char InttoBCD(unsigned char Int);

int main(int argc, char **argv){

	/* Read  program options */
	while(1){
		static struct option long_options[]=
		{
			{"verbose", no_argument,	&_DEBUG,	1},
			{"debug",	no_argument,	&_DEBUG,	1},

			{"help",	no_argument,		0,		'h'},
			{"read",	required_argument,	0,		'r'},
			{"write",	required_argument,	0, 		'w'},
			{"sync",	required_argument,	0,		's'},
			{0,		0, 			0,		0}
		};
		int option_index = 0;
		int c = getopt_long(argc, argv, "hs:w:r:", long_options, &option_index);

		if (c == -1)
		{
			break;
		}

		switch(c)
		{
			case 'v':
				_DEBUG = 1;
				break;
			case 'h':
				Print_Help();
				break;

			case 'r':
				DEVICE = atoi(optarg);
				Read_RTC();
				break;

			case 'w':
				DEVICE = atoi(optarg);
				Write_RTC();
				break;

			case 's':
				DEVICE = atoi(optarg);
				Sync_RTC();
				break;

			case '?':
				break;

			default:
				break;
		}

	}



	return 0;
}
void Print_Help(void){
	puts("\n\
Usage:	./MOD-RTC [--debug] [parameter] [argument]\n\n\
--verbose	- Print debug messages\n\
--debug		- Same as above\n\
-r, --read	- Read RTC\n\
-w, --write	- Set RTC according the system\n\
-s, --sync	- Syncing system clock with RTC\n\
--help		- Print this help\n");
exit(0);
}
void Read_RTC(void){

	int fd;
	unsigned char buff[10];
	unsigned char data[10];

	buff[0] = 0x02;




	/* Open I2C-BUS */
	I2C_Open(&fd, 0x51);

	/* Write register */
	I2C_Send(&fd, buff, 1) ;

	/* Read the RTC */
	I2C_Read(&fd, data, 7);

	/* Close I2C-BUS */
	I2C_Close(&fd);

	data[0] &= 0x7F;
	data[1] &= 0x7F;
	data[2] &= 0x3F;
	data[3] &= 0x3F;
	data[4] &= 0x07;
	data[5] &= 0x1F;
	data[6] &= 0xFF;

	printf("Sec: %d\n",BCDtoInt(data[0]));
	printf("Min: %d\n",BCDtoInt(data[1]));
	printf("Hour: %d\n",BCDtoInt(data[2]));
	printf("MDays: %d\n",BCDtoInt(data[3]));
	printf("WDays: %d\n",BCDtoInt(data[4]));
	printf("Month: %d\n",BCDtoInt(data[5]));
	printf("Year: %d\n",BCDtoInt(data[6]));

}
void Sync_RTC(void){

	time_t ltime;
	struct tm *rtc_tm;

	ltime = time(NULL);
	rtc_tm = localtime(&ltime);

	int fd;
	unsigned char buff[10];
	unsigned char data[10];
	buff[0] = 0x02;


	/* Open I2C-BUS */
	I2C_Open(&fd, 0x51);

	/* Write register */
	I2C_Send(&fd, buff, 1) ;

	/* Read the RTC */
	I2C_Read(&fd, data, 7);

	/* Close I2C-BUS */
	I2C_Close(&fd);

	data[0] &= 0x7F;
	data[1] &= 0x7F;
	data[2] &= 0x3F;
	data[3] &= 0x3F;
	data[4] &= 0x07;
	data[5] &= 0x1F;
	data[6] &= 0xFF;

	rtc_tm -> tm_sec = BCDtoInt(data[0]);
	rtc_tm -> tm_min = BCDtoInt(data[1]);
	rtc_tm -> tm_hour = BCDtoInt(data[2]);
	rtc_tm -> tm_mday = BCDtoInt(data[3]);
	rtc_tm -> tm_wday = BCDtoInt(data[4]);
	rtc_tm -> tm_mon = BCDtoInt(data[5])-1; //because local time counts months 0-11 but MOD-RTC 1-12
	rtc_tm -> tm_year = BCDtoInt(data[6]);

	const struct timeval tv = {mktime(rtc_tm), 0};
    settimeofday(&tv, 0);
	}


void Write_RTC(void){

	/* Syncin MOD-RTC with system clock */
	time_t ltime;
	struct tm *rtc_tm;
	unsigned char buff[10];
	int fd;


	ltime = time(NULL);
	rtc_tm = localtime(&ltime);


	buff[0] = 0x02;		//Pointer to the first register
	buff[1] = InttoBCD(rtc_tm -> tm_sec);
	buff[2] = InttoBCD(rtc_tm -> tm_min);
	buff[3] = InttoBCD(rtc_tm -> tm_hour);
	buff[4] = InttoBCD(rtc_tm -> tm_mday);
	buff[5] = InttoBCD(rtc_tm -> tm_wday);
	buff[6] = InttoBCD(rtc_tm -> tm_mon+1); //because local time counts months 0-11 but MOD-RTC 1-12
	buff[7] = InttoBCD(rtc_tm -> tm_year);

	I2C_Open(&fd, 0x51);
	I2C_Send(&fd, buff, 8);
	I2C_Close(&fd);


}

unsigned char BCDtoInt(unsigned char BCD){

	unsigned char a;
	unsigned char b;

	a = BCD & 0x0F;
	b = BCD >> 4;

	return b*10 + a;
}
unsigned char InttoBCD(unsigned char Int){

	unsigned char a;
	unsigned char b;

	a = Int % 10;
	b = Int / 10;

	return (b << 4) + a;
}
