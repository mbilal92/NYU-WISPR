/*
 * Added the command line arguments for interface and MAC Address
 * 
 * Based on raw Ethernet from austinmarton: https://gist.github.com/1922600
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

#include <arpa/inet.h>
#include <linux/if_packet.h>
#include <linux/ip.h>
#include <linux/udp.h>
#include <linux/tcp.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/ether.h>
#include <string.h>
// #include <arpa/inet.h>
// // #include <asm/byteorder.h>
// // #include <errno.h>
// #include <linux/ip.h>
// #include <linux/if_packet.h>
// // #include <netinet/ether.h>
// // #include <netinet/ip.h>  // declarations for ip header
// // #include <netinet/ip6.h> // declarations for ip6_hdr
// // #include <netinet/tcp.h> // declarations for tcp header
// #include <netinet/udp.h> // declarations for udp header
// // #include <signal.h>
// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <sys/ioctl.h>
// #include <sys/socket.h>
// #include <unistd.h>

// #include <arpa/inet.h>
// #include <linux/if_packet.h>
// #include <stdio.h>
// #include <string.h>
// #include <stdlib.h>
// #include <sys/ioctl.h>
// #include <sys/socket.h>
#include <net/if.h>
#include <netinet/ether.h>

#define DEFAULT_IF "veth4"
#define BUF_SIZ 1024
#define ETHER_TYPE  0x0800

struct MyStruct
{
    int age;
    char Name;
};


typedef struct {
  uint16_t vlan_id;
  uint16_t next_protocol;
} wispr_ingress_vlan;

#define DEST_MAC0   0x00
#define DEST_MAC1   0x00
#define DEST_MAC2   0x00
#define DEST_MAC3   0x00
#define DEST_MAC4   0x00
#define DEST_MAC5   0x00

struct gre_base_header {
  /* GRE header according to RFC 2784 and RFC 2890 */
  uint16_t gre_flags;       /* GRE flags */

  #define GRE_FLAGS_CP 0x8000 /* checksum present */
  #define GRE_FLAGS_RP 0x4000 /* routing present */
  #define GRE_FLAGS_KP 0x2000 /* key present */
  #define GRE_FLAGS_SP 0x1000 /* sequence present */
  #define GRE_STRICT 0x0800
  #define GRE_REC 0x0700
  #define GRE_ACK 0x0080
  #define GRE_FLAGS_MASK (GRE_FLAGS_CP | GRE_FLAGS_KP | GRE_FLAGS_SP)
uint16_t gre_proto; /* protocol type */
};



unsigned short checksum(unsigned short* buff, int _16bitword)
{
    unsigned long sum;
    for(sum=0;_16bitword>0;_16bitword--)
    sum+=htons(*(buff)++);
    sum = ((sum >> 16) + (sum & 0xFFFF));
    sum += (sum>>16);
    return (unsigned short)(~sum);
}

