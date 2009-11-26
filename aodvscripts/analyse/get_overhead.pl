#!/usr/bin/perl -w

use strict ;
use Getopt::Std ;
my($command) = $0 ; $command =~ s%.*/(\w+)%$1% ; # remove the path

my(%opts) ;

die <<"EOM"
Usage: $command  [options] <base-file-name>
options:
  h: print this help.
  v: enable verbose mode (tells you what the data means, if active it ignores -b)
  o: print statistics on HELLO packets
  c: print statistics on TC packets
  n: print statistics on HNA packets
  i: print statistics on MID packets
  t: print statistics on all packets
  b: print # received bytes
  p: print statistics on the size of HNA packets
  d: print statistics on drops
  a: print statistics on all packets
EOM
    if !getopts('hvocnitbap', \%opts) || defined $opts{h} ;

my($basef) = $ARGV[0] ;

# measure the amount (pkts and bytes) of overhead created by each of the different control packets
my($hello) 				= 0 ;
my($hello_bytes) 	= 0 ;
my($tc) 				= 0 ;
my($tc_bytes) 			= 0 ;
my($mid) 				= 0 ;
my($mid_bytes) 			= 0 ;
my($hna) 				= 0 ;
my($hna_bytes) 			= 0 ;
my($hna_max_pkt_size)	= 0 ;
my($hna_avg_pkt_size)	= 0 ;
my($drops)	= 0 ;
my($drop_bytes)	= 0 ;

# some definitions to be used in regular expressions later on in the script
my($time) 			= "([0-9]*\.[0-9]*)" ;
my($node_id)		= "([0-9]*)" ;
my($layer) 			= "(RTR)" ;
my($pkt_id) 		= "([0-9]*)" ;
my($ns_pkt_type)	= "(AODV|AODVUU)" ;
my($old_ns_sense)	= "([0-9]*)" ;
my ($pkt_size) 		= "([0-9]*)";

# parse the trace file
my($fn) = $ARGV[0];
die "Cannot open file $fn\n" if ! open F, $fn ;

while(<F>) {
	if (/^r $time _${node_id}_ $layer  --- $pkt_id $ns_pkt_type $pkt_size .*\(HELLO\)/) {
		$hello++ ;
		$hello_bytes += $6 ;
	} elsif (/^r $time _${node_id}_ $layer  --- $pkt_id $ns_pkt_type $pkt_size .*\((RREQ|REQUEST)\)/) {
		$tc++ ;
		$tc_bytes += $6 ;		
	} elsif (/^r $time _${node_id}_ $layer  --- $pkt_id $ns_pkt_type $pkt_size .*\((RERR|ERROR)\)/) {
		$hna++ ;
		$hna_bytes += $6 ;	
		if ($6 > $hna_max_pkt_size ) { $hna_max_pkt_size = $6 ; }
	} elsif (/^r $time _${node_id}_ $layer  --- $pkt_id $ns_pkt_type $pkt_size .*\((RREP|REPLY)\)/) {
		$mid++ ;
		$mid_bytes += $6 ;	
	} elsif (/^D $time _${node_id}_ MAC  --- $pkt_id raw $pkt_size/){
		$drops++ ;
		$drop_bytes += $4;
	}
}



#print <<"EOM"
# trace generated with $command @ARGV
#EOM
    ;

if ( defined $opts{o} || defined $opts{a} )  {
	if ( defined $opts{v} ) {
		print "HELLO --- # pkts received = $hello --- # bytes received = $hello_bytes\n" ;
	} elsif ( defined $opts{b} ) {
		print "$hello_bytes" ;
	} else {
		if (defined $opts{a}) { print "$hello\t" ; }
		else { print "$hello" ;}
	}
}

if ( defined $opts{c} || defined $opts{a} )  {
	if ( defined $opts{v} ) {
 		print "RREQ --- # pkts received = $tc --- # bytes received = $tc_bytes\n" ;
	} elsif ( defined $opts{b} ) {
		print "$tc_bytes" ;
	} else {
		if (defined $opts{a}) { print "$tc\t" ; }
		else { print "$tc" ;}
	}
}

if ( defined $opts{i} || defined $opts{a} )  {
	if ( defined $opts{v} ) {
		print "RREP --- # pkts received = $mid --- # bytes received = $mid_bytes\n" ;
	} elsif ( defined $opts{b} ) {
		print "$mid_bytes" ;
	} else {
		if (defined $opts{a}) { print "$mid\t" ; }
		else { print "$mid" ;}
	}
}

if ( defined $opts{n} || defined $opts{a} )  {
	if ( defined $opts{v} ) {
		print "RERR --- # pkts received = $hna --- # bytes received = $hna_bytes\n" ;
	} elsif ( defined $opts{b} ) {
		print "$hna_bytes" ;
	} else {
		if (defined $opts{a}) { print "$hna\t" ; }
		else { print "$hna" ;}
	}
}

if ( defined $opts{p} || defined $opts{a} )  {
	if ( $hna == 0 ) {
		$hna_avg_pkt_size = 0 ;
	} else {
		$hna_avg_pkt_size = $hna_bytes / $hna ;
	}
	if ( defined $opts{v} ) {
		print "RERR --- max pkt size = $hna_max_pkt_size --- avg pkt size = $hna_avg_pkt_size\n" ;
	} elsif ( defined $opts{b} ) {
		print "$hna_avg_pkt_size" ;
	} else {
		if (defined $opts{a}) { print "$hna_max_pkt_size\t" ; }
		else { print "$hna_max_pkt_size" ;}
	}
}

if ( defined $opts{d} || defined $opts{a} )  {
	if ( defined $opts{v} ) {
		print "DROPS --- # pkts dropped = $drops --- # bytes dropped = $drop_bytes\n" ;
	} elsif ( defined $opts{b} ) {
		print "$drop_bytes" ;
	} else {
		if (defined $opts{a}) { print "$drops\t" ; }
		else { print "$drops" ;}
	}
}

if ( defined $opts{t} || defined $opts{a} )  {
	my($total_pkts) = $hello + $tc + $hna + $mid ;
	my($total_bytes) = $hello_bytes + $tc_bytes + $hna_bytes + $mid_bytes ;
	if ( defined $opts{v} ) {
 		print "ALL --- # pkts received = $total_pkts --- # bytes received = $total_bytes\n" ;
	} elsif ( defined $opts{b} ) {
		print "$total_bytes" ;
	} else {
		if (defined $opts{a}) { print "$total_pkts" ; }
		else { print "$total_pkts" ;}
	}
}