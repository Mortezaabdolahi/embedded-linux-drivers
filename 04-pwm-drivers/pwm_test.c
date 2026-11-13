#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

int write_file(const char *path ,const char *value)
{
int fd =open(path,O_WRONLY);
if(fd<0){
perror(path);
return -1;
}
if(write(fd, value ,strlen(value)) < 0){
perror("write");
close(fd);
return -1;
}
close(fd);
return 0;
}
int main()
{
const char *export_path =   "/sys/class/pwm/pwmchip0/export";
const char *unexport_path = "/sys/class/pwm/pwmchip0/unexport";
const char *period_path =   "/sys/class/pwm/pwmchip0/pwm0/period";
const char *duty_path =     "/sys/class/pwm/pwmchip0/pwm0/duty_cycle";
const char *enable_path =   "/sys/class/pwm/pwmchip0/pwm0/enable";
char buf[32];
printf("Exporting PWM0...\n");
write_file(export_path,"0");
usleep(200000);

printf("Setting period to 1 ms (1000000ns)...\n");
write_file(period_path,"1000000");

printf("Setting duty_cycle t0 50% (500000ns)...\n");
write_file(duty_path,"500000");

printf("Enable PWM0...\n");
write_file(enable_path,"1");

sleep(10);

printf("Desable PWM0...\n");
write_file(enable_path,"0");

printf("Unexporting PWM0...\n");
write_file(unexport_path,"0");
sleep(2);
//-P2
printf("P2 is runing.......\n");
printf("Exporting PWM0...\n");
write_file(export_path,"0");
usleep(200000);

printf("Setting period to 1 ms (1000000ns)...\n");
write_file(period_path,"1000000");

printf("Setting initial duty = 0 ...\n");
write_file(duty_path,"500000");

printf("Enable PWM0...\n");
write_file(enable_path,"1");

int duty=0;


int fd = open(duty_path,O_WRONLY);

while(duty<1000000)
{
sprintf(buf,"%d",duty);
write(fd,"buf",strlen(buf));
sleep(1);
printf("%s\n",buf);
duty +=100000;
}

printf("Desable PWM0...\n");
write_file(enable_path,"0");

printf("Unexporting PWM0...\n");
write_file(unexport_path,"0");

printf("Done.\n");
return 0;
}
