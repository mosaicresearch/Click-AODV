#ifndef AODVSETRREPHEADERS_HH
#define AODVSETRREPHEADERS_HH
#include <click/element.hh>
#include "aodv_neighbours.hh"
/*
 * =c
 * AODVSetRREPHeaders()
 * =s AODV
 * =a AODVGenerateRREP
 * =d
 *
 * This element sets the RREP IP headers for RREPs. */

CLICK_DECLS

typedef HashMap<Packet*, IPAddress*> DestinationMap;
#include <click/bighashmap.cc>

class AODVSetRREPHeaders : public Element { 
	public:
	
		AODVSetRREPHeaders();
		~AODVSetRREPHeaders();
		
		const char *class_name() const	{ return "AODVSetRREPHeaders"; }
		const char *port_count() const	{ return PORTS_1_1; }
		const char *processing() const	{ return PUSH; }
		AODVSetRREPHeaders *clone() const	{ return new AODVSetRREPHeaders; }
		
		void cleanup(CleanupStage);
		
		virtual void push (int, Packet *);
		
		void addRREP(Packet*,IPAddress *);
	private:
		AODVNeighbours* neighbour_table;
		DestinationMap* destinations;
};

CLICK_ENDDECLS
#endif
