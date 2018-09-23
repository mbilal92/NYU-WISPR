/*
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 */
#pragma once
#ifndef WISPR_PROTO_C  //Include guard 
#define WISPR_PROTO_C

#include "wispr_net.h"
#include "wispr_proto.h"
#include <arpa/inet.h>
#include <asm/byteorder.h>
#include <errno.h>
#include <linux/if_packet.h>
#include <netinet/ether.h>
#include <netinet/ip.h>  // declarations for ip header
#include <netinet/ip6.h> // declarations for ip6_hdr
#include <netinet/tcp.h> // declarations for tcp header
#include <netinet/udp.h> // declarations for udp header
#include <netinet/ip_icmp.h> // declarations for udp header
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <limits.h>

#include <libdaemon/dexec.h>
#include <libdaemon/dfork.h>
#include <libdaemon/dlog.h>
#include <libdaemon/dpid.h>
#include <libdaemon/dsignal.h>


int packetCound = 0;
int VlanPacketReceived = 0;
int dropCount = 0;

wisprmirror_ht *wisprmirt;
// wisprparity_ht *wisprpart;
wispringress_ht_vlan *wispringress;
host_IP *hostIp;

double time_diff(struct timeval x, struct timeval y);

void parse_wispr_pkt(uint8_t *buf, ssize_t numbytes, struct gre_base_header **greh, wispr_ingress_key **key) {

  // uint8_t * temp_buf = (uint8_t *) malloc(sizeof(uint8_t) * numbytes);
  uint8_t iplength = 0;
  // uint8_t optlength = 0;
  // wisprK = malloc(sizeof(struct wispr_packet));

  struct ip *iph = (struct ip *)(buf + sizeof(struct ethhdr));
  iplength = iph->ip_hl;
  // optlength = (iplength - 5) * 4;

  // if (optlength > 0) {
  //   options = (struct ingressinfo *)(buf + sizeof(struct ethhdr) +
  //                                    sizeof(struct iphdr) + sizeof(uint16_t));
  //   daemon_log(LOG_INFO, "NUM PKTS: %d MODE: %d", options->num_pkts,
  //              options->mode);
  //   wisprp->wisprh.num_pkts = options->num_pkts;
  // }

  *greh =
      (struct gre_base_header *)(buf + sizeof(struct ethhdr) + (iplength * 4)); // + sizeof(struct iphdr))

      // daemon_log(LOG_INFO,
      //            "GRE flags: %04x GRE proto: %04x",
      //            ntohs((*greh)->gre_flags), ntohs((*greh)->gre_proto));
  // offset =
  //     sizeof(struct ethhdr) + (iplength * 4) + sizeof(struct gre_base_header);

  // Key must be present for WISPR functionality
  // if (ntohs(greh->gre_flags) & GRE_FLAGS_KP) {
    if (ntohs((*greh)->gre_flags) == GRE_FLAGS_KP  && ntohs((*greh)->gre_proto) == 0x0000)
    {
      *key = (wispr_ingress_key *)(buf + sizeof(struct ethhdr) + (iplength * 4) + sizeof(struct gre_base_header));

      // daemon_log(LOG_INFO,
      //            "GRE flags: %04x GRE proto: %04x WISPR mode: %04x WISPR index: "
      //            "%04x WISPR bitmask: %04x",
      //            ntohs((*greh)->gre_flags), ntohs((*greh)->gre_proto),
      //            (*key)->mode, (*key)->index, (*key)->bitmask);      
    } else {
      *key = NULL;
    }
  // } else {
  //   key = NULL;
  // }
}


// int parse_newingress_pkt(uint8_t *buf, ssize_t numbytes,
//                          wispr_ingress_key *wispri) {
//   uint16_t offset = 0;
//   uint16_t i = 0;
//   uint8_t iplength = 0;
//   uint8_t optlength = 0;
//   uint32_t *key;

//   /* We hide number of packets and wispr mode in the options field */
//   struct ingressinfo {
//     uint8_t num_pkts : 4;
//     uint8_t mode : 4;
//   };
//   struct ingressinfo *options;

//   struct ip *iph = (struct ip *)(buf + sizeof(struct ethhdr));
//   iplength = iph->ip_hl;
//   optlength = (iplength - 5) * 4;

//   if (optlength > 0) {
//     options = (struct ingressinfo*)(buf + sizeof(struct ethhdr) +
//                                     sizeof(struct iphdr) + sizeof(uint16_t));
//     daemon_log(LOG_INFO, "NUM PKTS: %d MODE: %d", options->num_pkts,
//                options->mode);
//   }

//   struct gre_base_header* greh =
//       (struct gre_base_header*)(buf + sizeof(struct ethhdr) + (iplength * 4));

//   offset =
//       sizeof(struct ethhdr) + (iplength * 4) + sizeof(struct gre_base_header);

//   if (ntohs(greh->gre_flags) & GRE_FLAGS_CP) offset += GRE_HEADER_SECTION;

//   key = (uint32_t*)(buf + offset);

//   if (*key != 0) {
//     daemon_log(LOG_INFO,
//                "KEY HAD ALREADY BEEN SET - IGNORE THIS PACKET! KEY: %08x",
//                *key);
//     return -1;
//   }
//   wispri->payload_length = (ssize_t)(ntohs(iph->ip_len));

//   // memset(wispri->paritypkt, 0, PACKET_SIZE * sizeof(uint8_t));
//   // memcpy(wispri->paritypkt, buf + sizeof(struct ethhdr),
//   // wispri->payload_length);
//   wispri->mode = 1;
//   wispri->num_pkts = 2;

//   return 1;
// }

// int parse_ingress_pkt(uint8_t *buf, ssize_t numbytes,
//                       wispr_ingress_key *wispri) {
//   uint16_t offset = 0;
//   uint16_t i = 0;
//   uint8_t iplength = 0;
//   uint8_t optlength = 0;
//   uint32_t *key;

//   /* We hide number of packets and wispr mode in the options field */
//   struct ingressinfo {
//     uint8_t num_pkts : 4;
//     uint8_t mode : 4;
//   };

//   struct ingressinfo *options;

//   struct ip *iph = (struct ip *)(buf + sizeof(struct ethhdr));
//   iplength = iph->ip_hl;
//   optlength = (iplength - 5) * 4;

//   if (optlength > 0) {
//     options = (struct ingressinfo *)(buf + sizeof(struct ethhdr) +
//                                      sizeof(struct iphdr) + sizeof(uint16_t));
//     daemon_log(LOG_INFO, "NUM PKTS: %d MODE: %d", options->num_pkts,
//                options->mode);
//   }

//   struct gre_base_header *greh =
//       (struct gre_base_header *)(buf + sizeof(struct ethhdr) + (iplength * 4));

//   offset =
//       sizeof(struct ethhdr) + (iplength * 4) + sizeof(struct gre_base_header);

//   if (ntohs(greh->gre_flags) & GRE_FLAGS_CP)
//     offset += GRE_HEADER_SECTION;

//   key = (uint32_t *)(buf + offset);

//   if (*key != 0) {
//     daemon_log(LOG_INFO,
//                "KEY HAD ALREADY BEEN SET - IGNORE THIS PACKET! KEY: %08x",
//                *key);
//     return -1;
//   }

//   wispri->payload_length = (ssize_t)(ntohs(iph->ip_len));

//   memset(wispri->paritypkt, 0, PACKET_SIZE * sizeof(uint8_t));
//   memcpy(wispri->paritypkt, buf + sizeof(struct ethhdr), wispri->payload_length);
//   wispri->mode = options->mode;
//   wispri->num_pkts = options->num_pkts;

