/**
* forward packets to the network
*/
elementclass OutputEth0{
	input[0]
		-> paint :: PaintSwitch;
	paint[0] 
		-> q :: Queue(2000);
	paint[2] 
		-> ToSimDump("s", MESSAGE_TYPE RREP) 
		-> q;
	paint[3] 
		-> DecIPTTL 
		-> ToSimDump("s", MESSAGE_TYPE raw) 
		-> q;
	paint[1]
		-> DecIPTTL 
		-> ToSimDump("f", MESSAGE_TYPE raw) 
		-> q;
	q
		-> ToDump("toeth0.dump",PER_NODE true)
		-> ToSimDevice(eth0);
}

/**
*	get data from network and drop data for other ethernet addresses
*/
elementclass InputEth0{
	$myaddr_ethernet |
	FromSimDevice(eth0,4096)
		-> ToDump("input.dump",PER_NODE true)
		-> HostEtherFilter($myaddr_ethernet, DROP_OWN false, DROP_OTHER true)
		//-> ToDump("inputnahostetherfilter.dump",PER_NODE true)
		-> [0]output
}

/**
*	get IP data from system and forward to router
*/
elementclass InputSystem{
	FromSimDevice(tap0,4096)
		-> CheckIPHeader
		-> MarkIPHeader
		-> ToSimDump("r", MESSAGE_TYPE raw)
		//-> ToDump("ipfromtun.dump",PER_NODE true, ENCAP IP)
		-> [0]output;
}

/**
* forward packets to the system
*/
elementclass OutputSystem{
	input[0]
		-> Queue(2000)
		-> CheckIPHeader
		//-> ToDump("iptotun.dump",PER_NODE true, ENCAP IP)
		-> ToSimDevice(tap0);
}
outputsystem::OutputSystem;

/**
* Classify ARP and others
* input[0] from network
* output[0] to network (ARP responses from me)
* output[1] to IP data processor
* output[2] to ARP querier (ARP responses from others)
*/
elementclass ClassifyARP{
	$myaddr, $myaddr_ethernet |
	
	// classifier output 0: ARP requests, output 1: ARP replies, output 2: rest
	input[0] 
		-> arpclass :: Classifier(12/0806 20/0001, 12/0806 20/0002, -);
	
	// reply requests and output to network
	arpclass[0] 
		-> ARPResponder($myaddr, $myaddr_ethernet)
		-> [0]output;
		
	arpclass[1] 
		-> [2]output
	
	arpclass[2] 
		-> CheckIPHeader(OFFSET 14)
		-> MarkIPHeader(14)
		//-> ToDump("ipdata",PER_NODE true)
		-> [1]output;
}

/**
*	classify AODV and not-AODV
*	input[0] IP Packets
*	output[0] AODV traffic
*	output[1] not AODV traffic
*/
elementclass ClassifyIP {
	input[0]
		//-> ToDump("niet arp.dump",PER_NODE true)
		-> aodvpackets :: IPClassifier(dst udp port 654, -);
	
	aodvpackets[0]
		//-> ToDump("aodv.dump",PER_NODE true)
		-> [0]output
		
	aodvpackets[1]
		-> ToDump("data.dump",PER_NODE true)
		-> [1]output
}

/**
* Filter out data for localhost, the other data we are supposed to forward go to route lookup
* input: data packets
* output: packets not destined for localhost
*/ 
elementclass FilterLocalhost{
	$myaddr |
	input
		-> localhost :: IPClassifier(dst host $myaddr, - );
	localhost[0] 
		-> StripToNetworkHeader
		-> [1]output;
	localhost[1] 
		-> [0]output;
}

/**
* classify the AODV packets
* input[0] AODV packets
* output[0] RREQ
* output[1] RRER
* output[2] HELLO
* output[3] RREP
* output[4] wrong type
*
*/
elementclass ClassifyAODV{
	// distinguish AODV message type: 0: RREQ, 1: RERR, 2: HELLO (TTL = 1), 3: RREP, 4: wrong type
	input[0]
		-> ToSimDump("r")
		-> aodvmessagetype :: Classifier(42/01, 42/03, 42/02 22/01, 42/02, - )
	
	aodvmessagetype[0]
		-> [0]output;
	aodvmessagetype[1]
		-> [1]output;
	aodvmessagetype[2]
		-> [2]output;
	aodvmessagetype[3]
		-> [3]output;
	aodvmessagetype[4]
		-> [4]output;
}

/**
* handle route discovery
* input[0] packets needing route discovery
* input[1] RREP
* output[0] packets with discovered routes (without ethernet header)
* output[1] timed out packets, to ICMP error generation
*/
elementclass RouteDiscovery{
	$genrreq |
	input[0]
		-> [0]discovery :: AODVWaitingForDiscovery($genrreq,neighbours);
	input[1]
		-> [1]discovery;
	discovery[0]
		-> [0]output;
	discovery[1]
		-> [1]output;
		
		
	AODVLinkNeighboursDiscovery(neighbours,discovery);
}

