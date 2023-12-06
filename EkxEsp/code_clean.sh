#! /bin/sh

if [ -z `which uncrustify` ]; then
	echo "Must install uncrustify before!"
	sudo apt install uncrustify
fi

find Application -type f -name "*.h" -exec uncrustify -c uncrustify.cfg --replace --no-backup {} \;
find Application -type f -name "*.c" -exec uncrustify -c uncrustify.cfg --replace --no-backup {} \;
find Application -type f -name "*.cpp" -exec uncrustify -c uncrustify.cfg --replace --no-backup {} \;

