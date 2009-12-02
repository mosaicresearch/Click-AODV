#ifndef AODVLINKNEIGHBOURSDISCOVERY_HH
#define AODVLINKNEIGHBOURSDISCOVERY_HH
#include <click/element.hh>
#include "aodv_neighbours.hh"
#include "aodv_waitingfordiscovery.hh"

/* 
 * =c
 * AODVLinkNeighboursDiscovry(NEIGHBOURS, DISCOVERY)
 * =s AODV
 * =a AODVNeighbours, AODVWaitingForDiscovery
 * =d
 *
 * This object links AODVNeighbours with AODVWaitingForDiscovery, so discovery knows when a new route is added which might release waiting packets */

CLICK_DECLS

class AODVLinkNeighboursDiscovery : public Element { 
	public:
	
		AODVLinkNeighboursDiscovery();
		~AODVLinkNeighboursDiscovery();
		
		const char *class_name() const	{ return "AODVLinkNeighboursDiscovery"; }
		const char *port_count() const	{ return PORTS_0_0; }
		const char *processing() const	{ return AGNOSTIC; }
		AODVLinkNeighboursDiscovery *clone() const	{ return new AODVLinkNeighboursDiscovery; }
		
		int configure(Vector<String> &, ErrorHandler *);
		int initialize(ErrorHandler *);
		
	private:
		AODVNeighbours * neighbour_table;
		AODVWaitingForDiscovery * discovery;
};

CLICK_ENDDECLS
#endif
