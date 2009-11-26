#ifndef CLICK_AODV_HH
#define CLICK_AODV_HH

#include <click/ipaddress.hh>
#include <click/string.hh>
#include <clicknet/ip.h>
#include <clicknet/ether.h>
#include <clicknet/udp.h>

// AODV specific constants 
// as suggested by RFC3561

CLICK_DECLS

#define AODV_PORT 654

//Intervals in ms
#define AODV_HELLO_INTERVAL 1000
#define AODV_ALLOWED_HELLO_LOSS 2
#define AODV_ACTIVE_ROUTE_TIMEOUT 1000
#define AODV_NET_DIAMETER 35
#define AODV_NODE_TRAVERSAL_TIME 40
#define AODV_NET_TRAVERSAL_TIME 2 * AODV_NODE_TRAVERSAL_TIME * AODV_NET_DIAMETER
#define AODV_PATH_DISCOVERY_TIME 2 * AODV_NET_TRAVERSAL_TIME
// "TTL_START should be set to at least 2 if Hello messages are used for local connectivity information."
#define AODV_TTL_START 2 
// factor: just one factor, other factors are variables
#define AODV_RING_TRAVERSAL_TIME_FACTOR 2 * AODV_NODE_TRAVERSAL_TIME
#define AODV_TIMEOUT_BUFFER 2
#define AODV_TTL_INCREMENT 2
#define AODV_TTL_TRESHOLD 7
#define AODV_RREQ_RETRIES 2
#define AODV_MY_ROUTE_TIMEOUT 2 * AODV_ACTIVE_ROUTE_TIMEOUT
// Hello messages used so that's the reference for delete period
#define AODV_DELETE_PERIOD AODV_ALLOWED_HELLO_LOSS * AODV_HELLO_INTERVAL

// Hello messages: RFC 6.9
#define AODV_HELLO_RARESERVED 0
#define AODV_HELLO_RESERVEDPREFIXSZ 0
#define AODV_HELLO_HOPCOUNT 0

// RREQ messages: RFC 5.1
#define AODV_RREQ_HOPCOUNT 0
#define AODV_RREQ_RESERVED 0

// RREQ messages: RFC 5.1
#define AODV_RERR_NRESERVED 0
#define AODV_RERR_RESERVED 0

//Message Types: RFC 12
#define AODV_RREQ_MESSAGE 1
#define AODV_RREP_MESSAGE 2
#define AODV_HELLO_MESSAGE 2
#define AODV_RERR_MESSAGE 3
#define AODV_RREP_ACK_MESSAGE 4
// extra: data handled by AODV
#define AODV_DATA_MESSAGE 0

#define AODV_RREQ_STRING "RREQ"
#define AODV_RREP_STRING "RREP"
#define AODV_RREP_ACK_STRING "RREP-ACK"
#define AODV_HELLO_STRING "HELLO"
#define AODV_RERR_STRING "RERR"
// extra: data handled by AODV
#define AODV_DATA_STRING "raw"

//AODV port: RFC 6
#define AODV_PORT 654

//RREQ message format: RFC 5.1
struct aodv_rreq_header{
	uint8_t type;
	uint8_t jrgdureserved;
	uint8_t reserved;
	uint8_t hopcount;
	uint32_t rreqid;
	in_addr destination;
	uint32_t destinationseqnr;
	in_addr originator;
	uint32_t originatorseqnr;
};

//RREP message format: RFC 5.2
struct aodv_rrep_header{
	uint8_t type;
	uint8_t rareserved;
	uint8_t reservedprefixsz;
	uint8_t hopcount;
	in_addr destination;
	uint32_t destinationseqnr;
	in_addr originator;
	uint32_t lifetime;
};

//RERR message format: RFC 5.3
struct aodv_rerr_header{
	uint8_t type;
	uint8_t nreserved;
	uint8_t reserved;
	uint8_t destcount;
};

//RERR link data for one link
struct aodv_rerr_linkdata{
	in_addr destination;
	uint32_t destinationseqnr;
};

//RREP-ACK message format: RFC 5.4
struct aodv_rrep_ack_header{
	uint8_t type;
	uint8_t reserved;
};

// define UDP header offset ones to avoid dependency's on ip versions!
#ifndef INC_AODV_HEADEROFFSET
#define INC_AODV_HEADEROFFSET
static const int aodv_headeroffset = sizeof(click_ether) + sizeof(click_ip) + sizeof(click_udp);
#endif

CLICK_ENDDECLS

#endif
