/*
 * =c
 * AODVPacketAnalyzer
 * =s AODV
 * =a ToSimDump
 * =d
 *
 * This element analyzes the packet type of an AODV packet
 */
#ifndef AODVPACKETANALYZER_HH
#define AODVPACKETANALYZER_HH
#include <click/element.hh>

CLICK_DECLS

class AODVPacketAnalyzer{
	public:
		static int getMessageType(Packet*);
		static String getMessageString(Packet *packet);
		static int getMessageType(const String &);
		static String getTypeString(int);
		
	private:
};

CLICK_ENDDECLS
#endif
