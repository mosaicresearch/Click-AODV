/*
 * AODVKnownClassifier.{cc,hh} -- classify incoming (RREQ) AODV packets
 * Bart Braem
 *
 */

// ALWAYS INCLUDE <click/config.h> FIRST
#include <click/config.h>
#include <click/error.hh>
#include <click/confparse.hh>
#include <clicknet/ip.h>

#include "click_aodv.hh"
#include "aodv_knownclassifier.hh"

CLICK_DECLS
AODVKnownClassifier::AODVKnownClassifier()
{
}

AODVKnownClassifier::~AODVKnownClassifier()
{
}

int
AODVKnownClassifier::configure(Vector<String> &conf, ErrorHandler *errh)
{
	int res = cp_va_kparse(conf, this, errh,
		"NEIGHBOURS", cpkP+cpkM, cpElementCast, "AODVNeighbours", &neighbour_table,
		cpEnd);
	if(res < 0) return res;
	myIP = &neighbour_table->getMyIP();
	return res;
}

void AODVKnownClassifier::push (int port, Packet * p){
	assert(port == 0);
	assert(p);
	WritablePacket * packet = p->uniqueify();
	assert(packet);
	aodv_rreq_header * rreq = (aodv_rreq_header*) (packet->data() + aodv_headeroffset);
	
	uint32_t rreqid = ntohl(rreq->rreqid);
	
	String key = String(long(rreqid));
	key += ' ';
	key += IPAddress(rreq->originator).s();
	
	// check RREQ buffer according to RFC 6.3	
	if (RREQBuffer.find_pair(key)){
		packet->kill();
		// click_chatter("discarded");
		return;
	}
	addKnownRREQ(key); // buffer for next time
	
	// increment hopcount according to RFC 6.5
	++rreq->hopcount;
	
	const click_ip * ipheader = packet->ip_header();
	
	uint32_t newlifetime = (2 * AODV_NET_TRAVERSAL_TIME) - (2 * rreq->hopcount * AODV_NODE_TRAVERSAL_TIME);
	
	neighbour_table->updateRoutetableEntry(rreq->originator, ntohl(rreq->originatorseqnr),rreq->hopcount, ipheader->ip_src, newlifetime);
	
	// RFC 6.5: "Whenever a RREQ message is received, ..." be certain, do update again
	neighbour_table->addLifeTime(rreq->originator,newlifetime);
	
	bool destinationOnly = rreq->jrgdureserved & (1 << 4);
	IPAddress* next = neighbour_table->nexthop(rreq->destination);
	uint32_t * storedSeqNr = neighbour_table->getSequenceNumber(rreq->destination);
	
	if (rreq->destination == *myIP || (!destinationOnly && next && (*storedSeqNr == ntohl(rreq->destinationseqnr) || AODVNeighbours::largerSequenceNumber(*storedSeqNr,ntohl(rreq->destinationseqnr))))){
		//click_chatter("destination found, replying");
		
		if(next) neighbour_table->addPrecursor(*next,rreq->destination); // RFC 6.2
		
		output(0).push(packet);
	} else {
		// RFC 6.5: "if a node does not generate a RREP...: update to maximum"
		if (storedSeqNr && AODVNeighbours::largerSequenceNumber(*storedSeqNr,ntohl(rreq->destinationseqnr))) {
			rreq->destinationseqnr = htonl(*storedSeqNr);
		}
		click_ip * ipheader = packet->ip_header();
		if (ipheader->ip_ttl > 1) {
			--ipheader->ip_ttl;
			ipheader->ip_src = myIP->in_addr();
			output(1).push(packet);
		} else {
			// time's up, kill
			packet->kill();
		}
	}
	delete next; // free memory
	delete storedSeqNr;
}

void AODVKnownClassifier::handleExpiry(Timer*, void * data){
	TimerData * timerdata = (TimerData*) data;
	assert(timerdata);
	timerdata->known->expire(timerdata->key);
	delete timerdata;
}

void AODVKnownClassifier::expire(const String & key){
	PastRREQMap::Pair* pair = RREQBuffer.find_pair(key);
	assert(pair);
	delete(pair->value);
	RREQBuffer.remove(key);
}

void AODVKnownClassifier::addKnownRREQ(const String & key){
	TimerData* timerdata = new TimerData();
	timerdata->known = this;
	timerdata->key = key;
	
	Timer * timer = new Timer(&AODVKnownClassifier::handleExpiry,timerdata);
	timer->initialize(this);
	timer->schedule_after_msec(AODV_PATH_DISCOVERY_TIME);
	
	RREQBuffer.insert(key,timer);
}

void AODVKnownClassifier::addKnownRREQ(uint32_t id, const IPAddress & ip){
	String key = String(long(id));
	key += ' ';
	key += ip.s();

	addKnownRREQ(key);
}

// macro magic to use bighashmap
#include <click/bighashmap.cc>
#if EXPLICIT_TEMPLATE_INSTANCES
template class HashMap<String, Timer*>;
#endif


CLICK_ENDDECLS

EXPORT_ELEMENT(AODVKnownClassifier)

