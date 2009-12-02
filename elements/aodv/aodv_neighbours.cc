/*
 * AODVNeighbours.{cc,hh} -- keep track of neighbours
 * Bart Braem
 *
 */

// ALWAYS INCLUDE <click/config.h> FIRST
#include <click/config.h>
#include <click/confparse.hh>
#include <click/error.hh>

#include "aodv_neighbours.hh"
#include "click_aodv.hh"

CLICK_DECLS
AODVNeighbours::AODVNeighbours():
	mySequenceNumber(0)
{
}

AODVNeighbours::~AODVNeighbours()
{
}

int
AODVNeighbours::configure(Vector<String> &conf, ErrorHandler *errh)
{
    return cp_va_kparse(conf, this, errh,
			"ADDR", cpkP+cpkM, cpIPAddress, &myIP,
			cpEnd);
}

void AODVNeighbours::setRouteUpdateWatcher(AODVRouteUpdateWatcher* w){
	watcher = w;
}

void AODVNeighbours::handleExpiry(Timer*, void * data){
	TimerData * timerdata = (TimerData*) data;
	assert(timerdata);
	timerdata->neighbours->expire(*timerdata->ip,timerdata);
}

void AODVNeighbours::expire(const IPAddress & ip, TimerData * timerdata){
	// pass timerdata too to clean up memory after timer expires completely
	NeighbourMap::Pair* pair = neighbours.find_pair(ip);
	assert(pair);
	if(pair->value.valid){
		pair->value.valid = false;
		pair->value.expiry->schedule_after_msec(AODV_DELETE_PERIOD);
	} else {
		delete(pair->value.expiry);
		neighbours.remove(ip);
		delete timerdata->ip;
		delete timerdata;
	}
}

int AODVNeighbours::calculateLifetime(int lifetime){
	assert(lifetime >= -1);
	return (lifetime != -1)?lifetime:AODV_ACTIVE_ROUTE_TIMEOUT;
}

void AODVNeighbours::editRoutetableEntry(NeighbourMap::Pair* pair, bool validDestinationSequenceNumber, uint32_t destinationSequenceNumber, uint32_t hopcount, const IPAddress & nexthop, int lifetime){
	assert(lifetime >= -1);
	assert(nexthop != myIP);
	pair->value.expiry->unschedule();
	pair->value.validDestinationSequenceNumber = validDestinationSequenceNumber;
	pair->value.destinationSequenceNumber = destinationSequenceNumber;
	pair->value.valid = true;
	pair->value.hopcount = hopcount;
	pair->value.nexthop = nexthop;
	pair->value.expiry->schedule_after_msec(calculateLifetime(lifetime));
	assert(watcher);
	watcher->newKnownDestination(pair->key,nexthop);
}

void AODVNeighbours::insertRoutetableEntry(bool validDestinationSequenceNumber, uint32_t destinationSequenceNumber, uint32_t hopcount, const IPAddress & nexthop, int lifetime, const IPAddress & ip){
	assert(lifetime >= -1);
	assert(nexthop != myIP);
	struct routing_table_entry data;
	data.validDestinationSequenceNumber = validDestinationSequenceNumber;
	
	data.destinationSequenceNumber = destinationSequenceNumber;
	data.valid = true;
	data.hopcount = hopcount;

	TimerData* timerdata = new TimerData();
	timerdata->ip = new IPAddress(ip);
	timerdata->neighbours = this;
	data.expiry = new Timer(&AODVNeighbours::handleExpiry,timerdata);
	data.expiry->initialize(this);
	data.expiry->schedule_after_msec(calculateLifetime(lifetime));
	data.nexthop = nexthop;
	assert(!neighbours.find_pair(ip));
	neighbours.insert(ip,data);
	assert(watcher);
	watcher->newKnownDestination(ip,nexthop);
}

// use overloading to smoothly process entries without known sequencenumber and lifetime
void AODVNeighbours::updateRoutetableEntry(const IPAddress & ip, uint32_t hopcount, const IPAddress & nexthop){
	assert (ip != myIP);
	// RFC 6.2: "The route is only updated if the new sequence number is either:..."
	if (NeighbourMap::Pair* pair = neighbours.find_pair(ip)){ 
		editRoutetableEntry(pair,false,0,hopcount,nexthop,-1);
	} else {
		insertRoutetableEntry(false,0,hopcount,nexthop,-1,ip);
	}
}

