#! /usr/bin/ruby

if ARGV.size != 2
	puts "Usage: number of nodes, minimal distance"
	exit
end

NROFNODES = ARGV[0].to_i
HORIZONTALCOUNT = Math::sqrt(ARGV[0]).to_i
VERTICALCOUNT = (ARGV[0].to_f / HORIZONTALCOUNT).to_i + 1
DISTANCE = ARGV[1].to_f

nodenumber = 0
0.upto(HORIZONTALCOUNT-1){ |hor|
	0.upto(VERTICALCOUNT-1){ |ver|
		x = hor * DISTANCE
		print "$node_(",nodenumber,")\tset X_ ",x,"\n"
		y = ver * DISTANCE
		print "$node_(",nodenumber,")\tset Y_ ",y,"\n"
		print "$node_(",nodenumber,")\tset Z_ 0\n\n"
		nodenumber += 1
		if nodenumber == NROFNODES.to_i
			exit
		end
	}
}
