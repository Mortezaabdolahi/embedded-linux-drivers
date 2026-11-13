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
#include <pthread.h>
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

int counter=0;
int counter2=100;
int counter3=200;
int counter4=300;
int counter5=400;



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
void *tcp_server_thread(void *arg){
int server_fd ,client_sock;
struct sockaddr_in address;
int opt = 1;
int addrlen = sizeof(address);
//create socket
server_fd = socket(AF_INET, SOCK_STREAM, 0);
printf("sock=%d,\n",server_fd);

if(server_fd <0)
{
perror("get sock failed\n");
return NULL;
}
if(setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt,sizeof(opt))<0){
perror("setsockopt failed\n");
return NULL;
}
address.sin_family = AF_INET;
//address.sin_addr.s_addr = inet_addr("192.168.6.100");
address.sin_addr.s_addr = INADDR_ANY;
address.sin_port = htons(80);
if(bind(server_fd,(struct sockaddr *)&address, sizeof(address)) < 0){
perror("Bind failed");
return NULL;
}else{
printf("server successfully bound to port 80\n");
}
listen(server_fd, 3);
printf("TCP Server listening on port 80....\n");
char json[84];


while(1)
{
client_sock = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
if(client_sock >=0)
{
char buffer[1024]={0};
read(client_sock, buffer,sizeof(buffer));
printf("Received: %s\n",buffer);

//--------------------------------------------------------------------------------
int len= snprintf(json,sizeof(json),"{\"counter\":%d,\"counter2\":%d,\"counter3\":%d,\"counter4\":%d,\"counter5\":%d}",counter,counter2,counter3,counter4,counter5);
char *body = 	
    "<head>"
	"<style>"
	"table { border-collapse: collapse; width: 50% }"
	"th,td{border:1px solid#000; padding:8px;text-align:center}"
	"th{background-color:#eee}"
	"</style>"
	"<html><body>"
  "<h1>STM32 Variable Monitoring :</h1>"
	"<table>"
  "<tr><th>Variable</th><th>Value</th></tr>"
  "<tr><td>Val_1</td><td id=\"cnt\">0</td></tr>"
	"<tr><td>Val_2</td><td id=\"cnt2\">0</td></tr>"
	"<tr><td>Val_3</td><td id=\"cnt3\">0</td></tr>"
	"<tr><td>Val_4</td><td id=\"cnt4\">0</td></tr>"
	"<tr><td>Val_5</td><td id=\"cnt5\">0</td></tr>"
	"</table>"
  "<script>"
  "async function getData(){"
  "const res = await fetch('data.json');"
  "const json= await res.json();"
  "document.getElementById('cnt').innerText = json.counter;"
	"document.getElementById('cnt2').innerText = json.counter2;"
	"document.getElementById('cnt3').innerText = json.counter3;"
	"document.getElementById('cnt4').innerText = json.counter4;"
	"document.getElementById('cnt5').innerText = json.counter5;}"
  "setInterval(getData,500);"
  "</script>"
  "</html></body>";
int body_len = strlen(body);

char response_HTTP [1024];
            sprintf(response_HTTP,
                 "HTTP/1.1 200 OK\r\n"
                 "Content-Type: text/html; charset-UTF-8\r\n"
                 "Content-Length:%d\r\n"
                 "Connection: close\r\n"
                 "\r\n"
                 "%s",body_len,body);


	        char header[128];
	        int hlen= snprintf(header,sizeof(header),
		"HTTP/1.1 200 OK\r\n"
	  "Content-Type: application/json\r\n"
	  "Content-Length: %d\r\n"
	  "Connection:close\r\n"
	  "\r\n",len);

//-------------------------------------------------------------------------------

if(strstr(buffer,"GET / HTTP/1.1"))
{
write(client_sock, response_HTTP,strlen(response_HTTP));
printf("httppppppppppp");
}

if(strstr(buffer,"data.json"))
{
write(client_sock, header ,hlen);
write(client_sock, json ,len);
printf("jsonnnnnnnnnnnn");
}

counter++;
counter2++;
counter3++;
counter4++;
counter5++;






close(client_sock);
}
}
return NULL;
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
pthread_t server_thread_id;
if(pthread_create(&server_thread_id, NULL,tcp_server_thread, NULL) !=0){
perror("Failed to create server thread");
}
pthread_detach(server_thread_id);


while(1)
{


}

close(sock);
return 0;
}






