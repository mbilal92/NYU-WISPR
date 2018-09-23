#pragma once
#ifndef WISPR_NET_C  //Include guard 
#define WISPR_NET_C
/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

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

#include "wispr_net.h"
#include "wispr_proto.h"

#include <libdaemon/dexec.h>
#include <libdaemon/dfork.h>
#include <libdaemon/dlog.h>
#include <libdaemon/dpid.h>
#include <libdaemon/dsignal.h>

struct sockaddr_ll saddr, daddr, paddr;
unsigned char mac[6];
struct wisprsock *init_wispr_sockets(char *interface, char *interface2, char *gateWayIporMAC) {
  int sockfd, sendfd, rawsd, i;
  int sockopt;
  struct ifreq ifopts;
  struct wisprsock *wisprs;
  wisprs = malloc(sizeof(struct wisprsock));

  /* Get interface name */
  strcpy(wisprs->ifName, interface);
  strcpy(wisprs->ifName2, interface2);
  if (gateWayIporMAC != NULL)
  {
  	strcpy(wisprs->gateWayIporMAC, gateWayIporMAC);
  	sscanf(gateWayIporMAC, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx", &wisprs->gateWayIporMAC[0], &wisprs->gateWayIporMAC[1], &wisprs->gateWayIporMAC[2], &wisprs->gateWayIporMAC[3], &wisprs->gateWayIporMAC[4], &wisprs->gateWayIporMAC[5]);
  } else {
  	strcpy(wisprs->gateWayIporMAC, "000000");
  }

  /* Open PF_PACKET socket, listening for ALL packet types */
  if ((sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) == -1) {
    daemon_log(LOG_INFO, "listener: socket");
    wisprs->wisprsd = -1;
    wisprs->sendsd = -1;
    wisprs->rawsd = -1;
  }

  /* Open a raw socket for sending untouched packets */
  if ((rawsd = socket(PF_PACKET, SOCK_RAW, IPPROTO_RAW)) == -1) {
    daemon_log(LOG_INFO, "listener: socket");
    wisprs->wisprsd = -1;
    wisprs->sendsd = -1;
    wisprs->rawsd = -1;
  }

  /* Open AF_INET socket to allow sending unencapsulated packets from layer3 */
  if ((sendfd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) == -1) {
    daemon_log(LOG_INFO, "listener: socket");
    wisprs->wisprsd = -1;
    wisprs->sendsd = -1;
    wisprs->rawsd = -1;
  }

  /* Set interface to promiscuous mode */
  strncpy(ifopts.ifr_name, wisprs->ifName, IFNAMSIZ - 1);
  ioctl(sockfd, SIOCGIFFLAGS, &ifopts);
  ifopts.ifr_flags |= IFF_PROMISC;
  ioctl(sockfd, SIOCSIFFLAGS, &ifopts);

  /* Allow the promiscuous socket to be reused - in case connection is closed
   * prematurely */
  if (setsockopt(rawsd, SOL_SOCKET, SO_REUSEADDR, &sockopt, sizeof sockopt) ==
      -1) {
    daemon_log(LOG_INFO, "setsockopt");
    destroy_wisprsock(wisprs);
    wisprs->wisprsd = -1;
    wisprs->sendsd = -1;
    wisprs->rawsd = -1;
  }

  /* Allow the promiscuous socket to be reused - in case connection is closed
  * prematurely */
  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &sockopt, sizeof sockopt) ==
      -1) {
    daemon_log(LOG_INFO, "setsockopt");
    destroy_wisprsock(wisprs);
    wisprs->wisprsd = -1;
    wisprs->sendsd = -1;
    wisprs->rawsd = -1;
  }
  /* Allow the sending socket to be reused - in case connection is closed
  * prematurely */
  if (setsockopt(sendfd, SOL_SOCKET, SO_REUSEADDR, &sockopt, sizeof sockopt) ==
      -1) {
    daemon_log(LOG_INFO, "setsockopt");
    destroy_wisprsock(wisprs);
    wisprs->wisprsd = -1;
    wisprs->sendsd = -1;
    wisprs->rawsd = -1;
  }

  /* Bind to device */
  if (setsockopt(sockfd, SOL_SOCKET, SO_BINDTODEVICE, wisprs->ifName,
                 IFNAMSIZ - 1) == -1) {
    daemon_log(LOG_INFO, "SO_BINDTODEVICE");
    destroy_wisprsock(wisprs);
    wisprs->wisprsd = -1;
    wisprs->sendsd = -1;
    wisprs->rawsd = -1;
  }


  memset(&saddr, 0, sizeof(struct sockaddr_ll));
  saddr.sll_family = AF_PACKET;
  saddr.sll_protocol = htons(ETH_P_ALL);
  saddr.sll_ifindex = if_nametoindex(wisprs->ifName); //ifopts.ifr_ifindex;
  // saddr.sll_ifindex = if_nametoindex(wisprs->ifName); //ifopts.ifr_ifindex;

  if (bind(sockfd, (struct sockaddr *)&saddr, sizeof(saddr)) < 0) {
    daemon_log(LOG_INFO, "%s SO_BINDTODEVICE sockfd", wisprs->ifName);
    destroy_wisprsock(wisprs);
    wisprs->wisprsd = -1;
    wisprs->sendsd = -1;
    wisprs->rawsd = -1;
  }

  memset(&daddr, 0, sizeof(struct sockaddr_ll));
  daddr.sll_family = AF_PACKET;
  daddr.sll_protocol = htons(ETH_P_ALL);
  daddr.sll_ifindex = if_nametoindex(wisprs->ifName2); //ifopts.ifr_ifindex;

  /* Bind to raw device */
  if (bind(rawsd, (struct sockaddr *)&daddr, sizeof(daddr)) < 0) {
    daemon_log(LOG_INFO, "SO_BINDTODEVICE RAW");
    destroy_wisprsock(wisprs);
    wisprs->wisprsd = -1;
    wisprs->sendsd = -1;
    wisprs->rawsd = -1;
  }

    struct ifreq ifreq_c2;
    memset(&ifreq_c2, 0, sizeof(ifreq_c2));
    snprintf(ifreq_c2.ifr_name, sizeof(ifreq_c2.ifr_name), wisprs->ifName2);
    // strncpy(ifreq_c2.ifr_name, "veth0", sizeof("veth0") - 1);

    if (setsockopt(rawsd, SOL_SOCKET, SO_BINDTODEVICE, (void *)&ifreq_c2, sizeof(ifreq_c2)) < 0) {
        perror("bind to eth1");
        }

    if (setsockopt(sockfd, SOL_SOCKET, SO_BINDTODEVICE, (void *)&ifreq_c2, sizeof(ifreq_c2)) < 0) {
        perror("bind to eth1");
        }
  // /* Bind to device */
  // if (setsockopt(sendfd, SOL_SOCKET, SO_BINDTODEVICE, "dummy2",//wisprs->ifName,
  //                IFNAMSIZ - 1) == -1) {
  //   daemon_log(LOG_INFO, "SEND SO_BINDTODEVICE");
  //   destroy_wisprsock(wisprs);
  //   wisprs->wisprsd = -1;
  //   wisprs->sendsd = -1;
  //   wisprs->rawsd = -1;
  // }

  /* Set flag so socket expects us to provide IPv4 header. */
  if (setsockopt(sendfd, IPPROTO_IP, IP_HDRINCL, &sockopt, sizeof(sockopt)) <
      0) {
    daemon_log(LOG_INFO, "IP_HDRINCL");
    destroy_wisprsock(wisprs);
    wisprs->wisprsd = -1;
    wisprs->sendsd = -1;
  }

  wisprs->wisprsd = sockfd;
  wisprs->sendsd = sendfd;
  wisprs->rawsd = rawsd;

  daemon_log(LOG_INFO, "SOCKETS: wispr: %d send: %d raw: %d", wisprs->wisprsd
             ,             wisprs->sendsd, wisprs->rawsd);

    struct ifreq ifreq_c;
    memset(&ifreq_c, 0, sizeof(struct ifreq));
    strncpy(ifreq_c.ifr_name, wisprs->ifName, sizeof(wisprs->ifName) - 1);

    if (ioctl(sockfd, SIOCGIFHWADDR, &ifreq_c) < 0)
        perror("SIOCGIFHWADDR");
	mac[0] = 0;
  	mac[1] = 0;
  	mac[2] = 0;
  	mac[3] = 0;
  	mac[4] = 0;
  	mac[5] = 0;
  
    mac[0] = (unsigned char)(ifreq_c.ifr_hwaddr.sa_data[0]);
    mac[1] = (unsigned char)(ifreq_c.ifr_hwaddr.sa_data[1]);
    mac[2] = (unsigned char)(ifreq_c.ifr_hwaddr.sa_data[2]);
    mac[3] = (unsigned char)(ifreq_c.ifr_hwaddr.sa_data[3]);
    mac[4] = (unsigned char)(ifreq_c.ifr_hwaddr.sa_data[4]);
    mac[5] = (unsigned char)(ifreq_c.ifr_hwaddr.sa_data[5]);

    daemon_log(LOG_INFO, "%X-%X-%X-%X-%X-%X \n", mac[0] , mac[1] , mac[2] , mac[3] , mac[4] , mac[5]);
  return wisprs;
}

