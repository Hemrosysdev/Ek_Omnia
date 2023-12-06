#!/bin/sh

set -e

cd /opt/HemroEkOmniaHmiDemonstrator/InstallMimeTypes

./install.sh

ldconfig /usr/lib
