#include <arpa/inet.h>
#include <asm/byteorder.h>
#include <errno.h>
#include <linux/if_packet.h>
#include <netinet/ether.h>
#include <netinet/ip.h>  // declarations for ip header
#include <netinet/ip6.h> // declarations for ip6_hdr
#include <netinet/tcp.h> // declarations for tcp header
#include <netinet/udp.h> // declarations for udp header
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <net/if.h>

int main()
{
    int saddr_size , data_size, daddr_size, bytes_sent;
    struct sockaddr_ll saddr, daddr;
    unsigned char *buffer=malloc(65535);

    int sock_raw = socket( AF_PACKET , SOCK_RAW , htons(ETH_P_ALL)) ; //For receiving
    int sock = socket( PF_PACKET , SOCK_RAW , IPPROTO_RAW) ;            //For sending

    memset(&saddr, 0, sizeof(struct sockaddr_ll));
    saddr.sll_family = AF_PACKET;
    saddr.sll_protocol = htons(ETH_P_ALL);
    saddr.sll_ifindex = if_nametoindex("dummy1");
    if (bind(sock_raw, (struct sockaddr*) &saddr, sizeof(saddr)) < 0) {
        perror("bind failed\n");
        close(sock_raw);
    }

    memset(&daddr, 0, sizeof(struct sockaddr_ll));
    daddr.sll_family = AF_PACKET;
    daddr.sll_protocol = htons(ETH_P_ALL);
    daddr.sll_ifindex = if_nametoindex("veth0");
    if (bind(sock, (struct sockaddr*) &daddr, sizeof(daddr)) < 0) {
      perror("bind failed\n");
      close(sock);
    }

    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    snprintf(ifr.ifr_name, sizeof(ifr.ifr_name), "veth0");
    if (setsockopt(sock, SOL_SOCKET, SO_BINDTODEVICE, (void *)&ifr, sizeof(ifr)) < 0) {
        perror("bind to eth1");
        }

    while(1)
    {
        saddr_size = sizeof (struct sockaddr);
        daddr_size = sizeof (struct sockaddr);
        //Receive a packet
        data_size = recvfrom(sock_raw , buffer , 65536 , 0 ,(struct sockaddr *) &saddr , (socklen_t*)&saddr_size);
        if(data_size <0 )
        {
            printf("Recvfrom error , failed to get packets\n");
            return 1;
        } else {
            printf("Received %d bytes\n",data_size);
          struct sockaddr_storage src_addr;
          struct sockaddr_storage dst_addr;
          char source[INET6_ADDRSTRLEN];
          char destination[INET6_ADDRSTRLEN];

          struct ethhdr* eth = (struct ethhdr*) buffer;
          struct iphdr *iph = (struct iphdr *)(buffer + sizeof(struct ethhdr));


          switch (ntohs(ethh->h_proto)) {
          case 0x86dd: // IPv6
            break;
          case 0x0800: // IPv4
              ((struct sockaddr_in *)&src_addr)->sin_addr.s_addr = iph->saddr;
              inet_ntop(AF_INET, &((struct sockaddr_in *)&src_addr)->sin_addr, source,
                        sizeof source);

              /* Get destination IP */
              ((struct sockaddr_in *)&dst_addr)->sin_addr.s_addr = iph->daddr;
              inet_ntop(AF_INET, &((struct sockaddr_in *)&dst_addr)->sin_addr, destination,
                        sizeof destination);
              printf("%d\n", htons(eth->h_proto));
              switch (iph->protocol) {
                case 1: // ICMP Protocol
                printf("ICMP PACKET %s, %s", source, destination);
                    printf("ICMp PACKET\n");
                    break;
                case 6:
                printf("TCP PACKET %s, %s", source, destination);
                    printf("TCP PACKET\n");
                    break;
                    default:
                        printf("PACKET %s, %s", source, destination);
                }
            break;
          default:
            daemon_log(LOG_INFO,"ARP");
            daemon_log(LOG_INFO, "%.2X-%.2X-%.2X-%.2X-%.2X-%.2X \n", ethh->h_dest[0] , ethh->h_dest[1] , ethh->h_dest[2] , ethh->h_dest[3] , ethh->h_dest[4] , ethh->h_dest[5]);
            send_packet_raw(buf, numbytes, wisprs->rawsd);
            break;
          }

       //Huge code to process the packet (optional)

        //Send the same packet out
        bytes_sent=write(sock,buffer,data_size);
        printf("Sent %d bytes\n",bytes_sent);
         if (bytes_sent < 0) {
            perror("sendto");
            exit(1);
         }

        }
    }
    close(sock_raw);
    return 0;
}