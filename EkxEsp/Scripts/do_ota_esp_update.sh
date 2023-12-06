#! /bin/bash

ESP_OTA_IP=192.168.4.1
ESP_OTA_PORT=80
ESP_IMAGE=EspFirmware.bin

case "$#" in 
	0) ;;
	1) ESP_IMAGE=$1;;
	*) echo 'syntax: ./do_ota_esp_update.sh [<ESP firmware file>] ';exit -1
esac

if [ ! -e "${ESP_IMAGE}" ]
then
	echo "error: firmware file <${ESP_IMAGE}> doesn't exist!"
	exit -1
fi

if [ -z `which curl` ]
then
	echo "Error: curl not installed"
	echo "Please install with 'sudo apt-get install curl'"
	exit -1
fi

unameOut="$(uname -s)"
case "${unameOut}" in
    Linux*)     machine=Linux;;
    Darwin*)    machine=Mac;;
    CYGWIN*)    machine=Cygwin;;
    MINGW*)     machine=MinGw;;
    *)          machine="UNKNOWN:${unameOut}"
esac

echo "Check ESP32 AP availability ..."

case "${machine}" in
    Linux)     ping ${ESP_OTA_IP} -c 1 > /dev/null;;
    Mac)       ping ${ESP_OTA_IP} -c 1 > /dev/null;;
    Cygwin)    ping ${ESP_OTA_IP} -c 1 > /dev/null;;
    MinGw)     ping ${ESP_OTA_IP} -n 1 > /dev/null;;
    *)
esac

if [ $? -ne 0 ]
then
	echo "Error: ESP32 not available!"
	echo "Please connect to ESP32 AP and ensure, that you are the only device (disconnect mobile phone)"
	exit -2
fi
	
curl -s -N -X POST --data-binary @${ESP_IMAGE} ${ESP_OTA_IP}:${ESP_OTA_PORT}/cmd?update
