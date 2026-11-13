#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <net/route.h>
#include <netinet/in.h>
#include <net/if_arp.h>
#include <sys/poll.h>
#include <signal.h>
#include <errno.h>
//--------------------------Macro-----------------------------//
#define CHECK_IOCTL(ret, msg)\
if(ret < 0 ){\
fprintf(stderr, "Error: %s failed. Reaso: %s\n",msg,strerror(errno));\
return -1;\
}

//--------------------------Variable---------------------------//
typedef struct{
const char *ifname;
unsigned char mac[6];
const char *ip;
const char *mask;
const char *gw;
}eth_config_t;

typedef enum {
STATE_CLOSED,
STATE_CONNECTING,
STATE_CONNECTED,
STATE_ERROR
}net_state_t;
int send_flag=0;
int connection_state = 0;
int tcp_sock = -1;
//--------------------------Function---------------------------//
int config_eth(int sock, eth_config_t *t)
{
struct ifreq ifr;
int ret;
struct sockaddr_in *addr = (struct sockaddr_in *)&ifr.ifr_addr;
memset(&ifr, 0 ,sizeof(ifr));
strncpy(ifr.ifr_name, t->ifname , IFNAMSIZ);
//Interface Down
ifr.ifr_flags &= ~IFF_UP;
if(ioctl(sock, SIOCSIFFLAGS, &ifr) < 0)
{
perror("Error setting interface DOWN");
}else{
printf("Interface Down Successful.waiting..\n");
sleep(1);
}
//set mac
ifr.ifr_hwaddr.sa_family = ARPHRD_ETHER;
memcpy(ifr.ifr_hwaddr.sa_data,t->mac ,6);
ret = ioctl(sock, SIOCSIFHWADDR, &ifr);
CHECK_IOCTL(ret, "Set MAC Address");
//bring interface up
ret = ioctl(sock, SIOCGIFFLAGS, &ifr);
CHECK_IOCTL(ret, "Get Flags");
ifr.ifr_flags |= IFF_UP;
ret = ioctl(sock, SIOCSIFFLAGS , &ifr);
CHECK_IOCTL(ret, "Set Flag UP");
//set ip
memset(&ifr, 0 ,sizeof(ifr));
strncpy(ifr.ifr_name, t->ifname , IFNAMSIZ);
addr->sin_family = AF_INET;
inet_pton(AF_INET, t->ip , &addr->sin_addr);
ret = ioctl(sock, SIOCSIFADDR, &ifr);
CHECK_IOCTL(ret, "Set IP Address");
//set netmask
memset(&ifr, 0 ,sizeof(ifr));
strncpy(ifr.ifr_name, t->ifname , IFNAMSIZ);
addr = (struct sockaddr_in *)&ifr.ifr_netmask;
addr->sin_family = AF_INET;
inet_pton(AF_INET, t->mask , &addr->sin_addr);
ret = ioctl(sock, SIOCSIFNETMASK, &ifr);
CHECK_IOCTL(ret, "Set Netmask Address");

//bring interface up
ret = ioctl(sock, SIOCGIFFLAGS, &ifr);
CHECK_IOCTL(ret, "Get Flags");
ifr.ifr_flags |= IFF_UP;
ret = ioctl(sock, SIOCSIFFLAGS , &ifr);
CHECK_IOCTL(ret, "Set Flag UP Verify");

//set getway
struct rtentry route;
memset(&route, 0 ,sizeof(route));

addr = (struct sockaddr_in *)&route.rt_dst;
addr->sin_family = AF_INET;
addr->sin_addr.s_addr = INADDR_ANY;

addr = (struct sockaddr_in *)&route.rt_gateway;
addr->sin_family = AF_INET;
inet_pton(AF_INET,t-> gw, &addr->sin_addr.s_addr);

addr = (struct sockaddr_in *)&route.rt_genmask;
addr->sin_family = AF_INET;
addr->sin_addr.s_addr = INADDR_ANY;

route.rt_flags = RTF_UP | RTF_GATEWAY;
route.rt_dev = (char *)t->ifname;

ret = ioctl(sock, SIOCADDRT,&route);
CHECK_IOCTL(ret, "Set Getway");

printf("Network [%s] configured successfully!\n", t->ifname);
return 0;
}
//-----------------------Main--------------------------------
int main()
{
signal(SIGPIPE, SIG_IGN);
eth_config_t my_config = {
.ifname = "eth0",
.mac    = {0x0c,0x29,0xAB,0x7C,0x00,0x01},
.ip     = "192.168.6.204",
.mask   = "255.255.255.0",
.gw     = "192.168.6.1"
};
int sock = socket(AF_INET, SOCK_STREAM, 0);
if(sock < 0)
{
perror("sock failed\n");
return -1;
}

config_eth(sock, & my_config);
sleep(1);
close(sock);

struct sockaddr_in srv;
memset(&srv,0,sizeof(srv));
srv.sin_family = AF_INET;
srv.sin_port = htons(50000);
if(inet_pton(AF_INET, "192.168.6.100",&srv.sin_addr) <= 0){
printf("invalid addr\n");
return 0;
}


while(1)
{
if(connection_state ==0)
{
tcp_sock = socket(AF_INET, SOCK_STREAM, 0);
if(tcp_sock < 0)
{
perror("socket creation failed");
sleep(2);
continue;
}
if(connect(tcp_sock,(struct sockaddr*)&srv,sizeof(srv)) == 0){
printf("connected\n");
connection_state = 1;
}else{
printf("server not ready, retrying....:%d-%s\n",errno,strerror(errno));
close(tcp_sock);
tcp_sock = -1;
sleep(2);
}
}

if(connection_state == 1)
{
char *message = "Hello, I am Linux\n";
if(send(tcp_sock,message,strlen(message),0) <0)
{
perror("send failed");
close(tcp_sock);
tcp_sock = -1;
connection_state = 0;
sleep(1);
}else{
printf("Data sent successfully.\n");
}
sleep(1);
}

}

close(sock);
return 0;
}






