QT += quick
QT += core
QT += widgets

TEMPLATE = app

CONFIG += c++14
CONFIG += qml_debug

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Refer to the documentation for the
# deprecated API to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# To build for target application add #DEFINES+=TARGETBUILD to your qmake call !!!


# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

contains( QMAKE_CC, ".*arm-linux-gcc" ) {
    message( "Building target version for ARM" )
    DEFINES += TARGETBUILD
    TARGETDEVICE = Target
} else {
    message( "Building host version for Linux" )
    TARGETDEVICE = Desktop
}

win32: DEFINES += SHOW_SPLASHSCREEN

linux: {
    versiontarget.target = version.h
    versiontarget.commands = make -f $${_PRO_FILE_PWD_}/version.mk
    versiontarget.depends = FORCE

    PRE_TARGETDEPS += version.h
    QMAKE_EXTRA_TARGETS += versiontarget
}

MOC_DIR     = ./moc
UI_DIR      = ./ui
OBJECTS_DIR = ./obj
RCC_DIR     = ./rcc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Additional import path used to resolve QML modules just for Qt Quick Designer
QML_DESIGNER_IMPORT_PATH =

INCLUDEPATH += Application

DISTFILES += \
    version.info \
    version.mk

SOURCES += \
        Application/EkxQuickView.cpp \
        Application/main.cpp

HEADERS += \
    Application/EkxGlobals.h \
    Application/EkxQuickView.h

RESOURCES += E65STestApp.qrc

target.path = /usr/local/bin
INSTALLS += target
