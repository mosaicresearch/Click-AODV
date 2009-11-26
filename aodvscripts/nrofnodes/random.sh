#!/bin/bash 
COUNTER=10
NSUU=../../aodv_upsula/ns-allinone-2.26/bin/ns
NSCLICK=../../click-1.4/ns-allinone-2.26/bin/ns
DISTANCE=200

echo AODV-UU tests
while [  $COUNTER -lt 101 ]; do
	echo "set nodecount $COUNTER" > aodv_uu_many_${COUNTER}.tcl
	echo "set destination $((COUNTER-1))" >> aodv_uu_many_${COUNTER}.tcl
	cat aodv_uu_many_begin.tcl >> aodv_uu_many_${COUNTER}.tcl
	./generatenrnodes.rb $COUNTER $DISTANCE >> aodv_uu_many_${COUNTER}.tcl
	cat aodv_uu_many_end.tcl >> aodv_uu_many_${COUNTER}.tcl
	time -p ${NSUU} aodv_uu_many_${COUNTER}.tcl 
	let COUNTER=COUNTER+10 
done

echo AODV-Click tests
COUNTER=10
while [  $COUNTER -lt 101 ]; do
	echo "set nodecount $COUNTER" > aodv_nsclick_many_${COUNTER}.tcl
	echo "set destination $((COUNTER-1))" >> aodv_nsclick_many_${COUNTER}.tcl
	cat aodv_nsclick_many_begin.tcl >> aodv_nsclick_many_${COUNTER}.tcl
	./generatenrnodes.rb $COUNTER $DISTANCE >> aodv_nsclick_many_${COUNTER}.tcl
	cat aodv_nsclick_many_end.tcl >> aodv_nsclick_many_${COUNTER}.tcl
	time -p ${NSCLICK} aodv_nsclick_many_${COUNTER}.tcl 
	let COUNTER=COUNTER+10 
done
