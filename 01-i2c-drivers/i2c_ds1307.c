#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include "at24c32_lib.h"

#define I2C_DEVICE "/dev/i2c-1"
#define EEPROM_ADDRESS 0x68
#define Second_Reg 0x00
#define Minutes_Reg 0x01
#define Hours_Reg 0x02
#define Day_Reg 0x03
#define Date_Reg 0x04
#define Momth_Reg 0x05
#define Year_Reg 0x06
#define Control_Reg 0x07
int fd=-1;
typedef struct{
uint8_t sec;
uint8_t min;
uint8_t hour;
uint8_t day;
uint8_t date;
uint8_t mon;
uint8_t year;
}rtc_time_t;
//-------------------------BCD Function-----------------------//
uint8_t bcd_to_dec(uint8_t val)
{
return (val >> 4) * 10 + (val & 0x0F);
}

uint8_t dec_to_bcd(uint8_t val)
{
return ((val / 10) << 4) | (val % 10);
}
//--------------------------Read Time------------------------//
int get_time(rtc_time_t *t)
{
uint8_t buf[7];
uint8_t addr=0x00;
//select register address 0x00
if( write(fd,&addr,1) != 1) return -1;
if(read(fd,buf,7) != 7) return -1;

t->sec = bcd_to_dec(buf[0] & 0x7F);
t->min = bcd_to_dec(buf[1]);
t->hour = bcd_to_dec(buf[2] & 0x3F);
t->day = bcd_to_dec(buf[3]);
t->date = bcd_to_dec(buf[4]);
t->mon = bcd_to_dec(buf[5]);
t->year = bcd_to_dec(buf[6]);
return 0;
}
//-------------------------Set Time--------------------//
int set_time(const rtc_time_t *t)
{
uint8_t buf[8];
buf[0]=0x00;// register address
//data
buf[1]= dec_to_bcd(t->sec) & 0x7F;//set bit 7 for 0 + set sec
buf[2]= dec_to_bcd(t->min);//set min
buf[3]= dec_to_bcd(t->hour);//set h->bit6=0-->24  /bit6=1-->12
buf[4]= dec_to_bcd(t->day);//week day
buf[5]= dec_to_bcd(t->date);//day
buf[6]= dec_to_bcd(t->mon);//mon
buf[7]= dec_to_bcd(t->year);//year
if(write(fd,buf,8) != 8) return -1;
printf("write param is done\n");
return 0;
}
//-------------------------------Main-----------------------//
int main()
{
//-----------------init_i2c---------------------------//
fd = open(I2C_DEVICE,O_RDWR);
if(fd < 0)
{
printf("open failed: %s\n",strerror(errno));
return 1;
}
if(ioctl(fd,I2C_SLAVE,EEPROM_ADDRESS) < 0)
{
printf("ioctl failed: %s\n",strerror(errno));
close(fd);
return 1;
}
//---------------------Set Param----------------------//
rtc_time_t init_t ={0,30,12,1,7,5,25};//sec,min,hour,day,date,mon,year
set_time(&init_t);
printf("Set Time Done\n");
//-------------------------------------------
while(1)
{
rtc_time_t now;
if(get_time(&now) == 0)
{
printf("Time : %02d:%02d:%02d Date: %02d:%02d:%02d (day=%d)\n", now.hour,now.min,now.sec,now.date,now.mon,now.year,now.day);
}else{
printf("DS1307 read failed\n");
}
sleep(1);
}
close(fd);
return 0;
}





