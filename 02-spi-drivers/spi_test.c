
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include <gpiod.h>

//variable
static const char  *spi_device = "/dev/spidev0.0"; //bus 0, cs0
static uint32_t spi_speed = 8000000;
static uint8_t spi_mode = SPI_MODE_0;
static uint8_t spi_bits = 8;
struct gpiod_chip *chip;
struct gpiod_line * line_0_reset;
struct gpiod_line * line_5_dc;
struct gpiod_line * line_6_cs;
//function
//---------------------spi_init----------------------
int spi_init(const char *device)
{
int fd= open(device , O_RDWR);
if(fd<0){perror("SPI:Can't open spi dev"); return -1;}
//mode
if(ioctl(fd, SPI_IOC_WR_MODE ,&spi_mode) < 0){
perror("SPI:Can't set mode");
return -1;
}
//Bit per word
if(ioctl(fd, SPI_IOC_WR_BITS_PER_WORD ,&spi_bits) < 0){
perror("SPI:Can't set bits");
return -1;
}
//max speed (Hz)
if(ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ ,&spi_speed) < 0){
perror("SPI:Can't set speed");
return -1;
}
return fd;
}
//------------------------spi_write--------------------
int spi_write(int fd, uint8_t *data, size_t len)
{
int ret = write(fd,data,len);
if(ret<0){perror("SPI write"); return -1;}
return ret;
}
//----------------------spi-full_duplex-----------------
int spi_transfer(int fd, uint8_t *tx, uint8_t *rx, size_t len)
{
struct spi_ioc_transfer tr = {
.tx_buf = (unsigned long)tx,
.rx_buf = (unsigned long)rx,
.len = len,
.speed_hz = spi_speed,
.bits_per_word = spi_bits,
.delay_usecs = 0
};
int ret = ioctl(fd,SPI_IOC_MESSAGE(1), &tr);
if(ret<0){perror("SPI transfer"); return -1;}
return ret;
}
//---------------------------ili9341_fun---------------------------

