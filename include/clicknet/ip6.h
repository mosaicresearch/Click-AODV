/* -*- mode: c; c-basic-offset: 4 -*- */
#ifndef CLICKNET_IP6_H
#define CLICKNET_IP6_H
#include <clicknet/ip.h>
#undef s6_addr
#undef s6_addr16
#undef s6_addr32
#undef s6_addr64

/*
 * <clicknet/ip6.h> -- our own definitions of IP6 headers
 * based on RFC 2460
 */

/* IPv6 address , same as from /usr/include/netinet/in.h  */
struct click_in6_addr {
    union {
	uint8_t		u6_addr8[16];
	uint16_t	u6_addr16[8];
	uint32_t	u6_addr32[4];
#ifdef HAVE_INT64_TYPES
	uint64_t	u6_addr64[2];
#endif
    } in6_u;
#define s6_addr in6_u.u6_addr8
#define s6_addr16 in6_u.u6_addr16
#define s6_addr32 in6_u.u6_addr32
#define s6_addr64 in6_u.u6_addr64
};

struct click_ip6 {
    union {
	uint32_t ip6_un1_flow;		/* 0-3   version, class, flow label */
	struct {
#if CLICK_BYTE_ORDER == CLICK_LITTLE_ENDIAN
	    unsigned ip6_un2_tc : 4;	/* 0     top of traffic class */
	    unsigned ip6_un2_v : 4;	/*       version == 6 */
#elif CLICK_BYTE_ORDER == CLICK_BIG_ENDIAN
	    unsigned ip6_un2_v : 4;	/* 0     version == 6 */
	    unsigned ip6_un2_tc : 4;	/*       top of traffic class */
#else
# error "unknown endianness!"
#endif
	    uint8_t ip6_un2_flow[3];
	} ip6_un2_vfc;
    } ip6_flun;
#define ip6_v				ip6_flun.ip6_un2_vfc.ip6_un2_v
#define ip6_tc				ip6_flun.ip6_un2_vfc.ip6_un2_tc
#define ip6_flow			ip6_flun.ip6_un1_flow
    uint16_t ip6_plen;			/* 4-5   payload length */
    uint8_t ip6_nxt;			/* 6     next header */
    uint8_t ip6_hlim;	     		/* 7     hop limit  */
    struct click_in6_addr ip6_src;	/* 8-23  source address */
    struct click_in6_addr ip6_dst;	/* 24-39 dest address */
};

#define IP6_FLOW_MASK			0x000FFFFFU
#define IP6_FLOW_SHIFT			0
#define IP6_CLASS_MASK			0x0FF00000U
#define IP6_CLASS_SHIFT			20
#define IP6_DSCP_MASK			0x0FC00000U
#define IP6_DSCP_SHIFT			22
#define IP6_V_MASK			0xF0000000U
#define IP6_V_SHIFT			28

CLICK_DECLS

uint16_t in6_fast_cksum(const struct click_in6_addr *saddr,
			const struct click_in6_addr *daddr,
			uint16_t len,
			uint8_t proto,
			uint16_t ori_csum,
			const unsigned char *addr,
			uint16_t len2);

uint16_t in6_cksum(const struct click_in6_addr *saddr,
		   const struct click_in6_addr *daddr,
		   uint16_t len,
		   uint8_t proto,
		   uint16_t ori_csum,
		   unsigned char *addr,
		   uint16_t len2);

CLICK_ENDDECLS
#endif
