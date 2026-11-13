#include "at24c32_lib.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

static int i2c_fd = -1;
static uint8_t dev_addr = 0;
uint16_t page_size =32;


int at24c32_init(int fd, uint8_t addr)
{
i2c_fd = fd;
dev_addr = addr;
return 0;
}

uint16_t make_addr(uint16_t page,uint16_t offset)
{
return page * page_size + offset;
}

int at24c32_write(uint16_t page , uint16_t offset ,uint8_t *data ,uint16_t len)
{
uint16_t addr = make_addr(page,offset);
uint8_t data_buf[len + 2];
data_buf[0] = (uint8_t) (addr >> 8);
data_buf[1] = (uint8_t) (addr & 0xff);
memcpy(&data_buf[2],data,len);
write(i2c_fd,data_buf,len + 2);
return 1;
}

int at24c32_read(uint16_t page , uint16_t offset ,uint8_t *buf ,uint16_t len)
{
uint16_t addr = make_addr(page,offset);
uint8_t addr_buf[2];
addr_buf[0] = (uint8_t) (addr >> 8);
addr_buf[1] = (uint8_t) (addr & 0xff);
write(i2c_fd,addr_buf, 2);
sleep(1);
read(i2c_fd, buf , len);
return 1;
}



