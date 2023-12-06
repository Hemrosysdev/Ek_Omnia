QT += quick
QT += serialport
QT += core
QT += sql
QT += widgets

TEMPLATE = app

CONFIG += c++11
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

#CONFIG( release,debug|release ): BUILD_DIR=$$PWD/build_$${TARGETDEVICE}_debug
#CONFIG( debug,release|debug ): BUILD_DIR=$$PWD/build_$${TARGETDEVICE}_release

#DESTDIR=$${BUILD_DIR}

#!exists($$BUILD_DIR):$$system( mkdir $${BUILD_DIR} )

MOC_DIR     = ./moc
UI_DIR      = ./ui
OBJECTS_DIR = ./obj
RCC_DIR     = ./rcc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Additional import path used to resolve QML modules just for Qt Quick Designer
QML_DESIGNER_IMPORT_PATH =

INCLUDEPATH += ../EspConnector/EspHostConnector/Sources
INCLUDEPATH += ../EspConnector/Sources
INCLUDEPATH += Application
INCLUDEPATH += Application/Drivers
INCLUDEPATH += Application/Drivers/Uppp/UpppCore
INCLUDEPATH += Application/Drivers/Uppp/UpppMcu
INCLUDEPATH += Application/Drivers/Uppp/UpppPdu
INCLUDEPATH += Application/Drivers/EspDrivers
INCLUDEPATH += Application/Drivers/HmiDrivers
INCLUDEPATH += Application/Recipes
INCLUDEPATH += Application/Settings
INCLUDEPATH += Application/Statistics
INCLUDEPATH += Application/Database

DISTFILES += \
    version.info \
    version.mk

SOURCES += \
        ../EspConnector/Sources/EspAdcIn.cpp \
        ../EspConnector/Sources/EspAt24c16In.cpp \
        ../EspConnector/Sources/EspAt24c16Out.cpp \
        ../EspConnector/Sources/EspConnectorClient.cpp \
        ../EspConnector/Sources/EspConnectorServer.cpp \
        ../EspConnector/Sources/EspDataContainer.cpp \
        ../EspConnector/Sources/EspDataInterface.cpp \
        ../EspConnector/Sources/EspDiscreteIn.cpp \
        ../EspConnector/Sources/EspHdc2010In.cpp \
        ../EspConnector/Sources/EspNeopixelOut.cpp \
        ../EspConnector/Sources/EspPwmOut.cpp \
        ../EspConnector/Sources/EspSocketCommander.cpp \
        ../EspConnector/Sources/EspSwUpdateIn.cpp \
        ../EspConnector/Sources/EspSwUpdateOut.cpp \
        ../EspConnector/Sources/EspUartIn.cpp \
        ../EspConnector/Sources/EspWifiIn.cpp \
        ../EspConnector/Sources/EspWifiOut.cpp \
        ../EspConnector/Sources/SocketVcomBridge.cpp \
        ../EspConnector/Sources/VirtualComPort.cpp \
        Application/Drivers/DeviceInfoCollector.cpp \
        Application/Drivers/EspDrivers/At24c16Driver.cpp \
        Application/Drivers/EspDrivers/EspDriver.cpp \
        Application/Drivers/EspDrivers/EspSwUpdateInDriver.cpp \
        Application/Drivers/EspDrivers/EspSwUpdateOutDriver.cpp \
        Application/Drivers/EspDrivers/LedPwmDriver.cpp \
        Application/Drivers/EspDrivers/NtcTempDriver.cpp \
        Application/Drivers/EspDrivers/StartStopButtonDriver.cpp \
        Application/Drivers/EspDrivers/WifiDriver.cpp \
        Application/Drivers/HmiDrivers/DddDriver.cpp \
        Application/Drivers/HmiDrivers/DisplayController.cpp \
        Application/Drivers/HmiDrivers/HmiTempDriver.cpp \
        Application/Drivers/UartDriver.cpp \
        Application/Drivers/Uppp/UpppCore/CcittCrc16.cpp \
        Application/Drivers/Uppp/UpppCore/UpppCore.cpp \
        Application/Drivers/Uppp/UpppCore/UpppCoreMsg.cpp \
        Application/Drivers/Uppp/UpppCore/UpppLevel3.cpp \
        Application/Drivers/Uppp/UpppCore/UpppLevel3Msg.cpp \
        Application/Drivers/Uppp/UpppMcu/McuDriver.cpp \
        Application/Drivers/Uppp/UpppMcu/UpppMcu.cpp \
        Application/Drivers/Uppp/UpppPdu/UpppPdu.cpp \
        Application/EkxQuickView.cpp \
        Application/MainStatemachine.cpp \
        Application/Recipes/LibraryRecipe.cpp \
        Application/Recipes/RecipeControl.cpp \
        Application/Recipes/TimeRecipe.cpp \
        Application/Settings/DiskUsageTime.cpp \
        Application/Settings/RadioButtonMenu.cpp \
        Application/Settings/RadioButtonMenuItem.cpp \
        Application/Settings/SettingsMenu.cpp \
        Application/Settings/SettingsSerializer.cpp \
        Application/Settings/SettingsStatemachine.cpp \
        Application/Settings/StatisticsBoard.cpp \
        Application/Settings/StatisticsBoardResult.cpp \
        Application/Database/SqlQueryWorker.cpp \
        Application/Database/SqliteInterface.cpp \
        Application/StandbyController.cpp \
        Application/Statistics/StatisticsRecord.cpp \
        Application/Statistics/TimePeriod.cpp \
        Application/SwUpdateController.cpp \
        Application/main.cpp

