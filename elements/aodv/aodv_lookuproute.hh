#ifndef AODVLOOKUPROUTE_HH
#define AODVLOOKUPROUTE_HH
#include <click/element.hh>
#include "aodv_neighbours.hh"

/*
 * =c
 * AODVLookUpRoute(NEIGHBOURS)
 * =s AODV
 * =a AODVNeighbours
 * =d
 *
 * This element determines wether we know the route to an incoming element, If we do move to output[0] otherwise to output[1]. */

CLICK_DECLS

class AODVLookUpRoute : public Element { 
	public:
	
		AODVLookUpRoute();
		~AODVLookUpRoute();
		
		const char *class_name() const	{ return "AODVLookUpRoute"; }
		const char *port_count() const	{ return "1/3"; }
		const char *processing() const	{ return PUSH; }
		AODVLookUpRoute *clone() const	{ return new AODVLookUpRoute; }
		
		int configure(Vector<String> &, ErrorHandler *);
		
		virtual void push (int, Packet *);
	private:
		AODVNeighbours* neighbour_table;
		const IPAddress * myIP;
};

CLICK_ENDDECLS
#endif
