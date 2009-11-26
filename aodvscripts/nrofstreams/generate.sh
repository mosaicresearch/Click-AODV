#!/bin/bash 
COUNTER=5
NSUU=../../aodv_upsula/ns-allinone-2.26/bin/ns
NSCLICK=../../click-1.4/ns-allinone-2.26/bin/ns

#50 nodes, so 5, 10, ..., 25 streams

echo AODV-UU
while [  $COUNTER -lt 26 ]; do
	cat aodv_uu_streams_begin.tcl > aodv_uu_streams_${COUNTER}.tcl
	./generatenrstreams.rb 50 $COUNTER >> aodv_uu_streams_${COUNTER}.tcl
	cat aodv_uu_streams_end.tcl >> aodv_uu_streams_${COUNTER}.tcl
	time -p ${NSUU} aodv_uu_streams_${COUNTER}.tcl 
	let COUNTER=COUNTER+5 
done

echo AODV-Click
COUNTER=5
while [  $COUNTER -lt 26 ]; do
	cat aodv_nsclick_streams_begin.tcl > aodv_nsclick_streams_${COUNTER}.tcl
	./generatenrstreams.rb 50 $COUNTER >> aodv_nsclick_streams_${COUNTER}.tcl
	cat aodv_nsclick_streams_end.tcl >> aodv_nsclick_streams_${COUNTER}.tcl
	time -p ${NSCLICK} aodv_nsclick_streams_${COUNTER}.tcl 
	let COUNTER=COUNTER+5 
done
