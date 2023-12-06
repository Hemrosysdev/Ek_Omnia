QT += quick
QT += serialport
QT += core
QT += sql
QT += widgets

TEMPLATE = app

#CONFIG += console
CONFIG += c++17
CONFIG += qml_debug

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Refer to the documentation for the
# deprecated API to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += SIMULATION_BUILD

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

# is not a shadow build
equals(PWD, $${OUT_PWD}) {
    CONFIG( release,release|debug ): BUILD_DIR=$$OUT_PWD/build_$${TARGETDEVICE}_release
    CONFIG( debug,debug|release ): BUILD_DIR=$$OUT_PWD/build_$${TARGETDEVICE}_debug

    #BUILD_DIR=$$OUT_PWD/builddir
    OUT_DIR=$$BUILD_DIR
    message( "set own dir " $$OUT_PWD )
    message( "set own dir " $$BUILD_DIR )
} else {
    message( "Do shadow build into" $$OUT_PWD )
    BUILD_DIR=$$OUT_PWD
}

linux: {
    versiontarget.target =  $$OUT_PWD/version.h
    versiontarget.commands = cd $$OUT_PWD;make -f $${_PRO_FILE_PWD_}/version.mk
    versiontarget.depends = FORCE

    PRE_TARGETDEPS +=  $$OUT_PWD/version.h
    QMAKE_EXTRA_TARGETS += versiontarget
}

!exists($$BUILD_DIR) {
    system( mkdir -p $${BUILD_DIR} )
}

DESTDIR=$${BUILD_DIR}
#DESTDIR=.

MOC_DIR     = $${DESTDIR}/moc
UI_DIR      = $${DESTDIR}/ui
OBJECTS_DIR = $${DESTDIR}/obj
RCC_DIR     = $${DESTDIR}/rcc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Additional import path used to resolve QML modules just for Qt Quick Designer
QML_DESIGNER_IMPORT_PATH =

INCLUDEPATH += ../EkxCommon
INCLUDEPATH += ../EkxUI/Application
INCLUDEPATH += ../EkxUI/Application/Drivers
INCLUDEPATH += ../EkxUI/Application/Drivers/Uppp/UpppCore
INCLUDEPATH += ../EkxUI/Application/Drivers/Uppp/UpppMcu
INCLUDEPATH += ../EkxUI/Application/Drivers/EspDrivers
INCLUDEPATH += ../EkxUI/Application/Drivers/EspDrivers/EkxProtocol
INCLUDEPATH += ../EkxUI/Application/Drivers/HmiDrivers
INCLUDEPATH += ../EkxUI/Application/Recipes
INCLUDEPATH += ../EkxUI/Application/Settings
INCLUDEPATH += ../EkxUI/Application/Statistics
INCLUDEPATH += ../EkxUI/Application/Database
INCLUDEPATH += $${BUILD_DIR}

DISTFILES += \
    ../../manifest/version.info \
    ../EkxUI/version.info \
    version.info \
    version.mk

