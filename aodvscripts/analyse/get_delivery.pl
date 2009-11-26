#!/usr/bin/perl -w
#
# Computes the packet end-to-end delay in a ns trace.
# The packet is assumed to be Tx and Rx by mobile nodes.
# (c) Llorenç Cerdà. July, 2001.

use strict ;
use Getopt::Std ;
# Configurable parameters.
my(%opts) ;

my($command) = $0 ; $command =~ s%.*/(\w+)%$1% ; # remove the path

die <<"EOM"
Usage: $command [options] <trace-file> <pkt-type> <src> <dst>
options:
  h: print this help.
EOM
    if $#ARGV != 3 || !getopts(':h', \%opts) || defined $opts{h} ;

my($tfile) = map { /\.(gz|Z)$/ ? "gunzip -c $_ |" : $_  } $ARGV[0] ;
my($pkt)   = $ARGV[1] ;
my($src)   = $ARGV[2] ; my($xsrc) = sprintf "%x", $src ;
my($dst)   = $ARGV[3] ; my($xdst) = sprintf "%x", $dst ;
my($uid, $i, $dy) ;
my(%p) ; # stores the packet Tx time (tx), last BS (bs), 
         # Rx time (rx), seq. number (sq), contention period (cp).
my($cf) = 0 ;


my($pkts_sent) = 0;
my($pkts_rcvd) = 0;

open(FILE, $tfile) || die "Can't open file: $tfile\n" ;

while(defined($_ = <FILE>)) {
    if(/^s.*Beacon/) {
	$cf = 1 ; 
    } elsif(/^s.*CFEnd/) {
	$cf = 0 ; 
    } elsif(/$pkt/) {
# s 50.000000000 _1_ AGT  --- 6165 cbr 1928 [0 0 0 0] ------- [1:0 0:0 32 0] [0] 0 0
# r 50.036430263 _0_ AGT  --- 6165 cbr 1948 [a2 0 1 800] ------- [1:0 0:0 32 0] [0] 1 0
# s 29.750051377 _9_ AGT  --- 928 exp 928 [0 0 0 0] ------- [9:3 0:18 32 0] [29] 0 0
# s 29.750515170 _9_ MAC  --- 928 exp 1000 [a2 0 9 800] ------- [9:3 0:18 32 0] [29] 0 0 -- Delay (ms) = 0.463793, 0.463793
		if(/^(.) (\d+\.\d+) _(\d+)_ AGT +-+ (\d+).* \[(.*)\]/) {
			# print "R=$1, t=$2, id=$3, uid=$4, sn=$5\n" ;
			if($1 eq "r" && $3 eq $dst) { # mark the arrival of the packet
				$pkts_rcvd++;
			} elsif($1 eq "s" && $3 eq $src) { # mark the tx of the packet
				$pkts_sent++;
			}
		} 
    }
}

my($fraction) = 0;

if ($pkts_sent == 0) {
	$fraction = 0;
} else {
	$fraction = ($pkts_rcvd/$pkts_sent)*100 ;
}

print "$src $dst $pkts_rcvd $pkts_sent $fraction\n"