//   for (i = 0; i < wispri->payload_length; i += 8) {
//     daemon_log(
//         LOG_INFO,
//         "INGRESS PACKET PAYLOAD: %02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X",
//         wispri->paritypkt[i], wispri->paritypkt[i + 1], wispri->paritypkt[i + 2],
//         wispri->paritypkt[i + 3], wispri->paritypkt[i + 4],
//         wispri->paritypkt[i + 5], wispri->paritypkt[i + 6],
//         wispri->paritypkt[i + 7]);
//   }
//   return 1;
// }
void process_wispr0_vlan(uint8_t *buf, ssize_t numbytes, struct wisprsock *wisprs, int _mtu) {
  // daemon_log(LOG_INFO, __func__);
  mtu = _mtu;

  wisprmirror_ht *tmp;
  wispr_key_t key;
  wispr_ingress_vlan* vlanThings;
  // struct iphdr *iph = (struct iphdr *)(buf + sizeof(struct ethhdr) + sizeof(wispr_ingress_vlan));
  // struct ip *iph = (struct ip *)(buf + sizeof(struct ethhdr));

  struct ip *iph = (struct ip *)(buf + sizeof(struct ethhdr) + sizeof(wispr_ingress_vlan));
  key.src_ip = (*iph).ip_src.s_addr;
  key.dst_ip = (*iph).ip_dst.s_addr;
  char source[INET6_ADDRSTRLEN];
  char destination[INET6_ADDRSTRLEN];
  struct sockaddr_storage src_addr;
  struct sockaddr_storage dst_addr;

  struct iphdr *iph2 = (struct iphdr *)(buf + sizeof(struct ethhdr) + sizeof(wispr_ingress_vlan));

  ((struct sockaddr_in *)&src_addr)->sin_addr.s_addr = iph2->saddr;
  inet_ntop(AF_INET, &((struct sockaddr_in *)&src_addr)->sin_addr, source,
            sizeof source);

  /* Get destination IP */
  ((struct sockaddr_in *)&dst_addr)->sin_addr.s_addr = iph2->daddr;
  inet_ntop(AF_INET, &((struct sockaddr_in *)&dst_addr)->sin_addr, destination,
              sizeof destination);

  daemon_log(LOG_INFO, "VLAN PACKET RECEIVED  %s, %s", source, destination);
  daemon_log(LOG_INFO, "VLAN Bytes Received: %d", numbytes);


  vlanThings = (wispr_ingress_vlan *) (buf + sizeof(struct ethhdr));

  struct ethhdr* eth = (struct ethhdr*) buf;
  // 00:50:56:ef:ba:73
  int packet_number = ntohs(vlanThings->vlan_id) & 0x000F;
  int group_id = (ntohs(vlanThings->vlan_id) & 0x00F0) >> 4;
  int wispr_mode = (ntohs(vlanThings->vlan_id) & 0x0F00) >> 8;
  uint16_t previous_protocol = ntohs(vlanThings->next_protocol);
  
  daemon_log(LOG_INFO, "RECEIVED: %4X %x %x %x -- %4x %4x ", ntohs(vlanThings->vlan_id),  wispr_mode, group_id,packet_number, vlanThings->next_protocol, previous_protocol);
  // daemon_log(LOG_INFO, "RECEIVED: %4X %x %x %x ", key->vlan_id, wispr_mode, group_id, packet_number );
 if (wispr_mode == 3) { // Parity Packet
    // parity_mode();
  } else { // mode stripping or mirrioring

    double timediff;
    struct timeval tv;
    gettimeofday(&tv, NULL);

    tmp = get_or_add_mirror_session(key);
    timediff = time_diff(tmp->tv[group_id][packet_number], tv);

    if (timediff > TIMEOUT || NUMBEROFPATHS ==1) {
      // daemon_log(LOG_INFO, "PACKET HASNT BEEN SEEN");
      tmp->tv[group_id][packet_number] = tv;

    // daemon_log(LOG_INFO, "%.2X-%.2X-%.2X-%.2X-%.2X-%.2X \n", eth->h_dest[0] , eth->h_dest[1] , eth->h_dest[2] , eth->h_dest[3] , eth->h_dest[4] , eth->h_dest[5]);
    // daemon_log(LOG_INFO, "%.2X-%.2X-%.2X-%.2X-%.2X-%.2X \n", wisprs->gateWayIporMAC[0] , wisprs->gateWayIporMAC[1] , wisprs->gateWayIporMAC[2] , wisprs->gateWayIporMAC[3] , wisprs->gateWayIporMAC[4] , wisprs->gateWayIporMAC[5]);
      // if (new != NULL || strcmp(wisprs->gateWayIp, source1) == 0 ) // found the dst ,, behave like egress NODE
    // if (new != NULL || (eth->h_dest[0] == wisprs->gateWayIporMAC[0] &&
    //     eth->h_dest[1] == wisprs->gateWayIporMAC[1] &&
    //     eth->h_dest[2] == wisprs->gateWayIporMAC[2] &&
    //     eth->h_dest[3] == wisprs->gateWayIporMAC[3] &&
    //     eth->h_dest[4] == wisprs->gateWayIporMAC[4] &&
    //     eth->h_dest[5] == wisprs->gateWayIporMAC[5]) ) { // found the dst ,, behave like egress NODE  {
      // daemon_log(LOG_INFO, "Key Found"); 
      uint8_t *rem_buf;
      uint8_t *new_packet;

      // remove GRE header and key
      int rem_len = numbytes - sizeof(struct ether_header) - sizeof(wispr_ingress_vlan);
      rem_buf = (uint8_t * ) malloc(rem_len);
      memcpy(rem_buf, buf + sizeof(struct ether_header) + sizeof(wispr_ingress_vlan), rem_len);

      memset(buf + sizeof(struct ether_header), 0, rem_len + sizeof(wispr_ingress_vlan));

      memcpy(buf + sizeof(struct ether_header) , rem_buf, rem_len);
     
      // struct ethhdr* eth = (struct ethhdr*) buf;

      eth->h_proto = htons(previous_protocol);//htons(ETH_P_IP); //means next header will be IP header

      send_packet_raw(buf, numbytes - sizeof(wispr_ingress_vlan), wisprs->rawsd);
      VlanPacketReceived += 1;
      daemon_log(LOG_INFO, "VLAN PACKETS COUNT: %d", VlanPacketReceived);

    } else {
     dropCount += 1;
     daemon_log(LOG_INFO, "PACKET ALREADY SEEN LATELY, DROPPING COUNT: %d", dropCount);
    }
  }

    
  // } else {
  //   daemon_log(LOG_INFO, "DST IP not found");
  //   send_packet_raw(buf, numbytes, wisprs->rawsd);    
  // }

}


void process_wispr0(uint8_t *buf, ssize_t numbytes, struct wisprsock *wisprs) {
// void process_wispr0(struct wispr_packet *wisprp, struct wisprsock *wisprs) {
  // daemon_log(LOG_INFO, __func__);
  struct iphdr *iph = (struct iphdr *)(buf + sizeof(struct ethhdr));
  // struct ip *iph = (struct ip *)(buf + sizeof(struct ethhdr));

  char source1[INET6_ADDRSTRLEN];
  struct sockaddr_storage src_addr;
  ((struct sockaddr_in *)&src_addr)->sin_addr.s_addr = iph->daddr;
  inet_ntop(AF_INET, &((struct sockaddr_in *)&src_addr)->sin_addr, source1,
            sizeof source1);

  // daemon_log(LOG_INFO, source1);
  host_IP *new = NULL;
  HASH_FIND(hh, hostIp, &(iph->daddr), sizeof(uint32_t), new);
  struct ethhdr* eth = (struct ethhdr*) buf;
  // 00:50:56:ef:ba:73

  // daemon_log(LOG_INFO, "%.2X-%.2X-%.2X-%.2X-%.2X-%.2X \n", eth->h_dest[0] , eth->h_dest[1] , eth->h_dest[2] , eth->h_dest[3] , eth->h_dest[4] , eth->h_dest[5]);
  // daemon_log(LOG_INFO, "%.2X-%.2X-%.2X-%.2X-%.2X-%.2X \n", wisprs->gateWayIporMAC[0] , wisprs->gateWayIporMAC[1] , wisprs->gateWayIporMAC[2] , wisprs->gateWayIporMAC[3] , wisprs->gateWayIporMAC[4] , wisprs->gateWayIporMAC[5]);
    // if (new != NULL || strcmp(wisprs->gateWayIp, source1) == 0 ) // found the dst ,, behave like egress NODE
  if (new != NULL || (eth->h_dest[0] == wisprs->gateWayIporMAC[0] &&
      eth->h_dest[1] == wisprs->gateWayIporMAC[1] &&
      eth->h_dest[2] == wisprs->gateWayIporMAC[2] &&
      eth->h_dest[3] == wisprs->gateWayIporMAC[3] &&
      eth->h_dest[4] == wisprs->gateWayIporMAC[4] &&
      eth->h_dest[5] == wisprs->gateWayIporMAC[5]) ) { // found the dst ,, behave like egress NODE  {
    //daemon_log(LOG_INFO, "Key Found"); 
    uint8_t *rem_buf;
    uint8_t *new_packet;

    // remove GRE header and key
    int rem_len = numbytes - sizeof(struct iphdr) - sizeof(struct ether_header) - sizeof(struct wispr_packet);
    rem_buf = (uint8_t * ) malloc(rem_len);
    memcpy(rem_buf, buf + sizeof(struct iphdr) + sizeof(struct ether_header) + sizeof(struct wispr_packet), rem_len);

    // struct wispr_packet *gre = (struct wispr_packet *)(buf + sizeof(struct iphdr) + sizeof(struct ether_header));
    // uint16_t temp_gre_flags = gre->header.gre_flags;
    // gre->header.gre_flags = htons(ntohl(temp_gre_flags) | GRE_FLAGS_KP);
    // gre->key = key->wispr_key;
    // gre->header.gre_proto = ntohl(ETHER_TYPE);
    // daemon_log(LOG_INFO,"GRE THINGS: %d %d %d %d %d\n", gre->header.gre_flags, gre->header.gre_proto, gre->key.mode, gre->key.index, gre->key.bitmask);
    //reset buffer to 0
    memset(buf + sizeof(struct ether_header) + sizeof(struct iphdr), 0, rem_len + sizeof(struct wispr_packet));

    // cope data in gre packet -- remove gre header+key
    memcpy(buf + sizeof(struct ether_header) + sizeof(struct iphdr), rem_buf, rem_len);
    // daemon_log(LOG_INFO,"%d", ntohs(iph->tot_len));
    // daemon_log(LOG_INFO, ntohs(iph->tot_len));
    // daemon_log(LOG_INFO, htons(iph->tot_len));

    iph->tot_len = htons(ntohs(iph->tot_len) - sizeof(struct wispr_packet));
    // daemon_log(LOG_INFO,"%d", ntohs(iph->tot_len));
    iph->protocol = (u_char) 6;
    daemon_log(LOG_INFO, "***%s***", wisprs->ifName);    
    send_packet_raw(buf, numbytes - sizeof(struct wispr_packet), wisprs->rawsd);
    // send_packet(wisprp->payload, wisprp->payload_length, wisprs->sendsd);
    
  } else {
    // daemon_log(LOG_INFO, "DST IP not found");
    send_packet_raw(buf, numbytes, wisprs->rawsd);    
  }

}

