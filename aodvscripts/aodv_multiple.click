/**
*	get IP data from system and forward to router and vice versa
*/
elementclass System{
	$myname, $myip,$mydst |
	
		ICMPPingSource($myip, $mydst)
		-> CheckIPHeader
		-> MarkIPHeader
		-> [0]output;
		
	input[0]
		-> Strip(14)
		-> EtherEncap(0x0800, 1:1:1:1:1:1, fake) // ensure ethernet for kernel
		-> CheckIPHeader(14)
		-> MarkIPHeader(14)
		-> ToDump(forme.dump);

}

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
		-> aodvpackets :: IPClassifier(dst udp port 654, -);
	
	aodvpackets[0]
		-> [0]output;
		
	aodvpackets[1]
		-> [1]output;
}

/**
* 	Filter out data for localhost, the other data we are supposed to forward go to route lookup
* 	input: data packets
* 	output[0]: packets destined for localhost
* 	output[1]: packets not destined for localhost
*	output[2]: ICMP echo replies
*/ 
elementclass FilterLocalhost{
	$myaddr |
	input
		-> localhost :: IPClassifier(dst host $myaddr, - );
	localhost[0] 
		-> ping :: ICMPPingResponder;
	ping[0]
		-> CheckICMPHeader
		-> Strip(14)
		-> CheckIPHeader
		-> MarkIPHeader
		-> [2]output;
	ping[1]
		-> [0]output;
	localhost[1] 
		-> [1]output;
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
		-> aodvmessagetype :: Classifier(42/01, 42/03, 42/02 22/01, 42/02, - );
	
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

elementclass AODV{
	$myname, $myip,$mydst,$myeth |
	
	AddressInfo(
		fake	$myip	$myeth
	);

	

	neighbours :: AODVNeighbours(fake);

	rerr:: AODVGenerateRERR(neighbours)
		-> UDPIPEncap(fake, 654, 255.255.255.255, 654)
		-> EtherEncap(0x0800, fake, ff:ff:ff:ff:ff:ff)
		-> output;

	hello::AODVHelloGenerator(neighbours)
		-> EtherEncap(0x0800, fake, ff:ff:ff:ff:ff:ff)
		-> output;
	hello[1]
		-> output;

	genrreq :: AODVGenerateRREQ(neighbours,knownclassifier)
		-> EtherEncap(0x0800, fake, ff:ff:ff:ff:ff:ff)
		-> hello;

	knownclassifier :: AODVKnownClassifier(neighbours);

	arpclass :: ClassifyARP(fake,fake);
	ipclass :: ClassifyIP;
	aodvclass :: ClassifyAODV;
	lookup :: AODVLookUpRoute(neighbours);
	arpquerier :: ARPQuerier(fake);
	routediscovery :: RouteDiscovery(genrreq);
	destinationclassifier :: AODVDestinationClassifier(neighbours);
	setrrepheaders::AODVSetRREPHeaders()
	routereply :: AODVGenerateRREP(neighbours,setrrepheaders);

	input[0]
		-> HostEtherFilter(fake, DROP_OWN false, DROP_OTHER true)
		-> arpclass;
	arpclass[0] 
		-> output;
	arpclass[1]
		-> AODVTrackNeighbours(rerr, neighbours)
		-> ipclass;
	arpclass[2]
		-> [1]arpquerier;

	arpquerier
		-> output;

	ipclass[0] 
		-> aodvclass;
	localhost::FilterLocalhost(fake);
	ipclass[1] 
		-> localhost;
	localhost[0]
		-> system :: System($myname, $myip,$mydst);
	localhost[1]
		-> Paint(1) // mark packets to be forwarded
		-> lookup;
	localhost[2]
		-> arpquerier;

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
		-> routereply;
	knownclassifier[1]
		-> SetIPChecksum
		-> output;

	routereply
		-> UDPIPEncap(fake, 654, 255.255.255.255, 654)
		-> setrrepheaders
		-> SetUDPChecksum
		-> SetIPChecksum
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
	
	system
		-> Paint(3)
		-> lookup;

	lookup[0] // known destination, dest IP annotation set
		-> StripToNetworkHeader
		->[0]arpquerier; 
	lookup[1] // unknown destination, routediscovery
		-> [0]routediscovery;
	lookup[2]
		-> rerr;
	routediscovery[0]
		-> SetIPChecksum
		-> StripToNetworkHeader
		-> [0]arpquerier;
	routediscovery[1]
		-> ICMPError(fake,3,1)
		-> system;
}

a::AODV('a', 10.0.0.1, 10.0.0.2, 00:0E:0C:6D:FF:BF);
b::AODV('b', 10.0.0.2, 10.0.0.1, 00:0E:0C:6D:FF:BD);

a -> b -> a;