HEADERS += \
    ../EspConnector/Sources/EspAdcIn.h \
    ../EspConnector/Sources/EspAt24c16In.h \
    ../EspConnector/Sources/EspAt24c16Out.h \
    ../EspConnector/Sources/EspConnectorClient.h \
    ../EspConnector/Sources/EspConnectorServer.h \
    ../EspConnector/Sources/EspDataContainer.h \
    ../EspConnector/Sources/EspDataInterface.h \
    ../EspConnector/Sources/EspDiscreteIn.h \
    ../EspConnector/Sources/EspHdc2010In.h \
    ../EspConnector/Sources/EspNeopixelOut.h \
    ../EspConnector/Sources/EspPwmOut.h \
    ../EspConnector/Sources/EspSocketCommander.h \
    ../EspConnector/Sources/EspSwUpdateIn.h \
    ../EspConnector/Sources/EspSwUpdateOut.h \
    ../EspConnector/Sources/EspUartIn.h \
    ../EspConnector/Sources/EspWifiIn.h \
    ../EspConnector/Sources/EspWifiOut.h \
    ../EspConnector/Sources/SocketVcomBridge.h \
    ../EspConnector/Sources/VirtualComPort.h \
    Application/Database/EkxSqlSchemata.h \
    Application/Database/EkxSqliteTypes.h \
    Application/Drivers/DeviceInfoCollector.h \
    Application/Drivers/EspDrivers/At24c16Driver.h \
    Application/Drivers/EspDrivers/EspDriver.h \
    Application/Drivers/EspDrivers/EspSwUpdateInDriver.h \
    Application/Drivers/EspDrivers/EspSwUpdateOutDriver.h \
    Application/Drivers/EspDrivers/LedPwmDriver.h \
    Application/Drivers/EspDrivers/NtcTempDriver.h \
    Application/Drivers/EspDrivers/StartStopButtonDriver.h \
    Application/Drivers/EspDrivers/WifiDriver.h \
    Application/Drivers/HmiDrivers/DddDriver.h \
    Application/Drivers/HmiDrivers/DisplayController.h \
    Application/Drivers/HmiDrivers/HmiTempDriver.h \
    Application/Drivers/UartDriver.h \
    Application/Drivers/Uppp/UpppCore/CcittCrc16.h \
    Application/Drivers/Uppp/UpppCore/UpppCore.h \
    Application/Drivers/Uppp/UpppCore/UpppCoreMsg.h \
    Application/Drivers/Uppp/UpppCore/UpppLevel3.h \
    Application/Drivers/Uppp/UpppCore/UpppLevel3Msg.h \
    Application/Drivers/Uppp/UpppMcu/McuDriver.h \
    Application/Drivers/Uppp/UpppMcu/UpppMcu.h \
    Application/Drivers/Uppp/UpppPdu/UpppPdu.h \
    Application/EkxGlobals.h \
    Application/EkxQuickView.h \
    Application/MainStatemachine.h \
    Application/Recipes/LibraryRecipe.h \
    Application/Recipes/RecipeControl.h \
    Application/Recipes/TimeRecipe.h \
    Application/Settings/DiskUsageTime.h \
    Application/Settings/RadioButtonMenu.h \
    Application/Settings/RadioButtonMenuItem.h \
    Application/Settings/SettingsMenu.h \
    Application/Settings/SettingsSerializer.h \
    Application/Settings/SettingsStatemachine.h \
    Application/Settings/StatisticsBoard.h \
    Application/Settings/StatisticsBoardResult.h \
    Application/Database/SqlQueryWorker.h \
    Application/Database/SqliteInterface.h \
    Application/StandbyController.h \
    Application/Statistics/StatisticsRecord.h \
    Application/Statistics/TimePeriod.h \
    Application/SwUpdateController.h

RESOURCES += UiPresentation.qrc

target.path = /usr/local/bin
INSTALLS += target