// void process_wispr1(struct wispr_packet *wisprp, struct wisprsock *wisprs) {
//   /* check if already in hash table (dedupe)
//  * if so, noop
//  * else, add entry to table, send packet */
//   wisprmirror_ht *tmp;
//   uint16_t index;
//   double timediff;
//   wispr_key_t key;

//   struct timeval tv;
//   gettimeofday(&tv, NULL);
//   /* Using 16 bit bitmask field rather than 12 bit index field */
//   index = wisprp->wisprh.bitmask;

//   /* WISPR tables are keyed by src & dst addresses */
//   struct ip *iph = (struct ip *)(wisprp->payload);
//   key.src_ip = (*iph).ip_src.s_addr;
//   key.dst_ip = (*iph).ip_dst.s_addr;

//   daemon_log(LOG_INFO, "SRC IP: %08x DST IP: %08x INDEX: %d", ntohl(key.src_ip),
//              ntohl(key.dst_ip), index); 

//   /* Does key (src, dst pair) already exist in the table?
//  * have we seen this packet? / or has it timed out?
//  * */
//   add_mirrorsession(key);

//   HASH_FIND(hh, wisprmirt, &key, sizeof(wispr_key_t), tmp);

//   timediff = time_diff(tmp->tv[index], tv);

//   if (timediff > TIMEOUT) {
//     daemon_log(LOG_INFO, "PACKET HASNT BEEN SEEN");
//     tmp->tv[index] = tv;
//     send_packet(wisprp->payload, wisprp->payload_length, wisprs->sendsd);
//   } else
//     daemon_log(LOG_INFO, "PACKET ALREADY SEEN LATELY, DROPPING");
// }

// void process_parity(struct wispr_packet *wisprp, struct wisprsock *wisprs) {
//   /* check if already in hash table (dedupe)
//  * if so, noop
//  * else, add entry to table, send packet */
//   wisprparity_ht *tmp;
//   uint16_t index;
//   uint16_t newbitmask;
//   uint16_t oldbitmask;
//   uint16_t bittest;
//   uint16_t paritytest = 0;
//   struct timeval tv;
//   double timediff;
//   wispr_key_t key;

//   gettimeofday(&tv, NULL);
//   index = wisprp->wisprh.index;

//   /* WISPR tables are keyed by src & dst addresses */
//   struct ip *iph = (struct ip *)(wisprp->payload);
//   key.src_ip = (*iph).ip_src.s_addr;
//   key.dst_ip = (*iph).ip_dst.s_addr;

//   daemon_log(LOG_INFO, "SRC IP: %08x DST IP: %08x INDEX: %d", ntohl(key.src_ip),
//              ntohl(key.dst_ip), index);

//   /* Does key (src, dst pair) already exist in the table?
//  * have we seen this packet? / or has it timed out?
//  * */
//   add_paritysession(key);

//   HASH_FIND(hh, wisprpart, &key, sizeof(wispr_key_t), tmp);

//   timediff = time_diff(tmp->paritytab.table[index].tv, tv);

//   if (timediff > PARITYTIMEOUT) {
//     daemon_log(LOG_INFO, "NEW PARITY ENTRY, ZEROING");
//     memset(&tmp->paritytab.table[index], 0, sizeof(wispr_entry));
//     tmp->paritytab.table[index].tv = tv;
//   } else
//     daemon_log(LOG_INFO, "RECENTLY USED PARITY ENTRY");

//   oldbitmask = tmp->paritytab.table[index].bitmask;
//   newbitmask = oldbitmask | wisprp->wisprh.bitmask;
//   if (!(newbitmask > oldbitmask))
//     daemon_log(LOG_INFO, "BITMASK NOT SUFFICIENTLY DIFFERENT %08x %08x %d",
//                oldbitmask, newbitmask, oldbitmask - newbitmask);
//   else {
//     daemon_log(LOG_INFO, "BITMASK HAS BEEN CHANGED %08x %08x %d", oldbitmask,
//                newbitmask, oldbitmask - newbitmask);
//     tmp->paritytab.table[index].bitmask = newbitmask;
//     xor_packetdata(tmp, wisprp);

//     /* If it is the parity packet, dont send on. Else, send */
//     paritytest = ~paritytest;
//     paritytest = paritytest << wisprp->wisprh.num_pkts;
//     paritytest += (1 << (wisprp->wisprh.num_pkts - 1));
//     if (wisprp->wisprh.bitmask != paritytest) {
//       daemon_log(LOG_INFO, "SENDING JUST ARRIVED PACKET");
//       send_packet(wisprp->payload, wisprp->payload_length, wisprs->sendsd);
//     } else
//       daemon_log(LOG_INFO, "NOT SENDING ARRIVED PARITY PACKET");

//     bittest = ~(tmp->paritytab.table[index].bitmask);
//     /* Check if only one packet is missing from the group, if so, potentially
//      * reconstruct */7
//     if (bittest && !(bittest & (bittest - 1))) {
//       /* If the only packet missing is the parity packet, DONT reconstruct */
//       if (bittest != (1 << (wisprp->wisprh.num_pkts - 1))) {
//         struct ip *iph = (struct ip *)(tmp->paritytab.table[index].pkt);
//         uint16_t length = ntohs(iph->ip_len);
//         daemon_log(LOG_INFO,
//                    "SENDING RECONSTRUCTED PACKET bitmask: %08x length: %d",
//                    bittest, length);
//         send_packet(tmp->paritytab.table[index].pkt, length, wisprs->sendsd);
//       }
//     }
//   }
// }

void process_ingress(uint8_t *buf,  ssize_t numbytes, struct wisprsock *wisprs, int mode, int _mtu) {
 // daemon_log(LOG_INFO, __func__);
  mtu = _mtu;

  switch (mode) {
  case 0: // WISPR 0 - STRIPING
    // daemon_log(LOG_INFO, "INGRESS STRIPING");
    process_ingress0(buf, numbytes, wisprs);
    break;
  case 1: // WISPR 1 - MIRRORING
    daemon_log(LOG_INFO, "INGRESS MIRRORING");
    // process_ingress1(buf, wispri, wisprs);
    break;
  case 4: // WISPR 4 - STATIC PARITY
    daemon_log(LOG_INFO, "INGRESS PARITY");
    // process_ingressparity(buf, wispri, wisprs);
    break;
  case 5: // WISPR 5 - DYNAMIC PARITY
    daemon_log(LOG_INFO, "INGRESS PARITY");
    // process_ingressparity(buf, wispri, wisprs);
    break;
  default:
    break;
  }
//  daemon_log(LOG_INFO, __func__);
}

