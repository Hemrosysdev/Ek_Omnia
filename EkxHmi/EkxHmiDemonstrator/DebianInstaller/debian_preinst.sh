#!/bin/sh

set -e

result=`ps aux | grep "/opt/HemroEkOmniaHmiDemonstrator/EkxHmiDemonstrator" | grep -v "grep" | wc -l`
if [ $result -ge 1 ]
then
	echo .
	echo "Please close running instance of EkxHmiDemonstrator first!"
	echo .
	exit 1
fi
