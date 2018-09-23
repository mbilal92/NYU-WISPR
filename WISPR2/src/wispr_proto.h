#pragma once
#ifndef WISPR_PROTO_H  //Include guard 
#define WISPR_PROTO_H

#include "uthash.h"
#include <net/if.h>
#include <stdint.h>
#include <sys/time.h>

#define PACKET_SIZE 1500
#define WISPR5ENTRIES 4096
#define WISPR1ENTRIES 65536

// struct wispr_header {
//   uint8_t mode : 4;
//   uint16_t index : 12;
//   uint16_t bitmask : 16;
//   uint8_t num_pkts : 4;
// };

typedef struct {
  uint16_t index : 12;
  uint8_t mode : 4;
  uint16_t bitmask : 16;
  // uint8_t num_pkts : 4;
  // uint8_t sent : 4;
  // uint8_t paritypkt[PACKET_SIZE];
  // ssize_t payload_length;
} wispr_ingress_key;

typedef struct {
  uint16_t vlan_id;
  uint16_t next_protocol;
} wispr_ingress_vlan;

struct gre_base_header {
  /* GRE header according to RFC 2784 and RFC 2890 */

  #define GRE_FLAGS_CP 0x8000 /* checksum present */
  #define GRE_FLAGS_RP 0x4000 /* routing present */
  #define GRE_FLAGS_KP 0x2000 /* key present */
  #define GRE_FLAGS_SP 0x1000 /* sequence present */
  #define GRE_STRICT 0x0800
  #define GRE_REC 0x0700
  #define GRE_ACK 0x0080
  #define GRE_FLAGS_MASK (GRE_FLAGS_CP | GRE_FLAGS_KP | GRE_FLAGS_SP)
  #define ETHER_TYPE 0x0800
  uint16_t gre_flags;       /* GRE flags */
  uint16_t gre_proto; /* protocol type */
};

struct wispr_packet {
  struct gre_base_header header;
  wispr_ingress_key key;
};

// typedef struct {
//   uint16_t bitmask : 16;
//   uint8_t pkt[PACKET_SIZE];
//   struct timeval tv;
// } wispr_entry;

// struct wisprparity_table {
//   wispr_entry table[WISPR5ENTRIES];
// };

typedef struct {
  uint32_t src_ip;
  uint32_t dst_ip;
} wispr_key_t;

/* WISPR 4 & 5 HASH TABLE */
// typedef struct {
//   wispr_key_t key;
//   struct wisprparity_table paritytab;
//   UT_hash_handle hh;
// } wisprparity_ht;

// /* WISPR 1 HASH TABLE */
// typedef struct {
//   wispr_key_t key;
//   struct timeval tv[WISPR1ENTRIES];
//   UT_hash_handle hh;
// } wisprmirror_ht;

/* WISPR INGRESS SESSION TABLE */
typedef struct {
  wispr_key_t key;
  wispr_ingress_key wispr_key;
  UT_hash_handle hh;
} wispringress_ht;

typedef struct {
  wispr_key_t key;
  uint16_t vlan_id;
  UT_hash_handle hh;
} wispringress_ht_vlan;

/* WISPR INGRESS SESSION TABLE */
typedef struct {
  uint32_t src_ip;
  UT_hash_handle hh;
} host_IP;

void process_wispr0(uint8_t *, ssize_t , struct wisprsock *);
// void process_wispr1(struct wispr_packet *, struct wisprsock *);
// void process_parity(struct wispr_packet *, struct wisprsock *);
void process_ingress(uint8_t *buf,  ssize_t numbytes, struct wisprsock *wisprs, int mode);
void process_ingress0(uint8_t *,ssize_t numbytes, struct wisprsock *wisprs);
void process_ingress1(uint8_t *,ssize_t numbytes, struct wisprsock *wisprs);
// void process_ingressparity(uint8_t *, wispr_ingress_key *, struct wisprsock *);
void parse_wispr_pkt(uint8_t *buf, ssize_t numbytes, struct gre_base_header **greh, wispr_ingress_key **key);
      // void parse_wispr_pkt(uint8_t *, ssize_t, struct wispr_packet *);
// int parse_newingress_pkt(uint8_t *, ssize_t, wispr_ingress_key *);
// int parse_ingress_pkt(uint8_t *, ssize_t, wispr_ingress_key *);
void init_wisprtables(void);
int send_packet_withkey(uint8_t *, ssize_t, uint16_t, wispringress_ht *);
// int send_paritypacket_withkey(uint8_t *, ssize_t, uint16_t, uint32_t);
void destroy_wisprtables(void);
// void add_mirrorsession(wispr_key_t);
// void add_paritysession(wispr_key_t);
wispringress_ht * get_or_add_ingresssession(wispr_key_t);
// void xor_packetdata(wisprparity_ht *, struct wispr_packet *);
// int parity_fullfilled(uint16_t);
#endif // WISPR_PROTO_H_
