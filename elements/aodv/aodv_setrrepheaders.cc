/*
 * AODVSetRREPHeaders.{cc,hh} -- set right RREP destination
 * Bart Braem
 *
 */

// ALWAYS INCLUDE <click/config.h> FIRST
#include <click/config.h>
#include <click/confparse.hh>
#include <click/error.hh>
#include <clicknet/udp.h>
#include <clicknet/ip.h>
#include "aodv_setrrepheaders.hh"
CLICK_DECLS

AODVSetRREPHeaders::AODVSetRREPHeaders()
{
	destinations = new DestinationMap();
}

AODVSetRREPHeaders::~AODVSetRREPHeaders()
{
}

void AODVSetRREPHeaders::push (int port, Packet * p){
	assert(port == 0);
	assert(p);
	WritablePacket * packet = p->uniqueify();
	assert(packet);
	
	// next hop ("RREP is unicasted back") is in destinations
	DestinationMap::Pair * pair = destinations->find_pair(packet);
	assert(pair);
	
	packet->set_dst_ip_anno(*(pair->value));
	click_ip * ipheader = packet->ip_header();
	ipheader->ip_dst = *pair->value;
	
	delete pair->value; // free memory again
	
	destinations->remove(packet);
	
	output(0).push(packet);
}

void AODVSetRREPHeaders::addRREP(Packet* rrep, IPAddress * ip){
	destinations->insert(rrep,ip);
}

void AODVSetRREPHeaders::cleanup(CleanupStage){
	delete destinations;
}

CLICK_ENDDECLS

EXPORT_ELEMENT(AODVSetRREPHeaders)