SOURCES += \
        ../EkxCommon/ColoredMessageHandler.cpp \
        ../EkxCommon/LockableFile.cpp \
        ../EkxCommon/UnixSignalCatcher.cpp \
        ../EkxUI/Application/AcyclicNotification.cpp \
        ../EkxUI/Application/Database/BufferedQueryWriter.cpp \
        ../EkxUI/Application/Database/EkxSqliteTypes.cpp \
        ../EkxUI/Application/Drivers/AgsaControl.cpp \
        ../EkxUI/Application/Drivers/AgsaLongDurationTest.cpp \
        ../EkxUI/Application/Drivers/EspDrivers/EkxProtocol/EkxProtocol.cpp \
        ../EkxUI/Application/Drivers/EspDrivers/EspDcHallMotorDriver.cpp \
        ../EkxUI/Application/Drivers/EspDrivers/EspDeviceDriver.cpp \
        ../EkxUI/Application/Drivers/EspDrivers/EspProtocolStack.cpp \
        ../EkxUI/Application/Drivers/EspDrivers/EspSerialDriver.cpp \
        ../EkxUI/Application/Drivers/EspDrivers/EspStepperMotorDriver.cpp \
        ../EkxUI/Application/Drivers/EspDrivers/HttpServerDriver.cpp \
        ../EkxUI/Application/Drivers/EspDrivers/JsonApi.cpp \
        ../EkxUI/Application/Drivers/EspDrivers/esp_rom_crc.c \
        ../EkxUI/Application/Drivers/HmiDrivers/AdcDmaDemuxer.cpp \
        ../EkxUI/Application/Drivers/HmiDrivers/AdcDmaReader.cpp \
        ../EkxUI/Application/Drivers/HmiDrivers/AdcVoltageInDriver.cpp \
        ../EkxUI/Application/Drivers/HmiDrivers/CpuMp157Driver.cpp \
        ../EkxUI/Application/Drivers/HmiDrivers/DddCouple.cpp \
        ../EkxUI/Application/Drivers/HmiDrivers/DeviceMemoryIo.cpp \
        ../EkxUI/Application/Drivers/HmiDrivers/GpioOut.cpp \
        ../EkxUI/Application/Drivers/HmiDrivers/PwmCaptureDriver.cpp \
        ../EkxUI/Application/Drivers/HmiDrivers/SystemDeviceFile.cpp \
        ../EkxUI/Application/JsonHelper.cpp \
        ../EkxUI/Application/Notification.cpp \
        ../EkxUI/Application/NotificationCenter.cpp \
        ../EkxUI/Application/Settings/DiskUsageTime.cpp \
        ../EkxUI/Application/Settings/SettingsMenuContentFactory.cpp \
        ../EkxUI/Application/Drivers/DeviceInfoCollector.cpp \
        ../EkxUI/Application/Drivers/EspDrivers/At24c16Driver.cpp \
        ../EkxUI/Application/Drivers/EspDrivers/EspDriver.cpp \
        ../EkxUI/Application/Drivers/EspDrivers/EspSwUpdateDriver.cpp \
        ../EkxUI/Application/Drivers/EspDrivers/SpotLightDriver.cpp \
        ../EkxUI/Application/Drivers/EspDrivers/StartStopLightDriver.cpp \
        ../EkxUI/Application/Drivers/EspDrivers/NtcTempDriver.cpp \
        ../EkxUI/Application/Drivers/EspDrivers/StartStopButtonDriver.cpp \
        ../EkxUI/Application/Drivers/EspDrivers/WifiDriver.cpp \
        ../EkxUI/Application/Drivers/EspDrivers/EspMcuDriver.cpp \
        ../EkxUI/Application/Drivers/HmiDrivers/DddDriver.cpp \
        ../EkxUI/Application/Drivers/HmiDrivers/DisplayController.cpp \
        ../EkxUI/Application/Drivers/Uppp/UpppCore/CcittCrc16.cpp \
        ../EkxUI/Application/Drivers/Uppp/UpppCore/UpppCore.cpp \
        ../EkxUI/Application/Drivers/Uppp/UpppCore/UpppCoreMsg.cpp \
        ../EkxUI/Application/Drivers/Uppp/UpppCore/UpppLevel3.cpp \
        ../EkxUI/Application/Drivers/Uppp/UpppCore/UpppLevel3Msg.cpp \
        ../EkxUI/Application/Drivers/Uppp/UpppMcu/McuDriver.cpp \
        ../EkxUI/Application/Drivers/Uppp/UpppMcu/UpppMcu.cpp \
        ../EkxUI/Application/EkxQuickView.cpp \
        ../EkxUI/Application/ElapsedTimer.cpp \
        ../EkxUI/Application/MainStatemachine.cpp \
        ../EkxUI/Application/QmlTypes.cpp \
        ../EkxUI/Application/Recipes/LibraryRecipe.cpp \
        ../EkxUI/Application/Recipes/LibraryGrammage.cpp \
        ../EkxUI/Application/Recipes/RecipeControl.cpp \
        ../EkxUI/Application/Recipes/TimeRecipe.cpp \
        ../EkxUI/Application/Settings/RadioButtonMenu.cpp \
        ../EkxUI/Application/Settings/RadioButtonMenuItem.cpp \
        ../EkxUI/Application/Settings/SettingsMenu.cpp \
        ../EkxUI/Application/Settings/SettingsSerializer.cpp \
        ../EkxUI/Application/Settings/SettingsStatemachine.cpp \
        ../EkxUI/Application/Settings/StatisticsBoard.cpp \
        ../EkxUI/Application/Settings/StatisticsBoardResult.cpp \
        ../EkxUI/Application/Database/SqlQueryWorker.cpp \
        ../EkxUI/Application/Database/SqliteInterface.cpp \
        ../EkxUI/Application/StandbyController.cpp \
        ../EkxUI/Application/Statistics/StatisticsRecord.cpp \
        ../EkxUI/Application/Statistics/TimePeriod.cpp \
        ../EkxUI/Application/SwUpdateController.cpp \
        main.cpp