void process_ingress0(uint8_t *buf, ssize_t numbytes, struct wisprsock *wisprs) {
  // daemon_log(LOG_INFO, __func__);
  int i;
  wispringress_ht_vlan *tmp, *mirror_tmp;

  wispr_key_t key;

  /* WISPR tables are keyed by src & dst addresses */
  struct ip *iph = (struct ip *)(buf + sizeof(struct ethhdr));
  key.src_ip = (*iph).ip_src.s_addr;
  key.dst_ip = (*iph).ip_dst.s_addr;

  /* Set up a WISPR session table between the source and dest */
  /* Does key (src, dst pair) already exist in the table? */
  // if (check_dst_ip_host(key.dst_ip) == -1) {

  //   add_Src_ip_host(key.src_ip);
  tmp = get_or_add_ingresssession(key);

  int packet_number = tmp->vlan_id & 0x000F;
  int group_id = (tmp->vlan_id & 0x00F0) >> 4;
  int wispr_mode = (tmp->vlan_id & 0x0F00) >> 8;

  if (tmp != NULL) {

    if (packet_number == NUMBEROFPATHS)
    {
      tmp->vlan_id = tmp->vlan_id & 0xFFF0;
      tmp->vlan_id = tmp->vlan_id + 1;     
      tmp->vlan_id = tmp->vlan_id + 0x10;
      group_id = (tmp->vlan_id & 0xF0) >> 4;     

      if (group_id == NUMBEROFGROUPS) {
        tmp->vlan_id = tmp->vlan_id & 0xFF0F;
        // send_paritypacket();
      }
    } else {
      tmp->vlan_id += 1;
    }
  } else {
    tmp->vlan_id = 1;
    tmp->parity_packet_length = 0;

    // for (i = 0; i < PACKET_SIZE; ++i) {
    //   tmp->pkt[i] = 0;
    // }

  }

  // for (i = 0; i < numbytes; ++i)
  //   tmp->pkt[i] = tmp->pkt[i] ^ buf[i];

  // if (tmp->parity_packet_length < numbytes)
  //   tmp->parity_packet_length = numbytes;
  
  // if (group_id == NUMBEROFGROUPS) {
  //   send_paritypacket();
  // }

  daemon_log(LOG_INFO, "***%s***", wisprs->ifName);
  send_packet_withkey(buf, numbytes, wisprs->rawsd, tmp);

  // } else {
  //   send_packet_raw(buf, numbytes, wisprs->rawsd);
  // }

  // newkey = tmp->wisprheader.mode << 28;
  // newkey = newkey | tmp->wisprheader.index << 16;
  // memcpy(newkey + 16, tmp->wisprheader.bitmask , sizeof(tmp->wisprheader.bitmask))

  // send_packet_withkey(buf, numbytes, wisprs->rawsd, tmp);

  // send Mirror Packcet
  // mirror_tmp = (wispringress_ht *) malloc(sizeof(wispringress_ht));
  // *mirror_tmp = *tmp;
  // mirror_tmp->wispr_key.mode = 1;
  // send_packet_withkey(buf, numbytes, wisprs->rawsd, mirror_tmp);

 // daemon_log(LOG_INFO, __func__);
}

// void process_ingress1(uint8_t *buf, ssize_t numbytes, struct wisprsock *wisprs) {

//   /* WISPR tables are keyed by src & dst addresses */
//   struct ip *iph = (struct ip *)(buf + sizeof(struct ethhdr));
//   key.src_ip = (*iph).ip_src.s_addr;
//   key.dst_ip = (*iph).ip_dst.s_addr;

//   /* Set up a WISPR session table between the source and dest */
//   /* Does key (src, dst pair) already exist in the table? */
//   add_ingresssession(key);

//   HASH_FIND(hh, wispringress, &key, sizeof(wispr_key_t), tmp);

//   tmp->wisprheader.mode = wispri->mode;
//   tmp->wisprheader.num_pkts = wispri->num_pkts;
//   // tmp->wisprheader.index = wispri->index;

//   daemon_log(LOG_INFO, "SRC IP: %08x DST IP: %08x INDEX: %d", ntohl(key.src_ip),
//              ntohl(key.dst_ip), tmp->wisprheader.index);

//   /* set up the GRE key */
//   newkey = tmp->wisprheader.mode << 28;
//   newkey = newkey | tmp->wisprheader.index << 16;
//   // newkey = newkey | (tmp->wisprheader.bitmask + (1 << tmp->wisprheader.sent));
//   daemon_log(LOG_INFO, "CRAFTED GRE KEY %08x", newkey);

//   /* Add to index for future packet seq number */
//   tmp->wisprheader.index += 1;
//   HASH_REPLACE(hh, wispringress, key, sizeof(wispr_key_t), tmp, old);

//   /* Modify the GRE KEY and send the packet */
//   // for (i = 0; i < wispri->num_pkts; i++)
//   //	daemon_log(LOG_INFO, "Sending packets %d",i);
//   send_packet_withkey(buf, wispri->payload_length, wisprs->rawsd, htonl(newkey));
// }

// void process_ingressparity(uint8_t *buf, wispr_ingress_key *wispri, struct wisprsock *wisprs) {
//   wispringress_ht *tmp;
//   wispr_key_t key;
//   uint16_t i = 0;
//   uint32_t newkey = 0;

//   /* WISPR tables are keyed by src & dst addresses */
//   struct ip *iph = (struct ip *)(buf + sizeof(struct ethhdr));
//   key.src_ip = (*iph).ip_src.s_addr;
//   key.dst_ip = (*iph).ip_dst.s_addr;

//   daemon_log(LOG_INFO, "SRC IP: %08x DST IP: %08x INDEX: %d", ntohl(key.src_ip),
//              ntohl(key.dst_ip), index);

//   /* Set up a WISPR session table between the source and dest */
//   /* Does key (src, dst pair) already exist in the table? */
//   add_ingresssession(key);

//   HASH_FIND(hh, wispringress, &key, sizeof(wispr_key_t), tmp);

//   tmp->wisprheader.mode = wispri->mode;
//   tmp->wisprheader.num_pkts = wispri->num_pkts;

//   /* Set up bitmask for number of packets */
//   if (tmp->wisprheader.bitmask == 0) {
//     daemon_log(LOG_INFO, "NEW BITMASK - SET TO ALL ONES AND BITSHIFT LEFT");
//     tmp->wisprheader.bitmask = ~tmp->wisprheader.bitmask;
//     tmp->wisprheader.bitmask = tmp->wisprheader.bitmask << wispri->num_pkts;
//   }

//   /* Parity packet needs to be the length of the largest XORed packet in the
//    * group */
//   if (wispri->payload_length > tmp->wisprheader.payload_length)
//     tmp->wisprheader.payload_length = wispri->payload_length;

//   /* XOR the packet */
//   daemon_log(LOG_INFO, "XORing ingress");
//   for (i = 0; i < wispri->payload_length; i++)
//     tmp->wisprheader.paritypkt[i] =
//         tmp->wisprheader.paritypkt[i] ^ wispri->paritypkt[i];

//   for (i = 0; i < tmp->wisprheader.payload_length; i += 8) {
//     daemon_log(
//         LOG_INFO, "XOR PAYLOAD: %02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X",
//         tmp->wisprheader.paritypkt[i], tmp->wisprheader.paritypkt[i + 1],
//         tmp->wisprheader.paritypkt[i + 2], tmp->wisprheader.paritypkt[i + 3],
//         tmp->wisprheader.paritypkt[i + 4], tmp->wisprheader.paritypkt[i + 5],
//         tmp->wisprheader.paritypkt[i + 6], tmp->wisprheader.paritypkt[i + 7]);
//   }
//   /* set up the GRE key */
//   newkey = tmp->wisprheader.mode << 28;
//   newkey = newkey | tmp->wisprheader.index << 16;
//   newkey = newkey | (tmp->wisprheader.bitmask + (1 << tmp->wisprheader.sent));
//   daemon_log(LOG_INFO, "CRAFTED GRE KEY %08x", newkey);
//   tmp->wisprheader.sent += 1;

//   /* Modify the GRE KEY and send the packet */
//   send_packet_withkey(buf, wispri->payload_length, wisprs->sendsd, htonl(newkey));

//   /* Check if we've sent the max number of packets for a group */
//   if (tmp->wisprheader.sent >= tmp->wisprheader.num_pkts - 1) {
//     daemon_log(LOG_INFO,
//                "BITMASK FULLY EXHAUSTED - SEND PARITY, ZERO OUT PARITY, "
//                "BITMASK, AND INCREMENT INDEX");
//     /* set up the GRE key */
//     newkey = tmp->wisprheader.mode << 28;
//     newkey = newkey | tmp->wisprheader.index << 16;
//     newkey = newkey | (tmp->wisprheader.bitmask + (1 << tmp->wisprheader.sent));
//     daemon_log(LOG_INFO, "CRAFTED FINAL GRE KEY %08x", newkey);
//     send_paritypacket_withkey(tmp->wisprheader.paritypkt,
//                               tmp->wisprheader.payload_length, wisprs->sendsd,
//                               htonl(newkey));
//     tmp->wisprheader.index += 1;
//     tmp->wisprheader.sent = 0;
//     memset(tmp->wisprheader.paritypkt, 0, PACKET_SIZE * sizeof(uint8_t));
//   }
// }

void init_wisprtables(void) {
  daemon_log(LOG_INFO, __func__);

  /* Initialize WISPR hash tables */
  // wisprmirt = NULL;
  // wisprpart = NULL;

  wispringress = NULL;
  hostIp = NULL;
  wisprmirt = NULL;
  // daemon_log(LOG_INFO, __func__);
}

