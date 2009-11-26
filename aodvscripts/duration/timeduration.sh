#!/bin/bash 
COUNTER=50
NSUU=../../aodv_upsula/ns-allinone-2.26/bin/ns
NSCLICK=../../click-1.4/ns-allinone-2.26/bin/ns

echo AODV-UU tests
while [  $COUNTER -lt 501 ]; do
	echo set "stoptime $COUNTER" > aodv_uu_time_${COUNTER}.tcl
	cat aodv_uu_time.tcl >> aodv_uu_time_${COUNTER}.tcl
	time -p ${NSUU} aodv_uu_time_${COUNTER}.tcl 
	let COUNTER=COUNTER+50 
done

echo AODV-Click tests
COUNTER=50

while [  $COUNTER -lt 501 ]; do
	echo set "stoptime $COUNTER" > aodv_nsclick_time_${COUNTER}.tcl
	cat aodv_nsclick_time.tcl >> aodv_nsclick_time_${COUNTER}.tcl
	time -p ${NSCLICK} aodv_nsclick_time_${COUNTER}.tcl 
	let COUNTER=COUNTER+50 
done
