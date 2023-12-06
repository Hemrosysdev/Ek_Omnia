#! /bin/bash

if [ -z `which socat` ]
then
	echo "Please install 'socat' first"
	exit 1 
fi

socat -d -d pty,raw,echo=0 pty,raw,echo=0
