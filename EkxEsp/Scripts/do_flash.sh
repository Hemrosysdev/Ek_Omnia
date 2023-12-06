#! /bin/bash

set -e

if [ "$#" != "1" ]
then
	echo 'syntax: ./do_flash.sh <tty device name>'
	echo "Flashes ESP32 via serial device"
	exit 0
fi

./esptool/esptool.py -p $1 -b 2000000 write_flash 0x1000 bootloader.bin 0x10000 EspFirmware.bin 0x8000 partition-table.bin 0xd000 ota_data_initial.bin
