#!/usr/bin/perl -w

use strict ;
my($command) = $0 ; $command =~ s%.*/(\w+)%$1% ; # remove the path
die "$command <base-file-name>\n" if $#ARGV != 0 ;

my($basef) = $ARGV[0] ;

my($tclf) = "${basef}.tcl" ;
die "file not found $tclf\n" if ! -f $tclf ;

#
# read the value of 'set opt($2)' from tcl script $1.
#
sub GetNodecountFromFile() {
    open F, $tclf ;
    while(<F>) {
		return $1 if /^set nodecount\s+([0-9]*)\b/ ;
    }
    die "set nodecount not found in $tclf\n" ;
}


my($mn) =  GetNodecountFromFile() ;
my($mn2) = $mn / 2 ;
my($mns) = 0 ;

while( $mns < $mn2 ) {
    my($mnd) = $mns + $mn2 -1;
    my($fn) = "${basef}_${mns}_${mnd}.data" ;
    system("./get_M2M_delay.pl ${basef}.tr $mns $mnd > $fn");
    $mns += 1;
}