/*
 * AODVUpdateNeighbours.{cc,hh} -- update neighbours when receiving hello, rrep or rreq packets
 * Bart Braem
 *
 */

// ALWAYS INCLUDE <click/config.h> FIRST
#include <click/config.h>
#include <click/confparse.hh>
#include <click/error.hh>
#include <clicknet/ip.h>

#include "aodv_updateneighbours.hh"
#include "aodv_packetanalyzer.hh"
#include "click_aodv.hh"

CLICK_DECLS
AODVUpdateNeighbours::AODVUpdateNeighbours():
	neighbour_table(0)
{
}

AODVUpdateNeighbours::~AODVUpdateNeighbours()
{
}

int
AODVUpdateNeighbours::configure(Vector<String> &conf, ErrorHandler *errh)
{
    return cp_va_kparse(conf, this, errh,
		"NEIGHBOURS", cpkP+cpkM, cpElementCast, "AODVNeighbours", &neighbour_table,
		cpEnd);
}

// RFC 6.2
void AODVUpdateNeighbours::push (int port, Packet * packet){
	assert(port == 0);
	assert(packet);
	const click_ip * ipheader = packet->ip_header();
	assert(ipheader);
	
	switch(AODVPacketAnalyzer::getMessageType(packet)){
		case AODV_RREP_MESSAGE: //RREP or HELLO
			{
			WritablePacket * writable = packet->uniqueify();
			aodv_rrep_header * rrep = (aodv_rrep_header*) (writable->data() + aodv_headeroffset);
		
			// RERRs aren't allowed here
			assert(rrep->type == 2);
			
			//click_chatter("AODV rrep/hello packet received from %s with seqnr %u", IPAddress(rrep->originator).s().c_str(), ntohl(rrep->destinationseqnr));
		
			// increment hopcount according to RFC 6.7
			++rrep->hopcount;
			
			if (ipheader->ip_ttl == 1){ //HELLO
				neighbour_table->updateRoutetableEntry(IPAddress(rrep->destination),ntohl(rrep->destinationseqnr),rrep->hopcount, IPAddress(ipheader->ip_src),AODV_ALLOWED_HELLO_LOSS * AODV_HELLO_INTERVAL);
			} else { // RREP
				// the information is only useful if I am not the destination (I might hear this packets due to routing changes)
				if (rrep->destination != neighbour_table->getMyIP()){ 
					neighbour_table->updateRoutetableEntry(IPAddress(rrep->destination), ntohl(rrep->destinationseqnr), rrep->hopcount, IPAddress(ipheader->ip_src), ntohl(rrep->lifetime));
				}
			}
			output(0).push(writable);
			break;
			}
		case AODV_RREQ_MESSAGE: //RREQ
			//aodv_rreq_header * header = (aodv_rreq_header*) (packet->data() + aodv_headeroffset);
			//click_chatter("AODV rreq packet received from %s with rreqid %u", IPAddress(header->originator).s().c_str(), ntohl(header->rreqid));
			
			// RFC 6.5: create or update route to previous hop ...
			neighbour_table->updateRoutetableEntry(ipheader->ip_src,1, ipheader->ip_src);
			
			output(0).push(packet);
			break;
		case AODV_RREP_ACK_MESSAGE:
			//click_chatter("AODV rrep-ack packet received");
			//no processing possible
			output(0).push(packet);
			break;
		default:
			assert(false); // not a rrep, hello or rreq: problems
	}
}


CLICK_ENDDECLS

EXPORT_ELEMENT(AODVUpdateNeighbours)
ELEMENT_REQUIRES(AODVPacketAnalyzer)
