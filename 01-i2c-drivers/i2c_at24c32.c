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
#define EEPROM_ADDRESS 0x50
int main()
{
//-----------------init_i2c---------------------------//
int fd = open(I2C_DEVICE,O_RDWR);
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
//-------------------init_at24c32-------------------------//
at24c32_init(fd,EEPROM_ADDRESS);
//---------------------write------------------------------//
uint8_t data_write[]="HELLO_WORD";

at24c32_write(1,0,data_write,sizeof(data_write));

printf("data written successfuly\n");

//-------------------------read----------------------------//
uint8_t data_read[sizeof(data_write)];
sleep(1);
at24c32_read(1,0,data_read,sizeof(data_write));
//-------------------------display---------------------------//
printf("Data read from e2prom :%s\n",data_read);

//---------------------write------------------------------//
uint16_t digit = 32768;

at24c32_write(2,0,(uint8_t *)&digit,2);

printf("data written successfuly\n");

//-------------------------read----------------------------//
uint16_t digit_read = 0;
sleep(1);
at24c32_read(2,0,(uint8_t *)&digit_read,2);
//-------------------------display---------------------------//
printf("Data read from e2prom :%d\n",digit_read);



close(fd);
return 0;
}



