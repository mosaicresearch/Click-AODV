/*
 * AODVGenerateRRER.{cc,hh} -- AODV RRER Packet generator
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

#include "aodv_generatererr.hh"
#include "click_aodv.hh"

CLICK_DECLS
AODVGenerateRERR::AODVGenerateRERR()
{
}

AODVGenerateRERR::~AODVGenerateRERR()
{
}

int
AODVGenerateRERR::configure(Vector<String> &conf, ErrorHandler *errh)
{
    return cp_va_kparse(conf, this, errh,
			"NEIGHBOURS", cpkP+cpkM, cpElementCast, "AODVNeighbours", &neighbour_table,
			cpEnd);
}

void AODVGenerateRERR::push (int port, Packet * packet){
	assert(port >= 0 && port <= 1);
	if(port == 0){
		// 6.9 case ii: receives packets from lookuproute coming from neighbours without known route
		const click_ip * ipheader = packet->ip_header();
		assert(ipheader);
		Vector<IPAddress> ips;
		ips.push_back(IPAddress(ipheader->ip_dst));
		Vector<uint32_t> seqnrs;
		seqnrs.push_back(0); // invalidate all other sequence numbers
		generateRERR(false,ips,seqnrs);
		
	} else {
		// 6.9 case iii: receives RERR from a neighbour for one or more active routes
		aodv_rerr_header * rerr = (aodv_rerr_header*) (packet->data() + aodv_headeroffset);
		assert(rerr->type == AODV_RERR_MESSAGE);
		Vector<IPAddress> ips;
		Vector<uint32_t> seqNrs;
		
		
		for(uint8_t i = 0; i < rerr->destcount; ++i){
			aodv_rerr_linkdata* data = (aodv_rerr_linkdata*) (packet->data() + aodv_headeroffset + sizeof(aodv_rerr_header) + i * sizeof(aodv_rerr_linkdata));
			Vector<IPAddress> haveNexthop = neighbour_table->getEntriesWithNexthop(data->destination);
			for(Vector<IPAddress>::iterator iter = haveNexthop.begin(); iter != haveNexthop.end(); ++iter){
				ips.push_back(*iter);
				uint32_t * seqNr = neighbour_table->getSequenceNumber(*iter);
				seqNrs.push_back(*seqNr);
				delete(seqNr);
			}
			neighbour_table->processRERR(data->destination); // process incoming RERRs here
		}
		generateRERR(false, ips, seqNrs);
	}
	packet->kill();

}

// RFC 6.11
void AODVGenerateRERR::generateRERR(bool nodelete, Vector<IPAddress> ips, Vector<uint32_t> seqnrs){
	assert(ips.size() == seqnrs.size());
	const uint nrOfDestinations = seqnrs.size();
	
	// update neighbourtable
	for(Vector<IPAddress>::iterator ipIter = ips.begin(); ipIter != ips.end(); ++ipIter){
		neighbour_table->processRERR(*ipIter);
	}

	if(ips.size() == 0) {
		// generating empty RERR is useless, neighbourtable has been updated and that's enough
		return; 
	} 
		
	// generate RERR
	int packet_size = sizeof(aodv_rerr_header) + nrOfDestinations * 2 * sizeof(uint32_t);
	WritablePacket *packet = Packet::make(aodv_headeroffset,0,packet_size, 0); // reserve no tailroom
	if ( packet == 0 ){
		click_chatter( "in %s: cannot make packet!", name().c_str());
		return;
	}
	memset(packet->data(), 0, packet->length());
	aodv_rerr_header * header = (aodv_rerr_header *) packet->data();
	header->type = AODV_RERR_MESSAGE;
	header->nreserved = AODV_RERR_NRESERVED;
	if (nodelete) header->nreserved += 1 << 7;
	header->reserved = AODV_RERR_RESERVED;
	header->destcount = nrOfDestinations;
	
	bool firstAddress = true;
	in_addr *address = 0;
	
	Vector<IPAddress>::iterator ipIter = ips.begin();
	Vector<uint32_t>::iterator seqIter = seqnrs.begin(); 
	while(ipIter != ips.end()) {
		if (firstAddress){
			address = (in_addr *) (header + 1);
			firstAddress = false;
		}
		else{
			address = (in_addr *) (address + 1);
		}
		*address = *ipIter;
		
		address = (in_addr *) (address + 1);
		// nasty trick to use uint32_t and in_addr based on same pointer
		uint32_t* uint32_tAddress = (uint32_t*)(void*)(address);
		*uint32_tAddress = htonl(*seqIter);
		
		++ipIter;
		++seqIter;
	}
	
	output(0).push(packet);
}

#include <click/vector.cc>
#if EXPLICIT_TEMPLATE_INSTANCES
template class Vector<IPAddress>;
#endif

CLICK_ENDDECLS

EXPORT_ELEMENT(AODVGenerateRERR);