void destroy_wisprtables(void) {
  //daemon_log(LOG_INFO, __func__);
  // wisprmirror_ht *session, *tmp;

  // HASH_ITER(hh, wisprmirt, session, tmp) {
  //   HASH_DEL(wisprmirt, session); /* delete it (session advances to next) */
  //   free(session);               /* free it */
  // }

  // wisprparity_ht *r5session, *r5tmp;

  // HASH_ITER(hh, wisprpart, r5session, r5tmp) {
  //   HASH_DEL(wisprpart, r5session); /* delete it (session advances to next) */
  //   free(r5session);               /* free it */
  // }

  wispringress_ht *ringress, *ingresstmp;

  HASH_ITER(hh, wispringress, ringress, ingresstmp) {
    HASH_DEL(wispringress, ringress); /* delete it (session advances to next) */
    free(ringress);                  /* free it */
  }

  host_IP *rSrcIp, *srcIptmp;

  HASH_ITER(hh, hostIp, rSrcIp, srcIptmp) {
    HASH_DEL(hostIp, rSrcIp); /* delete it (session advances to next) */
    free(rSrcIp);                  /* free it */
  }

  wisprmirror_ht *rMirrio, *sMirrortmp;

  HASH_ITER(hh, wisprmirt, rMirrio, sMirrortmp) {
    HASH_DEL(wisprmirt, rMirrio); /* delete it (session advances to next) */
    free(rMirrio);                  /* free it */
  }

  //daemon_log(LOG_INFO, __func__);
}

void add_mirrorsession(wispr_key_t key) {
  wisprmirror_ht *new;

  HASH_FIND(hh, wisprmirt, &key, sizeof(wispr_key_t), new);
  // HASH_FIND_INT(wispr1t, &key, new); /* id already in the hash? */
  if (new == NULL) {
    //daemon_log(LOG_INFO, "MIRROR TABLE DOESNT EXIST");
    new = (wisprmirror_ht *)malloc(sizeof(wisprmirror_ht));
    memset(new, 0, sizeof(wisprmirror_ht));
    new->key = key;
    HASH_ADD(hh, wisprmirt, key, sizeof(wispr_key_t), new);
    // HASH_ADD_INT(wispr1t, key, new); /* id: name of key field */
   // daemon_log(LOG_INFO, "MIRROR TABLE CREATED");
  }// else
   // daemon_log(LOG_INFO, "MIRROR TABLE ALREADY EXISTS");
}

// void add_paritysession(wispr_key_t key) {
//   wisprparity_ht *new;

//   HASH_FIND(hh, wisprpart, &key, sizeof(wispr_key_t), new);
//   // HASH_FIND_INT(wisprpart, &key, new); /* id already in the hash? */
//   if (new == NULL) {
//     daemon_log(LOG_INFO, "PARITY TABLE DOESNT EXIST");
//     new = (wisprparity_ht *)malloc(sizeof(wisprparity_ht));
//     memset(new, 0, sizeof(wisprparity_ht));
//     new->key = key;
//     HASH_ADD(hh, wisprpart, key, sizeof(wispr_key_t), new);
//     // HASH_ADD_INT(wispr1t, key, new); /* id: name of key field */
//     daemon_log(LOG_INFO, "PARITY TABLE CREATED");
//   } else
//     daemon_log(LOG_INFO, "PARITY TABLE ALREADY EXISTS");
// }

wispringress_ht_vlan* get_or_add_ingresssession(wispr_key_t key) {
  // daemon_log(LOG_INFO, __func__);
  wispringress_ht_vlan *new;

  HASH_FIND(hh, wispringress, &key, sizeof(wispr_key_t), new);
  if (new == NULL) {
    // daemon_log(LOG_INFO, "WISPR INGRESS SESSION DOESNT EXIST");
    new = (wispringress_ht_vlan *)malloc(sizeof(wispringress_ht_vlan));
    memset(new, 0, sizeof(wispringress_ht_vlan));
    new->key = key;
    HASH_ADD(hh, wispringress, key, sizeof(wispr_key_t), new);
    // HASH_ADD_INT(wispr1t, key, new); /* id: name of key field */
    // daemon_log(LOG_INFO, "INGRESS SESSION CREATED");
  } //else
    // daemon_log(LOG_INFO, "INGRESS SESSION ALREADY EXISTS");

  // daemon_log(LOG_INFO, __func__);
  return new;
}

wisprmirror_ht * get_or_add_mirror_session(wispr_key_t key) {
  // daemon_log(LOG_INFO, __func__);
  wisprmirror_ht *new;

  HASH_FIND(hh, wisprmirt, &key, sizeof(wispr_key_t), new);
  if (new == NULL) {
    //daemon_log(LOG_INFO, "MIRROR INGRESS SESSION DOESNT EXIST");
    new = (wisprmirror_ht *) malloc(sizeof(wisprmirror_ht));
    memset(new, 0, sizeof(wisprmirror_ht));
    new->key = key;
    HASH_ADD(hh, wisprmirt, key, sizeof(wispr_key_t), new);
    // HASH_ADD_INT(wispr1t, key, new); /* id: name of key field */
    //daemon_log(LOG_INFO, "MIRROR SESSION CREATED");
  } //else
    // daemon_log(LOG_INFO, "MIRROR SESSION ALREADY EXISTS");

  // daemon_log(LOG_INFO, __func__);
  return new;
}

int check_dst_ip_host(uint32_t key) {
  //daemon_log(LOG_INFO, __func__);
  host_IP *new;
  int returnValue = 0;

  HASH_FIND(hh, hostIp, &key, sizeof(uint32_t), new);
  if (new == NULL)
    returnValue = -1;
  else
    returnValue = 1;

  //daemon_log(LOG_INFO, __func__);
  return returnValue;
}

void add_Src_ip_host(uint32_t key) {
  //daemon_log(LOG_INFO, __func__);
  host_IP *new;

  host_IP *s;
  for(s=hostIp; s != NULL; s=(host_IP*)(s->hh.next)) {
      char source1[INET6_ADDRSTRLEN];
      struct sockaddr_storage src_addr;
      ((struct sockaddr_in *)&src_addr)->sin_addr.s_addr = s->src_ip;
      inet_ntop(AF_INET, &((struct sockaddr_in *)&src_addr)->sin_addr, source1,
                sizeof source1);

    //  daemon_log(LOG_INFO,"Source Keys:  %s : %d ", source1, s->src_ip);
  }

  HASH_FIND(hh, hostIp, &key, sizeof(uint32_t), new);
  if (new == NULL) {
   // daemon_log(LOG_INFO, "SOURCE IP DOESNT EXIST");
    new = (host_IP *) malloc(sizeof(host_IP));
    memset(new, 0, sizeof(host_IP));
    new->src_ip = key;
    HASH_ADD(hh, hostIp, src_ip, sizeof(uint32_t), new);
    // HASH_ADD_INT(wispr1t, key, new); /* id: name of key field */
   // daemon_log(LOG_INFO, "SOURCE IP SESSION SESSION CREATED");
  }// else
    //daemon_log(LOG_INFO, "SOURCE IP ALREADY EXISTS");

  //daemon_log(LOG_INFO, __func__);
}

// void xor_packetdata(wisprparity_ht *ht, struct wispr_packet *wisprp) {
//   uint32_t i;
//   uint16_t index;
//   index = wisprp->wisprh.index;

//   daemon_log(LOG_INFO, "XORING PACKETS");
//      for (i = 0; i < PACKET_SIZE; i += 8) {
//        daemon_log(
//            LOG_INFO, "NEW PACKET PAYLOAD:
//            %02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X",
//            wisprp->payload[i], wisprp->payload[i + 1], wisprp->payload[i + 2],
//            wisprp->payload[i + 3], wisprp->payload[i + 4], wisprp->payload[i +
//            5],
//            wisprp->payload[i + 6], wisprp->payload[i + 7]);
//      }
//      for (i = 0; i < PACKET_SIZE; i += 8) {
//        daemon_log(
//            LOG_INFO, "CONTAINER PACKET OLD VALUE:
//            %02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X",
//            ht->paritytab.table[index].pkt[i],
//            ht->paritytab.table[index].pkt[i + 1],
//            ht->paritytab.table[index].pkt[i + 2],
//            ht->paritytab.table[index].pkt[i + 3],
//            ht->paritytab.table[index].pkt[i + 4],
//            ht->paritytab.table[index].pkt[i + 5],
//            ht->paritytab.table[index].pkt[i + 6],
//            ht->paritytab.table[index].pkt[i + 7]);
//      }
//   for (i = 0; i < wisprp->payload_length; i++)
//     ht->paritytab.table[index].pkt[i] =
//         ht->paritytab.table[index].pkt[i] ^ wisprp->payload[i];
//   //    for (i = 0; i < PACKET_SIZE; i += 8) {
//   //      daemon_log(
//   //          LOG_INFO, "CONTAINER PACKET NEW VALUE:
//   //          %02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X",
//   //          ht->paritytab.table[index].pkt[i],
//   //          ht->paritytab.table[index].pkt[i + 1],
//   //          ht->paritytab.table[index].pkt[i + 2],
//   //          ht->paritytab.table[index].pkt[i + 3],
//   //          ht->paritytab.table[index].pkt[i + 4],
//   //          ht->paritytab.table[index].pkt[i + 5],
//   //          ht->paritytab.table[index].pkt[i + 6],
//   //          ht->paritytab.table[index].pkt[i + 7]);
//   //    }
// }