void process_newpkt(struct wisprsock *wisprs) {
  ssize_t numbytes;
  uint8_t buf[PACKET_SIZE];
  char source[INET6_ADDRSTRLEN];
  
  memset(buf, 0, PACKET_SIZE * sizeof(uint8_t));
  int saddr_size = sizeof (struct sockaddr_ll);
  // numbytes = recvfrom(wisprs->wisprsd, buf, PACKET_SIZE, 0, NULL, NULL);
  numbytes = recvfrom(wisprs->wisprsd, buf, PACKET_SIZE, 0, (struct sockaddr *)&paddr, &saddr_size);

  struct ethhdr *ethh = (struct ethhdr *)(buf);

  // if (paddr.sll_pkttype == PACKET_OUTGOING) {
  //  	daemon_log(LOG_INFO,"PACKET_OUTGOING");
  //   return;
  // }
  // send_packet(buf, numbytes, wisprs->rawsd);

  // Only deal with IPv6 or IPv6 packets
  if (ethh->h_dest[0] == mac[0] &&
  		ethh->h_dest[1] == mac[1] &&
  		ethh->h_dest[2] == mac[2] &&
  		ethh->h_dest[3] == mac[3] &&
  		ethh->h_dest[4] == mac[4] &&
  		ethh->h_dest[5] == mac[5]  )
  {
  	// daemon_log(LOG_INFO, "PACKET FOR ME");
  	return;
  }

  if (ethh->h_dest[0] == 0xff &&
  		ethh->h_dest[1] == 0xff &&
  		ethh->h_dest[2] == 0xff &&
  		ethh->h_dest[3] == 0xff &&
  		ethh->h_dest[4] == 0xff &&
  		ethh->h_dest[5] == 0xff  )
  {
  	// daemon_log(LOG_INFO, "BROADCAST PACKET");
  	return;
  }
  if (ethh->h_dest[0] == 0x00 &&
  		ethh->h_dest[1] == 0x0C &&
  		ethh->h_dest[2] == 0x29 &&
  		ethh->h_dest[3] == 0xAE &&
  		ethh->h_dest[4] == 0x32 &&
  		ethh->h_dest[5] == 0x68  )
  {
  	// daemon_log(LOG_INFO, "BROADCAST PACKET");
  	return;
  }
  if (ethh->h_dest[0] == 0x01 &&
  		ethh->h_dest[1] == 0x80 &&
  		ethh->h_dest[2] == 0xC2 &&
  		ethh->h_dest[3] == 0x00 &&
  		ethh->h_dest[4] == 0x00 &&
  		ethh->h_dest[5] == 0x0E  )
  {
  	// daemon_log(LOG_INFO, "LLDP BROADCAST PACKET");
  	return;
  }

    int i=0;
    for (i = 0; i < (int)(numbytes) + (numbytes % 8); i += 8) {
    daemon_log(LOG_INFO, "PAYLOAD: %02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X",
                 buf[i], buf[i + 1], buf[i + 2], buf[i + 3], buf[i + 4],
                 buf[i + 5], buf[i + 6], buf[i + 7]);
    }

  daemon_log(LOG_INFO, "%u \n", (unsigned short)ntohs(ethh->h_proto));

  switch (ntohs(ethh->h_proto)) {
  case 0x8100: // VLAN
    daemon_log(LOG_INFO, "VLAN");
    handle_vlanPkt(buf, numbytes, wisprs);
    break;
  case 0x86dd: // IPv6
    handle_ipv6pkt(buf, numbytes, wisprs);
    break;
  case 0x0800: // IPv4
    handle_ipv4pkt(buf, numbytes, wisprs);
    break;
  case 0x0806: // arp
  	daemon_log(LOG_INFO,"ARP");
	// daemon_log(LOG_INFO, "%.2X-%.2X-%.2X-%.2X-%.2X-%.2X \n", ethh->h_dest[0] , ethh->h_dest[1] , ethh->h_dest[2] , ethh->h_dest[3] , ethh->h_dest[4] , ethh->h_dest[5]);
  //    struct iphdr *iph = (struct iphdr *)(buf + sizeof(struct ethhdr));
	//   char source[INET6_ADDRSTRLEN];
	//   struct sockaddr_storage src_addr;
	//   ((struct sockaddr_in *)&src_addr)->sin_addr.s_addr = iph->saddr;
	//   inet_ntop(AF_INET, &((struct sockaddr_in *)&src_addr)->sin_addr, source,
	//             sizeof source);
	//   daemon_log(LOG_INFO, source);
	// add_Src_ip_host(iph->saddr);
    send_packet_raw(buf, numbytes, wisprs->rawsd);
    break;
  default:
    // send_packet_raw(buf, numbytes, wisprs->rawsd);
    break;
  }
}

