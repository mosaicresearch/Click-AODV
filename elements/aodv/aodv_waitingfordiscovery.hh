#ifndef AODVWAITINGFORDISCOVERY_HH
#define AODVWAITINGFORDISCOVERY_HH
#include <click/element.hh>
#include <click/bighashmap.hh>
#include <click/timer.hh>
#include "aodv_generaterreq.hh"
#include "aodv_neighbours.hh"
#include "aodv_routeupdatewatcher.hh"

/*
 * =c
 * AODVWaitingForDiscovery(GENERATERREQ, NEIGHBOURS)
 * =s AODV
 * =a AODVGenerateRREQ, AODVNeighbours
 * =d
 *
 * This element maintains packets until their discovery. */

CLICK_DECLS

struct WaitingPackets{
	Timer* timer;
	int nrOfRetries;
	uint8_t ttl;
	bool maxTTL;
	Vector<Packet*> packets;
};
typedef HashMap<IPAddress,WaitingPackets*> Buffer;

class AODVWaitingForDiscovery : public Element, public AODVRouteUpdateWatcher { 
	public:
	
		AODVWaitingForDiscovery();
		~AODVWaitingForDiscovery();
		
		const char *class_name() const	{ return "AODVWaitingForDiscovery"; }
		const char *port_count() const	{ return "2/2"; }
		const char *processing() const	{ return PUSH; }
		AODVWaitingForDiscovery *clone() const	{ return new AODVWaitingForDiscovery; }
		
		int configure(Vector<String> &, ErrorHandler *);
		
		virtual void push (int, Packet *);
		
		virtual void newKnownDestination(const IPAddress &, const IPAddress &);
	private:
		// data necessary for the timer callback function
		struct TimerData{
			AODVWaitingForDiscovery* waitingForDiscovery;
			IPAddress destination;
		};
		static void handleTask(Timer*, void *); // calback function for timers
		void runTask(const IPAddress &, TimerData*);
		
		AODVGenerateRREQ* rreq;
		AODVNeighbours* neighbour_table;
		Buffer buffer;
		
};



CLICK_ENDDECLS
#endif
