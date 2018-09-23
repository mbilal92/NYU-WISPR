/*
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 */


#include <arpa/inet.h>
#include <linux/if_packet.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/ether.h>
#include <linux/tcp.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
// #include <linux/ip.h>

#define MY_DEST_MAC0	0x00
#define MY_DEST_MAC1	0x00
#define MY_DEST_MAC2	0x00
#define MY_DEST_MAC3	0x00
#define MY_DEST_MAC4	0x00
#define MY_DEST_MAC5	0x00

#define DEFAULT_IF	"eth0"
#define BUF_SIZ		65535

unsigned short checksum(void *b, int len)
{	unsigned short *buf = b;
	unsigned int sum=0;
	unsigned short result;

	for ( sum = 0; len > 1; len -= 2 )
		sum += *buf++;
	if ( len == 1 )
		sum += *(unsigned char*)buf;
	sum = (sum >> 16) + (sum & 0xFFFF);
	sum += (sum >> 16);
	result = ~sum;
	return result;
}


unsigned short checksum1(uint8_t* buff, int _16bitword)
{
    unsigned long sum;
    for(sum=0;_16bitword>0;_16bitword--)
    sum+=htons(*(buff)++);
    sum = ((sum >> 16) + (sum & 0xFFFF));
    sum += (sum>>16);
    return (unsigned short)(~sum);
}


int MIN(int x, int y) {
	if (x <= y)
		return x;
	return y;
}

