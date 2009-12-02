/*
 * AODVWaitingForDiscovery.{cc,hh} -- keep packets waiting for discovery
 * Bart Braem
 *
 */

// ALWAYS INCLUDE <click/config.h> FIRST
#include <click/config.h>
#include <click/error.hh>
#include <click/confparse.hh>
#include <click/packet_anno.hh>
#include <clicknet/ip.h>

#include "aodv_waitingfordiscovery.hh"
#include "click_aodv.hh"

CLICK_DECLS

AODVWaitingForDiscovery::AODVWaitingForDiscovery()
{
}

AODVWaitingForDiscovery::~AODVWaitingForDiscovery()
{
}

int
AODVWaitingForDiscovery::configure(Vector<String> &conf, ErrorHandler *errh)
{
    return cp_va_kparse(conf, this, errh,
		"GENERATERREQ", cpkP+cpkM, cpElementCast, "AODVGenerateRREQ", &rreq,
		"NEIGHBOURS", cpkP+cpkM, cpElementCast, "AODVNeighbours", &neighbour_table,
		cpEnd);
}

void AODVWaitingForDiscovery::runTask(const IPAddress & destination, TimerData* data){
	Buffer::Pair* pair = buffer.find_pair(destination);
	assert(pair); // wrong timers are horrible, this might fail if a reply is received immediately
	assert(pair->value);
	assert(pair->value->timer);
	assert(!pair->value->timer->scheduled());
	
	if (pair->value->maxTTL && pair->value->nrOfRetries == AODV_RREQ_RETRIES){
		delete(pair->value->timer);
		pair->value->timer = 0;
		
		// it's over, clean up everything
		Vector<Packet*>::iterator iter = pair->value->packets.begin();
		// at least one packet is still waiting, use it to generate ICMP error
		output(1).push(*iter); 
		iter = pair->value->packets.erase(iter);
		
		// drop all other packets from buffer
		while(iter != pair->value->packets.end()) {
			(*iter)->kill();
			iter = pair->value->packets.erase(iter);
		}
		
		delete pair->value;
		
		buffer.remove(destination);
		assert(!buffer.find_pair(destination));
		
		delete data;
	}
	else{	
		if (pair->value->ttl < AODV_TTL_TRESHOLD) {
			pair->value->ttl = pair->value->ttl + AODV_TTL_INCREMENT;
			rreq->generateRREQ(destination,false,pair->value->ttl);
			assert(pair->value->timer);
			pair->value->timer->schedule_after_msec(AODV_RING_TRAVERSAL_TIME_FACTOR * (pair->value->ttl + AODV_TIMEOUT_BUFFER));
			
		} else {
			pair->value->ttl = AODV_NET_DIAMETER;
			pair->value->maxTTL = true;
			++pair->value->nrOfRetries;
			rreq->generateRREQ(destination,false,pair->value->ttl);
			pair->value->timer->schedule_after_msec(AODV_RING_TRAVERSAL_TIME_FACTOR * (pair->value->ttl + AODV_TIMEOUT_BUFFER));
		}
	}
}

void AODVWaitingForDiscovery::handleTask(Timer* , void * data){
	TimerData * timerdata = (TimerData*) data;
	assert(timerdata);
	timerdata->waitingForDiscovery->runTask(timerdata->destination,timerdata);
}

