#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>
#include <pthread.h>

//--------------------Global Variables-----------------//
volatile int IsRunning=1;
int fd = -1;
char buffer[1024];
//--------------------Baudrate map--------------------//
speed_t baud_to_flag(int baud)
{
switch(baud) 
{
case 9600 :  return B9600;
case 19200:  return B19200;
case 38400:  return B38400;
case 57600:  return B57600;
case 115200: return B115200;
case 230400: return B230400;
default:     return B115200;
}
}
//---------------------UART_Config----------------------//
int UART_Settings(const char *serial_port,int baud)
{
struct termios options;
fd = open(serial_port, O_RDWR | O_NOCTTY);
if(fd < 0)
{
perror("Error openning serial port");
return -1;
}
if(tcgetattr(fd, &options) !=0)
{
perror("tcgetattr error");
}
//--RAW Mode
cfmakeraw(&options);
//--Baudrate
speed_t speed = baud_to_flag(baud);
cfsetispeed(&options, speed);
cfsetospeed(&options, speed);
//--UART 8N1
options.c_cflag &= ~ PARENB;
options.c_cflag &= ~ CSTOPB;
options.c_cflag &= ~ CSIZE;
options.c_cflag |=  CS8;
options.c_cflag |= CREAD;
options.c_cflag |= CLOCAL;

//--Timeouts
options.c_cc[VMIN] = 1;
options.c_cc[VTIME]= 0;
//--Apply Settings
if(tcsetattr(fd,TCSANOW, &options) != 0)
{
perror("tcsetattr error");
close(fd);
return -1;
}
return 0;

}
//-------------------UART RX Thread----------------//
void* UART_Thread(void *arg)
{
while(IsRunning)
{
int len = read(fd, buffer, sizeof(buffer) -1);
if(len > 0)
{
buffer[len] = '\0';
printf(" Received: %s\n", buffer);

//Echo back (optional)
write(fd,"Data Received\n", 15);
}
}
return NULL;
}
//-----------------------Main------------------------//
int main()
{
const char *sp = "/dev/ttyAMA0";
if( UART_Settings(sp, 115200) != 0)
{
printf("UART Connection failed.\n");
return 1;
}
printf("UART Connected Successfuly.\n");

write(fd, "Hello from UART\n",16);
pthread_t thread_id;
IsRunning=1;
pthread_create(&thread_id,NULL,UART_Thread,NULL);

//Main loop
while(1)
{
printf("Hello from main\n");
sleep(5);
}
IsRunning = 0;
close(fd);
return 0;

}
