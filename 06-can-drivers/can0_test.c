#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>

#include <sys/socket.h>
#include <sys/ioctl.h>

#include <linux/can.h>
#include <linux/can/raw.h>
#include <linux/if.h>

#include <netlink/netlink.h>
#include <netlink/socket.h>
#include <netlink/cache.h>
#include <netlink/route/link.h>
#include <netlink/route/link/can.h>
//-----------------------------------Macro-----------------------------------//
#define CHECK_IOCTL(ret,msg)\
if(ret < 0){\
fprintf(stderr,"ERROR:%s failed.Reaso:%s\n",msg,strerror(errno));\
return -1;\
}
//---------------------------------Variable----------------------------------//

static volatile int running =1;
uint8_t my_data[8] = {0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08};



//---------------------------------Function----------------------------------//

static void siginit_handler(int sig)
{
(void)sig;
running = 0;
}

static int set_can_interface(const char *ifname, int bitrate)
{

struct nl_sock *sock = NULL;
struct rtnl_link *link = NULL;
struct rtnl_link *change = NULL;
struct nl_cache *link_cache = NULL;
// int ifindex;
int ret;

sock = nl_socket_alloc();
if(!sock){
fprintf(stderr,"nl_socket_alloc failed\n");
return -1;
}


ret = nl_connect(sock, NETLINK_ROUTE);
if(ret < 0) {
fprintf(stderr,"nl_connect failed:%s\n",nl_geterror(ret));
nl_socket_free(sock);
return -1;
}

ret = rtnl_link_alloc_cache(sock, AF_UNSPEC, &link_cache);
if(ret < 0) {
fprintf(stderr,"rtnl_link_alloc_cache failed:%s\n",nl_geterror(ret));
nl_close(sock);
nl_socket_free(sock);
return -1;
}

link = rtnl_link_get_by_name(link_cache,ifname);
if(!link) {
fprintf(stderr,"Interface %s not found\n",ifname);
nl_cache_free(link_cache);
nl_close(sock);
nl_socket_free(sock);
return -1;
}
/*
*Step 1:bring Interface DOWN
*/
change = rtnl_link_alloc();
if(!change){
fprintf(stderr,"rtnl_link_alloc failed\n",ifname);
rtnl_link_put(link);
nl_cache_free(link_cache);
nl_close(sock);
nl_socket_free(sock);
return -1;
}

rtnl_link_unset_flags(change, IFF_UP);

ret = rtnl_link_change(sock,link,change,0);
rtnl_link_put(change);
change = NULL;

if(ret < 0){
fprintf(stderr,"Failes to set %s down: %s\n",ifname,nl_geterror(ret));
rtnl_link_put(link);
nl_cache_free(link_cache);
nl_close(sock);
nl_socket_free(sock);
return -1;
}

usleep(100000);
/*
*Step 2: Set CAN bitrate
*/

change = rtnl_link_alloc();
if(!change) {
fprintf(stderr,"rtnl_link_alloc failed\n");
rtnl_link_put(link);
nl_cache_free(link_cache);
nl_close(sock);
nl_socket_free(sock);
return -1;
}


rtnl_link_set_type(change,"can");

ret = rtnl_link_can_set_bitrate(change, bitrate);
if(ret < 0){
fprintf(stderr,"rtnl_link_can_set_bitrate failed: %s\n",nl_geterror(ret));
rtnl_link_put(change);
rtnl_link_put(link);
nl_cache_free(link_cache);
nl_close(sock);
nl_socket_free(sock);
return -1;
}

ret = rtnl_link_change(sock,link,change,0);
rtnl_link_put(change);
change = NULL;

if(ret < 0){
fprintf(stderr,"Failed to set bitrate %d on %s: %s\n",bitrate,ifname,nl_geterror(ret));
rtnl_link_put(link);
nl_cache_free(link_cache);
nl_close(sock);
nl_socket_free(sock);
return -1;
}

usleep(100000);

/*
*Step 3: bring Interface UP
*/

change = rtnl_link_alloc();
if(!change) {
fprintf(stderr,"rtnl_link_alloc failed\n");
rtnl_link_put(link);
nl_cache_free(link_cache);
nl_close(sock);
nl_socket_free(sock);
return -1;
}

rtnl_link_set_flags(change,IFF_UP);

ret = rtnl_link_change(sock,link,change,0);
rtnl_link_put(change);
change = NULL;

if(ret < 0){
fprintf(stderr,"Failed to set bitrate %d on %s: %s\n",bitrate,ifname,nl_geterror(ret));
rtnl_link_put(link);
nl_cache_free(link_cache);
nl_close(sock);
nl_socket_free(sock);
return -1;
}

printf("CAN interface %s configured: bitrate=%d, state=UP--success\n",ifname,bitrate);

rtnl_link_put(link);
nl_cache_free(link_cache);
nl_close(sock);
nl_socket_free(sock);

//return ifindex;
return 0;
}


