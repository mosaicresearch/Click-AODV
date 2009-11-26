/*
 * AODVHelloGenerator.{cc,hh} -- AODV Hello Packet generator
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

#include "aodv_hellogenerator.hh"
#include "aodv_broadcastheader.hh"
#include "click_aodv.hh"

CLICK_DECLS
AODVHelloGenerator::AODVHelloGenerator():
	timer(this)
{
}

AODVHelloGenerator::~AODVHelloGenerator()
{
}

int
AODVHelloGenerator::initialize(ErrorHandler *)
{
	timer.initialize(this);
	timer.schedule_after_msec(AODV_HELLO_INTERVAL);
	return 0;
}

int
AODVHelloGenerator::configure(Vector<String> &conf, ErrorHandler *errh)
{
	Element * neighbour_table_element;
	int res = cp_va_parse(conf, this, errh, cpElement, "AODVNeighbour table", &neighbour_table_element, 0);
	if(res < 0) return res;
	if (!(neighbour_table=(AODVNeighbours*)neighbour_table_element->cast("AODVNeighbours"))){
		return errh->error("Supplied element is not a valid AODVNeighbours element (cast failed)");
	}
	
	myIP = &neighbour_table->getMyIP();

	return 0;
}

void AODVHelloGenerator::run_timer(){
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
	header->type = AODV_HELLO_MESSAGE;
	header->rareserved = AODV_HELLO_RARESERVED;
	header->reservedprefixsz = AODV_HELLO_RESERVEDPREFIXSZ;
	header->hopcount = AODV_HELLO_HOPCOUNT;
	header->destination = myIP->in_addr();
	header->destinationseqnr = htonl(neighbour_table->getMySequenceNumber());
	header->originator = myIP->in_addr();
	header->lifetime = htonl(AODV_ALLOWED_HELLO_LOSS * AODV_HELLO_INTERVAL);
	
	output(0).push(AODVBroadcastHeader::setBroadcastHeader(packet,*myIP,1));
	timer.schedule_after_msec(AODV_HELLO_INTERVAL);
}

// RFC 6.9: "Every ... ms, the node checks whether is has sent a broadcast (...) within the last ..."
// RREQs are pushed trough here, so every time a packet arrives reset timer
void AODVHelloGenerator::push (int port, Packet * packet){
	assert(port == 0);
	timer.schedule_after_msec(AODV_HELLO_INTERVAL);
	output(1).push(packet);
}

CLICK_ENDDECLS

EXPORT_ELEMENT(AODVHelloGenerator)