void ili9341_enable(void)
{
gpiod_line_set_value(line_0_reset,1);
}
void ili9341_reset(void)
{
gpiod_line_set_value(line_0_reset,0);
usleep(10000);
gpiod_line_set_value(line_6_cs,0);
usleep(10000);
gpiod_line_set_value(line_0_reset,1);
gpiod_line_set_value(line_6_cs,1);
}
void ili9341_write_buffer(uint8_t *buf , uint16_t len , int fd)
{
gpiod_line_set_value(line_5_dc,1);
gpiod_line_set_value(line_6_cs,0);
spi_write(fd,buf,len);

}
void ili9341_write_cmd(uint8_t cmd , int fd)
{
gpiod_line_set_value(line_5_dc,0);
gpiod_line_set_value(line_6_cs,0);
spi_write(fd,&cmd,1);
}
void ili9341_write_data(uint8_t data , int fd)
{
gpiod_line_set_value(line_5_dc,1);
gpiod_line_set_value(line_6_cs,0);
spi_write(fd,&data,1);
}
void ili9341_setRotation(uint8_t rotation , int fd)
{
ili9341_write_cmd(0x36 ,fd);
usleep(1000);
switch(rotation)
{
case 0 :
ili9341_write_data(0x40 | 0x08 ,fd);
break;
case 1 :
ili9341_write_data(0x20 | 0x08 ,fd);
break;
case 2 :
ili9341_write_data(0x80 | 0x08 ,fd);
break;
case 3 :
ili9341_write_data(0x40 | 0x08 | 0x20 | 0x08 ,fd);
break;
default :
break;
}
}
void ilil9341_init(int fd)
{
ili9341_enable();
ili9341_reset();
sleep(2);
//SOFTWARE RESET
ili9341_write_cmd(0x01,fd);
usleep(10000);
//POWER CONTROL A
ili9341_write_cmd(0xCB,fd);
ili9341_write_data(0x39,fd);
ili9341_write_data(0x2C,fd);
ili9341_write_data(0x00,fd);
ili9341_write_data(0x34,fd);
ili9341_write_data(0x02,fd);
usleep(10000);
//POWER CONTROL B
ili9341_write_cmd(0xCF,fd);
ili9341_write_data(0x00,fd);
ili9341_write_data(0xC1,fd);
ili9341_write_data(0x30,fd);
usleep(10000);
// DRIVER TIMING CONTROL A
ili9341_write_cmd(0xE8,fd);
ili9341_write_data(0x85,fd);
ili9341_write_data(0x00,fd);
ili9341_write_data(0x78,fd);
usleep(10000);
// DRIVER TIMING CONTROL B
ili9341_write_cmd(0xEA,fd);
ili9341_write_data(0x00,fd);
ili9341_write_data(0x00,fd);
usleep(10000);
//POWER ON SEQUENCE CONTROL
ili9341_write_cmd(0xED,fd);
ili9341_write_data(0x64,fd);
ili9341_write_data(0x03,fd);
ili9341_write_data(0x12,fd);
ili9341_write_data(0x81,fd);
usleep(10000);
//PUMP RATIO CONTROL
ili9341_write_cmd(0xE7,fd);
ili9341_write_data(0x20,fd);
usleep(10000);
//POWER CONTROL, VRH[5:0]
ili9341_write_cmd(0xC0,fd);
ili9341_write_data(0x23,fd);
usleep(10000);
//POWER CONTROL, SAP[2:0];BT[3:0]
ili9341_write_cmd(0xC1,fd);
ili9341_write_data(0x10,fd);
usleep(10000);
//VCM CONTROL
ili9341_write_cmd(0xC5,fd);
ili9341_write_data(0x3E,fd);
ili9341_write_data(0x28,fd);
usleep(10000);
//VCM CONTROL 2
ili9341_write_cmd(0xC7,fd);
ili9341_write_data(0x86,fd);
usleep(10000);
//MEMORY ACCESS CONTROL
ili9341_write_cmd(0x36,fd);
ili9341_write_data(0x48,fd);
usleep(10000);
//PIXEL FORMAT
ili9341_write_cmd(0x3A,fd);
ili9341_write_data(0x55,fd);
usleep(10000);
//FRAME RATIO CONTROL
ili9341_write_cmd(0xB1,fd);
ili9341_write_data(0x00,fd);
ili9341_write_data(0x18,fd);
usleep(10000);
//DESPLAY FUNCTION CONTROL
ili9341_write_cmd(0xB6,fd);
ili9341_write_data(0x80,fd);
ili9341_write_data(0x82,fd);
ili9341_write_data(0x27,fd);
usleep(10000);
//3GAME FUNCTION DISABLE
ili9341_write_cmd(0xF2,fd);
ili9341_write_data(0x00,fd);
usleep(10000);
//GAMMA CURVE SELECTED
ili9341_write_cmd(0x26,fd);
ili9341_write_data(0x01,fd);
usleep(10000);
//POSITIVE GAMMA CORRECTION
ili9341_write_cmd(0xE0,fd);
ili9341_write_data(0x31,fd);
ili9341_write_data(0x2B,fd);
ili9341_write_data(0x0C,fd);
ili9341_write_data(0x0E,fd);
ili9341_write_data(0x08,fd);
ili9341_write_data(0x4E,fd);
ili9341_write_data(0xF1,fd);
ili9341_write_data(0x37,fd);
ili9341_write_data(0x07,fd);
ili9341_write_data(0x10,fd);
ili9341_write_data(0x03,fd);
ili9341_write_data(0x0E,fd);
ili9341_write_data(0x09,fd);
ili9341_write_data(0x00,fd);
usleep(10000);
//NEGATIV GAMMA CORRECTION
ili9341_write_cmd(0xE1,fd);
ili9341_write_data(0x00,fd);
ili9341_write_data(0x0E,fd);
ili9341_write_data(0x14,fd);
ili9341_write_data(0x03,fd);
ili9341_write_data(0x11,fd);
ili9341_write_data(0x07,fd);
ili9341_write_data(0x31,fd);
ili9341_write_data(0xC1,fd);
ili9341_write_data(0x48,fd);
ili9341_write_data(0x08,fd);
ili9341_write_data(0x0F,fd);
ili9341_write_data(0x0C,fd);
ili9341_write_data(0x31,fd);
ili9341_write_data(0x36,fd);
ili9341_write_data(0x0F,fd);
usleep(10000);
//EXIT SLEEP
ili9341_write_cmd(0x11,fd);
usleep(100000);
//TURN ON DESPLAY
ili9341_write_cmd(0x29,fd);
//STARTING ROTATION
ili9341_setRotation( 2, fd);
}
void ili9341_setaddr(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, int fd)
{
uint8_t buffer[4];
buffer[0]= x1 >> 8;
buffer[1]= x1;
buffer[2]= x2 >> 8;
buffer[3]= x2;
ili9341_write_cmd(0x2A, fd);
ili9341_write_buffer(buffer , sizeof(buffer) ,  fd);
buffer[0]= y1 >> 8;
buffer[1]= y1;
buffer[2]= y2 >> 8;
buffer[3]= y2;
ili9341_write_cmd(0x2B, fd);
ili9341_write_buffer(buffer , sizeof(buffer) ,  fd);
ili9341_write_cmd(0x2C, fd);
}
void ili9341_drawcolorburst(uint16_t color, uint32_t size ,int fd)
{
uint32_t buffersize = 0;
if((size * 2)< 500)
{
buffersize = size;
}
else
{
buffersize = 500;
}
gpiod_line_set_value(line_5_dc,1);
gpiod_line_set_value(line_6_cs,0);
uint8_t chifted = color >> 8;
uint8_t burstbuffer[buffersize];
for(uint32_t j=0;j<buffersize;j+=2)
{
burstbuffer[j] = chifted;
burstbuffer[j+1]= color;
}
uint32_t sendingsize = size * 2;
uint32_t sendinginblock = sendingsize / buffersize;
uint32_t remainderfromblock = sendingsize & buffersize;
if(sendinginblock != 0)
{
for(uint32_t j=0; j< (sendinginblock);j+=1)
{
spi_write(fd,burstbuffer,buffersize);
}
}
spi_write(fd,burstbuffer,remainderfromblock);
gpiod_line_set_value(line_6_cs,1);
}





void ili9341_fillscreen(uint16_t color , int fd)
{
ili9341_setaddr(0, 0, 320 , 240 , fd);
ili9341_drawcolorburst( color, 320*240 , fd);
}


int main()
{
//---------------------------gpio_init-----------------------------
chip = gpiod_chip_open_by_name("gpiochip0");
line_0_reset = gpiod_chip_get_line(chip , 0);
line_5_dc = gpiod_chip_get_line(chip , 5);
line_6_cs = gpiod_chip_get_line(chip , 6);
gpiod_line_request_output(line_0_reset, "test" ,0);
gpiod_line_request_output(line_5_dc, "test" ,0);
gpiod_line_request_output(line_6_cs, "test" ,0);
//---------------------------spi_init-----------------------------
int fd= spi_init(spi_device);
if(fd<0){printf("SPI init failed\n");return-1;}
//---------------------------ILI9341_init-----------------------------
ilil9341_init(fd);
sleep(1);
ili9341_fillscreen(0x0000 , fd);


while(1)
{

}



printf("Done.\n");
return 0;
}