double time_diff(struct timeval x, struct timeval y) {
  //daemon_log(LOG_INFO, __func__);
  double x_ms, y_ms, diff;

  x_ms = (double)x.tv_sec * 1000000 + (double)x.tv_usec;
  y_ms = (double)y.tv_sec * 1000000 + (double)y.tv_usec;

  diff = (double)y_ms - (double)x_ms;

  return diff;
 // daemon_log(LOG_INFO, __func__);
}

char *int2bin(unsigned n, char *buf)
{
    #define BITS (sizeof(n) * CHAR_BIT)

    static char static_buf[BITS + 1];
    int i;

    if (buf == NULL)
        buf = static_buf;

    for (i = BITS - 1; i >= 0; --i) {
        buf[i] = (n & 1) ? '1' : '0';
        n >>= 1;
    }

    buf[BITS] = '\0';
    return buf;

    #undef BITS
}

// int parity_fulfilled(uint16_t bitmask) {
//    Check to see if there is only one bit not set - meaning we can
//    * reconstruct a parity packet 
//   uint16_t b = bitmask;
//   return b && !(b & (b - 1));
// }

// int send_packet_withkey(uint8_t *buf, ssize_t numbytes, uint16_t sendsd,
//                         wispringress_ht * key) {

// unsigned short checksum(unsigned short* buff, int _16bitword)
// {
//     unsigned long sum;
//     for(sum=0;_16bitword>0;_16bitword--)
//     sum+=htons(*(buff)++);
//     sum = ((sum >> 16) + (sum & 0xFFFF));
//     sum += (sum>>16);
//     return (unsigned short)(~sum);
// }

