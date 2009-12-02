/*
 * AODVTrackNeighbours.{cc,hh} -- update neighbours' lifetime when receiving packets
 * Bart Braem
 *
 */

// ALWAYS INCLUDE <click/config.h> FIRST
#include <click/config.h>
#include <click/confparse.hh>
#include <click/error.hh>
#include <clicknet/ip.h>
#include "click_aodv.hh"

#include "aodv_trackneighbours.hh"

CLICK_DECLS
AODVTrackNeighbours::AODVTrackNeighbours()
{
}

AODVTrackNeighbours::~AODVTrackNeighbours()
{
}

int
AODVTrackNeighbours::configure(Vector<String> &conf, ErrorHandler *errh)
{
	int res = cp_va_kparse(conf, this, errh,
		"GENERATERERR", cpkP+cpkM, cpElementCast, "AODVGenerateRERR", &generateRerr,
		"NEIGHBOURS", cpkP+cpkM, cpElementCast, "AODVNeighbours", &neighbour_table,
		cpEnd);
	if(res < 0) return res;
	myIP = &neighbour_table->getMyIP();
	return res;
}

void AODVTrackNeighbours::handleExpiry(Timer*, void * data){
	TimerData * timerdata = (TimerData*) data;
	assert(timerdata);
	assert(timerdata->me);
	assert(timerdata->ip);
	timerdata->me->expire(timerdata->ip);
	delete timerdata->ip;
	delete timerdata;
	// delete all from timerdata but trackneighbours itself
}

// neighbour doesn't respond any more, send RERR
void AODVTrackNeighbours::expire(IPAddress* ip){
	assert(ip);
	TimerMap::Pair* pair = neighbour_timers.find_pair(*ip);
	assert(pair);
	delete(pair->value); // delete timer here
	neighbour_timers.remove(pair->key);
	
	if(Vector<IPAddress>* precursors = neighbour_table->getPrecursors(*ip)){
		Vector<uint32_t> seqNrs;
		Vector<IPAddress> ips;
		uint32_t* seqNr = neighbour_table->getSequenceNumber(*ip);
		seqNrs.push_back(*seqNr);
		ips.push_back(*ip); // add expired IP too, that's the main source for the RERR
		
		for(Vector<IPAddress>::iterator iter = precursors->begin(); iter != precursors->end(); ++iter){
			// it's possible to be your own precursor in case of HELLOs, for clean RERRs let's filter out those
			if(*iter != *ip){
				seqNr = neighbour_table->getSequenceNumber(*iter);
				assert(seqNr);
				seqNrs.push_back(*seqNr);
				ips.push_back(*iter);
				delete(seqNr); // clean garbage!
			}
		}
	
		generateRerr->generateRERR(true,ips,seqNrs);
		delete(precursors); // we got a pointer so clean it
	}
}

//RFC 6.9: keep track of hello messages, and react on "Hello messages or otherwise" so everything from that node
void AODVTrackNeighbours::push (int port, Packet * packet){
	assert(port == 0);
	assert(packet);
	const click_ip * ipheader = packet->ip_header();
	assert(ipheader);
	
	if (ipheader->ip_src != *myIP) {
		if (packet->length() == aodv_headeroffset + sizeof(aodv_rrep_header)) { //RREP or HELLO - wait with cast of data until now because expensive
		
			const aodv_rrep_header * rrep = (const aodv_rrep_header*) (packet->data() + aodv_headeroffset);
		
			// don't use RERR information, must be AODV type 2, ttl 1 and non-existing entry
			if (rrep->type == 2 && ipheader->ip_ttl == 1 && !neighbour_timers.find_pair(rrep->originator)) {
				TimerData* timerdata = new TimerData();
				timerdata->ip = new IPAddress(rrep->originator);
				timerdata->me = this;
				Timer * timer = new Timer(&AODVTrackNeighbours::handleExpiry,timerdata);
				timer->initialize(this);
				timer->schedule_after_msec(AODV_ALLOWED_HELLO_LOSS * AODV_HELLO_INTERVAL);
				neighbour_timers.insert(rrep->originator,timer);
			}
		}
		TimerMap::Pair* pair = neighbour_timers.find_pair(ipheader->ip_src);
		if (pair) pair->value->schedule_after_msec(AODV_ALLOWED_HELLO_LOSS * AODV_HELLO_INTERVAL);
	}
	output(0).push(packet);
}

// macro magic to use bighashmap
#include <click/bighashmap.cc>
#if EXPLICIT_TEMPLATE_INSTANCES
template class HashMap<IPAddress, Timer*>;
#endif

CLICK_ENDDECLS

EXPORT_ELEMENT(AODVTrackNeighbours)

