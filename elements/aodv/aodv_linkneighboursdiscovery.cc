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
	Element * neighbour_table_element;
	Element * discovery_element;
	int res = cp_va_parse(conf, this, errh, cpElement, "AODVNeighbour table", &neighbour_table_element,
	cpElement, "AODVWaitingForDiscovery", &discovery_element, 0);
	if(res < 0) return res;
	if (!(neighbour_table=(AODVNeighbours*)neighbour_table_element->cast("AODVNeighbours"))){
		return errh->error("Supplied element is not a valid AODVNeighbours element (cast failed)");
	}
	if (!(discovery=(AODVWaitingForDiscovery*)discovery_element->cast("AODVWaitingForDiscovery"))){
		return errh->error("Supplied element is not a valid AODVWaitingForDiscovery element (cast failed)");
	}
	return 0;
}

int AODVLinkNeighboursDiscovery::initialize(ErrorHandler *)
{
	neighbour_table->setRouteUpdateWatcher(discovery);
	return 0;
}



CLICK_ENDDECLS

EXPORT_ELEMENT(AODVLinkNeighboursDiscovery)
