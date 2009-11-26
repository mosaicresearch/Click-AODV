#! /usr/bin/ruby

if ARGV.size < 4 || ARGV.size > 6
	puts "Usage: x-size, y-size, number of nodes per line, number of nodes per column [,marginX, marginY]"
	exit
end

HORIZONTALCOUNT = ARGV[2].to_i
VERTICALCOUNT = ARGV[3].to_i
XOFFSET = ARGV[4].to_f
YOFFSET = ARGV[5].to_f
HORIZONTALSPACING = (ARGV[0].to_f - 2 * XOFFSET) / (HORIZONTALCOUNT-1)
VERTICALSPACING = (ARGV[1].to_f - 2 * YOFFSET) / (VERTICALCOUNT-1)

nodenumber = 0
0.upto(HORIZONTALCOUNT-1){ |hor|
	0.upto(VERTICALCOUNT-1){ |ver|
		x = hor * HORIZONTALSPACING + XOFFSET
		print "$node_(",nodenumber,")\tset X_ ",x,"\n"
		y = ver * VERTICALSPACING + YOFFSET
		print "$node_(",nodenumber,")\tset Y_ ",y,"\n"
		print "$node_(",nodenumber,")\tset Z_ 0\n\n"
		nodenumber += 1
	}
}
