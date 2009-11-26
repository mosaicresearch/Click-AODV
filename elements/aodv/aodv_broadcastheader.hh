/*
  =c
  AODV specific element, adds a UDP/IP broadcastheader (with AODV port) to a packet

  =s
  AODV

  =io
  Not a real element
  
  =processing
  NONE

  =d
  
  =a
  AODVGenerateRREQ, AODVHelloGenerator
*/
#ifndef AODVBROADCASTHEADER_HH
#define AODVBROADCASTHEADER_HH
#include <click/element.hh>

CLICK_DECLS

class AODVBroadcastHeader{
	public:
		static Packet * setBroadcastHeader(Packet*, const IPAddress &, int);
	private:
};

CLICK_ENDDECLS
#endif
