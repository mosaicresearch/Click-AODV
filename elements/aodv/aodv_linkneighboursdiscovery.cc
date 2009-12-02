/*
 * AODVLinkNeighboursDiscovery.{cc,hh} -- link AODVNeighbours with AODVDiscovery
 * Bart Braem
 *
 */

// ALWAYS INCLUDE <click/config.h> FIRST
#include <click/config.h>
#include <click/confparse.hh>
#include <click/error.hh>
#include "aodv_linkneighboursdiscovery.hh"

CLICK_DECLS
AODVLinkNeighboursDiscovery::AODVLinkNeighboursDiscovery():
	neighbour_table(0),
	discovery(0)
{
}

AODVLinkNeighboursDiscovery::~AODVLinkNeighboursDiscovery()
{
}

int
AODVLinkNeighboursDiscovery::configure(Vector<String> &conf, ErrorHandler *errh)
{
	return cp_va_kparse(conf, this, errh,
		"NEIGHBOURS", cpkP+cpkM, cpElementCast, "AODVNeighbours", &neighbour_table,
		"DISCOVERY", cpkP+cpkM, cpElementCast, "AODVWaitingForDiscovery", &discovery,
		cpEnd);
}

int AODVLinkNeighboursDiscovery::initialize(ErrorHandler *)
{
	neighbour_table->setRouteUpdateWatcher(discovery);
	return 0;
}



CLICK_ENDDECLS

EXPORT_ELEMENT(AODVLinkNeighboursDiscovery)
