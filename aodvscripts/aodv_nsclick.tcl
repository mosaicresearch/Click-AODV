#
# Set some general simulation parameters
#

#
# Unity gain, omnidirectional antennas, centered 1.5m above each node.
# These values are lifted from the ns-2 sample files.
#
Antenna/OmniAntenna set X_ 0
Antenna/OmniAntenna set Y_ 0
Antenna/OmniAntenna set Z_ 1.5
Antenna/OmniAntenna set Gt_ 1.0
Antenna/OmniAntenna set Gr_ 1.0

#
# Initialize the SharedMedia interface with parameters to make
# it work like the 914MHz Lucent WaveLAN DSSS radio interface
# These are taken directly from the ns-2 sample files.
#
# capture threshold (db)
Phy/WirelessPhy set CPThresh_ 	10.0
# carrier sense threshold (W)
Phy/WirelessPhy set CSThresh_ 	1.559e-11
# receive power threshold (W)
Phy/WirelessPhy set RXThresh_	7.33648e-10
Phy/WirelessPhy set Rb_ 	2*1e6
# Pt -- transmitted signal power
Phy/WirelessPhy set Pt_ 	1
# frequency
Phy/WirelessPhy set freq_ 	914e+6
# L -- system loss (L >= 1)
Phy/WirelessPhy set L_	 	1.0

# 
# Set the size of the playing field and the topography.
#
set xsize 1000
set ysize 1000
set wtopo	[new Topography]
$wtopo load_flatgrid $xsize $ysize

#
# The network channel, physical layer, MAC, propagation model,
# and antenna model are all standard ns-2.
#  
set netchan	Channel/WirelessChannel
set netphy	Phy/WirelessPhy
set netmac	Mac/802_11
set netprop     Propagation/TwoRayGround
set antenna     Antenna/OmniAntenna

Mac/802_11 set dataRate_ 1e6

#
# We have to use a special queue and link layer. This is so that
# Click can have control over the network interface packet queue,
# which is vital if we want to play with, e.g. QoS algorithms.
#
set netifq	Queue/ClickQueue
set netll	LL/Ext
LL set delay_			1ms

#
# These are pretty self-explanatory, just the number of nodes.
# and when we'll stop
#
set nodecount 		6

set stoptime 		300

#
# With nsclick, we have to worry about details like which network
# port to use for communication. This sets the default ports to 5000.
#
Agent/Null set sport_		5000
Agent/Null set dport_		5000

Agent/CBR set sport_		5000
Agent/CBR set dport_		5000

#
# Standard ns-2 stuff here - create the simulator object.
#
Simulator set MacTrace_ OFF
set ns_		[new Simulator]

#
# Create and activate trace files.
#
set tracefd	[open "aodv_nsclick.tr" w]
$ns_ trace-all $tracefd
$ns_ use-newtrace

# AODV trace support
set T [new Trace/Generic]
$T target [$ns_ set nullAgent_]
$T attach $tracefd

#
# Create the "god" object. This is another artifact of using
# the mobile node type. We have to have this even though
# we never use it.
#
set god_ [create-god $nodecount]

#
# Tell the simulator to create Click nodes.
#
Simulator set node_factory_ Node/MobileNode/ClickNode

#
# Create a network Channel for the nodes to use. One channel
# per LAN. Also set the propagation model to be used.
#
set chan_1_ [new $netchan]
set chan_2_ [new $netchan]
set prop_ [new $netprop]

#
# In nsclick we have to worry about assigning IP and MAC addresses
# to out network interfaces. Here we generate a list of IP and MAC
# addresses, one per node since we've only got one network interface
# per node in this case. Also note that this scheme only works for
# fewer than 255 nodes, and we aren't worrying about subnet masks.
#

set iptemplate "192.168.0.%d"
set mactemplate "00:04:57:80:89:%0x"
for {set i 0} {$i < $nodecount} {incr i} {
	set node_ip($i)	[format $iptemplate [expr $i+1]]
	set node_mac($i)	[format $mactemplate [expr $i+1]]
}

#
# We set the routing protocol to "Empty" so that ns-2 doesn't do
# any packet routing. All of the routing will be done by the
# Click script.
#
$ns_ rtproto Empty

