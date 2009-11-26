#ifndef AODVROUTEUPDATEWATCHER_HH
#define AODVROUTEUPDATEWATCHER_HH
#include <click/config.h>
#include <click/ipaddress.hh>

CLICK_DECLS

class AODVRouteUpdateWatcher{ 
	public:
		virtual void newKnownDestination(const IPAddress &,const IPAddress &) = 0;
		virtual ~AODVRouteUpdateWatcher(){};
};

CLICK_ENDDECLS
#endif