static int open_can_socket(const char *ifname )
{

int s;
struct sockaddr_can addr;
struct ifreq ifr;

s = socket(PF_CAN, SOCK_RAW, CAN_RAW);
if(s < 0)
{
perror("socket (PF_CAN)");
return -1;
}

memset(&ifr, 0 ,sizeof(ifr));
strncpy(ifr.ifr_name, ifname, IFNAMSIZ -1);

if(ioctl(s,SIOCGIFINDEX, &ifr) < 0) {
perror("ioctl(SIOCGIFINDEX)");
close(s);
return -1;
}

memset(&addr, 0 ,sizeof(addr));
addr.can_family = AF_CAN;
addr.can_ifindex = ifr.ifr_ifindex;


struct can_filter rfilter[1];

rfilter[0].can_id = 0x123;
rfilter[0].can_mask = CAN_SFF_MASK;
if(setsockopt(s,SOL_CAN_RAW,CAN_RAW_FILTER,&rfilter,sizeof(rfilter)) < 0)
{
    perror("Filter set is FAILLED\n");
    close(s);
    return 1;
}else{
    printf("Filter Applyed");
}








if(bind(s,(struct sockaddr *)&addr, sizeof(addr)) <0) {
perror("bind");
close(s);
return -1;
}

return s;

}

static void print_can_frame( const struct can_frame *frame)
{

int i;

if(frame->can_id & CAN_EFF_FLAG){
printf("EXT");
printf("ID=0x%08X",frame->can_id & CAN_EFF_MASK);
}else {
printf("STD");
printf("ID=0x%03X",frame->can_id & CAN_SFF_MASK);
}
if(frame->can_id & CAN_ERR_FLAG) {
printf("ERR ");
}
if(frame->can_id & CAN_RTR_FLAG) {
printf("RTR ");
}

printf("DLC=%d DATA=",frame->can_dlc);

for(i = 0;i<frame->can_dlc;i++)
{
printf("%02X", frame->data[i]);
}

printf("\n");
fflush(stdout);
}

int send_can_frame(int can_sock, uint32_t id, uint8_t *data, uint8_t dlc)
{
    struct  can_frame Tx_frame;
memset(&Tx_frame,0,sizeof(struct can_frame));
Tx_frame.can_id = id;
if(dlc > 8){
    Tx_frame.can_dlc = 8;
}else{
  Tx_frame.can_dlc = dlc;  
}
    memcpy(Tx_frame.data,data,Tx_frame.can_dlc);
    int tx_len = write(can_sock,&Tx_frame,sizeof(struct can_frame));
    if(tx_len != sizeof(struct can_frame))
    {
        perror("Error writing to CAN socket");
        return -1;
    }
    return 0;
}
int main(int argc , char *argv[])
{
const char *ifname = "can0";
int bitrate = 500000;
int can_sock;
int ret;

if(argc >=2){
ifname = argv[1];
}
if(argc >=3){
bitrate = atoi(argv[2]);
}
signal(SIGINT, siginit_handler);

printf("Configuring CAN interface ...\n");
printf("Interface:%s\n",ifname);
printf("bitrate :%d\n",bitrate);

ret = set_can_interface(ifname, bitrate);
if(ret < 0)
{
fprintf(stderr,"Failed to configure CAN interface\n");
return -1;
}
can_sock = open_can_socket(ifname);
if(can_sock < 0)
{
fprintf(stderr,"Failed to open CAN socket\n");
return -1;
}
printf("Listenning on %s....\n",ifname);


while(running)
{

/*
struct can_frame frame;
ssize_t nbytes;
memset(&frame, 0, sizeof(frame));
nbytes = read(can_sock, &frame, sizeof(frame));


if(nbytes < (ssize_t)sizeof(struct can_frame)){
fprintf(stderr, "Incomplete CAN frame received\n");
continue;
}

print_can_frame(&frame);
*/
if(send_can_frame( can_sock, 0x124, my_data, 8) == 0)
{
    printf(" message sent successfully \n");
}
sleep(1);

}

close(can_sock);
printf("\nStopped.\n");

return 0;
}






















