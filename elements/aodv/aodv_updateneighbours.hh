#ifndef AODVUPDATENEIGHBOURS_HH
#define AODVUPDATENEIGHBOURS_HH
#include <click/element.hh>
#include "aodv_neighbours.hh"

/*
 * =c
 * AODVUpdateNeighbours(NEIGHBOURS)
 * =s AODV
 * =a AODVNeighbours
 * =d
 *
 * This element processes incoming HELLO packets and updates the neighbours. */

CLICK_DECLS

class AODVUpdateNeighbours : public Element { 
	public:
	
		AODVUpdateNeighbours();
		~AODVUpdateNeighbours();
		
		const char *class_name() const	{ return "AODVUpdateNeighbours"; }
		const char *port_count() const	{ return PORTS_1_1; }
		const char *processing() const	{ return PUSH; }
		AODVUpdateNeighbours *clone() const	{ return new AODVUpdateNeighbours; }
		
		int configure(Vector<String> &, ErrorHandler *);
		
		virtual void push (int, Packet *);
	private:
		AODVNeighbours* neighbour_table;
};

CLICK_ENDDECLS
#endif
