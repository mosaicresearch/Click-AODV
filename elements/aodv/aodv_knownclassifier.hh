#ifndef AODVKNOWNCLASSIFIER_HH
#define AODVKNOWNCLASSIFIER_HH
#include <click/element.hh>
#include <clicknet/ip.h>
#include <click/timer.hh>
#include "aodv_neighbours.hh"

/*
 * =c
 * AODVKnownClassifier(NEIGHBOURS)
 * =s AODV
 * =a AODVNeighbours
 * =d
 *
 * This element classifies RREQ AODV packets on known destinations. */

CLICK_DECLS

// save known RREQs as strings of form "id ip"
typedef HashMap<String,Timer*> PastRREQMap;

class AODVKnownClassifier : public Element { 
	public:
	
		AODVKnownClassifier();
		~AODVKnownClassifier();
		
		const char *class_name() const	{ return "AODVKnownClassifier"; }
		const char *port_count() const	{ return "1/2"; }
		const char *processing() const	{ return PUSH; }
		AODVKnownClassifier *clone() const	{ return new AODVKnownClassifier; }
		
		int configure(Vector<String> &, ErrorHandler *);
		
		virtual void push (int, Packet *);
		
		void addKnownRREQ(uint32_t,const IPAddress &);
	private:
		AODVNeighbours* neighbour_table;
		PastRREQMap RREQBuffer;
		
		void addKnownRREQ(const String &);
		
		static void handleExpiry(Timer*, void *); // calback function for timers
		void expire(const String &);
		
		// data necessary for the timer callback function
		struct TimerData{
			AODVKnownClassifier* known;
			String key;
		};

		const IPAddress * myIP;
};

CLICK_ENDDECLS
#endif
