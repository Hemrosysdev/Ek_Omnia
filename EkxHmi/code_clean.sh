#! /bin/sh

if [ -z `which uncrustify` ]; then
	echo "Must install uncrustify before!"
	sudo apt install uncrustify
fi

find EkxCommon -type f -name "*.h" -exec uncrustify -c uncrustify.cfg --replace --no-backup {} \;
find EkxCommon -type f -name "*.c" -exec uncrustify -c uncrustify.cfg --replace --no-backup {} \;
find EkxCommon -type f -name "*.cpp" -exec uncrustify -c uncrustify.cfg --replace --no-backup {} \;

find McuSimulator/Sources -type f -name "*.h" -exec uncrustify -c uncrustify.cfg --replace --no-backup {} \;
find McuSimulator/Sources -type f -name "*.c" -exec uncrustify -c uncrustify.cfg --replace --no-backup {} \;
find McuSimulator/Sources -type f -name "*.cpp" -exec uncrustify -c uncrustify.cfg --replace --no-backup {} \;

find EkxWsServer/Sources -type f -name "*.h" -exec uncrustify -c uncrustify.cfg --replace --no-backup {} \;
find EkxWsServer/Sources -type f -name "*.c" -exec uncrustify -c uncrustify.cfg --replace --no-backup {} \;
find EkxWsServer/Sources -type f -name "*.cpp" -exec uncrustify -c uncrustify.cfg --replace --no-backup {} \;

find EkxHmiDemonstrator -type f -name "*.h" -exec uncrustify -c uncrustify.cfg --replace --no-backup {} \;
find EkxHmiDemonstrator -type f -name "*.c" -exec uncrustify -c uncrustify.cfg --replace --no-backup {} \;
find EkxHmiDemonstrator -type f -name "*.cpp" -exec uncrustify -c uncrustify.cfg --replace --no-backup {} \;

find EkxUI/Application -type f -name "*.h" -exec uncrustify -c uncrustify.cfg --replace --no-backup {} \;
find EkxUI/Application -type f -name "*.c" -exec uncrustify -c uncrustify.cfg --replace --no-backup {} \;
find EkxUI/Application -type f -name "*.cpp" -exec uncrustify -c uncrustify.cfg --replace --no-backup {} \;
