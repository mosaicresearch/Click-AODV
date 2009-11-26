/*
 * AODVDestinationClassifier.{cc,hh} -- classify incoming (RREP) AODV packets
 * Bart Braem
 *
 */

// ALWAYS INCLUDE <click/config.h> FIRST
#include <click/config.h>
#include <click/confparse.hh>
#include <click/error.hh>

#include "click_aodv.hh"
#include "aodv_destinationclassifier.hh"

CLICK_DECLS
AODVDestinationClassifier::AODVDestinationClassifier()
{
}

AODVDestinationClassifier::~AODVDestinationClassifier()
{
}

int
AODVDestinationClassifier::configure(Vector<String> &conf, ErrorHandler *errh)
{
	Element* neighbour_table_element;
	int res = cp_va_parse(conf, this, errh, cpElement, "AODVNeighbour table", &neighbour_table_element, 0);
	if(res < 0) return res;
	if (!(neighbour_table=(AODVNeighbours*)neighbour_table_element->cast("AODVNeighbours"))){
		return errh->error("Supplied element is not a valid AODVNeighbours element (cast failed)");
	}
	
	myIP = &neighbour_table->getMyIP();

	return 0;
}

// set ip_src and ip_dst to right values (me and nexthop) so that information can be reused
void AODVDestinationClassifier::push (int port, Packet * packet){
	assert(port == 0);
	assert(packet);
	const aodv_rrep_header * rrep = (aodv_rrep_header*) (packet->data() + aodv_headeroffset);
	//click_chatter("rrep for %s from %s received in %s", IPAddress(rrep->originator).s().c_str(), IPAddress(rrep->destination).s().c_str(), myIP->s().c_str());
	if (rrep->originator == *myIP){
		output(0).push(packet); // new information for waitingfordiscovery
	}
	else {
		assert(rrep->originator != rrep->destination);
		WritablePacket* writable = packet->uniqueify();
		writable->ip_header()->ip_src = myIP->in_addr(); // make sure next node knows previous hop
		
		IPAddress* nexthop = neighbour_table->nexthop(rrep->originator);
		if (nexthop){
			writable->set_dst_ip_anno(*nexthop);
			writable->ip_header()->ip_dst = nexthop->in_addr();
			output(1).push(writable);
		} else {
			writable->set_dst_ip_anno(rrep->originator); // set annotation for waitinfordiscovery
			output(2).push(writable);
		}
	}
}

CLICK_ENDDECLS

EXPORT_ELEMENT(AODVDestinationClassifier)

