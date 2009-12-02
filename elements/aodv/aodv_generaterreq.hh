#ifndef AODVGENERATERREQ_HH
#define AODVGENERATERREQ_HH
#include <click/element.hh>
#include "aodv_neighbours.hh"
#include "aodv_knownclassifier.hh"

/*
 * =c
 * AODVGenerateRREQ(NEIGHBOURS, KNOWNCLASSIFIER)
 * =s AODV
 * =a AODVNeighbours, AODVKnownClassifier
 * =d
 *
 * This element generates AODV RREQ Packets, conforming the RFC 6.3 */

CLICK_DECLS

class AODVGenerateRREQ : public Element { 
	public:
	
		AODVGenerateRREQ();
		~AODVGenerateRREQ();
		
		const char *class_name() const	{ return "AODVGenerateRREQ"; }
		const char *port_count() const	{ return PORTS_0_1; }
		const char *processing() const	{ return PUSH; }
		AODVGenerateRREQ *clone() const	{ return new AODVGenerateRREQ; }
		
		int configure(Vector<String> &, ErrorHandler *);
		
		void generateRREQ(const IPAddress &, bool,uint8_t);
	private:
		AODVNeighbours* neighbour_table;
		uint32_t rreqid;
		AODVKnownClassifier* known_classifier;
		const IPAddress * myIP;
};

CLICK_ENDDECLS
#endif
