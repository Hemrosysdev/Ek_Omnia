#! /bin/bash

cd HemroEkxUiDemonstrator
rm -rf release
mkdir -p release
cd release
cp ../../../build-EkxUI-Desktop_Qt_5_15_1_MinGW_64_bit-Release/release/EkxUI.exe .
cp /c/Qt/Tools/mingw810_64/bin/libgcc_s_seh-1.dll .
cp /c/Qt/Tools/mingw810_64/bin/libstdc++-6.dll .
cp /c/Qt/Tools/mingw810_64/bin/libwinpthread-1.dll .
/c/Qt/5.15.1/mingw81_64/bin/windeployqt.exe --qmldir /c/Qt/5.15.1/mingw81_64/qml EkxUI.exe