// next hop for destination is nexthop
void AODVWaitingForDiscovery::newKnownDestination(const IPAddress & destination, const IPAddress & nexthop){
	if (Buffer::Pair* pair = buffer.find_pair(destination)) {
		pair->value->timer->unschedule();
		delete(pair->value->timer);
		
		for(Vector<Packet*>::iterator iter = pair->value->packets.begin(); iter != pair->value->packets.end(); ){
			// RFC 6.2
			if (PAINT_ANNO(*iter) == 2){ // forwarding of RREP
				const click_ip * ipheader = (*iter)->ip_header();
				assert(ipheader);
				aodv_rrep_header * rrep = (aodv_rrep_header*) ((*iter)->data() + aodv_headeroffset);
								
				// RFC 6.7 last paragraph: 
				// destination contains next hop (towards destination)
				if(!neighbour_table->getSequenceNumber(rrep->destination)){
					// the information might be outdated, update it
					neighbour_table->updateRoutetableEntry(IPAddress(rrep->destination), ntohl(rrep->destinationseqnr), rrep->hopcount, IPAddress(ipheader->ip_src), ntohl(rrep->lifetime));
				}
				neighbour_table->addPrecursor(rrep->destination,nexthop); 
				// nexthop towards destination contains next hop towards source
				neighbour_table->addPrecursor(nexthop,ipheader->ip_src); 
			}
			
			(*iter)->set_dst_ip_anno(nexthop);
			
			if(PAINT_ANNO(*iter) == 1 || PAINT_ANNO(*iter) == 3){ // forwarded packet
				const click_ip * ipheader = (*iter)->ip_header();
				assert(ipheader);
				neighbour_table->updateRouteLifetime(ipheader->ip_src,ipheader->ip_dst);
				output(0).push(*iter);
			} else if (PAINT_ANNO(*iter) == 2){ // RREP needs changed destination
				WritablePacket* writable = (*iter)->uniqueify();
				assert(writable->ip_header());
				writable->ip_header()->ip_dst = nexthop.in_addr();
				writable->set_dst_ip_anno(nexthop);
				output(0).push(writable);
			} else assert(false); // only data or RREPs allowed here
			
			iter = pair->value->packets.erase(iter);
		}
		
		delete pair->value;
		
		buffer.remove(destination);
		assert(!buffer.find_pair(destination));
	} else {
		// route already discovered / no waiting packets, pity
	}
}

void AODVWaitingForDiscovery::push (int port, Packet * packet){
	// insertion happens at the back, traversal from front: FIFO
	assert(port >= 0 && port <= 1);
	assert(packet);
	if (port == 0){ //DATA
		assert(packet->dst_ip_anno());
		if (Buffer::Pair* pair = buffer.find_pair(packet->dst_ip_anno())){
			// destinations already being looked up so join the club
			pair->value->packets.push_back(packet);
		} else {
			rreq->generateRREQ(packet->dst_ip_anno(),false,AODV_TTL_START);
			WaitingPackets* waiting = new WaitingPackets();
			waiting->packets.push_back(packet);
			waiting->nrOfRetries = 0;
			// besides using the old hopcount everything is the same for previous and new destinations
			int8_t hopcount = neighbour_table->getHopcount(packet->dst_ip_anno());
			if(hopcount != -1){
				waiting->ttl = hopcount + AODV_TTL_INCREMENT;
			} else {
				waiting->ttl = AODV_TTL_START;
			}
			waiting->maxTTL = false;
			
			TimerData* data = new TimerData();
			data->destination = packet->dst_ip_anno();
			data->waitingForDiscovery = this;
			
			waiting->timer = new Timer(&AODVWaitingForDiscovery::handleTask,data); // run handletask when timer goes off
			waiting->timer->initialize(this);
			waiting->timer->schedule_after_msec(AODV_RING_TRAVERSAL_TIME_FACTOR * (AODV_TTL_START + AODV_TIMEOUT_BUFFER));
			
			buffer.insert(packet->dst_ip_anno(),waiting);
			
			neighbour_table->addLifeTime(packet->dst_ip_anno(),2*AODV_NET_TRAVERSAL_TIME); //rfc 6.4 p 15
			
			//click_chatter("RREQ generated and waiting in %s",neighbour_table->getMyIP().s().c_str());
		}
	} else { //RREP
		assert(packet->ip_header());
		aodv_rrep_header * rrep = (aodv_rrep_header*) (packet->data() + aodv_headeroffset);
		
		//click_chatter("RREP from %s at destination (%s)", IPAddress(rrep->destination).s().c_str(), IPAddress(rrep->originator).s().c_str());
		
		const click_ip * ipheader = packet->ip_header();
		assert(ipheader);
		
		newKnownDestination(rrep->destination, ipheader->ip_src);
		
		packet->kill();
	}
}

// macro magic to use bighashmap
#include <click/bighashmap.cc>
#if EXPLICIT_TEMPLATE_INSTANCES
template class HashMap<long, WaitingPackets>;
#endif


CLICK_ENDDECLS

EXPORT_ELEMENT(AODVWaitingForDiscovery)

