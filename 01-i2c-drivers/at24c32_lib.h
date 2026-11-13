#ifndef  AT24C32_LIB_H
#define  AT24C32_LIB_H
#include <stdint.h>


int at24c32_init(int fd, uint8_t addr);
int at24c32_write(uint16_t page , uint16_t offset ,uint8_t *data ,uint16_t len);
int at24c32_read(uint16_t page , uint16_t offset ,uint8_t *buf ,uint16_t len);






#endif