#
# Here is where we actually create all of the nodes.
#
for {set i 0} {$i < $nodecount} {incr i} {
    set node_($i) [$ns_ node]

    #
    # After creating the node, we add one wireless network interface to
    # it. By default, this interface will be named "eth0". If we
    # added a second interface it would be named "eth1", a third
    # "eth2" and so on.
    #
    $node_($i) add-interface $chan_1_ $prop_ $netll $netmac $netifq 1 $netphy $antenna

    #
    # Now configure the interface eth0
    #
    $node_($i) setip "eth0" $node_ip($i)
    $node_($i) setmac "eth0" $node_mac($i)

    #
    # Set some node properties
    #

    $node_($i) topography $wtopo
    $node_($i) nodetrace $tracefd
    
    # The node name is used by Click to distinguish information
    # coming from different nodes. For example, a "Print" element
    # prepends this to the printed string so it's clear exactly
    # which node is doing the printing.
    #
    [$node_($i) set classifier_] setnodename "$node_ip($i)-aodv"
    
    # Load the appropriate Click router script for the node.
    # All nodes in this simulation are using the same script,
    # but there's no reason why each node couldn't use a different
    # script.
    #
    $T set src_ [$node_($i) id]
    [$node_($i) entry] tracetarget $T
    [$node_($i) entry] loadclick "aodv.click"
}


#
# Define node network traffic. There isn't a whole lot going on
# in this simple test case, we're just going to have the first node
# send packets to the last node, starting at 1 second, and ending at 10.
# There are Perl scripts available to automatically generate network
# traffic.
#


#
# Start transmitting at $startxmittime, $xmitrate packets per second.
#
set startxmittime 20
set xmitrate 4
set xmitinterval 0.1
set packetsize 64

#
# We use the "raw" packet type, which sends real packet data
# down the pipe.
#
set origin 1
set destination 3

set raw_(0) [new Agent/Raw]
$ns_ attach-agent $node_($origin) $raw_(0)

set null_(0) [new Agent/Null]
$ns_ attach-agent $node_($destination) $null_(0)

#
# The CBR object is just the default ns-2 CBR object, so
# no change in the meaning of the parameters.
#
set cbr_(0) [new Application/Traffic/CBR]
$cbr_(0) set packetSize_ $packetsize
$cbr_(0) set interval_ $xmitinterval
$cbr_(0) set maxpkts_ [expr ($stoptime - $startxmittime)*$xmitrate]
$cbr_(0) attach-agent $raw_(0)

#
# The Raw agent creates real UDP packets, so it has to know
# the source and destination IP addresses and port numberes.
#
$raw_(0) set-srcip [$node_($origin) getip eth0]
$raw_(0) set-srcport 5000
$raw_(0) set-destport 5000
$raw_(0) set-destip [$node_($destination) getip eth0]

$ns_ at $startxmittime "$cbr_(0) start"


	$node_(0) 	set X_ 		0
	$node_(0) 	set Y_ 		0
	$node_(0) 	set Z_ 		0
	
	$node_(1) 	set X_ 		450
	$node_(1) 	set Y_ 		0
	$node_(1) 	set Z_ 		0
	
	$node_(2) 	set X_ 		0
	$node_(2) 	set Y_ 		400
	$node_(2) 	set Z_ 		0
	
	$node_(3) 	set X_ 		200
	$node_(3) 	set Y_ 		500
	$node_(3) 	set Z_ 		0
	
	$node_(4) 	set X_ 		200
	$node_(4) 	set Y_ 		200
	$node_(4) 	set Z_ 		0
	
	$node_(5) 	set X_ 		200
	$node_(5) 	set Y_ 		0
	$node_(5) 	set Z_ 		0

	
# Start nodes at 0
#
for {set i 0} {$i < $nodecount} {incr i} {
	$node_($i) start ;# necessary for movement tracing
	$ns_ at 0 "[$node_($i) entry] runclick"
}

#
# Stop the simulation
#
$ns_ at  $stoptime "puts \"NS EXITING...\" ; $ns_ halt"

puts "Starting Simulation..."
$ns_ run