int main(int argc, char *argv[])
{
 printf("Hello\n");
int sockfd;
struct ifreq if_idx;
struct ifreq ifreq_c;
int tx_len = 0;
// char sendbuf[BUF_SIZ];
uint8_t *sendbuf;
char recbuf[BUF_SIZ];

char ifName[IFNAMSIZ];
unsigned int mac[6]; //using as uint8_t

memset(mac, 0, 24);
sendbuf = malloc(BUF_SIZ);
// memset(sendbuf, 0, BUF_SIZ);
/* Get interface name */
if (argc > 1){
  strcpy(ifName, argv[1]);
    if (argc > 2) { /* 2 arguments, second argument is mac */
      sscanf(argv[2], "%02x:%02x:%02x:%02x:%02x:%02x", &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]);
      //printf("mac:\n");
      //printf("%2x:%2x:%2x:%2x:%2x:%2x\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    }
}
else
strcpy(ifName, DEFAULT_IF);

printf("%s\n", ifName );
/* Open RAW socket to send on */
if ((sockfd = socket(PF_PACKET, SOCK_RAW, IPPROTO_RAW)) == -1) {
    perror("socket");
}

/* Get the index of the interface to send on */
memset(&if_idx, 0, sizeof(struct ifreq));
strncpy(if_idx.ifr_name, ifName, IFNAMSIZ-1);
if (ioctl(sockfd, SIOCGIFINDEX, &if_idx) < 0)
perror("SIOCGIFINDEX");

/* Get the MAC address of the interface to send on */
memset(&ifreq_c, 0, sizeof(struct ifreq));
strncpy(ifreq_c.ifr_name, ifName, IFNAMSIZ-1);
if (ioctl(sockfd, SIOCGIFHWADDR, &ifreq_c) < 0)
perror("SIOCGIFHWADDR");

// struct ifreq ifreq_ip;
// memset(&ifreq_ip, 0, sizeof(ifreq_ip));
// strncpy(ifreq_ip.ifr_name, ifName, IFNAMSIZ-1);//giving name of Interface
// if(ioctl(sockfd,SIOCGIFADDR,&ifreq_ip)<0) //getting IP Address
// {
// printf("error in SIOCGIFADDR \n");
// }

/* Construct the Ethernet header */
memset(sendbuf, 0, BUF_SIZ);
memset(recbuf, 0, BUF_SIZ);
/* Ethernet header */
struct ethhdr *eth = (struct ethhdr *)(sendbuf);
 
eth->h_source[0] = 0x00;//(unsigned char)(ifreq_c.ifr_hwaddr.sa_data[0]);
eth->h_source[1] = 0x00;//(unsigned char)(ifreq_c.ifr_hwaddr.sa_data[1]);
eth->h_source[2] = 0x00;//(unsigned char)(ifreq_c.ifr_hwaddr.sa_data[2]);
eth->h_source[3] = 0x00;//(unsigned char)(ifreq_c.ifr_hwaddr.sa_data[3]);
eth->h_source[4] = 0x00;//(unsigned char)(ifreq_c.ifr_hwaddr.sa_data[4]);
eth->h_source[5] = 0x02;//(unsigned char)(ifreq_c.ifr_hwaddr.sa_data[5]);
 
/* filling destination mac. DESTMAC0 to DESTMAC5 are macro having octets of mac address. */
eth->h_dest[0] = mac[0];
eth->h_dest[1] = mac[1];
eth->h_dest[2] = mac[2];
eth->h_dest[3] = mac[3];
eth->h_dest[4] = mac[4];
eth->h_dest[5] = mac[5];

eth->h_dest[0] = 0x00;
eth->h_dest[1] = 0x00;
eth->h_dest[2] = 0x00;
eth->h_dest[3] = 0x00;
eth->h_dest[4] = 0x00;
eth->h_dest[5] = 0x01;

eth->h_proto = htons(ETH_P_8021Q); //means next header will be IP header
 
/* end of ethernet header */
tx_len+=sizeof(struct ethhdr);
wispr_ingress_vlan *vlan = (wispr_ingress_vlan *)(sendbuf + sizeof(struct ether_header));
vlan->vlan_id =  htons(3)  ;
vlan->next_protocol = htons(ETH_P_8021Q);
tx_len+=sizeof(wispr_ingress_vlan);

wispr_ingress_vlan *vlan2 = (wispr_ingress_vlan *)(sendbuf + sizeof(struct ether_header) + sizeof(wispr_ingress_vlan));
vlan2->vlan_id =  htons(3)  ;
vlan2->next_protocol = htons(0x0800);
tx_len+=sizeof(wispr_ingress_vlan);

struct iphdr *iph = (struct iphdr *) (sendbuf + sizeof(struct ether_header) + sizeof(wispr_ingress_vlan) + sizeof(wispr_ingress_vlan));

iph->ihl = 5;
iph->version = 4;
iph->tos = 16;
iph->id = htons(10201);
iph->ttl = 64;
iph->protocol = 6;
iph->saddr = inet_addr("10.0.0.2");//(((struct sockaddr_in *)&(ifreq_ip.ifr_addr))->sin_addr)))
iph->daddr = inet_addr("10.0.0.1");//inet_ntoa((((struct sockaddr_in *)&(ifreq_ip.ifr_addr))->sin_addr))); // put destination IP address

tx_len += sizeof(struct iphdr);

// struct MyStruct *my = (struct MyStruct *)(sendbuf + sizeof(struct iphdr) + sizeof(struct ethhdr));
// my->age = 5;
// my->Name = 'C';

// tx_len += sizeof(struct MyStruct);

struct tcphdr *uh = (struct tcphdr *)(sendbuf + sizeof(struct iphdr) + sizeof(struct ethhdr) + sizeof(wispr_ingress_vlan) + sizeof(wispr_ingress_vlan)); 

 
uh->source = htons(23451);
uh->dest = htons(23452);
uh->seq = 0;
// uh->doff = 5;
uh->seq = random();
uh->ack = 0;  //Only 0 on initial SYN 
// uh->th_flags = TH_SYN;   //SYN flag set
// uh->th_win = htonl(65535);   //used for segmentation
// uh->th_sum = 0;              //Kernel fill this out
// uh->th_urp = 0;

tx_len += sizeof(struct tcphdr);
/* Packet data */
sendbuf[tx_len++] = 'B';
sendbuf[tx_len++] = 'I';
sendbuf[tx_len++] = 'L';
sendbuf[tx_len++] = 'A';
sendbuf[tx_len++] = 'L';

uh->doff = 5;//(tx_len - sizeof(struct iphdr) - sizeof(struct ethhdr))/32;

//UDP length field
// iph->tot_len = sizeof(struct iphdr) + sizeof(struct tcphdr);//htons(tx_len); //  - sizeof(struct ethhdr)
iph->check = checksum((unsigned short*)(sendbuf + sizeof (struct ethhdr) + sizeof(wispr_ingress_vlan)+ sizeof(wispr_ingress_vlan)), (sizeof(struct iphdr)/2));

struct sockaddr_ll sadr_ll;
sadr_ll.sll_ifindex = if_idx.ifr_ifindex; // index of interface
sadr_ll.sll_halen = ETH_ALEN; // length of destination mac address
sadr_ll.sll_addr[0] = 0x00;
sadr_ll.sll_addr[1] = 0x00;
sadr_ll.sll_addr[2] = 0x00;
sadr_ll.sll_addr[3] = 0x00;
sadr_ll.sll_addr[4] = 0x00;
sadr_ll.sll_addr[5] = 0x03;
// struct sockaddr_in *sin;
// sin = (struct sockaddr_in *) malloc(sizeof(struct sockaddr_in));
// memset(sin, 0, sizeof(sin));
// sin->sin_family = AF_INET;
// // sin->sin_port = htons(55000);    //port to send packet to
// sin->sin_addr.s_addr = inet_addr("11.0.2.1");   


int i = 0;
// printf("%d\n",  tx_len);
// printf("%d\n", (int)strlen(sendbuf));
  for (i = 0; i < (int)(tx_len); i += 8) {
    printf("%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:",
               sendbuf[i], sendbuf[i + 1], sendbuf[i + 2], sendbuf[i + 3], sendbuf[i + 4],
               sendbuf[i + 5], sendbuf[i + 6], sendbuf[i + 7]);
  }

/* Send packet */
// if((i = sendto(sockfd,sendbuf, tx_len, 0, (struct sockaddr *)sin, sizeof(struct sockaddr))) < 0){
if (sendto(sockfd, sendbuf, tx_len, 0, (struct sockaddr*)&sadr_ll, sizeof(struct sockaddr_ll)) < 0) {
    printf("Send failed\n");
    perror("BC");
}else{
    printf("%d\n", i);
    printf("%s\n", "HAHAHA");
}

// ssize_t numbytes = recvfrom(sockfd, buf, BUF_SIZ, 0, NULL, NULL);
// rece("dummy");

return 0;
}

