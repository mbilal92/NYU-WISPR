#pragma once
#ifndef WISPR_NET_H  //Include guard 
#define WISPR_NET_H

#include <net/if.h>
#include <stdint.h>
#include <arpa/inet.h>

#define PACKET_SIZE 1500
#define ENTRIES 4096

struct wisprsock {
  uint16_t wisprsd;
  uint16_t sendsd;
  uint16_t rawsd;
  uint8_t ifName[IFNAMSIZ];
  uint8_t ifName2[IFNAMSIZ];
  uint8_t gateWayIporMAC[INET_ADDRSTRLEN];
};

struct wisprsock *init_wispr_sockets(char *interface, char *interface2, char *gateWayIp);

void process_newpkt(struct wisprsock *);
void handle_ipv6pkt(uint8_t *, ssize_t, struct wisprsock *);
void handle_ipv4pkt(uint8_t *, ssize_t, struct wisprsock *);
int send_packet(uint8_t *, ssize_t, uint16_t);
int send_packet_raw(uint8_t *, ssize_t, uint16_t);
void destroy_wisprsock(struct wisprsock *);

#endif // WISPR_NET_H_