AddressInfo(me0 eth0:simnet);
output :: OutputEth0;


neighbours :: AODVNeighbours(me0);

rerr:: AODVGenerateRERR(neighbours)
	-> UDPIPEncap(me0, 654, 255.255.255.255, 654)
	-> EtherEncap(0x0800, me0, ff:ff:ff:ff:ff:ff)
	-> ToSimDump("s", MESSAGE_TYPE RERR)
	-> output;

hello::AODVHelloGenerator(neighbours)
	-> EtherEncap(0x0800, me0, ff:ff:ff:ff:ff:ff)
	-> ToSimDump("s", MESSAGE_TYPE HELLO)
	-> output;
hello[1]
	-> ToSimDump("s", MESSAGE_TYPE RREQ)
	-> output;

genrreq :: AODVGenerateRREQ(neighbours,knownclassifier)
	-> EtherEncap(0x0800, me0, ff:ff:ff:ff:ff:ff)
	//-> ToDump("rreqout.dump",PER_NODE true)
	-> hello;

knownclassifier :: AODVKnownClassifier(neighbours);

arpclass :: ClassifyARP(me0,me0);
ipclass :: ClassifyIP;
aodvclass :: ClassifyAODV;
lookup :: AODVLookUpRoute(neighbours);
arpquerier :: ARPQuerier(me0);
routediscovery :: RouteDiscovery(genrreq);
destinationclassifier :: AODVDestinationClassifier(neighbours);
setrrepheaders::AODVSetRREPHeaders()
routereply :: AODVGenerateRREP(neighbours,setrrepheaders);

InputEth0(me0) 
	-> arpclass;
arpclass[0] 
	//-> ToDump("arpreply.dump",PER_NODE true)
	-> output;
arpclass[1]
	-> AODVTrackNeighbours(rerr, neighbours)
	-> ipclass;
arpclass[2]
	-> [1]arpquerier;

arpquerier
	//-> ToDump("arpoutput.dump",PER_NODE true)
	-> output;

ipclass[0] 
	-> aodvclass;
localhost::FilterLocalhost(me0);
ipclass[1] 
	-> ToSimDump("r", MESSAGE_TYPE raw)
	-> localhost;
localhost[0]
	-> Paint(1) // mark packets to be forwarded
	-> lookup;
localhost[1]
	-> outputsystem;

aodvclass[0]  // RREQ
	-> AODVUpdateNeighbours(neighbours)
	-> knownclassifier;
aodvclass[1] //RERR
	-> [1]rerr;
aodvclass[2] // HELLO
	-> AODVUpdateNeighbours(neighbours)
	-> Discard;
aodvclass[3] // RREP
	-> AODVUpdateNeighbours(neighbours)
	-> destinationclassifier;
aodvclass[4] // wrong type so unusable
	-> Discard; 

knownclassifier[0]
	//-> ToDump("knownrreq.dump",PER_NODE true)
	-> routereply;
knownclassifier[1]
	-> SetIPChecksum
	//-> ToDump("unknownrreq.dump",PER_NODE true)
	-> ToSimDump("s", MESSAGE_TYPE RREQ)
	-> output;

routereply
	-> UDPIPEncap(me0, 654, 255.255.255.255, 654)
	-> setrrepheaders
	-> SetUDPChecksum
	-> SetIPChecksum
	-> ToSimDump("s", MESSAGE_TYPE RREP)
	-> [0]arpquerier;

destinationclassifier[0]
	-> [1]routediscovery;
destinationclassifier[1]
	-> SetIPChecksum  // ip_src and ip_dst are changed
	-> StripToNetworkHeader
	-> Paint(2) // distinguish RREPs for precursors
	-> [0]arpquerier;
destinationclassifier[2] // no nexthop -> discovery
	-> Paint(2) // distinguish RREPs for precursors
	-> [0]routediscovery;
	
InputSystem 
	-> Paint(3)
	-> lookup;
lookup[0] // known destination, dest IP annotation set
	-> StripToNetworkHeader
	-> ToDump("lookedup.dump",PER_NODE true, ENCAP IP)
	->[0]arpquerier; 
lookup[1] // unknown destination, routediscovery
	-> [0]routediscovery;
lookup[2]
	-> rerr;
routediscovery[0]
	-> SetIPChecksum
	-> StripToNetworkHeader
	//-> ToDump("discovered.dump",PER_NODE true, ENCAP IP)
	-> [0]arpquerier;
routediscovery[1]
	-> ICMPError(me0,3,1)
//	-> outputsystem; temporarily disable due to ns not understanding ICMP
	-> Print("timed out")
	-> Discard;
