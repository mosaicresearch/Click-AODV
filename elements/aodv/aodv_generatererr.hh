#ifndef AODVGENERATERERR_HH
#define AODVGENERATERERR_HH
#include <click/element.hh>
#include <click/ipaddress.hh>
#include "aodv_neighbours.hh"

/*
 * =c
 * AODVGenerateRERR(NEIGHBOURS)
 * =s AODV
 * =a 
 * =d
 *
 * This element generates AODV RRER Packets conforming the RFC 6.11 */

CLICK_DECLS

class AODVGenerateRERR : public Element { 
	public:
	
		AODVGenerateRERR();
		~AODVGenerateRERR();
		
		const char *class_name() const	{ return "AODVGenerateRERR"; }
		const char *port_count() const	{ return "2/1"; }
		const char *processing() const	{ return PUSH; }
		AODVGenerateRERR *clone() const	{ return new AODVGenerateRERR; }
		
		int configure(Vector<String> &, ErrorHandler *);
		
		void generateRERR(bool, Vector<IPAddress>, Vector<uint32_t>);
		
		virtual void push (int, Packet *);
	private:
		AODVNeighbours* neighbour_table;
};

CLICK_ENDDECLS
#endif
