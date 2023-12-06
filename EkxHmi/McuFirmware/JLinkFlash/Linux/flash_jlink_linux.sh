#!/bin/bash

if [ `id -u` -ne 0 ]
then 
	echo "Please run as root (sudo)"
	exit 1
fi

JLINKEXE=`which JLinkExe`

if [ ! -n "${JLINKEXE}" ]
then
	echo "Error, can't found application JLinkExe"
	echo "Please download JLink flasher from https://www.segger.com/downloads/jlink/"
	echo "Install with: sudo dpkg -i JLink_Linux_V620e_x86_64.deb"
	exit 2
fi

cp Rules/*.rules /etc/udev/rules.d

# STM32F030C8T6
${JLINKEXE} -CommanderScript Config/jlink.cmd