HEADERS += \
    ../EkxCommon/ColoredMessageHandler.h \
    ../EkxCommon/LockableFile.h \
    ../EkxCommon/UnixSignalCatcher.h \
    ../EkxUI/Application/AcyclicNotification.h \
    ../EkxUI/Application/Database/BufferedQueryWriter.h \
    ../EkxUI/Application/Drivers/AgsaControl.h \
    ../EkxUI/Application/Drivers/AgsaLongDurationTest.h \
    ../EkxUI/Application/Drivers/EspDrivers/EkxProtocol/EkxProtocol.h \
    ../EkxUI/Application/Drivers/EspDrivers/EspDcHallMotorDriver.h \
    ../EkxUI/Application/Drivers/EspDrivers/EspDeviceDriver.h \
    ../EkxUI/Application/Drivers/EspDrivers/EspProtocolStack.h \
    ../EkxUI/Application/Drivers/EspDrivers/EspSerialDriver.h \
    ../EkxUI/Application/Drivers/EspDrivers/EspStepperMotorDriver.h \
    ../EkxUI/Application/Drivers/EspDrivers/HttpServerDriver.h \
    ../EkxUI/Application/Drivers/EspDrivers/JsonApi.h \
    ../EkxUI/Application/Drivers/EspDrivers/esp_rom_crc.h \
    ../EkxUI/Application/Drivers/HmiDrivers/AdcDmaDemuxer.h \
    ../EkxUI/Application/Drivers/HmiDrivers/AdcDmaReader.h \
    ../EkxUI/Application/Drivers/HmiDrivers/AdcVoltageInDriver.h \
    ../EkxUI/Application/Drivers/HmiDrivers/ArithmeticFilter.h \
    ../EkxUI/Application/Drivers/HmiDrivers/CpuMp157Driver.h \
    ../EkxUI/Application/Drivers/HmiDrivers/DddCouple.h \
    ../EkxUI/Application/Drivers/HmiDrivers/DeviceMemoryIo.h \
    ../EkxUI/Application/Drivers/HmiDrivers/GpioOut.h \
    ../EkxUI/Application/Drivers/HmiDrivers/PwmCaptureDriver.h \
    ../EkxUI/Application/Drivers/HmiDrivers/SystemDeviceFile.h \
    ../EkxUI/Application/JsonHelper.h \
    ../EkxUI/Application/Notification.h \
    ../EkxUI/Application/NotificationCenter.h \
    ../EkxUI/Application/Settings/DiskUsageTime.h \
    ../EkxUI/Application/Settings/SettingsMenuContentFactory.h \
    ../EkxUI/Application/Database/EkxSqliteTypes.h \
    ../EkxUI/Application/Drivers/DeviceInfoCollector.h \
    ../EkxUI/Application/Drivers/EspDrivers/At24c16Driver.h \
    ../EkxUI/Application/Drivers/EspDrivers/EspDriver.h \
    ../EkxUI/Application/Drivers/EspDrivers/EspSwUpdateDriver.h \
    ../EkxUI/Application/Drivers/EspDrivers/SpotLightDriver.h \
    ../EkxUI/Application/Drivers/EspDrivers/StartStopLightDriver.h \
    ../EkxUI/Application/Drivers/EspDrivers/NtcTempDriver.h \
    ../EkxUI/Application/Drivers/EspDrivers/StartStopButtonDriver.h \
    ../EkxUI/Application/Drivers/EspDrivers/WifiDriver.h \
    ../EkxUI/Application/Drivers/EspDrivers/EspMcuDriver.h \
    ../EkxUI/Application/Drivers/HmiDrivers/DddDriver.h \
    ../EkxUI/Application/Drivers/HmiDrivers/DisplayController.h \
    ../EkxUI/Application/Drivers/Uppp/UpppCore/CcittCrc16.h \
    ../EkxUI/Application/Drivers/Uppp/UpppCore/UpppCore.h \
    ../EkxUI/Application/Drivers/Uppp/UpppCore/UpppCoreMsg.h \
    ../EkxUI/Application/Drivers/Uppp/UpppCore/UpppLevel3.h \
    ../EkxUI/Application/Drivers/Uppp/UpppCore/UpppLevel3Msg.h \
    ../EkxUI/Application/Drivers/Uppp/UpppMcu/McuDriver.h \
    ../EkxUI/Application/Drivers/Uppp/UpppMcu/UpppMcu.h \
    ../EkxUI/Application/EkxGlobals.h \
    ../EkxUI/Application/EkxQuickView.h \
    ../EkxUI/Application/ElapsedTimer.h \
    ../EkxUI/Application/MainStatemachine.h \
    ../EkxUI/Application/QmlTypes.h \
    ../EkxUI/Application/Recipes/LibraryRecipe.h \
    ../EkxUI/Application/Recipes/LibraryGrammage.h \
    ../EkxUI/Application/Recipes/RecipeControl.h \
    ../EkxUI/Application/Recipes/TimeRecipe.h \
    ../EkxUI/Application/Settings/RadioButtonMenu.h \
    ../EkxUI/Application/Settings/RadioButtonMenuItem.h \
    ../EkxUI/Application/Settings/SettingsMenu.h \
    ../EkxUI/Application/Settings/SettingsSerializer.h \
    ../EkxUI/Application/Settings/SettingsStatemachine.h \
    ../EkxUI/Application/Settings/StatisticsBoard.h \
    ../EkxUI/Application/Settings/StatisticsBoardResult.h \
    ../EkxUI/Application/Database/SqlQueryWorker.h \
    ../EkxUI/Application/Database/SqliteInterface.h \
    ../EkxUI/Application/StandbyController.h \
    ../EkxUI/Application/Statistics/StatisticsRecord.h \
    ../EkxUI/Application/Statistics/TimePeriod.h \
    ../EkxUI/Application/SwUpdateController.h

RESOURCES += ../EkxUI/EkxUi.qrc
RESOURCES += EkxHmiDemonstrator.qrc

target.path = /usr/local/bin
INSTALLS += target

ICON = Images/fav_32x32.png

win32:RC_ICONS += Images/fav_32x32.ico

VERSION = $${VERSION_NO}
QMAKE_TARGET_COMPANY = "Ultratronik GmbH"
QMAKE_TARGET_DESCRIPTION = "Hemro EKX grinder demonstrator of HMI functionality"
QMAKE_TARGET_COPYRIGHT = "2021 by Hemro Group AG"
QMAKE_TARGET_PRODUCT = "Hemro EKX Demonstrator"
RC_LANG=de
RC_CODEPAGE=1252
