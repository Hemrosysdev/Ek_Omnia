QT -= gui
QT += websockets

CONFIG += c++14
CONFIG += console
CONFIG -= app_bundle

#DESTDIR = ./

TARGET = EkxWsServer
TEMPLATE = app
BUILD_DIR = .
message( "Build dir " $$BUILD_DIR )

OBJECTS_DIR = $$BUILD_DIR/obj
MOC_DIR     = $$BUILD_DIR/moc
RCC_DIR     = $$BUILD_DIR/rcc
UI_DIR      = $$BUILD_DIR/ui

!exists( $$BUILD_DIR ):system( mkdir -p $$BUILD_DIR )

INCLUDEPATH += Sources
INCLUDEPATH += ../Sources
INCLUDEPATH += ../EkxCommon

contains( QMAKE_CC, ".*arm-linux-gcc" ) {
    message( "Building target version for ARM" )
    DEFINES += TARGETBUILD
} else {
    message( "Building host version for Linux" )
}

versiontarget.target = version.h
versiontarget.commands = make -f $${_PRO_FILE_PWD_}/version.mk
versiontarget.depends = FORCE

PRE_TARGETDEPS += version.h
QMAKE_EXTRA_TARGETS += versiontarget

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        ../EkxCommon/LockableFile.cpp \
        Sources/EkxWsServer.cpp \
        Sources/main.cpp

HEADERS += \
    ../EkxCommon/LockableFile.h \
    Sources/EkxWsServer.h

DISTFILES += \
    EkxWsServerVersion.json \
    Test/websocket_test.html \
    version.info \
    version.mk
