QT += quick
QT += serialport
QT += core
QT += sql

TEMPLATE = app

CONFIG += c++17
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

INCLUDEPATH += ../EkxCommon
INCLUDEPATH += Application
INCLUDEPATH += Application/Drivers
INCLUDEPATH += Application/Drivers/Uppp/UpppCore
INCLUDEPATH += Application/Drivers/Uppp/UpppMcu
INCLUDEPATH += Application/Drivers/EspDrivers
INCLUDEPATH += Application/Drivers/EspDrivers/EkxProtocol
INCLUDEPATH += Application/Drivers/HmiDrivers
INCLUDEPATH += Application/Recipes
INCLUDEPATH += Application/Settings
INCLUDEPATH += Application/Statistics
INCLUDEPATH += Application/Database

DISTFILES += \
    ../../manifest/version.info \
    ../EkxHmiDemonstrator/version.info \
    version.info \
    version.mk

SOURCES += \
        ../EkxCommon/ColoredMessageHandler.cpp \
        ../EkxCommon/LockableFile.cpp \
        ../EkxCommon/UnixSignalCatcher.cpp \
        Application/AcyclicNotification.cpp \
        Application/Database/BufferedQueryWriter.cpp \
        Application/Database/EkxSqliteTypes.cpp \
        Application/Drivers/AgsaControl.cpp \
        Application/Drivers/AgsaLongDurationTest.cpp \
        Application/Drivers/DeviceInfoCollector.cpp \
        Application/Drivers/EspDrivers/At24c16Driver.cpp \
        Application/Drivers/EspDrivers/EkxProtocol/EkxProtocol.cpp \
        Application/Drivers/EspDrivers/EspDcHallMotorDriver.cpp \
        Application/Drivers/EspDrivers/EspDeviceDriver.cpp \
        Application/Drivers/EspDrivers/EspDriver.cpp \
        Application/Drivers/EspDrivers/EspProtocolStack.cpp \
        Application/Drivers/EspDrivers/EspSerialDriver.cpp \
        Application/Drivers/EspDrivers/EspStepperMotorDriver.cpp \
        Application/Drivers/EspDrivers/EspSwUpdateDriver.cpp \
        Application/Drivers/EspDrivers/EspSystemDriver.cpp \
        Application/Drivers/EspDrivers/HttpServerDriver.cpp \
        Application/Drivers/EspDrivers/JsonApi.cpp \
        Application/Drivers/EspDrivers/NtcTempDriver.cpp \
        Application/Drivers/EspDrivers/SpotLightDriver.cpp \
        Application/Drivers/EspDrivers/StartStopButtonDriver.cpp \
        Application/Drivers/EspDrivers/StartStopLightDriver.cpp \
        Application/Drivers/EspDrivers/WifiDriver.cpp \
        Application/Drivers/EspDrivers/esp_rom_crc.c \
        Application/Drivers/HmiDrivers/AdcDmaDemuxer.cpp \
        Application/Drivers/HmiDrivers/AdcDmaReader.cpp \
        Application/Drivers/HmiDrivers/AdcVoltageInDriver.cpp \
        Application/Drivers/HmiDrivers/CpuMp157Driver.cpp \
        Application/Drivers/HmiDrivers/DddCouple.cpp \
        Application/Drivers/HmiDrivers/DddDriver.cpp \
        Application/Drivers/HmiDrivers/DeviceMemoryIo.cpp \
        Application/Drivers/HmiDrivers/DisplayController.cpp \
        Application/Drivers/HmiDrivers/GpioOut.cpp \
        Application/Drivers/HmiDrivers/HmiTempDriver.cpp \
        Application/Drivers/HmiDrivers/PwmCaptureDriver.cpp \
        Application/Drivers/HmiDrivers/SystemDeviceFile.cpp \
        Application/Drivers/EspDrivers/EspMcuDriver.cpp \
        Application/Drivers/Uppp/UpppCore/CcittCrc16.cpp \
        Application/Drivers/Uppp/UpppCore/UpppCore.cpp \
        Application/Drivers/Uppp/UpppCore/UpppCoreMsg.cpp \
        Application/Drivers/Uppp/UpppCore/UpppLevel3.cpp \
        Application/Drivers/Uppp/UpppCore/UpppLevel3Msg.cpp \
        Application/Drivers/Uppp/UpppMcu/McuDriver.cpp \
        Application/Drivers/Uppp/UpppMcu/UpppMcu.cpp \
        Application/EkxQuickView.cpp \
        Application/ElapsedTimer.cpp \
        Application/JsonHelper.cpp \
        Application/MainStatemachine.cpp \
        Application/Notification.cpp \
        Application/NotificationCenter.cpp \
        Application/QmlTypes.cpp \
        Application/Recipes/LibraryGrammage.cpp \
        Application/Recipes/LibraryRecipe.cpp \
        Application/Recipes/RecipeControl.cpp \
        Application/Recipes/TimeRecipe.cpp \
        Application/Settings/DiskUsageTime.cpp \
        Application/Settings/RadioButtonMenu.cpp \
        Application/Settings/RadioButtonMenuItem.cpp \
        Application/Settings/SettingsMenu.cpp \
        Application/Settings/SettingsMenuContentFactory.cpp \
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
    ../EkxCommon/ColoredMessageHandler.h \
    ../EkxCommon/LockableFile.h \
    ../EkxCommon/UnixSignalCatcher.h \
    Application/AcyclicNotification.h \
    Application/Database/BufferedQueryWriter.h \
    Application/Database/EkxSqlSchemata.h \
    Application/Database/EkxSqliteTypes.h \
    Application/Drivers/AgsaControl.h \
    Application/Drivers/AgsaLongDurationTest.h \
    Application/Drivers/DeviceInfoCollector.h \
    Application/Drivers/EspDrivers/At24c16Driver.h \
    Application/Drivers/EspDrivers/EkxProtocol/EkxProtocol.h \
    Application/Drivers/EspDrivers/EspDcHallMotorDriver.h \
    Application/Drivers/EspDrivers/EspDeviceDriver.h \
    Application/Drivers/EspDrivers/EspDriver.h \
    Application/Drivers/EspDrivers/EspProtocolStack.h \
    Application/Drivers/EspDrivers/EspSerialDriver.h \
    Application/Drivers/EspDrivers/EspStepperMotorDriver.h \
    Application/Drivers/EspDrivers/EspSwUpdateDriver.h \
    Application/Drivers/EspDrivers/EspSystemDriver.h \
    Application/Drivers/EspDrivers/HttpServerDriver.h \
    Application/Drivers/EspDrivers/JsonApi.h \
    Application/Drivers/EspDrivers/NtcTempDriver.h \
    Application/Drivers/EspDrivers/SpotLightDriver.h \
    Application/Drivers/EspDrivers/StartStopButtonDriver.h \
    Application/Drivers/EspDrivers/StartStopLightDriver.h \
    Application/Drivers/EspDrivers/WifiDriver.h \
    Application/Drivers/EspDrivers/esp_rom_crc.h \
    Application/Drivers/HmiDrivers/AdcDmaDemuxer.h \
    Application/Drivers/HmiDrivers/AdcDmaReader.h \
    Application/Drivers/HmiDrivers/AdcVoltageInDriver.h \
    Application/Drivers/HmiDrivers/ArithmeticFilter.h \
    Application/Drivers/HmiDrivers/CpuMp157Driver.h \
    Application/Drivers/HmiDrivers/DddCouple.h \
    Application/Drivers/HmiDrivers/DddDriver.h \
    Application/Drivers/HmiDrivers/DeviceMemoryIo.h \
    Application/Drivers/HmiDrivers/DisplayController.h \
    Application/Drivers/HmiDrivers/GpioOut.h \
    Application/Drivers/HmiDrivers/HmiTempDriver.h \
    Application/Drivers/HmiDrivers/MedianFilter.hpp \
    Application/Drivers/HmiDrivers/PwmCaptureDriver.h \
    Application/Drivers/HmiDrivers/SystemDeviceFile.h \
    Application/Drivers/EspDrivers/EspMcuDriver.h \
    Application/Drivers/Uppp/UpppCore/CcittCrc16.h \
    Application/Drivers/Uppp/UpppCore/UpppCore.h \
    Application/Drivers/Uppp/UpppCore/UpppCoreMsg.h \
    Application/Drivers/Uppp/UpppCore/UpppLevel3.h \
    Application/Drivers/Uppp/UpppCore/UpppLevel3Msg.h \
    Application/Drivers/Uppp/UpppMcu/McuDriver.h \
    Application/Drivers/Uppp/UpppMcu/UpppMcu.h \
    Application/EkxGlobals.h \
    Application/EkxQuickView.h \
    Application/ElapsedTimer.h \
    Application/JsonHelper.h \
    Application/MainStatemachine.h \
    Application/Notification.h \
    Application/NotificationCenter.h \
    Application/QmlTypes.h \
    Application/Recipes/LibraryGrammage.h \
    Application/Recipes/LibraryRecipe.h \
    Application/Recipes/RecipeControl.h \
    Application/Recipes/TimeRecipe.h \
    Application/Settings/DiskUsageTime.h \
    Application/Settings/RadioButtonMenu.h \
    Application/Settings/RadioButtonMenuItem.h \
    Application/Settings/SettingsMenu.h \
    Application/Settings/SettingsMenuContentFactory.h \
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

RESOURCES += EkxUi.qrc

target.path = /usr/local/bin
INSTALLS += target
