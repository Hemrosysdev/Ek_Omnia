#!/bin/bash

set -e

PACKAGE_NAME=hemro-ek-omnia-hmi-demonstrator-amd64
APP_NAME=HemroEkOmniaHmiDemonstrator
PURE_APP_NAME=EkxHmiDemonstrator

if [ -e version.info ]
then
	echo "No version.info file found! Please run build script first"
	exit 1
fi

source version.info

cd ../DebianInstaller

PACKAGE_DIR=${PACKAGE_NAME}-${VERSION_NO}
DEBION_INSTALLER=${PACKAGE_NAME}-${VERSION_NO}.deb

if [ -e ${DEBIAN_INSTALLER} ]
then
	echo "No installer found! Please run build script first"
	exit 1
fi

read -p "Passwort fuer u-experten server [Leerstring ueberspringt]: " uxtools_password

if [ "${uxtools_password}" != "" ]
then
	WWW_SERVER=root@ux_tools:/var/www/html/tools/modules/hemro-ekx-hmi-demonstrator
	echo "Copy files to ux-tools.u-experten.de"
	sshpass -p ${uxtools_password} scp ${PACKAGE_NAME}-${VERSION_NO}.deb ${WWW_SERVER}/downloads/linux/
	sshpass -p ${uxtools_password} scp version.info ${WWW_SERVER}/
	sshpass -p ${uxtools_password} scp ../release_notes.txt ${WWW_SERVER}/
fi