// RFC 6.2
void AODVNeighbours::updateRoutetableEntry(const IPAddress & ip, uint32_t sequenceNumber, uint32_t hopcount, const IPAddress & nexthop, uint32_t lifetime)
{
	assert(lifetime > 0);
	assert (ip != myIP);
	
	// RFC 6.2: "The route is only updated if the new sequence number is either:..."
	if (NeighbourMap::Pair* pair = neighbours.find_pair(ip)){
		if (!pair->value.valid || largerSequenceNumber(pair->value.destinationSequenceNumber,sequenceNumber) || (pair->value.destinationSequenceNumber == sequenceNumber && hopcount < pair->value.hopcount)) {
			editRoutetableEntry(pair,true,sequenceNumber,hopcount,nexthop,lifetime);
		}
	} else {
		insertRoutetableEntry(true,sequenceNumber,hopcount,nexthop,lifetime,ip);
	}
}

IPAddress * AODVNeighbours::nexthop(const IPAddress & destination) const{
	NeighbourMap::Pair* pair = neighbours.find_pair(destination);
	IPAddress * result = 0;
	if (pair && pair->value.valid){
		result = new IPAddress(pair->value.nexthop); 
		assert(*result != myIP);
	}
	return result;
}

// the route may get more lifetime
void AODVNeighbours::addLifeTime(const IPAddress & destination, uint32_t ms){
	assert(ms > 0);
	NeighbourMap::Pair* pair = neighbours.find_pair(destination);
	if (!pair) return; // route didn't exist / already expunged
	Timestamp newer = calculateTimeval(ms);
	const Timestamp & old = pair->value.expiry->expiry();
	if (old < newer) pair->value.expiry->schedule_at(newer);
}

void AODVNeighbours::updateLifetime(NeighbourMap::Pair* pair){
	Timestamp newer = calculateTimeval(calculateLifetime(-1)); // use existing code
	const Timestamp & old = pair->value.expiry->expiry();
	pair->value.valid = true;
	if (old < newer) pair->value.expiry->schedule_at(newer);
}

// RFC 6.2
void AODVNeighbours::updateRouteLifetime(const IPAddress & from, const IPAddress & to){
	//RFC 6.2: update lifetime of from, to and their nexthops:
	/*Each time a route is used to forward a data packet, its Active Route
	Lifetime field of the source, destination and the next hop on the
	path to the destination is updated to be no less than the current
	time plus ACTIVE_ROUTE_TIMEOUT. ... the
	Active Route Lifetime for the previous hop, along the reverse path
	back to the IP source, is also updated to be no less than the current
	time plus ACTIVE_ROUTE_TIMEOUT.*/
	
	// it is possible that we don't have information about the previous hop (highly likely)
	// it is also possible that we don't have information about the next hop (eg. node sends data before transmitting HELLO or RREQ so we don't have its sequence number, filling in a new entry is not feasible)
	
	assert(to != myIP);
	if (from != myIP){
		// update lifetime when using local paths but don't update previous hop then
		NeighbourMap::Pair* frompair = neighbours.find_pair(from);
		assert(frompair); // we must have received a rreq to have packets pass this way so we have a route table entry for it
		updateLifetime(frompair); // "of the source"
		
		NeighbourMap::Pair* frompairNexthop = neighbours.find_pair(from);
		if(frompairNexthop) updateLifetime(frompairNexthop); // the Active Route Lifetime for the previous hop
	}
	NeighbourMap::Pair* topair = neighbours.find_pair(to);
	assert(topair); // we are going to use this route so we have a route table entry for it
	updateLifetime(topair); // "destination"
	
	NeighbourMap::Pair* topairNexthop = neighbours.find_pair(topair->value.nexthop);
	if (topairNexthop) updateLifetime(topairNexthop); // "and the next hop on the path to the destination"
}

