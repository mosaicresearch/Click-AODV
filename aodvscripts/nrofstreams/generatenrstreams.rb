#! /usr/bin/ruby

if ARGV.size != 2
	puts "Usage: number of nodes, number of streams"
	exit
end

NROFNODES = ARGV[0].to_i
NROFSTREAMS = ARGV[1].to_i

if NROFNODES / 2 < NROFSTREAMS
	puts "Too many streams, maximum: number of nodes / 2"
	exit
end

puts "set streamcount #{NROFSTREAMS}"

0.upto(NROFSTREAMS-1){ |streamnr|
	puts "set origin(#{streamnr}) #{streamnr}"
	puts "set destination(#{streamnr}) #{NROFNODES - streamnr - 1}"
}

=begin
Output is supposed to be:
generatestreams.rb 10 5

set streamcount 5
set origin(0) 0
set destination(0) 9
set origin(1) 1
set destination(1) 8
set origin(2) 2
set destination(2) 7
set origin(3) 3
set destination(3) 6
set origin(4) 4
set destination(4) 5
=end