int main(int argc, char *argv[])
{
	int sockfd;
	struct ifreq if_idx;
	struct ifreq if_mac;
	int tx_len = 0;
	char sendbuf1[] = "00000000000100000000000208004500004ef9db400040112cc10a0000020a000001c8841389003a144effff99c15b698108000a1f150000000000000001000013890000003200100000fffffc183637383930313233343536373839", *pos = sendbuf1;
	unsigned char sendbuf[92];
	int count = 0;
	for (count = 0; count < sizeof sendbuf1/sizeof *sendbuf; count++) {
        sscanf(pos, "%2hhx", &sendbuf[count]);
        pos += 2;
    }
	// for (r = 0 ; r<92 ; r++) {
	// 	sendbuf[r] = sendbuf1[r] - '0';
	// }
	struct ether_header *eh = (struct ether_header *) sendbuf;
	struct iphdr *iph = (struct iphdr *) (sendbuf + sizeof(struct ether_header));
	struct sockaddr_ll socket_address;
	char ifName[IFNAMSIZ];
	
	/* Get interface name */
	if (argc > 1)
		strcpy(ifName, argv[1]);
	else
		strcpy(ifName, DEFAULT_IF);

	/* Open RAW socket to send on */
	if ((sockfd = socket(AF_PACKET, SOCK_RAW, IPPROTO_RAW)) == -1) {
	    perror("socket");
	}

	/* Get the index of the interface to send on */
	memset(&if_idx, 0, sizeof(struct ifreq));
	strncpy(if_idx.ifr_name, ifName, IFNAMSIZ-1);
	if (ioctl(sockfd, SIOCGIFINDEX, &if_idx) < 0)
	    perror("SIOCGIFINDEX");
	/* Get the MAC address of the interface to send on */
	memset(&if_mac, 0, sizeof(struct ifreq));
	strncpy(if_mac.ifr_name, ifName, IFNAMSIZ-1);
	if (ioctl(sockfd, SIOCGIFHWADDR, &if_mac) < 0)
	    perror("SIOCGIFHWADDR");

	/* Construct the Ethernet header */
	// memset(sendbuf, 0, BUF_SIZ);
	// /* Ethernet header */
	// eh->ether_shost[0] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[0];
	// eh->ether_shost[1] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[1];
	// eh->ether_shost[2] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[2];
	// eh->ether_shost[3] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[3];
	// eh->ether_shost[4] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[4];
	// eh->ether_shost[5] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[5];
	// eh->ether_dhost[0] = MY_DEST_MAC0;
	// eh->ether_dhost[1] = MY_DEST_MAC1;
	// eh->ether_dhost[2] = MY_DEST_MAC2;
	// eh->ether_dhost[3] = MY_DEST_MAC3;
	// eh->ether_dhost[4] = MY_DEST_MAC4;
	// eh->ether_dhost[5] = MY_DEST_MAC5;
	// /* Ethertype field */
	// eh->ether_type = htons(ETH_P_IP);
	// tx_len += sizeof(struct ether_header);
	// // struct iphdr *iph = (struct iphdr *) (sendbuf + sizeof(struct ether_header) );

	// iph->ihl = 5;
	// iph->version = 4;
	// iph->tos = 16;
	// iph->id = htons(10201);
	// iph->ttl = 64;
	// iph->protocol = 1;
	// iph->saddr = inet_addr("10.0.0.2");//(((struct sockaddr_in *)&(ifreq_ip.ifr_addr))->sin_addr)))
	// iph->daddr = inet_addr("10.0.0.1");//inet_ntoa((((struct sockaddr_in *)&(ifreq_ip.ifr_addr))->sin_addr))); // put destination IP address

	// tx_len += sizeof(struct iphdr);


	// struct icmphdr *uh = (struct icmphdr *)(sendbuf + sizeof(struct iphdr) + sizeof(struct ethhdr)); 
	// uh->un.frag.mtu = htons(1500);
	// uh->type = ICMP_DEST_UNREACH;
	// uh->code = ICMP_FRAG_NEEDED;
	// uh->checksum = 0;
	// uh->checksum = checksum((uint8_t *	)sendbuf + sizeof(struct iphdr) + sizeof(struct ethhdr), sizeof(struct icmphdr));


	// // struct tcphdr *uh = (struct tcphdr *)(sendbuf + sizeof(struct iphdr) + sizeof(struct ethhdr)); 

	 
	// // uh->source = htons(23451);
	// // uh->dest = htons(23452);
	// // uh->seq = 0;
	// // uh->doff = 5;
	// // uh->seq = random();
	// // uh->ack = 0;  //Only 0 on initial SYN 
	// // uh->th_flags = TH_SYN;   //SYN flag set
	// // uh->th_win = htonl(65535);   //used for segmentation
	// // uh->th_sum = 0;              //Kernel fill this out
	// // uh->th_urp = 0;

	// tx_len += sizeof(struct icmphdr);

	// /* Packet data */
	// // while (tx_len < BUF_SIZ-50) {
	// // 	sendbuf[tx_len++] = 0xde;
	// // }
	/* Index of the network device */
	socket_address.sll_ifindex = if_idx.ifr_ifindex;
	/* Address length*/
	socket_address.sll_halen = ETH_ALEN;
	/* Destination MAC */
	socket_address.sll_addr[0] = 0x0;
	socket_address.sll_addr[1] = 0x0;
	socket_address.sll_addr[2] = 0x0;
	socket_address.sll_addr[3] = 0x0;
	socket_address.sll_addr[4] = 0x0;
	socket_address.sll_addr[5] = 0x01;
	 int q = 0;
	/* Send packet */

		// size_t sendlen = MIN(tx_len, 1024);
	 //    size_t remlen  = tx_len;
	 //    const void *curpos = sendbuf;

	 //    while (remlen > 0)
	 //    {
	 //        ssize_t len = sendto(sockfd, curpos, sendlen, 0, (struct sockaddr*)&socket_address, sizeof(struct sockaddr_ll));
	 //        if (len == -1)
	 //            return -1;

	 //        curpos += len;
	 //        remlen -= len;
	 //        sendlen = MIN(remlen, 1024);
	 //    }

	if ((q = sendto(sockfd, sendbuf, 92, 0, (struct sockaddr*)&socket_address, sizeof(struct sockaddr_ll))) < 0)
	    {
	    	printf("Send failed\n");
	    	perror("go fucket");
	    }

	return 0;
}