void handle_ipv6pkt(uint8_t *buf, ssize_t numbytes, struct wisprsock *wisprs) {
  /* ToDo: do something with an ipv6 packet */
}
void handle_vlanPkt(uint8_t *buf, ssize_t numbytes, struct wisprsock *wisprs) {
          // switch (wisprK->mode) {
          // case 0: // WISPR 0 - STRIPING
            process_wispr0_vlan(buf, numbytes, wisprs);
          //   break;
          // case 1: // WISPR 1 - MIRRORING
          //   // process_wispr1(wisprH, wisprK);
          //   break;
          // case 4: // WISPR 4 - STATIC PARITY
          //   // process_parity(wisprH, wisprK);
          //   break;
          // case 5: // WISPR 5 - DYNAMIC PARITY
          //   // process_parity(wisprH, wisprK);
          //   break;
          // default:
          //   break;
          // }

}

void handle_ipv4pkt(uint8_t *buf, ssize_t numbytes, struct wisprsock *wisprs) {
  //daemon_log(LOG_INFO, __func__);

  // struct wispr_packet *wisprP;
  struct gre_base_header *wisprH = NULL;
  wispr_ingress_key *wisprK = NULL;

  // memset(wisprP, 0, sizeof(struct wispr_packet));
  // daemon_log(LOG_INFO, "HERE 2");

  struct sockaddr_storage src_addr;
  struct sockaddr_storage dst_addr;
  char source[INET6_ADDRSTRLEN];
  char destination[INET6_ADDRSTRLEN];
  struct ifreq if_ip;

  memset(&if_ip, 0, sizeof(struct ifreq));

  struct ethhdr* eth = (struct ethhdr*) buf;
  struct iphdr *iph = (struct iphdr *)(buf + sizeof(struct ethhdr));

  ((struct sockaddr_in *)&src_addr)->sin_addr.s_addr = iph->saddr;
  inet_ntop(AF_INET, &((struct sockaddr_in *)&src_addr)->sin_addr, source,
            sizeof source);

  /* Get destination IP */
  ((struct sockaddr_in *)&dst_addr)->sin_addr.s_addr = iph->daddr;
  inet_ntop(AF_INET, &((struct sockaddr_in *)&dst_addr)->sin_addr, destination,
	            sizeof destination);

  if (strcmp(source, "127.0.0.1") == 0 || strcmp(destination, "127.0.0.1") == 0)
  {
      // daemon_log(LOG_INFO, "%.2X-%.2X-%.2X-%.2X-%.2X-%.2X \n", eth->h_dest[0] , eth->h_dest[1] , eth->h_dest[2] , eth->h_dest[3] , eth->h_dest[4] , eth->h_dest[5]);
      // daemon_log(LOG_INFO, "%.2X-%.2X-%.2X-%.2X-%.2X-%.2X \n", eth->h_source[0] , eth->h_source[1] , eth->h_source[2] , eth->h_source[3] , eth->h_source[4] , eth->h_source[5] );
    return;
  }

  // Get the IP Header part of the packet header

  ///// Bilal- TODO: make code linux independent by using struct ip rather then using iphdr

  if (eth->h_dest[0] == mac[0] &&
  		eth->h_dest[1] == mac[1] &&
  		eth->h_dest[2] == mac[2] &&
  		eth->h_dest[3] == mac[3] &&
  		eth->h_dest[4] == mac[4] &&
  		eth->h_dest[5] == mac[5]  )
  {
  	daemon_log(LOG_INFO, "PACKET FOR ME");
  	return;
  }

  if (eth->h_dest[0] == 0xff &&
  		eth->h_dest[1] == 0xff &&
  		eth->h_dest[2] == 0xff &&
  		eth->h_dest[3] == 0xff &&
  		eth->h_dest[4] == 0xff &&
  		eth->h_dest[5] == 0xff  )
  {
  	daemon_log(LOG_INFO, "BROADCAST PACKET");
  	return;
  }

	// daemon_log(LOG_INFO, "PACKET %s, %s", source, destination);


  // Check incoming protocol
  switch (iph->protocol) {
  case 1: // ICMP Protocol
    daemon_log(LOG_INFO, "ICMP PACKET %s, %s", source, destination);
    // daemon_log(LOG_INFO, "%.2X-%.2X-%.2X-%.2X-%.2X-%.2X \n", eth->h_dest[0] , eth->h_dest[1] , eth->h_dest[2] , eth->h_dest[3] , eth->h_dest[4] , eth->h_dest[5]);
    // daemon_log(LOG_INFO, "%.2X-%.2X-%.2X-%.2X-%.2X-%.2X \n", eth->h_source[0] , eth->h_source[1] , eth->h_source[2] , eth->h_source[3] , eth->h_source[4] , eth->h_source[5] );
    // MUST MAKE SURE ICMP PACKETS MAKE THEIR WAY THROUGH NETWORK - SEND PACKET
    // ALONG
	  int i=0;
	  // for (i = 0; i < (int)(numbytes) + (numbytes % 8); i += 8) {
	  //   daemon_log(LOG_INFO, "PAYLOAD: %02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X",
	  //              buf[i], buf[i + 1], buf[i + 2], buf[i + 3], buf[i + 4],
	  //              buf[i + 5], buf[i + 6], buf[i + 7]);
	  // }

	  if (buf[0] == 0x11)
	  {
	   	daemon_log(LOG_INFO,"YDAA GYA");
	  	return;
	  }

	  // buf[0] = 0x11;
  	// add_Src_ip_host(iph->saddr);
    send_packet_raw(buf, numbytes, wisprs->rawsd);
    break;
  case 17: // UDP Protocol
  	daemon_log(LOG_INFO, "UDP PACKET %s, %s", source, destination);
  	send_packet_raw(buf, numbytes, wisprs->rawsd);
  	break;
  case 6: // TCP Protocol
  daemon_log(LOG_INFO, "TCP PACKET %s, %s", source, destination);
  // daemon_log(LOG_INFO, "%.2X-%.2X-%.2X-%.2X-%.2X-%.2X \n", eth->h_dest[0] , eth->h_dest[1] , eth->h_dest[2] , eth->h_dest[3] , eth->h_dest[4] , eth->h_dest[5]);
  // daemon_log(LOG_INFO, "%.2X-%.2X-%.2X-%.2X-%.2X-%.2X \n", eth->h_source[0] , eth->h_source[1] , eth->h_source[2] , eth->h_source[3] , eth->h_source[4] , eth->h_source[5] );

    // If this node is the initial endpoint, process ingress, else move packet
    // along
    // if (do some strcmp based on path table handed from controller) {
        // if (parse_ingress_pkt(buf, numbytes, wispri) != -1)

    // if (strcmp(source, "10.225.23.212") == 0)
    if (strcmp(source, "10.0.0.1") == 0 || strcmp(source, "10.0.0.2") == 0 || strcmp(source, "192.168.61.130") == 0)
    // // if (strstr(&source, "10.0.0"))
    {
      // daemon_log(LOG_INFO, "%.2X-%.2X-%.2X-%.2X-%.2X-%.2X \n", eth->h_dest[0] , eth->h_dest[1] , eth->h_dest[2] , eth->h_dest[3] , eth->h_dest[4] , eth->h_dest[5]);
      // daemon_log(LOG_INFO, "%.2X-%.2X-%.2X-%.2X-%.2X-%.2X \n", eth->h_source[0] , eth->h_source[1] , eth->h_source[2] , eth->h_source[3] , eth->h_source[4] , eth->h_source[5] );
      process_ingress(buf, numbytes, wisprs, 0);
    // } else {

    }
      // }
      // process_ingress(buf, wispri, wisprs);
    // }
    // else {
    //   daemon_log(LOG_INFO, "NON-MATCHED TCP PACKET");
    //   send_packet(buf, numbytes, wisprs->sendsd);
    // }
    // send_packet_raw(buf, numbytes, wisprs->rawsd);

    break;
  case 47: // GRE Protocol

    daemon_log(LOG_INFO, "GRE PACKET %s, %s", source, destination);
    /* Check if this machine is the source or destination */
    // strncpy(if_ip.ifr_name, wisprs->ifName, IFNAMSIZ - 1);
    // if (ioctl(wisprs->wisprsd, SIOCGIFADDR, &if_ip) >= 0) {
    //   // This machine is the source - INGRESS point for WISPR
    //   if (strcmp(source,
    //              inet_ntoa(((struct sockaddr_in *)&if_ip.ifr_addr)->sin_addr)) ==
    //       0) {
    //     daemon_log(LOG_INFO, "SOURCE MATCH %s, %s", source, destination);
    //     // if (parse_ingress_pkt(buf, numbytes, wispri) != -1)
    //       process_ingress(buf, numbytes, wisprs, 2);
    //   }
    //   /* This machine is the destination - EGRESS point for WISPR */
    //   else if (strcmp(destination,
    //                   inet_ntoa(
    //                       ((struct sockaddr_in *)&if_ip.ifr_addr)->sin_addr)) ==
    //            0) {
    //     daemon_log(LOG_INFO, "DEST MATCH %s, %s", source, destination);

         // Parse the WISPR packet to get WISPR header that includes mode, index,
         // * bitmask, GRE payload (encapsulated IP packet), and payload length
			///
        parse_wispr_pkt(buf, numbytes, &wisprH, &wisprK);
        // parse_wispr_pkt(buf, numbytes, wisprP);
        if (wisprK != NULL)
          switch (wisprK->mode) {
          case 0: // WISPR 0 - STRIPING
            process_wispr0(buf, numbytes, wisprs);
            break;
          case 1: // WISPR 1 - MIRRORING
            // process_wispr1(wisprH, wisprK);
            break;
          case 4: // WISPR 4 - STATIC PARITY
            // process_parity(wisprH, wisprK);
            break;
          case 5: // WISPR 5 - DYNAMIC PARITY
            // process_parity(wisprH, wisprK);
            break;
          default:
            break;
          }
        // } else {
        //   // send_packet_raw(buf, numbytes, wisprs->rawsd);
        //   break;
        // }
    break;
  default:
    // daemon_log(LOG_INFO, "MOVING UNKNOWN PACKET ALONG");
    send_packet_raw(buf, numbytes, wisprs->rawsd);
    break;
  }
  int i=0;
  //   if (strcmp(source, "10.0.0.2") == 0 || strcmp(source, "10.0.0.3") == 0)
  //   {
		//   for (i = 0; i < (int)(numbytes) + (numbytes % 8); i += 8) {
		//     daemon_log(LOG_INFO, "PAYLOAD: %02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X",
		//                buf[i], buf[i + 1], buf[i + 2], buf[i + 3], buf[i + 4],
		//                buf[i + 5], buf[i + 6], buf[i + 7]);
		//   }
		// send_packet_raw(buf, numbytes, wisprs->rawsd);
  //   }

  // free(wisprH);
  // free(wisprK);
}