// RFC 6.2
void AODVNeighbours::addPrecursor(const IPAddress & neighbour, const IPAddress & precursor){
	NeighbourMap::Pair* pair = neighbours.find_pair(neighbour);
	assert(pair); // if we can't find a neighbour we're in big trouble with the RREP
	
	pair->value.precursorlist.insert(precursor,0); // 0: doesn't matter, we need a set instead of a list
}

uint32_t* AODVNeighbours::getSequenceNumber(const IPAddress & destination) const{
	NeighbourMap::Pair* pair = neighbours.find_pair(destination);
	if (!pair) return 0;
	return new uint32_t(pair->value.destinationSequenceNumber);
}

int8_t AODVNeighbours::getHopcount(const IPAddress & destination) const{
	NeighbourMap::Pair* pair = neighbours.find_pair(destination);
	if (!pair) return -1;
	else return pair->value.hopcount;
}

uint32_t AODVNeighbours::getAndIncrementMySequenceNumber(){
	return ++mySequenceNumber;
}

uint32_t AODVNeighbours::getMySequenceNumber() const{
	return mySequenceNumber;
}

//RFC 6.1
uint32_t AODVNeighbours::updateMySequenceNumber(uint32_t received){
	// update my sequence number to maximum of current sequence number and received sequence number
	if (largerSequenceNumber(mySequenceNumber,received)) return mySequenceNumber;
	return mySequenceNumber = received;
}

const IPAddress & AODVNeighbours::getMyIP() const{
	return myIP;
}

uint32_t AODVNeighbours::getLifetime(const IPAddress & ip) const{
	NeighbourMap::Pair* pair = neighbours.find_pair(ip);
	assert(pair);
	const Timestamp & expiry = pair->value.expiry->expiry();
	Timestamp now = Timestamp::now();
	uint32_t result = (expiry - now).msecval();
	return (result == 0)?1:result; // avoid returning 0 to avoid confusion: this entry is still valid!
}

// RFC 6.11
void AODVNeighbours::processRERR(const IPAddress & ip){
	NeighbourMap::Pair* pair = neighbours.find_pair(ip);
	if (!pair) return;
	// 1.
	if (pair->value.valid) ++pair->value.destinationSequenceNumber;
	// 2.
	pair->value.valid = false;
	// 3.
	pair->value.expiry->schedule_after_msec(AODV_DELETE_PERIOD);
}

Vector<IPAddress>* AODVNeighbours::getPrecursors(const IPAddress & ip) const{
	NeighbourMap::Pair* pair = neighbours.find_pair(ip);
	if (!pair || !pair->value.valid) return 0;
	// return a copy of the list
	Vector<IPAddress>* res = new Vector<IPAddress>;
	for(PrecursorMap::iterator iter = pair->value.precursorlist.begin(); iter != pair->value.precursorlist.end(); ++iter){
		// some results may not be valid any more, instead of deleting those on expiry don't return them
		if (getSequenceNumber(iter.key())) res->push_back(iter.key());
	}
	return res;
}

Vector<IPAddress> AODVNeighbours::getEntriesWithNexthop(const IPAddress & nexthop) const{
	Vector<IPAddress> res;
	for(NeighbourMap::const_iterator iter = neighbours.begin(); iter != neighbours.end(); ++iter){
		if(iter.value().valid && iter.value().nexthop == nexthop) {
			res.push_back(iter.key());
		}
	}
	return res;
}

// returns wether, according to RFC 6.1, sequencenumber first is larger than sequencenumber second
bool AODVNeighbours::largerSequenceNumber(uint32_t first, uint32_t second){
	// "comparison MUST be done using signed 32-bit artihmetic"
	int32_t signedFirst = static_cast<int32_t>(first);
	int32_t signedSecond = static_cast<int32_t>(second);
	return signedFirst > signedSecond;
}

// macro magic to use bighashmap
#include <click/bighashmap.cc>
#if EXPLICIT_TEMPLATE_INSTANCES
template class HashMap<IPPair, void *>;
template class HashMap<IPAddress, int>;
#endif

CLICK_ENDDECLS

EXPORT_ELEMENT(AODVNeighbours)

