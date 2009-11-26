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
Usage: $command [options] <trace-file> <src> <dst>
options:
  h: print this help.
EOM
    if $#ARGV != 2 || !getopts(':h', \%opts) || defined $opts{h} ;

my($tfile) = map { /\.(gz|Z)$/ ? "gunzip -c $_ |" : $_  } $ARGV[0] ;
my($src)   = $ARGV[1] ; 
my($dst)   = $ARGV[2] ; 
my($uid, $i, $dy) ;
my(%p) ; # stores the packet Tx time (tx), last BS (bs), 
         # Rx time (rx), seq. number (sq).

open(FILE, $tfile) || die "Can't open file: $tfile\n" ;

print "# trace generated with $command @ARGV.\n" ;
print "# Tx-time Rx-time delay seq-num uid contention.\n" ;
print "#  (F: contention free, C: contention period).\n" ;

while(defined($_ = <FILE>)) {
# s 50.000000000 _1_ AGT  --- 6165 cbr 1928 [0 0 0 0] ------- [1:0 0:0 32 0] [0] 0 0
# r 50.036430263 _0_ AGT  --- 6165 cbr 1948 [a2 0 1 800] ------- [1:0 0:0 32 0] [0] 1 0
# s 29.750051377 _9_ AGT  --- 928 exp 928 [0 0 0 0] ------- [9:3 0:18 32 0] [29] 0 0
# s 29.750515170 _9_ MAC  --- 928 exp 1000 [a2 0 9 800] ------- [9:3 0:18 32 0] [29] 0 0 -- Delay (ms) = 0.463793, 0.463793
	if(/^(.) (\d+\.\d+) _(\d+)_ AGT +-+ (\d+).* \[.*\]/) {
	    if($1 eq "r" && $3 eq $dst) { # mark the arrival of the packet
		$uid = $4 ;
		next if defined($p{$uid}->{rx}) ;
		$p{$uid}->{rx} = $2 ;
	    } elsif($1 eq "s" && $3 eq $src) { # mark the tx of the packet
		$uid = $4 ;
		$p{$uid}->{tx} = $2 ;
	    }
	} 
}

foreach $i (sort {$a <=> $b} keys(%p)) {
    next if !defined($p{$i}->{tx}) ;
    if(!defined($p{$i}->{rx})) {
	$p{$i}->{rx} = "!"  ;
	$dy = "!"  ;
    } else {
	$dy = sprintf("%-10f", $p{$i}->{rx} - $p{$i}->{tx}) ;
    }
    print "$p{$i}->{tx} $p{$i}->{rx} $dy $i\n" ;
}
