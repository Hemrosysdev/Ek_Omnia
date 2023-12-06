#! /bin/bash

set -e 

export QTDIR=/c/Qt/5.15.2/mingw81_64
export MINGW_DIR=/c/Qt/Tools/mingw810_64
export MINGW_MAKE=${MINGW_DIR}/bin/mingw32-make
export APPNAME=EkxHmiDemonstrator
export ARCHITECTURE=x64
export COMPILER=mingw32
export SETUP=Setup${APPNAME}_${COMPILER}_${ARCHITECTURE}
export TARGET=${APPNAME}_${COMPILER}_${ARCHITECTURE}
export DEPLOY_DIR=deploy_${COMPILER}_${ARCHITECTURE}
export BUILD_DIR=build_Desktop_release

export PATH=${QTDIR}/bin:${MINGW_DIR}/bin:${PATH}

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

rm -rf ${BUILD_DIR}

${QTDIR}/bin/qmake ${APPNAME}.pro -spec win32-g++ CONFIG+=release
cd ${BUILD_DIR};${MINGW_MAKE} -f ../version.mk;cd -
${MINGW_DIR}/bin/mingw32-make.exe -f Makefile.release -j 8 

rm -rf release
rm -rf debug
rm -rf build_Desktop_debug
rm -rf *.rc
rm -rf *.Debug
rm -rf *.zip

sync

cd HemroEkOmniaHmiDemonstratorWindows
rm -rf release
mkdir -p release
cd release
cp ../../${BUILD_DIR}/EkxHmiDemonstrator.exe .
cp /c/Qt/Tools/mingw810_64/bin/libgcc_s_seh-1.dll .
cp /c/Qt/Tools/mingw810_64/bin/libstdc++-6.dll .
cp /c/Qt/Tools/mingw810_64/bin/libwinpthread-1.dll .
/c/Qt/5.15.2/mingw81_64/bin/windeployqt.exe --qmldir ../.. --qmldir ../../../EkxUI/QmlComponents EkxHmiDemonstrator.exe

cd ../..

source ${BUILD_DIR}/version.sh

echo "Compress HemroEkOmniaHmiDemonstratorWindows ..."
powershell Compress-Archive -Path HemroEkOmniaHmiDemonstratorWindows -DestinationPath HemroEkOmniaHmiDemonstratorWindows_V${VERSION_NO}.zip

echo "Finished!"
