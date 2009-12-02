#ifndef AODVTRACKNEIGHBOURS_HH
#define AODVTRACKNEIGHBOURS_HH
#include <click/element.hh>
#include <click/timer.hh>
#include <click/hashmap.hh>
#include "aodv_generatererr.hh"

/*
 * =c
 * AODVTrackNeighbours(GENERATERERR)
 * =s AODV
 * =a AODVNeighbours, AODVGenerateRERR
 * =d
 *
 * This element processes packets and updates the neighbours: they are alive! */

CLICK_DECLS

typedef HashMap<IPAddress, Timer*> TimerMap;

class AODVTrackNeighbours : public Element { 
	public:
	
		AODVTrackNeighbours();
		~AODVTrackNeighbours();
		
		const char *class_name() const	{ return "AODVTrackNeighbours"; }
		const char *port_count() const	{ return PORTS_1_1; }
		const char *processing() const	{ return PUSH; }
		AODVTrackNeighbours *clone() const	{ return new AODVTrackNeighbours; }
		
		int configure(Vector<String> &, ErrorHandler *);
		
		virtual void push (int, Packet *);
	private:
		static void handleExpiry(Timer*, void *); // calback function for timers
		void expire(IPAddress*);
		
		AODVGenerateRERR * generateRerr;
		AODVNeighbours* neighbour_table;
		
		TimerMap neighbour_timers;
		struct TimerData{
			AODVTrackNeighbours* me;
			IPAddress * ip;
		};
		
		const IPAddress * myIP;
};

CLICK_ENDDECLS
#endif
