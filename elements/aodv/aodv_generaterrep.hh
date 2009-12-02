#ifndef AODVGENERATERREP_HH
#define AODVGENERATERREP_HH
#include <click/element.hh>
#include "aodv_neighbours.hh"
#include "aodv_setrrepheaders.hh"

/*
 * =c
 * AODVGenerateRREP(NEIGHBOURS, SETRREP)
 * =s AODV
 * =a AODVNeighbours
 * =d
 *
 * This element generates AODV RREP Packets, conforming the RFC chapter 6.6 */

CLICK_DECLS

class AODVGenerateRREP : public Element { 
	public:
	
		AODVGenerateRREP();
		~AODVGenerateRREP();
		
		const char *class_name() const	{ return "AODVGenerateRREP"; }
		const char *port_count() const	{ return PORTS_1_1; }
		const char *processing() const	{ return AGNOSTIC; }
		AODVGenerateRREP *clone() const	{ return new AODVGenerateRREP; }
		
		int configure(Vector<String> &, ErrorHandler *);
		
		void push(int, Packet*);
	private:
		AODVNeighbours* neighbour_table;
		AODVSetRREPHeaders* setrrepheaders;
		const IPAddress * myIP;
};

CLICK_ENDDECLS
#endif
