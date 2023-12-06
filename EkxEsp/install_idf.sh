#!/bin/bash

set -e

if [ `id -u` -ne 0 ]
then 
	echo "Please run as root (sudo)"
	exit 1
fi

if [ -z ${SUDO_USER} ]
then
	echo "Must be called via sudo!"
	exit 1
fi

if [ "${SUDO_USER}" == "root" ]
then
	echo "May not be called from root account!"
	exit 1
fi

export ESP_IDF_VERSION=v5.0
if [ "$1" != "" ];
then
	ESP_IDF_VERSION=$1
fi

export ESP_INSTALL_DIR=/opt/esp
export ESP_DIR=esp-idf-${ESP_IDF_VERSION}

echo Installing ESP pre-requisites ...
apt-get install -y wget libncurses-dev flex bison gperf python3 python3-pip python3-setuptools python3-serial python3-click python3-cryptography python3-future python3-pyparsing python3-pyelftools python3-venv cmake ninja-build ccache

echo Installing ESP IDF version ${ESP_IDF_VERSION} into ${ESP_INSTALL_DIR} ...

mkdir -p ${ESP_INSTALL_DIR}
cd ${ESP_INSTALL_DIR}
rm -rf ${ESP_DIR}

git clone -b ${ESP_IDF_VERSION} --recursive https://github.com/espressif/esp-idf.git "${ESP_DIR}"

chown -R ${SUDO_USER}.entwicklung ${ESP_INSTALL_DIR}

cd "${ESP_DIR}"
./install.sh

chown -R ${SUDO_USER}.entwicklung ~/.espressif

echo "Successfully done!"
