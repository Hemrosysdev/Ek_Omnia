#! /bin/bash

set -e 

cd HemroEkOmniaHmiDemonstratorWindows
rm -rf release
mkdir -p release
cd release
cp ../../../build-EkxHmiDemonstrator-Desktop_Qt_5_15_2_MinGW_64_bit-Release/EkxHmiDemonstrator.exe .
cp /c/Qt/Tools/mingw810_64/bin/libgcc_s_seh-1.dll .
cp /c/Qt/Tools/mingw810_64/bin/libstdc++-6.dll .
cp /c/Qt/Tools/mingw810_64/bin/libwinpthread-1.dll .
/c/Qt/5.15.2/mingw81_64/bin/windeployqt.exe --qmldir ../.. --qmldir ../../../EkxUI/QmlComponents EkxHmiDemonstrator.exe
