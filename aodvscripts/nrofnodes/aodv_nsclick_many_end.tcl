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
