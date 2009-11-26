#!/bin/bash 
COUNTER=10
while [  $COUNTER -lt 101 ]; do
	echo "set nodecount $COUNTER" > aodv_nsclick_random_${COUNTER}.tcl
	cat aodv_nsclick_random.tcl >> aodv_nsclick_random_${COUNTER}.tcl
	
	time -p ../ns/bin/ns aodv_nsclick_random_${COUNTER}.tcl 
	let COUNTER=COUNTER+10 
done
