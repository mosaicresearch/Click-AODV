#ifndef AODVHELLOGENERATOR_HH
#define AODVHELLOGENERATOR_HH
#include <click/element.hh>
#include <click/timer.hh>
#include "aodv_neighbours.hh"

/*
 * =c
 * AODVHelloGenerator(NEIGHBOURS)
 * =s AODV
 * =a AODVNeighbours, AODVUpdateNeighbours
 * =d
 *
 * This element peridocially generates AODV Hello Packets, based on a host with ip address IP, conforming the RFC chapter 6.9. */

CLICK_DECLS

class AODVHelloGenerator : public Element { 
	public:
	
		AODVHelloGenerator();
		~AODVHelloGenerator();
		
		const char *class_name() const	{ return "AODVHelloGenerator"; }
		const char *port_count() const	{ return "1/2"; }
		const char *processing() const	{ return PUSH; }
		AODVHelloGenerator *clone() const	{ return new AODVHelloGenerator; }
		
		int configure(Vector<String> &, ErrorHandler *);
		int initialize(ErrorHandler *);
		
		virtual void push (int, Packet *);
		
		void run_timer();
		
	private:
		Timer timer;
		AODVNeighbours * neighbour_table;
		const IPAddress * myIP;
};

CLICK_ENDDECLS
#endif
