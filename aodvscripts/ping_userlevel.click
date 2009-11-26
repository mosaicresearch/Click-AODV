AddressInfo(
	fake	192.168.0.1/16	00:0E:0C:6D:FF:AB
);


/**
* forward packets to the network
* input[0] packets for the network
*/
elementclass OutputEth0{
	input[0]
		-> Queue(2000)
		-> ToDevice(eth1);
}

/**
* get data from network and drop data for other ethernet addresses
* output[0] packets from the network
*/
elementclass InputEth0{
	$myaddr_ethernet |
	FromDevice(eth1, PROMISC true)
		-> HostEtherFilter($myaddr_ethernet, DROP_OWN false, DROP_OTHER true)
		-> output;
}


/**
*	get IP data from system and forward to router and vice versa
*/
elementclass System{
	tohost :: ToHost(fake0);
	
	FromHost(fake0, fake, ETHER fake)
		-> fromhost_cl :: Classifier(12/0806, 12/0800);
	fromhost_cl[0] 
		-> ARPResponder(0.0.0.0/0 1:1:1:1:1:1) 
		-> tohost;
	fromhost_cl[1]
		-> Strip(14)
		-> CheckIPHeader
		-> MarkIPHeader
		-> [0]output;
		
	input[0]
		-> Strip(14)
		-> EtherEncap(0x0800, 2:2:2:2:2:2, 1:1:1:1:1:1) // ensure ethernet for kernel
		-> tohost;

}


system::System;

/**
* Classify ARP and others
* input[0] from network
* output[0] to network (ARP responses from me)
* output[1] to IP data processor
* output[2] to ARP querier (ARP responses from others)
*/
elementclass ClassifyARP{
	$myaddr, $myaddr_ethernet |
	
	// output 0: ARP requests, output 1: ARP replies, output 2: rest
	input[0] 
		-> arpclass :: Classifier(12/0806 20/0001, 12/0806 20/0002, -);
	
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
		-> [1]output;
	ping[1]
		-> [0]output;
	localhost[1] 
		-> [1]output;
}

InputEth0(fake) 
	-> arpclass :: ClassifyARP(fake,fake);
arpclass[0] 
	-> output :: OutputEth0;
arpclass[1]
	-> localhost::FilterLocalhost(fake);
arpclass[2]
	-> [1]arpquerier :: ARPQuerier(fake);

system[0]
	-> arpquerier;

arpquerier
	-> output;

localhost[0]
	-> system
localhost[1]
	-> arpquerier;