unsigned short checksum(void *b, int len)
{ unsigned short *buf = b;
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

int send_packet_withkey(uint8_t *buf, ssize_t numbytes, uint16_t sendsd, wispringress_ht_vlan * key) {
  daemon_log(LOG_INFO, "IN %s", __func__);

  // daemon_log(LOG_INFO, "SENDING MANIPULATED KEY WISPR PACKET");

  uint8_t iplength = 0;
  uint16_t offset = 0;
  uint16_t sentbytes = 0;
  int i;
  uint8_t *rem_buf = NULL;

  char str[numbytes + sizeof(wispr_ingress_vlan) + sizeof(wispr_ingress_vlan)];
  daemon_log(LOG_INFO, "Bytes Received: %d", numbytes);
  packetCound += 1;
  daemon_log(LOG_INFO, "SEND PACKETS: %d", packetCound);

  // for (i = 0; i < (int)(numbytes); i += 16) {
  //   daemon_log(LOG_INFO, "PAYLOAD: %02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X",
  //              buf[i], buf[i + 1], buf[i + 2], buf[i + 3], buf[i + 4],
  //              buf[i + 5], buf[i + 6], buf[i + 7], buf[i + 8], buf[i + 9], buf[i + 10], buf[i + 11], buf[i + 12],
  //              buf[i + 13], buf[i + 14], buf[i + 15]);
  // }
  
  // struct ip *iph = (struct ip *)(buf + sizeof(struct ethhdr));
  // struct iphdr *iph = (struct iphdr *)(buf + sizeof(struct ethhdr));
  // iplength = iph->ip_hl;
  // iph->ip_p = (u_char) 47;
  // iph->protocol = (u_char) 47;
  
  struct ethhdr* eth = (struct ethhdr*) buf;
  uint16_t previous_protocol = ntohs(eth->h_proto);
  eth->h_proto = htons(ETH_P_8021Q); //means next header will be IP header

  struct ip *iph = (struct ip *)(buf + sizeof(struct ethhdr));
  int iplHength = iph->ip_hl * 4;
  unsigned int ip_payload_length = iph->ip_len;

  uint8_t *ipHeader = (uint8_t *) malloc(iplHength);
  // memcpy(rem_buf, buf + sizeof(struct ether_header), rem_len);
  memset(ipHeader, 0, iplHength);
  memcpy(ipHeader, buf + sizeof(struct ether_header), iplHength);


  int packet_number = key->vlan_id & 0x000F;
  int group_id = (key->vlan_id & 0x00F0) >> 4;
  int wispr_mode = (key->vlan_id & 0x0F00) >> 8;

  daemon_log(LOG_INFO, "SEND: %4X %x %x %x ", key->vlan_id, wispr_mode, group_id, packet_number );

  int myPacketSize = (int) numbytes + sizeof(wispr_ingress_vlan) + sizeof(wispr_ingress_vlan);
  // daemon_log(LOG_INFO, "Size %d, MTU %d", myPacketSize , mtu);

  // if (myPacketSize <= mtu + 20) 
  // {
    int rem_len = numbytes - sizeof(struct ether_header);
    rem_buf = (uint8_t * ) malloc(rem_len);
    memcpy(rem_buf, buf + sizeof(struct ether_header), rem_len);

    wispr_ingress_vlan *vlan = (wispr_ingress_vlan *)(buf + sizeof(struct ether_header));
    vlan->vlan_id =  htons(key->vlan_id);
    vlan->next_protocol = htons(ETH_P_8021Q);

    wispr_ingress_vlan *vlan2 = (wispr_ingress_vlan *)(buf + sizeof(struct ether_header)+ sizeof(wispr_ingress_vlan));
    vlan2->vlan_id =  htons(key->vlan_id)  ;
    vlan2->next_protocol = htons(previous_protocol);

    memcpy(buf + sizeof(struct ether_header) + sizeof(wispr_ingress_vlan) + sizeof(wispr_ingress_vlan), rem_buf, rem_len);
    sentbytes = sendto(sendsd, buf , numbytes + sizeof(wispr_ingress_vlan) + sizeof(wispr_ingress_vlan), 0, NULL, NULL);
    vlan->vlan_id =  htons(key->vlan_id | 0x0100);
    vlan2->vlan_id =  htons(key->vlan_id | 0x0100);
    sentbytes = sendto(sendsd,   buf , numbytes + sizeof(wispr_ingress_vlan) + sizeof(wispr_ingress_vlan), 0, NULL, NULL);    
  // } else {
    
  //   // fragmaentation
  //   int ipFragmentbit = (ntohs(iph->ip_off) & 0x4000) >> 14;

  //   daemon_log(LOG_INFO, "brra packet, ipFragmentbit: %d, %04x, %04x, %d, %d, %d, %d", ipFragmentbit, iph->ip_off, ntohs(iph->ip_off), (ntohs(iph->ip_off) & 0xFF00) >> 15, (ntohs(iph->ip_off) & 0xFF00) >> 14, (ntohs(iph->ip_off) & 0xFF00) >> 13, (ntohs(iph->ip_off) & 0xFF00) >> 12);
  //   if (  ipFragmentbit == 1 )
  //   {
  //       struct icmphdr *uh = (struct icmphdr *)(buf + sizeof(struct iphdr) + sizeof(struct ethhdr)); 
  //       memset(uh,0, sizeof(struct icmphdr));
  //       uh->un.frag.mtu = htons(1480);
  //       uh->type = ICMP_DEST_UNREACH;
  //       uh->code = ICMP_FRAG_NEEDED;
  //       uh->checksum = 0;
  //       uh->checksum = checksum((uint8_t *  )buf + sizeof(struct iphdr) + sizeof(struct ethhdr), sizeof(struct icmphdr));
      
  //       uint32_t tmp = (*iph).ip_src.s_addr;
  //       (*iph).ip_src.s_addr = (*iph).ip_dst.s_addr;
  //       (*iph).ip_dst.s_addr = tmp;      

  //       iph->ip_sum = 0;
  //       iph->ip_p = 1;
  //       iph->ip_len = htons(20 + sizeof(struct icmphdr));
  //       iph->ip_sum = checksum((unsigned short*)(buf + sizeof (struct ethhdr) ), (iplHength/2));
        
  //       char tempEth[6];
  //       tempEth[0] == eth->h_dest[0];
  //       tempEth[1] == eth->h_dest[1];
  //       tempEth[2] == eth->h_dest[2];
  //       tempEth[3] == eth->h_dest[3];
  //       tempEth[4] == eth->h_dest[4];
  //       tempEth[5] == eth->h_dest[5];

  //       eth->h_dest[0] == eth->h_source[0];
  //       eth->h_dest[1] == eth->h_source[1];
  //       eth->h_dest[2] == eth->h_source[2];
  //       eth->h_dest[3] == eth->h_source[3];
  //       eth->h_dest[4] == eth->h_source[4];
  //       eth->h_dest[5] == eth->h_source[5];

  //       eth->h_source[0] == tempEth[0];
  //       eth->h_source[1] == tempEth[1];
  //       eth->h_source[2] == tempEth[2];
  //       eth->h_source[3] == tempEth[3];
  //       eth->h_source[4] == tempEth[4];
  //       eth->h_source[5] == tempEth[5];

  //       eth->h_proto = htons(ETH_P_IP);

  //       sentbytes = sendto(sendsd, buf , sizeof(struct iphdr) + sizeof(struct ethhdr) + sizeof(struct icmphdr), 0, NULL, NULL);    

  //   } else {
  //     int original_packetSize = numbytes - iplHength - sizeof(struct ethhdr);
  //     unsigned int totlen;
  //     unsigned int ip_frag_size; /* fragment size */
  //     unsigned int last_frag_extra; /* extra bytes possible in the last frag */
  //     unsigned int ip_payload = ntohs(ip_payload_length);//iph->ip_len;
      
  //     unsigned int last_frag_offs;
  //     int frg_no;

  //     int rem_len = numbytes - sizeof(struct ether_header) - iplHength;
  //     rem_buf = (uint8_t * ) malloc(rem_len);
  //     memcpy(rem_buf, buf + sizeof(struct ether_header) + iplHength, rem_len);

  //     // memset(buf + sizeof(struct iphdr) + sizeof(struct ether_header), 0, rem_len);
  //     wispr_ingress_vlan *vlan = (wispr_ingress_vlan *)(buf + sizeof(struct ether_header));
  //     vlan->vlan_id =  htons(key->vlan_id);
  //     vlan->next_protocol = htons(ETH_P_8021Q);

  //     // int packet_number = vlan->vlan_id & 0x0001;
  //     // int group_id = (vlan->vlan_id & 0x0010) >> 4;
  //     // daemon_log(LOG_INFO, " SENDING: Vlan Header %04x, group ID :%d, PacketNumber %d", vlan->vlan_id, group_id, packet_number);
      
  //     wispr_ingress_vlan *vlan2 = (wispr_ingress_vlan *)(buf + sizeof(struct ether_header)+ sizeof(wispr_ingress_vlan));
  //     vlan2->vlan_id =  htons(key->vlan_id)  ;
  //     vlan2->next_protocol = htons(previous_protocol);

      
  //     memcpy(buf + sizeof(struct ether_header) + sizeof(wispr_ingress_vlan) + sizeof(wispr_ingress_vlan), ipHeader, iplHength);
  //     iph = (struct ip *)(buf + sizeof(struct ether_header) + sizeof(wispr_ingress_vlan) + sizeof(wispr_ingress_vlan));
  //     int allHeaderSize = (int) sizeof(struct ether_header) + sizeof(wispr_ingress_vlan) + sizeof(wispr_ingress_vlan) + iplHength;
      
  //     ip_frag_size = (mtu - iplHength - 16) & (~7);
  //     last_frag_extra = (mtu - iplHength - 16) & 7; /* rest */
      
  //     frg_no = ip_payload / ip_frag_size +
  //          ((ip_payload % ip_frag_size) > last_frag_extra);
  //     last_frag_offs = (frg_no - 1) * ip_frag_size;
  //     //last_frag_start = buf + last_frag_offs ;
  //     // iph->ip_id = htons((rand() % 65534) + 1);
  //     iph->ip_len = htons(ip_frag_size);
  //     iph->ip_off = htons(0x2000); /* set MF */
  //     int ipOfSet = 0;
  //     int counter = 0;
  //     daemon_log(LOG_INFO, "ip_frag_size %d, frg_no %d, ip length %d , id %d", ip_frag_size , frg_no, ip_payload , ntohs(iph->ip_id));
  //     uint8_t * buffer = buf + sizeof(struct ether_header) + sizeof(wispr_ingress_vlan) + sizeof(wispr_ingress_vlan) + iplHength;

  //     while (counter < frg_no - 1) {
  //       iph->ip_off = htons((counter * ip_frag_size) | 0x2000);
  //       iph->ip_sum = 0;
  //       iph->ip_sum = checksum((unsigned short*)(buf + sizeof (struct ethhdr) + sizeof(wispr_ingress_vlan)+ sizeof(wispr_ingress_vlan)), (iplHength/2));
  //       memcpy(buffer , rem_buf + ipOfSet, ip_frag_size);
  //       sentbytes = sendto(sendsd, buf , allHeaderSize+ip_frag_size, 0, NULL, NULL);
  //       counter+=1;
  //       ipOfSet += ip_frag_size;
  //       daemon_log(LOG_INFO, "%d %04x %d, %d", counter,counter * ip_frag_size | 0x2000, ipOfSet, sentbytes);
  //     }

  //     iph->ip_off = htons((counter * ip_frag_size)  & (~(0x2000)) );
  //     iph->ip_sum = 0;
  //     iph->ip_sum = checksum((unsigned short*)(buf + sizeof (struct ethhdr) + sizeof(wispr_ingress_vlan)+ sizeof(wispr_ingress_vlan)), (iplHength/2));
  //     memcpy(buffer, rem_buf + ipOfSet, rem_len - ipOfSet);
  //     sentbytes = sendto(sendsd, buf , allHeaderSize + (rem_len - ipOfSet), 0, NULL, NULL);
  //     daemon_log(LOG_INFO, "LAST SENT BYTES %d", sentbytes);
  //   }
  // }

    // vlan->vlan_id =  htons(key->vlan_id | 0x0100);
    // vlan2->vlan_id =  htons(key->vlan_id | 0x0100);
    // sentbytes = sendto(sendsd, buf , numbytes + sizeof(wispr_ingress_vlan) + sizeof(wispr_ingress_vlan), 0, NULL, NULL);    

  // if (numbytes > 1400)
  // {
  //   for (i = 0; i < (int)(numbytes + sizeof(wispr_ingress_vlan) + sizeof(wispr_ingress_vlan)); i += 8) {
  //   daemon_log(LOG_INFO,
  //            "MODIFIED PAYLOAD: %02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X",
  //            buf[i], buf[i + 1], buf[i + 2], buf[i + 3], buf[i + 4],
  //            buf[i + 5], buf[i + 6], buf[i + 7]);
  //   }
  //   daemon_log(LOG_INFO, "SENT I value: %d", i);
  // }

// sentbytes = sendto(sendsd, buf + sizeof(struct ethhdr)  , numbytes + sizeof(struct wispr_packet) , 0, (struct sockaddr *) &sin, sizeof(sin));
  free(rem_buf);
  free(ipHeader);
  // daemon_log(LOG_INFO, "SENT BYTES WISPR %d", sentbytes);
  if (sentbytes <= 0) {
    daemon_log(LOG_INFO, "FAILED WISPR PACKET %s", strerror(errno));
    // daemon_log(LOG_INFO, __func__);
    return -1;
  } else {
    daemon_log(LOG_INFO, "SUCCESS WISPR PACKET %d", sentbytes);
    // daemon_log(LOG_INFO, __func__);
    return 1;
  }


  // uint8_t *rem_buf;
  // int rem_len = numbytes - sizeof(struct ether_header);
  // rem_buf = (uint8_t * ) malloc(rem_len);
  // memcpy(rem_buf, buf + sizeof(struct ether_header), rem_len);

  // // memset(buf + sizeof(struct iphdr) + sizeof(struct ether_header), 0, rem_len);
  // wispr_ingress_vlan *vlan = (wispr_ingress_vlan *)(buf + sizeof(struct ether_header));
  // vlan->vlan_id =  htons(key->vlan_id);
  // vlan->next_protocol = htons(ETH_P_8021Q);

  // // int packet_number = vlan->vlan_id & 0x0001;
  // // int group_id = (vlan->vlan_id & 0x0010) >> 4;
  // // daemon_log(LOG_INFO, " SENDING: Vlan Header %04x, group ID :%d, PacketNumber %d", vlan->vlan_id, group_id, packet_number);
  
  // wispr_ingress_vlan *vlan2 = (wispr_ingress_vlan *)(buf + sizeof(struct ether_header)+ sizeof(wispr_ingress_vlan));
  // vlan2->vlan_id =  htons(key->vlan_id)  ;
  // vlan2->next_protocol = htons(previous_protocol);

  // unsigned short *buffer = (unsigned short*)&(vlan->vlan_id);
  // daemon_log(LOG_INFO, "0x%x -- %d\n", buffer[0], buffer[0]) ;
  // daemon_log(LOG_INFO,"%s\n", int2bin(buffer[0], NULL));

  // daemon_log(LOG_INFO,"GRE THINGS: %d %d %d %d %d\n", gre->header.gre_flags, gre->header.gre_proto, gre->key.mode, gre->key.index, gre->key.bitmask);

  // iph->tot_len = htons(ntohs(iph->tot_len) + sizeof(struct wispr_packet));
  // iph->ip_len = htons(numbytes + sizeof(struct wispr_packet) - sizeof(struct ethhdr));
  // iph->ip_p = 47;

  //   for (i = 0; i <= (int)(numbytes + sizeof(struct wispr_packet) ); i += 16) {
  //     daemon_log(LOG_INFO, "PAYLOAD2: %02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X",
  //                buf[i], buf[i + 1], buf[i + 2], buf[i + 3], buf[i + 4],
  //                buf[i + 5], buf[i + 6], buf[i + 7], buf[i + 8], buf[i + 9], buf[i + 10], buf[i + 11], buf[i + 12],
  //                buf[i + 13], buf[i + 14], buf[i + 15]);
  //   }

  // daemon_log(LOG_INFO, "\n");
  // daemon_log(LOG_INFO, "SIZES: ethhdr: %d iphdr: %d greh: %d offset: %d, TL: %d",
  //            sizeof(struct ethhdr), (iplength * 4),
  //            sizeof(struct wispr_packet), numbytes, (int)(numbytes + sizeof(struct wispr_packet)));


  // /* Cast the buffer into an IP header to get the destination address */
  // struct ip *iph = (struct ip *)(buf + sizeof(struct ethhdr));
  // iplength = iph->ip_hl;
  // struct gre_base_header *greh =
  //     (struct gre_base_header *)(buf + sizeof(struct ethhdr) + (iplength * 4));
  // offset =
  //     sizeof(struct ethhdr) + (iplength * 4) + sizeof(struct gre_base_header);

  // // Key must be present for WISPR functionality
  // if (ntohs(greh->gre_flags) & GRE_FLAGS_CP)
  //   offset += GRE_HEADER_SECTION;


  // gre_base_header * gre = buf + sizeof(ether_header) + sizeof(iphdr)
  // memcpy(buf + offset, &key, sizeof(uint32_t));

  // for (i = 0; i < (int)(numbytes + sizeof(struct ethhdr)); i += 8) {
  //   daemon_log(LOG_INFO,
  //              "MODIFIED PAYLOAD: %02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X",
  //              buf[i], buf[i + 1], buf[i + 2], buf[i + 3], buf[i + 4],
  //              buf[i + 5], buf[i + 6], buf[i + 7]);
  // }
  // if (ntohs(greh->gre_flags) & GRE_FLAGS_KP)
  //   offset += GRE_HEADER_SECTION;

  /* The kernel is going to prepare layer 2 information (ethernet frame header)
   * for us. For that, we need to specify a destination for the kernel in order
   * for it to decide where to send the raw datagram. We fill in a struct 
   * in_addr with the desired destination IP address, and pass this structure
   * to the sendto() function.*/
  // struct sockaddr_in sin;
  // memset(&sin, 0, sizeof(struct sockaddr_in));
  // sin.sin_family = AF_INET;

  // //must revert this change
  // //sin.sin_addr.s_addr = (*iph).ip_src.s_addr;

  // struct  in_addr tmp_in_addr = iph->ip_src;
  // iph->ip_src = iph->ip_dst;
  // iph->ip_dst = tmp_in_addr;

  // sin.sin_addr.s_addr = (*iph).ip_src.s_addr;
  // sin.sin_addr.s_addr = (*iph).ip_dst.s_addr;
  
  // sentbytes = send_packet(buf, numbytes + sizeof(struct wispr_packet), sendsd);

}

int send_paritypacket(uint8_t *buf,uint8_t *parity, ssize_t numbytes, uint16_t sendsd) {
  daemon_log(LOG_INFO, "IN %s", __func__);

  daemon_log(LOG_INFO, "SENDING PARITY WISPR PACKET");
  uint8_t iplength = 0;
  uint16_t offset = 0;
  uint16_t sentbytes = 0;
  int i;
  char str[numbytes + sizeof(wispr_ingress_vlan) + sizeof(wispr_ingress_vlan)];
 
  struct ethhdr* eth = (struct ethhdr*) buf;
  uint16_t previous_protocol = ntohs(eth->h_proto);
  eth->h_proto = htons(ETH_P_8021Q); //means next header will be IP header

  uint8_t *rem_buf;
  int rem_len = numbytes - sizeof(struct ether_header);
  rem_buf = (uint8_t * ) malloc(rem_len);
  memcpy(rem_buf, buf + sizeof(struct ether_header), rem_len);

  wispr_ingress_vlan *vlan = (wispr_ingress_vlan *)(buf + sizeof(struct ether_header));
  // vlan->vlan_id =  htons(key->vlan_id);
  vlan->next_protocol = htons(ETH_P_8021Q);
  
  wispr_ingress_vlan *vlan2 = (wispr_ingress_vlan *)(buf + sizeof(struct ether_header)+ sizeof(wispr_ingress_vlan));
  // vlan2->vlan_id =  htons(key->vlan_id)  ;
  vlan2->next_protocol = htons(previous_protocol);
  memcpy(buf + sizeof(struct ether_header) + sizeof(wispr_ingress_vlan) + sizeof(wispr_ingress_vlan), rem_buf, rem_len);
  
  sentbytes = sendto(sendsd, buf , numbytes + sizeof(wispr_ingress_vlan) + sizeof(wispr_ingress_vlan), 0, NULL, NULL);

  // mirrio Packet
  // vlan->vlan_id =  htons(key->vlan_id | 0x0100);
  // vlan2->vlan_id =  htons(key->vlan_id | 0x0100);
  sentbytes = sendto(sendsd, buf , numbytes + sizeof(wispr_ingress_vlan) + sizeof(wispr_ingress_vlan), 0, NULL, NULL);

  // sentbytes = sendto(sendsd, buf + sizeof(struct ethhdr)  , numbytes + sizeof(struct wispr_packet) , 0, (struct sockaddr *) &sin, sizeof(sin));
  free(rem_buf);
  daemon_log(LOG_INFO, "SENT BYTES WISPR %d", sentbytes);
  if (sentbytes <= 0) {
    daemon_log(LOG_INFO, "FAILED WISPR PACKET %s", strerror(errno));
    daemon_log(LOG_INFO, __func__);
    return -1;
  } else {
    daemon_log(LOG_INFO, "SUCCESS WISPR PACKET");
    daemon_log(LOG_INFO, __func__);
    return 1;
  }
}
// int send_paritypacket_withkey(uint8_t *buf, ssize_t numbytes, uint16_t sendsd,
//                               uint32_t key) {
//   daemon_log(LOG_INFO, "SENDING PARITY PACKET");
//   uint8_t iplength = 0;
//   uint16_t offset = 0;
//   int i;
//   struct sockaddr_in sin;

//   for (i = 0; i < (int)(numbytes); i += 8) {
//     daemon_log(LOG_INFO, "PAYLOAD: %02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X",
//                buf[i], buf[i + 1], buf[i + 2], buf[i + 3], buf[i + 4],
//                buf[i + 5], buf[i + 6], buf[i + 7]);
//   }

//   /* Cast the buffer into an IP header to get the destination address */
//   struct ip *iph = (struct ip *)(buf);
//   iplength = iph->ip_hl;
//   struct gre_base_header *greh =
//       (struct gre_base_header *)(buf + (iplength * 4));
//   offset = (iplength * 4) + sizeof(struct gre_base_header);

//   // Key must be present for WISPR functionality
//   if (ntohs(greh->gre_flags) & GRE_FLAGS_CP)
//     offset += GRE_HEADER_SECTION;
//   daemon_log(LOG_INFO, "SIZES: iphdr: %d greh: %d offset: %d", (iplength * 4),
//              sizeof(struct gre_base_header), offset);

//   memcpy(buf + offset, &key, sizeof(uint32_t));

//   for (i = 0; i < (int)(numbytes); i += 8) {
//     daemon_log(LOG_INFO,
//                "MODIFIED PAYLOAD: %02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X",
//                buf[i], buf[i + 1], buf[i + 2], buf[i + 3], buf[i + 4],
//                buf[i + 5], buf[i + 6], buf[i + 7]);
//   }
//   if (ntohs(greh->gre_flags) & GRE_FLAGS_KP)
//     offset += GRE_HEADER_SECTION;
//   /* The kernel is going to prepare layer 2 information (ethernet frame header)
//    * for us. For that, we need to specify a destination for the kernel in order
//    * for it to decide where to send the raw datagram. We fill in a struct
//    * in_addr with the desired destination IP address, and pass this structure
//    * to the sendto() function.*/
//   memset(&sin, 0, sizeof(struct sockaddr_in));
//   sin.sin_family = AF_INET;
//   sin.sin_addr.s_addr = (*iph).ip_dst.s_addr;

//   if (sendto(sendsd, buf, numbytes, 0, (struct sockaddr *)&sin,
//              sizeof(struct sockaddr)) < 0) {
//     daemon_log(LOG_INFO, "FAILED WISPR PACKET %s", strerror(errno));
//     return -1;
//   } else {
//     daemon_log(LOG_INFO, "SUCCESS WISPR PACKET %d");
//     return 1;
//   }
// }
#endif // WISPR_NET_H_