#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

#define I2C_DEVICE "/dev/i2c-1"
#define EEPROM_ADDRESS 0x50
uint16_t addr=0x0000;
int main()
{
char data_write[]="Hello Word";
char data_read[sizeof(data_write)];
unsigned char write_buf[sizeof(data_write) + 2];
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
//---------------------Data Write---------------------------------//
write_buf[0] = (uint8_t)(addr >> 8);
write_buf[1] = (uint8_t)(addr & 0xff);
memcpy(&write_buf[2],data_write,sizeof(data_write));

if(write(fd ,write_buf,sizeof(write_buf)) < 0 )
{
printf("Failed to write to EEPROM %s\n",strerror(errno));
close(fd);
return 1;
}

printf("Data written successfulyy.\n");
printf("wait for 3 second.\n");
sleep(3);
//--------------------------Set Addres for Read-----------------------//
uint8_t addr_buf[2];
addr_buf[0]= (uint8_t)(addr >> 8);
addr_buf[1] =(uint8_t)(addr & 0xff);
if(write(fd ,addr_buf ,2) < 0 )
{
printf("Failed to set read addr %s\n",strerror(errno));
close(fd);
return 1;
}
//-------------------------Data Read---------------------------------//
if(read(fd ,data_read,sizeof(data_write)) < 0 )
{
printf("Failed to read from EEPROM %s\n",strerror(errno));
close(fd);
return 1;
}
data_read[sizeof(data_write) -1] = '\0';
printf("Data read from EEPROM: %s\n",data_read);

close(fd);
return 0;
}
