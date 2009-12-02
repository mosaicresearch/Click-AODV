#ifndef AODVDESTINATIONCLASSIFIER_HH
#define AODVDESTINATIONCLASSIFIER_HH
#include <click/element.hh>
#include <clicknet/ip.h>
#include "aodv_neighbours.hh"

/*
 * =c
 * AODVDestinationClassifier(NEIGHBOURS)
 * =s AODV
 * =a AODVNeighbours
 * =d
 *
 * This element classifies RREP AODV packets on destination. */

CLICK_DECLS

class AODVDestinationClassifier : public Element { 
	public:
	
		AODVDestinationClassifier();
		~AODVDestinationClassifier();
		
		const char *class_name() const	{ return "AODVDestinationClassifier"; }
		const char *port_count() const	{ return "1/3"; }
		const char *processing() const	{ return PUSH; }
		AODVDestinationClassifier *clone() const	{ return new AODVDestinationClassifier; }
		
		int configure(Vector<String> &, ErrorHandler *);
		
		virtual void push (int, Packet *);
	private:
		AODVNeighbours* neighbour_table;
		const IPAddress * myIP;
};

CLICK_ENDDECLS
#endif
