#!/usr/bin/python

import gzip
import shutil
import sys
import glob

if len(sys.argv) < 2:
	print( "usage: gzip_helper.py <in filename> <out filename>", file=sys.stderr )
	sys.exit(1)

infile = glob.glob( sys.argv[1] )
outfile = sys.argv[2]

with open( infile[0], 'rb') as f_in:
	with gzip.open( outfile, 'wb' ) as f_out:
		shutil.copyfileobj( f_in, f_out )

print( "Done!" )
