# get the origin and destination parameters from the script

for {set i 0} {$i < $streamcount} {incr i} {
	set raw_($i) [new Agent/Raw]
	$ns_ attach-agent $node_($origin($i)) $raw_($i)
	
	set null_($i) [new Agent/Null]
	$ns_ attach-agent $node_($destination($i)) $null_($i)
	
	set cbr_($i) [new Application/Traffic/CBR]
	$cbr_($i) set packetSize_ $packetsize
	$cbr_($i) set interval_ $xmitinterval
	$cbr_($i) set maxpkts_ [expr ($stoptime - $startxmittime)*$xmitrate]
	$cbr_($i) attach-agent $raw_($i)
	
	$raw_($i) set-srcip [$node_($origin($i)) getip eth0]
	$raw_($i) set-srcport 5000
	$raw_($i) set-destport 5000
	$raw_($i) set-destip [$node_($destination($i)) getip eth0]
	
	$ns_ at $startxmittime "$cbr_($i) start"
}

$node_(0)	set X_ 0.0
$node_(0)	set Y_ 0.0
$node_(0)	set Z_ 0

$node_(1)	set X_ 0.0
$node_(1)	set Y_ 200.0
$node_(1)	set Z_ 0

$node_(2)	set X_ 0.0
$node_(2)	set Y_ 400.0
$node_(2)	set Z_ 0

$node_(3)	set X_ 0.0
$node_(3)	set Y_ 600.0
$node_(3)	set Z_ 0

$node_(4)	set X_ 0.0
$node_(4)	set Y_ 800.0
$node_(4)	set Z_ 0

$node_(5)	set X_ 0.0
$node_(5)	set Y_ 1000.0
$node_(5)	set Z_ 0

$node_(6)	set X_ 0.0
$node_(6)	set Y_ 1200.0
$node_(6)	set Z_ 0

$node_(7)	set X_ 0.0
$node_(7)	set Y_ 1400.0
$node_(7)	set Z_ 0

$node_(8)	set X_ 200.0
$node_(8)	set Y_ 0.0
$node_(8)	set Z_ 0

$node_(9)	set X_ 200.0
$node_(9)	set Y_ 200.0
$node_(9)	set Z_ 0

$node_(10)	set X_ 200.0
$node_(10)	set Y_ 400.0
$node_(10)	set Z_ 0

$node_(11)	set X_ 200.0
$node_(11)	set Y_ 600.0
$node_(11)	set Z_ 0

$node_(12)	set X_ 200.0
$node_(12)	set Y_ 800.0
$node_(12)	set Z_ 0

$node_(13)	set X_ 200.0
$node_(13)	set Y_ 1000.0
$node_(13)	set Z_ 0

$node_(14)	set X_ 200.0
$node_(14)	set Y_ 1200.0
$node_(14)	set Z_ 0

$node_(15)	set X_ 200.0
$node_(15)	set Y_ 1400.0
$node_(15)	set Z_ 0

$node_(16)	set X_ 400.0
$node_(16)	set Y_ 0.0
$node_(16)	set Z_ 0

$node_(17)	set X_ 400.0
$node_(17)	set Y_ 200.0
$node_(17)	set Z_ 0

$node_(18)	set X_ 400.0
$node_(18)	set Y_ 400.0
$node_(18)	set Z_ 0

$node_(19)	set X_ 400.0
$node_(19)	set Y_ 600.0
$node_(19)	set Z_ 0

$node_(20)	set X_ 400.0
$node_(20)	set Y_ 800.0
$node_(20)	set Z_ 0

$node_(21)	set X_ 400.0
$node_(21)	set Y_ 1000.0
$node_(21)	set Z_ 0

$node_(22)	set X_ 400.0
$node_(22)	set Y_ 1200.0
$node_(22)	set Z_ 0

$node_(23)	set X_ 400.0
$node_(23)	set Y_ 1400.0
$node_(23)	set Z_ 0

$node_(24)	set X_ 600.0
$node_(24)	set Y_ 0.0
$node_(24)	set Z_ 0

$node_(25)	set X_ 600.0
$node_(25)	set Y_ 200.0
$node_(25)	set Z_ 0

$node_(26)	set X_ 600.0
$node_(26)	set Y_ 400.0
$node_(26)	set Z_ 0

$node_(27)	set X_ 600.0
$node_(27)	set Y_ 600.0
$node_(27)	set Z_ 0

$node_(28)	set X_ 600.0
$node_(28)	set Y_ 800.0
$node_(28)	set Z_ 0

$node_(29)	set X_ 600.0
$node_(29)	set Y_ 1000.0
$node_(29)	set Z_ 0

$node_(30)	set X_ 600.0
$node_(30)	set Y_ 1200.0
$node_(30)	set Z_ 0

$node_(31)	set X_ 600.0
$node_(31)	set Y_ 1400.0
$node_(31)	set Z_ 0

$node_(32)	set X_ 800.0
$node_(32)	set Y_ 0.0
$node_(32)	set Z_ 0

$node_(33)	set X_ 800.0
$node_(33)	set Y_ 200.0
$node_(33)	set Z_ 0

$node_(34)	set X_ 800.0
$node_(34)	set Y_ 400.0
$node_(34)	set Z_ 0

$node_(35)	set X_ 800.0
$node_(35)	set Y_ 600.0
$node_(35)	set Z_ 0

$node_(36)	set X_ 800.0
$node_(36)	set Y_ 800.0
$node_(36)	set Z_ 0

$node_(37)	set X_ 800.0
$node_(37)	set Y_ 1000.0
$node_(37)	set Z_ 0

$node_(38)	set X_ 800.0
$node_(38)	set Y_ 1200.0
$node_(38)	set Z_ 0

$node_(39)	set X_ 800.0
$node_(39)	set Y_ 1400.0
$node_(39)	set Z_ 0

$node_(40)	set X_ 1000.0
$node_(40)	set Y_ 0.0
$node_(40)	set Z_ 0

$node_(41)	set X_ 1000.0
$node_(41)	set Y_ 200.0
$node_(41)	set Z_ 0

$node_(42)	set X_ 1000.0
$node_(42)	set Y_ 400.0
$node_(42)	set Z_ 0

$node_(43)	set X_ 1000.0
$node_(43)	set Y_ 600.0
$node_(43)	set Z_ 0

$node_(44)	set X_ 1000.0
$node_(44)	set Y_ 800.0
$node_(44)	set Z_ 0

$node_(45)	set X_ 1000.0
$node_(45)	set Y_ 1000.0
$node_(45)	set Z_ 0

$node_(46)	set X_ 1000.0
$node_(46)	set Y_ 1200.0
$node_(46)	set Z_ 0

$node_(47)	set X_ 1000.0
$node_(47)	set Y_ 1400.0
$node_(47)	set Z_ 0

$node_(48)	set X_ 1200.0
$node_(48)	set Y_ 0.0
$node_(48)	set Z_ 0

$node_(49)	set X_ 1200.0
$node_(49)	set Y_ 200.0
$node_(49)	set Z_ 0

for {set i 0} {$i < $nodecount} {incr i} {
    $node_($i) random-motion 0  ;# disable random motion
    $node_($i) start
    $ns_ at 0 "[$node_($i) entry] runclick"
}
#
# Stop the simulation
#
$ns_ at  $stoptime "puts \"NS EXITING...\" ; $ns_ halt"

#
# Let nam know that the simulation is done.
#
$ns_ at  $stoptime	"$ns_ nam-end-wireless $stoptime"


puts "Starting Simulation..."
$ns_ run
