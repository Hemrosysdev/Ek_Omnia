#! /bin/bash

set -e

if [[ $# = 0 ]]
then
	./esptool/esptool.py erase_flash
elif [[ $# = 1 ]]
then
	./esptool/esptool.py -p $1 erase_flash
else
	echo 'syntax: ./do_erase_flash.sh [<tty device name>]'
	echo "Erases complete flash ESP32 via serial device"
	exit 0
fi


