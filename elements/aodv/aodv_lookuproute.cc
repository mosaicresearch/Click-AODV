/*
 * AODVLookUpRoute.{cc,hh} -- determine wether we know a route
 * Bart Braem
 *
 */

// ALWAYS INCLUDE <click/config.h> FIRST
#include <click/config.h>
#include <click/confparse.hh>
#include <click/error.hh>
#include <click/packet_anno.hh>
#include <clicknet/ip.h>

#include "aodv_lookuproute.hh"

CLICK_DECLS
AODVLookUpRoute::AODVLookUpRoute():
	neighbour_table(0)
{
}

AODVLookUpRoute::~AODVLookUpRoute()
{
}

int
AODVLookUpRoute::configure(Vector<String> &conf, ErrorHandler *errh)
{
	int res = cp_va_kparse(conf, this, errh,
		"NEIGHBOURS", cpkP+cpkM, cpElementCast, "AODVNeighbours", &neighbour_table,
		cpEnd);
	if(res < 0) return res;
	myIP = &neighbour_table->getMyIP();
	return 0;
}

void AODVLookUpRoute::push (int port, Packet * packet){
	assert(port == 0);
	assert(packet);
	assert(PAINT_ANNO(packet) == 1 || PAINT_ANNO(packet) == 3);
	IPAddress destination = packet->dst_ip_anno();
	IPAddress* nexthop = neighbour_table->nexthop(destination);
	if (nexthop){ /* destination known so fill in and push for network */
		assert(*nexthop != *myIP);
		packet->set_dst_ip_anno(*nexthop);
		
		const click_ip * ipheader = packet->ip_header();
		assert(ipheader);
		assert(packet->ip_header()->ip_src != destination);
		neighbour_table->updateRouteLifetime(ipheader->ip_src,destination);
		
		delete nexthop;
		
		output(0).push(packet);
	} else { /* destination unknown so push for route discovery if packet comes from localhost*/
		if (PAINT_ANNO(packet) == 1){
			//click_chatter("unknown destination %s in %s: RERR",destination.s().c_str(),myIP->s().c_str());
			output(2).push(packet);
		} else { // local data to be forwarded (anno 3) -> route discovery
			output(1).push(packet);
		}
	}

}


CLICK_ENDDECLS

EXPORT_ELEMENT(AODVLookUpRoute)

