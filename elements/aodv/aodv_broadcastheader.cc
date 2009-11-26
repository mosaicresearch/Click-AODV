// Copyright (c) 2004 by the University of Antwerp
// All rights reserved.
//

// ALWAYS INCLUDE <click/config.h> FIRST
#include <click/config.h>
#include "aodv_broadcastheader.hh"
#include "click_aodv.hh"

CLICK_DECLS

Packet* AODVBroadcastHeader::setBroadcastHeader(Packet *packet, const IPAddress & myIP, int ttl){
	static const IPAddress destination("255.255.255.255"); // broadcast
	
	WritablePacket * writable = packet->push(sizeof(click_udp) + sizeof(click_ip));
	click_ip *ip = reinterpret_cast<click_ip *>(writable->data());
	click_udp *udp = reinterpret_cast<click_udp *>(ip + 1);
	// set up IP header
	ip->ip_v = 4;
	ip->ip_hl = sizeof(click_ip) >> 2;
	ip->ip_len = htons(writable->length());
	ip->ip_id = htons(1); // we may use a static ID as fragmentation of this packets gives problems anyway
	ip->ip_p = IP_PROTO_UDP;
	ip->ip_src = myIP.in_addr();
	ip->ip_dst = destination.in_addr();
	ip->ip_tos = 0;
	ip->ip_off = 0;
	ip->ip_ttl = ttl;
	ip->ip_sum = 0;
	#if HAVE_FAST_CHECKSUM && FAST_CHECKSUM_ALIGNED
	if (_aligned)
		ip->ip_sum = ip_fast_csum((unsigned char *)ip, sizeof(click_ip) >> 2);
	else
		ip->ip_sum = click_in_cksum((unsigned char *)ip, sizeof(click_ip));
	#elif HAVE_FAST_CHECKSUM
	ip->ip_sum = ip_fast_csum((unsigned char *)ip, sizeof(click_ip) >> 2);
	#else
	ip->ip_sum = click_in_cksum((unsigned char *)ip, sizeof(click_ip));
	#endif

	writable->set_dst_ip_anno(destination);
	writable->set_ip_header(ip, sizeof(click_ip));

	// set up UDP header
	udp->uh_sport = htons(AODV_PORT);
	udp->uh_dport = htons(AODV_PORT);
	uint16_t len = writable->length() - sizeof(click_ip);
	udp->uh_ulen = htons(len);
	udp->uh_sum = 0;
	unsigned csum = click_in_cksum((unsigned char *)udp, len);
	udp->uh_sum = click_in_cksum_pseudohdr(csum, ip, len);
	
	return  writable;
}


CLICK_ENDDECLS
ELEMENT_PROVIDES(AODVBroadcastHeader)
