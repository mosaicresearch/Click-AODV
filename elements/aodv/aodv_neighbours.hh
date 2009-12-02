#ifndef AODVNEIGHBOURS_HH
#define AODVNEIGHBOURS_HH
#include <click/element.hh>
#include <click/hashmap.hh>
#include <click/timer.hh>
#include "click_aodv.hh"
#include "aodv_routeupdatewatcher.hh"

/*
 * =c
 * AODVNeighbours(ADDR)
 * =s AODV
 * =a AODVRouteUpdateWatcher
 * 
 * =d
 *
 * This is element keeps track of the neighbours of an AODV element. */


CLICK_DECLS

typedef HashMap<IPAddress,int> PrecursorMap;
struct routing_table_entry{
	uint32_t destinationSequenceNumber;
	bool validDestinationSequenceNumber;
	bool valid;
	uint32_t hopcount;
	in_addr nexthop;
	PrecursorMap precursorlist; // use hashmap to get set
	Timer* expiry;
};

typedef HashMap<IPAddress, routing_table_entry> NeighbourMap;

class AODVNeighbours : public Element { 
	public:
		AODVNeighbours();
		~AODVNeighbours();
		
		const char *class_name() const	{ return "AODVNeighbours"; }
		const char* port_count() const	{ return PORTS_0_0; }
		const char *processing() const	{ return AGNOSTIC; }
		AODVNeighbours *clone() const	{ return new AODVNeighbours; }
		
		int configure(Vector<String> &, ErrorHandler *);
		
		void updateRoutetableEntry(const IPAddress &,uint32_t, uint32_t, const IPAddress &, uint32_t);
		void updateRoutetableEntry(const IPAddress &,uint32_t, const IPAddress &);
		void updateRouteLifetime(const IPAddress &, const IPAddress &);
		void addLifeTime(const IPAddress &, uint32_t);
		IPAddress* nexthop(const IPAddress &) const;
		uint32_t* getSequenceNumber(const IPAddress &) const;
		int8_t getHopcount(const IPAddress &) const;
		uint32_t getAndIncrementMySequenceNumber();
		uint32_t updateMySequenceNumber(uint32_t);
		uint32_t getMySequenceNumber() const;
		const IPAddress & getMyIP() const;
		void addPrecursor(const IPAddress &, const IPAddress &);
		static bool largerSequenceNumber(uint32_t, uint32_t);
		uint32_t getLifetime(const IPAddress &) const;
		void setRouteUpdateWatcher(AODVRouteUpdateWatcher*);
		void processRERR(const IPAddress &);
		Vector<IPAddress>* getPrecursors(const IPAddress &) const;
		Vector<IPAddress> getEntriesWithNexthop(const IPAddress &) const;
	private:
		IPAddress myIP;
		uint32_t mySequenceNumber;
		NeighbourMap neighbours;
		AODVRouteUpdateWatcher * watcher;
		
		// data necessary for the timer callback function
		struct TimerData{
			AODVNeighbours* neighbours;
			IPAddress * ip;
		};
		
		static void handleExpiry(Timer*, void *); // calback function for timers
		void expire(const IPAddress &, TimerData *);
		
		void editRoutetableEntry(NeighbourMap::Pair*, bool, uint32_t , uint32_t , const IPAddress & , int);
		void insertRoutetableEntry(bool, uint32_t, uint32_t, const IPAddress &, int, const IPAddress &);
	
		// some usefull time functions, for general usage
		static int calculateLifetime(int lifetime);
		static void updateLifetime(NeighbourMap::Pair* pair);

		static inline Timestamp calculateTimeval(int ms) {
			return Timestamp::now() + Timestamp::make_msec(ms);
		}
};

CLICK_ENDDECLS
#endif