int send_packet(uint8_t *buf, ssize_t numbytes, uint16_t sendsd) {
  struct sockaddr_in sin;
  memset(&sin, 0, sizeof(struct sockaddr_in));
  sin.sin_family = AF_INET;
  struct ip *iph = (struct ip *)(buf + sizeof(struct ethhdr));
  sin.sin_addr.s_addr = (*iph).ip_src.s_addr;
  int sentbytes;
  // sentbytes = send_packet(buf, numbytes + sizeof(struct wispr_packet), sendsd);
  sentbytes = sendto(sendsd, buf + sizeof(struct ethhdr), numbytes - sizeof(struct ethhdr), 0, (struct sockaddr *) &sin, sizeof(sin));
  return sentbytes;
}

int send_packet_raw(uint8_t *buf, ssize_t numbytes, uint16_t sendsd) {
  int i = 4;
  struct sockaddr_ll sin;
  // daemon_log(LOG_INFO,__func__);
  // sin = (struct sockaddr_in *) malloc(sizeof (struct sockaddr_in) );

  /* Cast the buffer into an IP header to get the destination address */
  // struct iphdr *iph = (struct iphdr *)(buf + sizeof(struct ethhdr));

  // The kernel is going to prepare layer 2 information (ethernet frame header)
  // for us.
  // For that, we need to specify a destination for the kernel in order for it
  // to decide where to send the raw datagram. We fill in a struct in_addr with
  // the desired destination IP address, and pass this structure to the sendto()
  // function.

  // daemon_log(LOG_INFO, "OK1");
  // int sizeTosend = numbytes - sizeof(struct ethhdr);
  // uint8_t *temp_buf = (uint8_t *) malloc(sizeTosend);
  // memset(temp_buf, 0, sizeTosend);
  // memcpy(temp_buf, buf + sizeof(struct ethhdr), sizeTosend);
  // char tempDest[1024];
  // char tempDest2[1024];

  // inet_ntop(AF_INET, &iph->daddr, tempDest,
  //           sizeof tempDest);
  // daemon_log(LOG_INFO, "OK2: %s", tempDest);

  // memset(&sin, 0, sizeof(struct sockaddr_in));
  // sin.sin_family = AF_INET;
  // sin.sin_addr.s_addr = inet_addr(tempDest);//iph->daddr;

  // inet_ntop(AF_INET, &(sin->sin_addr), tempDest2, sizeof (tempDest2));

  // daemon_log(LOG_INFO, "OK4: %s", tempDest2);

 // inet_ntop(AF_INET, &(sin->sin_addr.s_addr), tempDest, sizeof tempDest);
 //   daemon_log(LOG_INFO, tempDest);
  // /* Print out payload for sanity check - remove later*/
  // for (i = 0; i < (int)(numbytes) + (numbytes % 8); i += 8) {
  //   daemon_log(LOG_INFO, "PAYLOAD: %02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X",
  //              buf[i], buf[i + 1], buf[i + 2], buf[i + 3], buf[i + 4],
  //              buf[i + 5], buf[i + 6], buf[i + 7]);
  // }
  // daemon_log(LOG_INFO, "\n");
  // buf[numbytes-10] = '}';
  // daemon_log(LOG_INFO,"\n");
  // int SSin = sizeof(sin);

  // daemon_log(LOG_INFO, "%d,%d,%d", numbytes, strlen(temp_buf), SSin);
  // for (i = 0; i < sizeTosend + (sizeTosend % 8); i += 8) {
  //   daemon_log(LOG_INFO, "PAYLOAD1: %02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X",
  //              temp_buf[i], temp_buf[i + 1], temp_buf[i + 2], temp_buf[i + 3], temp_buf[i + 4],
  //              temp_buf[i + 5], temp_buf[i + 6], temp_buf[i + 7]);
  // }
    // return -1;

  if ( (i = sendto(sendsd, buf, numbytes, 0, NULL, NULL)) < 0) {
  // if ( (i = write(sendsd, buf, numbytes) < 0) ) {
    daemon_log(LOG_INFO, "FAILED MY ASS %s, %d", strerror(errno), errno);
    // daemon_log(LOG_INFO, "FAILED MY ASS 2:  %d", i);
    return -1;
  } else {
    // daemon_log(LOG_INFO, "sent bytes RAW %d", i);
    return i;
  }
}


void destroy_wisprsock(struct wisprsock *wisprs) {
  daemon_log(LOG_INFO, "Destroying WISPR socket");
  close(wisprs->wisprsd);
  close(wisprs->sendsd);
  free(wisprs);
}

#endif // WISPR_NET_C_