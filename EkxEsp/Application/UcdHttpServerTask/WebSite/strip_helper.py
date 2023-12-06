#!/usr/bin/python

import sys
import glob

if len(sys.argv) < 3:
	print( "usage: strip_helper.py <in filename> <out filename>", file=sys.stderr )
	sys.exit(1)

infile = glob.glob( sys.argv[1] )
outfile = sys.argv[2]

with open( outfile, "w", encoding='utf-8' ) as outf:
	with open( infile[0], encoding='utf-8' ) as inf:
		for line in inf:
			line = line.strip()
			if line:
				print( line, file=outf )

print( "Done!" )
