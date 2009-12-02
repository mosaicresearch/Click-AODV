/*
 * AODVGenerateRREQ.{cc,hh} -- AODV RREQ Packet generator
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

#include "aodv_generaterreq.hh"
#include "aodv_broadcastheader.hh"
#include "click_aodv.hh"

CLICK_DECLS
AODVGenerateRREQ::AODVGenerateRREQ():
	neighbour_table(0),
	rreqid(0)
{
}

AODVGenerateRREQ::~AODVGenerateRREQ()
{
}

int
AODVGenerateRREQ::configure(Vector<String> &conf, ErrorHandler *errh)
{
	int res = cp_va_kparse(conf, this, errh,
		"NEIGHBOURS", cpkP+cpkM, cpElementCast, "AODVNeighbours", &neighbour_table,
		"KNOWNCLASSIFIER", cpkP+cpkM, cpElementCast, "AODVKnownClassifier", &known_classifier,
		cpEnd);
	if(res < 0) return res;
	myIP = &neighbour_table->getMyIP();
	return res;
}

// RFC 6.3
void AODVGenerateRREQ::generateRREQ(const IPAddress & destination, bool destinationonly, uint8_t ttl){
	// no tailroom needed, fixed size
	int tailroom = 0;
	int packet_size = sizeof(aodv_rreq_header);
	WritablePacket *packet = Packet::make(aodv_headeroffset,0,packet_size, tailroom);
	  
	if ( packet == 0 ){
		click_chatter( "in %s: cannot make packet!", name().c_str());
		return;
	}
	memset(packet->data(), 0, packet->length());
	aodv_rreq_header * header = (aodv_rreq_header *) packet->data();
	header->type = AODV_RREQ_MESSAGE;
	header->jrgdureserved = 0;
	// don't use multicast
	// if (join) header->jrgdureserved += 1 << 7;
	// if (repair) header->jrgdureserved += 1 << 6;
	// always use gratuitous, good thing (tm)
	header->jrgdureserved += 1 << 5;
	if (destinationonly) header->jrgdureserved += 1 << 4;
	
	uint32_t* seqNr = neighbour_table->getSequenceNumber(destination);
	
	if (!seqNr){
		header->jrgdureserved += 1 << 3;
		header->destinationseqnr = 0;
	} else {
		header->destinationseqnr = htonl(*seqNr);
	}
	delete(seqNr);
	
	header->reserved = AODV_RREQ_RESERVED;
	header->hopcount = AODV_RREQ_HOPCOUNT;
	header->rreqid = htonl(++rreqid);
	known_classifier->addKnownRREQ(rreqid,*myIP);
	header->destination = destination.in_addr();
	header->originator = myIP->in_addr();
	header->originatorseqnr = htonl(neighbour_table->getAndIncrementMySequenceNumber());
	
	output(0).push(AODVBroadcastHeader::setBroadcastHeader(packet,*myIP,ttl)); // have ethernet and UDP headers applied
}

// macro magic to use bighashmap
#include <click/bighashmap.cc>
#if EXPLICIT_TEMPLATE_INSTANCES
template class HashMap<uint32_t, uint8_t>;
#endif

CLICK_ENDDECLS

EXPORT_ELEMENT(AODVGenerateRREQ)

