#! /bin/sh

if [ "$#" = "0" ]
then
	echo "syntax error: ./make_string.sh <html file name>"
	exit 1
fi

cat $1 | sed -e 's/\\/\\\\/g' -e 's/\"/\\"/g' -e ' s/^/"/g' -e 's/$/\\n"/g' > $1.inc

echo "Done!"
