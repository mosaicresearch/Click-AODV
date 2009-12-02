/*
 * AODVGenerateRREP.{cc,hh} -- AODV RREP Packet generator
 * Bart Braem
 *
 */

// ALWAYS INCLUDE <click/config.h> FIRST
#include <click/config.h>
#include <click/confparse.hh>
#include <click/error.hh>
#include <clicknet/ip.h>
#include <clicknet/ether.h>
#include <clicknet/udp.h>

#include "aodv_generaterrep.hh"
#include "click_aodv.hh"

CLICK_DECLS
AODVGenerateRREP::AODVGenerateRREP():
	neighbour_table(0)
{
}

AODVGenerateRREP::~AODVGenerateRREP()
{
}

int
AODVGenerateRREP::configure(Vector<String> &conf, ErrorHandler *errh)
{
	int res = cp_va_kparse(conf, this, errh,
		"NEIGHBOURS", cpkP+cpkM, cpElementCast, "AODVNeighbours", &neighbour_table,
		"SETRREP", cpkP+cpkM, cpElementCast, "AODVSetRREPHeaders", &setrrepheaders,
		cpEnd);
	if(res < 0) return res;
	myIP = &neighbour_table->getMyIP();
	return res;
}

void AODVGenerateRREP::push (int port, Packet * rreq){
	assert(port == 0);

	aodv_rreq_header * rreq_header = (aodv_rreq_header*) (rreq->data() + aodv_headeroffset);
	bool imdestination = (rreq_header->destination == *myIP);
	if (imdestination) neighbour_table->updateMySequenceNumber(ntohl(rreq_header->destinationseqnr)); //RFC 6.1
	
	// no tailroom needed, fixed size
	int tailroom = 0;
	int packet_size = sizeof(aodv_rrep_header);
	WritablePacket *packet = Packet::make(aodv_headeroffset,0,packet_size, tailroom);
	  
	if ( packet == 0 ){
		click_chatter( "in %s: cannot make packet!", name().c_str());
		return;
	}
	memset(packet->data(), 0, packet->length());
	aodv_rrep_header * header = (aodv_rrep_header *) packet->data();
	header->type = AODV_RREP_MESSAGE;
	header->rareserved = AODV_HELLO_RARESERVED;
	header->reservedprefixsz = AODV_HELLO_RESERVEDPREFIXSZ;
	
	
	header->destination = rreq_header->destination;
	
	if (imdestination){
		header->destinationseqnr = htonl(neighbour_table->getMySequenceNumber());
		header->lifetime = htonl(AODV_MY_ROUTE_TIMEOUT);
		header->hopcount = 0;
	} else {
		uint32_t * destinationseqnr = neighbour_table->getSequenceNumber(rreq_header->destination);
		assert(destinationseqnr); // if we don't have a sequence number why are we responding...
		header->destinationseqnr = htonl(*destinationseqnr);
		header->lifetime = htonl(neighbour_table->getLifetime(rreq_header->destination));
		header->hopcount = neighbour_table->getHopcount(rreq_header->destination);
		delete destinationseqnr;
	}
	
	header->originator = rreq_header->originator;
	
	
	assert(header->originator != header->destination);
	
	const click_ip * ipheader = rreq->ip_header();
	assert(ipheader);
	
	if (!imdestination) neighbour_table->addPrecursor(ipheader->ip_src,rreq_header->originator); // RFC 6.2
	
	IPAddress* nexthop = neighbour_table->nexthop(IPAddress(header->originator));
	assert(nexthop);
	setrrepheaders->addRREP(packet,nexthop);
	
	output(0).push(packet);
	
	//RFC 6.6.3 : Generating Gratuitous RREPs
	if (!imdestination && (rreq_header->jrgdureserved & 1 << 5)){ 
		// no tailroom needed, fixed size
		int tailroom = 0;
		int packet_size = sizeof(aodv_rrep_header);
		WritablePacket * grrep = Packet::make(aodv_headeroffset,0,packet_size, tailroom);
		
		aodv_rrep_header * header = (aodv_rrep_header *) grrep->data();
		header->type = AODV_RREP_MESSAGE;
		header->rareserved = AODV_HELLO_RARESERVED;
		header->reservedprefixsz = AODV_HELLO_RESERVEDPREFIXSZ;
		
		header->hopcount = neighbour_table->getHopcount(rreq_header->originator);
		header->destination = rreq_header->originator;
		header->destinationseqnr = rreq_header->originatorseqnr;
		header->lifetime = htonl(neighbour_table->getLifetime(rreq_header->originator));
		
		header->originator = rreq_header->destination;
		
		assert(header->originator != header->destination);

		IPAddress* nexthop = neighbour_table->nexthop(IPAddress(rreq_header->destination));
		assert(nexthop);
		setrrepheaders->addRREP(grrep,nexthop);
		
		output(0).push(grrep);
	}
	
	rreq->kill(); // release old data
}
CLICK_ENDDECLS

EXPORT_ELEMENT(AODVGenerateRREP)

