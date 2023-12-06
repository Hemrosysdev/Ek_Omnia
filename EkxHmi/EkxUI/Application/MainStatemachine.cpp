///////////////////////////////////////////////////////////////////////////////
///
/// @file MainStatemachine.cpp
///
/// @brief Implementation file of class MainStatemachine.
///
/// @author Ultratronik GmbH
///         Dornierstr. 9
///         D-82205 Gilching
///         Germany
///         http://www.ultratronik.de
///
/// @author written by Florian Sonntag, Forschung & Entwicklung, gesser@ultratronik.de
///
/// @date 31.03.2021
///
/// @copyright Copyright 2021 by Hemro International AG
///            Hemro International AG
///            Länggenstrasse 34
///            CH 8184 Bachenbülach
///            Switzerland
///            Homepage: www.hemrogroup.com
///
///////////////////////////////////////////////////////////////////////////////

#include <QQmlContext>
#include <QQmlEngine>
#include <QtDebug>
#include <QDir>
#include <QJsonArray>
#include <QJsonObject>
#include <QProcess>
#include <QJsonDocument>

#include "MainStatemachine.h"
#include "EkxGlobals.h"
#include "McuDriver.h"
#include "StandbyController.h"
#include "DisplayController.h"
#include "EspDriver.h"
#include "SpotLightDriver.h"
#include "StartStopLightDriver.h"
#include "EspMcuDriver.h"
#include "StartStopButtonDriver.h"
#include "SqliteInterface.h"
#include "NotificationCenter.h"
#include "DiskUsageTime.h"
#include "JsonHelper.h"
#include "EspDcHallMotorDriver.h"
#include "EspStepperMotorDriver.h"
#include "LockableFile.h"
#include "SettingsMenuContentFactory.h"
#include "StatisticsBoard.h"
#include "NtcTempDriver.h"
#include "AgsaControl.h"
#include "AgsaLongDurationTest.h"
#include "DeviceInfoCollector.h"
#include "DddCouple.h"
#include "DddDriver.h"
#include "AdcDmaDemuxer.h"
#include "AdcDmaReader.h"
#include "AdcVoltageInDriver.h"
#include "RecipeControl.h"
#include "SettingsStatemachine.h"
#include "SwUpdateController.h"
#include "WifiDriver.h"
#include "At24c16Driver.h"
#include "EspSwUpdateDriver.h"
#include "CpuMp157Driver.h"
#include "EspProtocolStack.h"

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#define STATE_JSON                      "State"
#define STATE_TIME_RECIPE               "StateTimeRecipe"
#define STATE_LIBRARY_RECIPE            "StateLibraryRecipe"
#define STATE_LIBRARY_GRAMMAGE          "StateLibraryGrammage"

#define SAVE_STATE_PATH                 "/config/LastState.json"
#define CUSTOM_SPLASH_SCREEN_FILE       "/config/custom-splash-screen"

#define MAX_GRIND_DURATION              300000     // 300 secs
#define AUTOLOGOUT_TIMEOUT              180000     // 3 min

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

MainStatemachine::MainStatemachine( QQmlEngine * pEngine,
                                    QObject *    parent )
    : QObject( parent )
    , m_pNotificationCenter( new NotificationCenter() )
    , m_pSqliteInterface( new SqliteInterface() )
    , m_pRecipeControl( new RecipeControl() )
    , m_pEspDriver( new EspDriver() )
    , m_pDddCouple( new DddCouple() )
    , m_pDddCoarseDriver( new SystemIo::DddDriver() )
    , m_pDddFineDriver( new SystemIo::DddDriver() )
    , m_pDisplayController( new DisplayController() )
    , m_pStandbyController( new StandbyController() )
    , m_pDeviceInfoCollector( new DeviceInfoCollector() )
    , m_pSwUpdateController( new SwUpdateController() )
    , m_pSettingsSerializer( new SettingsSerializer() )
    , m_pSettingsStatemachine( new SettingsStatemachine() )
    , m_pAgsaControl( new AgsaControl( this ) )
    , m_pAdcDmaDemuxer( new SystemIo::AdcDmaDemuxer() )
    , m_pCpuMp157Driver( new SystemIo::CpuMp157Driver() )
{
    ensureExistingDirectory( rootPath() + SAVE_STATE_PATH );
    ensureExistingDirectory( rootPath() + CUSTOM_SPLASH_SCREEN_FILE );

#ifdef TARGETBUILD
    m_bIgnoreMcu = QFile( rootPath() + "/config/mcu_ignored" ).exists();

    // TODO: remove if new protocol is released to master branch
    ::system( "killall slattach" );
#else
    m_bIgnoreMcu = true;
#endif

    if ( m_bIgnoreMcu )
    {
        qWarning() << "MainStatemachine(): MCU will be ignored";
    }

    prepareContextProperties( pEngine );
    prepareComponents();
    prepareConnections();

    m_timerSaveEkxMainstate.setInterval( 5000 );   // 5secs
    m_timerSaveEkxMainstate.setSingleShot( true );

    m_timerMaxGrindDuration.setInterval( MAX_GRIND_DURATION );
    m_timerMaxGrindDuration.setSingleShot( true );

    m_timerOneMinute.setInterval( 60000 );
    m_timerOneMinute.setSingleShot( false );
    m_timerOneMinute.start();

    m_timerStandbyCounter.setInterval( 60000 );
    m_timerStandbyCounter.setSingleShot( false );

    m_timerDelayedMotorStop.setInterval( 2000 );
    m_timerDelayedMotorStop.setSingleShot( true );

    m_timerAutoLogout.setInterval( AUTOLOGOUT_TIMEOUT );
    m_timerAutoLogout.setSingleShot( true );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

MainStatemachine::~MainStatemachine()
{
    m_timerOneMinute.stop();
    m_timerStandbyCounter.stop();
    m_timerMaxGrindDuration.stop();
    m_timerSaveEkxMainstate.stop();
    m_timerDelayedMotorStop.stop();
    m_timerAutoLogout.stop();

    disconnect( m_pEspDriver->startStopButtonDriver(), &StartStopButtonDriver::buttonStateChanged, this, &MainStatemachine::processStartStopButton );
    disconnect( m_pSwUpdateController, &SwUpdateController::swUpdateStarted, this, &MainStatemachine::processSwUpdateStart );
    disconnect( m_pSwUpdateController, &SwUpdateController::swUpdateStopped, this, &MainStatemachine::processSwUpdateStop );
    disconnect( &m_timerSaveEkxMainstate, &QTimer::timeout, this, &MainStatemachine::saveEkxMainState );

    m_pCpuMp157Driver = nullptr;
    m_pAdcDmaDemuxer  = nullptr;

    delete m_pDddCoarseDriver;
    m_pDddCoarseDriver = nullptr;

    delete m_pDddFineDriver;
    m_pDddFineDriver = nullptr;

    delete m_pAgsaControl;
    m_pAgsaControl = nullptr;

    delete m_pNotificationCenter;
    m_pNotificationCenter = nullptr;

    delete m_pDisplayController;
    m_pDisplayController = nullptr;

    delete m_pStandbyController;
    m_pStandbyController = nullptr;

    delete m_pDeviceInfoCollector;
    m_pDeviceInfoCollector = nullptr;

    delete m_pDddCouple;
    m_pDddCouple = nullptr;

    delete m_pEspDriver;
    m_pEspDriver = nullptr;

    delete m_pRecipeControl;
    m_pRecipeControl = nullptr;

    delete m_pSwUpdateController;
    m_pSwUpdateController = nullptr;

    delete m_pSettingsStatemachine;
    m_pSettingsStatemachine = nullptr;

    delete m_pSqliteInterface;
    m_pSqliteInterface = nullptr;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void MainStatemachine::prepareContextProperties( QQmlEngine * const pEngine )
{
    pEngine->rootContext()->setContextProperty( "mainStatemachine", this );
    pEngine->rootContext()->setContextProperty( "notificationCenter", m_pNotificationCenter );
    pEngine->rootContext()->setContextProperty( "dddCouple", m_pDddCouple );
    pEngine->rootContext()->setContextProperty( "ddd1Driver", m_pDddCoarseDriver );
    pEngine->rootContext()->setContextProperty( "ddd2Driver", m_pDddFineDriver );
    pEngine->rootContext()->setContextProperty( "settingsSerializer", m_pSettingsSerializer );
    pEngine->rootContext()->setContextProperty( "settingsStateMachine", m_pSettingsStatemachine );
    pEngine->rootContext()->setContextProperty( "settingsMenuContentFactory", &m_pSettingsStatemachine->contentFactory() );
    pEngine->rootContext()->setContextProperty( "statisticsBoard", &m_pSettingsStatemachine->statisticsBoard() );
    pEngine->rootContext()->setContextProperty( "diskUsageTime", &m_pSettingsStatemachine->diskUsageTime() );
    pEngine->rootContext()->setContextProperty( "standbyController", m_pStandbyController );
    pEngine->rootContext()->setContextProperty( "deviceInfoCollector", m_pDeviceInfoCollector );
    pEngine->rootContext()->setContextProperty( "recipeControl", m_pRecipeControl );
    pEngine->rootContext()->setContextProperty( "displayController", m_pDisplayController );
    pEngine->rootContext()->setContextProperty( "swUpdateController", m_pSwUpdateController );
    pEngine->rootContext()->setContextProperty( "pduDcHallMotorDriver", m_pEspDriver->pduDcHallMotorDriver() );
    pEngine->rootContext()->setContextProperty( "agsaStepperMotorDriver", m_pEspDriver->agsaStepperMotorDriver() );
    pEngine->rootContext()->setContextProperty( "ntcTempDriver", m_pEspDriver->ntcTempDriver() );
    pEngine->rootContext()->setContextProperty( "wifiDriver", m_pEspDriver->wifiDriver() );
    pEngine->rootContext()->setContextProperty( "at24c16Driver", m_pEspDriver->at24c16Driver() );
    pEngine->rootContext()->setContextProperty( "mcuDriver", m_pEspDriver->espMcuDriver()->mcuDriver() );
    pEngine->rootContext()->setContextProperty( "spotLightDriver", m_pEspDriver->spotLightDriver() );
    pEngine->rootContext()->setContextProperty( "startStopLightDriver", m_pEspDriver->startStopLightDriver() );
    pEngine->rootContext()->setContextProperty( "agsaControl", agsaControl() );
    pEngine->rootContext()->setContextProperty( "agsaLongDurationTest", agsaControl()->longDurationTest() );
    pEngine->rootContext()->setContextProperty( "sqliteInterface", m_pSqliteInterface );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void MainStatemachine::prepareComponents()
{
#ifdef TARGETBUILD
    m_pCpuMp157Driver->dumpRegisterAddressMap();
    m_pCpuMp157Driver->readAdcParameters();
#endif

    // lambda function used for DMA reading
    auto fakeStartDmaReadFunc = []()
                                {
                                };
    auto ddd1AdcProcessFunc = [this]( const int nAdcValue )
                              {
                                  m_pDddCoarseDriver->adcVoltageInDriver()->processDmaValue( nAdcValue );
                              };
    auto ddd2AdcProcessFunc = [this]( const int nAdcValue )
                              {
                                  m_pDddFineDriver->adcVoltageInDriver()->processDmaValue( nAdcValue );
                              };

    m_acycNotificationStateFileCorrupted.create( m_pNotificationCenter,
                                                 EkxSqliteTypes::SqliteNotificationType::SqliteNotificationType_MAIN_STATE_FILE_CORRUPTED );
    m_acycNotificationStateFileWrite.create( m_pNotificationCenter,
                                             EkxSqliteTypes::SqliteNotificationType::SqliteNotificationType_MAIN_STATE_FILE_WRITE_FAILED );

    m_pDeviceInfoCollector->create( this );
    m_pNotificationCenter->create( this );
    m_pSqliteInterface->create( notificationCenter(),
                                settingsSerializer() );

    m_pSettingsSerializer->create( this,
                                   m_pEspDriver->pduDcHallMotorDriver(),
                                   agsaControl(),
                                   m_pDeviceInfoCollector,
                                   m_pEspDriver->wifiDriver() );

    m_pSettingsStatemachine->create( this,
                                     m_pSettingsSerializer,
                                     m_pSqliteInterface );

    m_pRecipeControl->create( this );
    m_pEspDriver->create( this );
    m_pStandbyController->create( this );

    m_pAdcDmaDemuxer->create( "/dev/iio:device3", AdcDemuxerPosition::LastOrDemuxerSize );
    m_pAdcDmaDemuxer->addChannel( AdcDemuxerPosition::Ddd1,
                                  fakeStartDmaReadFunc,
                                  ddd1AdcProcessFunc );
    m_pAdcDmaDemuxer->addChannel( AdcDemuxerPosition::Ddd2,
                                  fakeStartDmaReadFunc,
                                  ddd2AdcProcessFunc );

    SystemIo::DddDriver::DddType nDddType = SystemIo::DddDriver::DddType::DddTypeAdc;
    if ( m_pDeviceInfoCollector->isDddPwmVariant() )
    {
        nDddType = SystemIo::DddDriver::DddType::DddTypePwm;
    }

    m_pDddCoarseDriver->create( "DddCoarseDriver",
                                nDddType,
                                0x44000000,
                                4,
                                3,
                                "/sys/bus/platform/devices/48003000.adc:adc@100/iio:device3",
                                "11",
                                50,
                                10 );
    m_pDddFineDriver->create( "DddFineDriver",
                              nDddType,
                              0x44000000,
                              4,
                              0,
                              "/sys/bus/platform/devices/48003000.adc:adc@100/iio:device3",
                              "19",
                              50,
                              10 );
    m_pDddCouple->create( this, m_pDddCoarseDriver, m_pDddFineDriver );

    m_pSwUpdateController->create( this );
    m_pAgsaControl->create( deviceInfoCollector()->ekxUiSerialNo(),
                            espDriver()->agsaStepperMotorDriver(),
                            dddCouple() );

    m_pDisplayController->setBrightness( m_pSettingsSerializer->displayBrightness() );
    m_pDisplayController->setBrightnessDisable( m_pStandbyController->isStandbyActive() );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void MainStatemachine::prepareConnections()
{
    connect( swUpdateController(), &SwUpdateController::swUpdateStarted, this, &MainStatemachine::processSwUpdateStart );
    connect( swUpdateController(), &SwUpdateController::swUpdateStopped, this, &MainStatemachine::processSwUpdateStop );

    connect( recipeControl(), &RecipeControl::currentTimeRecipeIndexChanged, this, &MainStatemachine::processTimeRecipeIndexChanged );
    connect( recipeControl(), &RecipeControl::currentLibraryIndexChanged, this, &MainStatemachine::processLibraryRecipeIndexChanged );
    connect( recipeControl(), &RecipeControl::currentLibraryGrammageIndexChanged, this, &MainStatemachine::processLibraryGrammageIndexChanged );

    connect( espDriver(), &EspDriver::connectedChanged, notificationCenter(), &NotificationCenter::processEspInterfaceFailure );

    connect( agsaControl(), &AgsaControl::failedChanged, notificationCenter(), &NotificationCenter::processAgsaFailure );

    connect( espDriver()->startStopButtonDriver(), &StartStopButtonDriver::buttonStateChanged, this, &MainStatemachine::processStartStopButton );
    connect( espDriver()->startStopButtonDriver(), &StartStopButtonDriver::buttonStateChanged, standbyController(), &StandbyController::wakeUp );

    connect( espDriver()->pduDcHallMotorDriver(), &EspDcHallMotorDriver::validChanged, this, &MainStatemachine::checkPduStatus );
    connect( espDriver()->pduDcHallMotorDriver(), &EspDcHallMotorDriver::motorTestChanged, this, &MainStatemachine::checkPduStatus );
    connect( espDriver()->pduDcHallMotorDriver(), &EspDcHallMotorDriver::faultPinActiveChanged, notificationCenter(), &NotificationCenter::processPduFaultPinActiveFailure );

    connect( espDriver()->espMcuDriver()->mcuDriver(), &McuDriver::mcuAliveChanged, this, &MainStatemachine::checkPduStatus );
    connect( espDriver()->espMcuDriver()->mcuDriver(), &McuDriver::systemStatusHopperDismountedChanged, this, &MainStatemachine::checkPduStatus );
    connect( espDriver()->espMcuDriver()->mcuDriver(), &McuDriver::productIdValidChanged, deviceInfoCollector(), &DeviceInfoCollector::collectMcuVersion );
    connect( espDriver()->espMcuDriver()->mcuDriver(), &McuDriver::productIdRequestTimeout, deviceInfoCollector(), &DeviceInfoCollector::mcuVersionTimeout );
    connect( espDriver()->espMcuDriver()->mcuDriver(), &McuDriver::mcuAliveChanged, notificationCenter(), &NotificationCenter::processMcuInterfaceFailure );

    connect( espDriver()->at24c16Driver(), &At24c16Driver::productIdChanged, deviceInfoCollector(), &DeviceInfoCollector::collectEspVersion );
    connect( espDriver()->at24c16Driver(), &At24c16Driver::serialNumberChanged, deviceInfoCollector(), &DeviceInfoCollector::collectEspVersion );
    connect( espDriver()->at24c16Driver(), &At24c16Driver::validChanged, deviceInfoCollector(), &DeviceInfoCollector::collectEspVersion );

    connect( espDriver()->swUpdateDriver(), &EspSwUpdateDriver::statusChanged, deviceInfoCollector(), &DeviceInfoCollector::collectEspVersion );

    connect( espDriver()->agsaStepperMotorDriver(), &EspStepperMotorDriver::runningChanged, agsaControl(), &AgsaControl::processRunningChanged );
    connect( espDriver()->agsaStepperMotorDriver(), &EspStepperMotorDriver::runningChanged, agsaControl()->longDurationTest(), &AgsaLongDurationTest::processMotorRunningChanged );
    connect( espDriver()->agsaStepperMotorDriver(), &EspStepperMotorDriver::faultPinActiveChanged, notificationCenter(), &NotificationCenter::processAgsaFaultPinActiveFailure );

    connect( standbyController(), &StandbyController::standbyChanged, this, &MainStatemachine::processStandbyChanged );
    connect( standbyController(), &StandbyController::settingsModeExitTimeout, this, &MainStatemachine::processSettingsModeExitTimeout );
    connect( standbyController(), &StandbyController::standbyChanged, displayController(), &DisplayController::setBrightnessDisable );
    connect( standbyController(), &StandbyController::standbyChanged, espDriver()->spotLightDriver(), &SpotLightDriver::processStandbyChanged );
    connect( standbyController(), &StandbyController::standbyChanged, espDriver()->startStopLightDriver(), &StartStopLightDriver::processStandbyChanged );

    connect( settingsSerializer(), &SettingsSerializer::mcuConfigChanged, espDriver()->espMcuDriver(), &EspMcuDriver::processMcuConfigChanged );
    connect( settingsSerializer(), &SettingsSerializer::showSpotLightChanged, espDriver()->spotLightDriver(), &SpotLightDriver::processShowSpotLightChanged );
    connect( settingsSerializer(), &SettingsSerializer::wifiModeChanged, espDriver()->wifiDriver(), &WifiDriver::processWifiModeChanged );
    connect( settingsSerializer(), &SettingsSerializer::displayBrightnessChanged, displayController(), &DisplayController::setBrightness );
    connect( settingsSerializer(), &SettingsSerializer::showTempWarningChanged, espDriver()->ntcTempDriver(), &NtcTempDriver::updateOvertempWarning );
    connect( settingsSerializer(), &SettingsSerializer::discUsageMaxHoursChanged, &settingsStatemachine()->statisticsBoard(), &StatisticsBoard::setDiskUsageMaxHours );
    connect( settingsSerializer(), &SettingsSerializer::discUsageMaxHoursChanged, &settingsStatemachine()->diskUsageTime(), &DiskUsageTime::setMaxDiscUsageTimeHours );
    connect( settingsSerializer(), &SettingsSerializer::stdbyTimeIndexChanged, standbyController(), &StandbyController::processStandbyTimeChanged );
    connect( settingsSerializer(), &SettingsSerializer::settingsModeExitTimeIndexChanged, standbyController(), &StandbyController::processSettingsModeExitTimeChanged );
    connect( settingsSerializer(), &SettingsSerializer::userAutoLogoutEnableChanged, this, &MainStatemachine::startOrStopAutoLogoutTimer );
    connect( settingsSerializer(), &SettingsSerializer::currentLoggedInUserRoleChanged, this, &MainStatemachine::startOrStopAutoLogoutTimer );

    connect( notificationCenter(), &NotificationCenter::recoverAction, this, &MainStatemachine::processRecoverAction );
    connect( notificationCenter(), &NotificationCenter::popNextRecoverActionStep, this, &MainStatemachine::processNextRecoverActionStep );

    connect( settingsStatemachine(), &SettingsStatemachine::popNextRecoverActionStep, this, &MainStatemachine::processNextRecoverActionStep );

    connect( this, &MainStatemachine::ekxMainstateChanged, standbyController(), &StandbyController::prelongStandbyTrigger );
    connect( this, &MainStatemachine::ekxMainstateChanged, settingsStatemachine(), &SettingsStatemachine::processMainstateChanged );
    connect( this, &MainStatemachine::ekxMainstateChanged, recipeControl(), &RecipeControl::processMainStateChanged );

    connect( &m_timerSaveEkxMainstate, &QTimer::timeout, this, &MainStatemachine::saveEkxMainState );
    connect( &m_timerMaxGrindDuration, &QTimer::timeout, this, &MainStatemachine::processMaxGrindDuration );
    connect( &m_timerOneMinute, &QTimer::timeout, m_pSqliteInterface, &SqliteInterface::incrementSystemOnTimeCounterByMinute );
    connect( &m_timerStandbyCounter, &QTimer::timeout, m_pSqliteInterface, &SqliteInterface::incrementStandbyTimeCounterByMinute );
    connect( &m_timerDelayedMotorStop, &QTimer::timeout, this, &MainStatemachine::processTimeoutDelayedMotorStop );
    connect( &m_timerAutoLogout, &QTimer::timeout, this, &MainStatemachine::processTimeoutAutoLogout );

    connect( sqliteInterface(), &SqliteInterface::queryAborted, &settingsStatemachine()->statisticsBoard(), &StatisticsBoard::processQueryAbort );
    connect( sqliteInterface(), &SqliteInterface::queryFinished, &settingsStatemachine()->statisticsBoard(), &StatisticsBoard::processQueryFinish );

    connect( dddCouple(), &DddCouple::rawDddValueChanged, agsaControl(), &AgsaControl::processRawDddValue );
    connect( dddCouple(), &DddCouple::dddValueChanged, agsaControl(), &AgsaControl::processDddValue );
    connect( dddCouple(), &DddCouple::failedChanged, agsaControl(), &AgsaControl::processDddFailed );
    connect( dddCouple(), &DddCouple::dddValueChanged, agsaControl()->longDurationTest(), &AgsaLongDurationTest::processDddValue );
    connect( dddCouple(), &DddCouple::dddValueChanged, standbyController(), &StandbyController::wakeUp );

    connect( dddFineDriver(), &SystemIo::DddDriver::failedChanged, notificationCenter(), &NotificationCenter::processDddFineFailure );
    connect( dddFineDriver(), &SystemIo::DddDriver::failedChanged, dddCouple(), &DddCouple::failedChanged );
    connect( dddFineDriver(), &SystemIo::DddDriver::rawAngleChanged, dddCouple(), &DddCouple::scaleRawToDddValue );
    connect( dddFineDriver(), &SystemIo::DddDriver::filterAngleChanged, dddCouple(), &DddCouple::scaleToDddValue );
    notificationCenter()->processDddFineFailure( dddFineDriver()->isFailed() );

    connect( dddCoarseDriver(), &SystemIo::DddDriver::failedChanged, notificationCenter(), &NotificationCenter::processDddCourseFailure );
    connect( dddCoarseDriver(), &SystemIo::DddDriver::failedChanged, dddCouple(), &DddCouple::failedChanged );
    connect( dddCoarseDriver(), &SystemIo::DddDriver::filterAngleChanged, dddCouple(), &DddCouple::scaleToDddValue );
    notificationCenter()->processDddCourseFailure( dddCoarseDriver()->isFailed() );

    emit dddCouple()->failedChanged( dddCouple()->isDddFailed() );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

int MainStatemachine::maxGrindDuration() const
{
    return MAX_GRIND_DURATION;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

MainStatemachine::EkxMainstates MainStatemachine::ekxMainstate()
{
    return m_nEkxMainstate;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool MainStatemachine::isGrindRunning() const
{
    return m_bGrindRunning;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void MainStatemachine::setGrindRunning( const bool bRunning )
{
    if ( m_bGrindRunning != bRunning )
    {
        if ( bRunning )
        {
            // switch on?
            if ( m_bGrindingAllowed )
            {
                if ( m_pEspDriver->espMcuDriver()->mcuDriver()->startMotor()
                     || m_bIgnoreMcu )
                {
                    qInfo() << "setGrindRunning(): grinding started";

                    if ( m_pEspDriver->pduDcHallMotorDriver()->isMotorTestOk() )
                    {
                        m_pEspDriver->pduDcHallMotorDriver()->startMotor();
                    }

                    m_bGrindRunning = bRunning;
                    emit grindRunningChanged();

                    m_timerMaxGrindDuration.start();

                    // reactions: (better use signal connections)
                    m_pEspDriver->spotLightDriver()->startGrinding();
                    m_pEspDriver->startStopLightDriver()->startGrinding();

                    m_grindingTimer.start();

                    notificationCenter()->removeNotification( EkxSqliteTypes::SqliteNotificationType::SqliteNotificationType_START_GRIND_FAILED );
                }
                else
                {
                    qWarning() << "setGrindRunning(): grinding start failed!";
                    notificationCenter()->raiseNotification( EkxSqliteTypes::SqliteNotificationType::SqliteNotificationType_START_GRIND_FAILED );
                }
            }
            else
            {
                qInfo() << "setGrindRunning(): grinding not allowed!";
            }
        }
        else
        {
            if ( m_pEspDriver->pduDcHallMotorDriver()->isMotorTestOk() )
            {
                m_pEspDriver->pduDcHallMotorDriver()->stopMotor();
                m_timerDelayedMotorStop.start();
            }
            else
            {
                processTimeoutDelayedMotorStop();
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void MainStatemachine::processTimeoutDelayedMotorStop()
{
    // switch off
    if ( m_pEspDriver->espMcuDriver()->mcuDriver()->stopMotor()
         || m_bIgnoreMcu )
    {
        qInfo() << "setGrindRunning(): grinding stopped";

        m_bGrindRunning = false;
        emit grindRunningChanged();

        m_timerMaxGrindDuration.stop();

        // reactions: (better use signal connections)
        m_pEspDriver->spotLightDriver()->stopGrinding();
        m_pEspDriver->startStopLightDriver()->stopGrinding();
        if ( m_grindingTimer.isValid() )
        {
            sqliteInterface()->addGrind( recipeControl()->currentRecipeUuid(), m_grindingTimer.elapsed() / 10 );
            m_grindingTimer.invalidate();
        }

        notificationCenter()->removeNotification( EkxSqliteTypes::SqliteNotificationType::SqliteNotificationType_STOP_GRIND_FAILED );
    }
    else
    {
        qWarning() << "setGrindRunning(): grinding stop failed!";
        notificationCenter()->raiseNotification( EkxSqliteTypes::SqliteNotificationType::SqliteNotificationType_STOP_GRIND_FAILED );
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void MainStatemachine::processTimeoutAutoLogout()
{
    qDebug() << "MainStatemachine: automatic logout";
    m_pSettingsStatemachine->logout();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void MainStatemachine::doFactoryReset()
{
    m_pSettingsSerializer->restoreToFactorySettings();
    m_pRecipeControl->factoryReset();
    resetEkxMainstate();

    QFile::remove( rootPath() + CUSTOM_SPLASH_SCREEN_FILE );
#ifdef TARGETBUILD
    QProcess proc;
    proc.start( "/etc/init.d/S01splashscreen", QStringList() << "restart" );
    proc.waitForFinished();
#endif
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool MainStatemachine::tryActivateEkxMainstate( const MainStatemachine::EkxMainstates nEkxMainstate )
{
    bool bSuccess = true;

    if ( m_nEkxMainstate != nEkxMainstate )
    {
        switch ( nEkxMainstate )
        {
            case EKX_STANDBY:
                bSuccess = tryActivateStandbyState();
                break;
            case EKX_CLASSIC_MODE:
                bSuccess = tryActivateClassicState();
                break;
            case EKX_TIME_MODE:
                bSuccess = tryActivateTimeModeState();
                break;
            case EKX_LIBRARY_MODE:
                bSuccess = tryActivateLibraryModeState();
                break;
            case EKX_SETTINGS_MODE:
                bSuccess = tryActivateMenuState();
                break;
            case EKX_ERROR:
                bSuccess = tryActivateErrorState();
                break;
            case EKX_SWUPDATE:
                bSuccess = tryActivateSwUpdateState();
                break;
            default:
                bSuccess = false;
                break;
        }
    }

    return bSuccess;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void MainStatemachine::setEkxMainstate( const MainStatemachine::EkxMainstates nEkxMainstate )
{
    if ( m_nEkxMainstate != nEkxMainstate )
    {
        m_nEkxMainstate = nEkxMainstate;
        emit ekxMainstateChanged();
        m_timerSaveEkxMainstate.start();
        setGrindRunning( false );
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool MainStatemachine::tryActivateStandbyState()
{
    bool bSuccess;

    switch ( m_nEkxMainstate )
    {
        case EKX_STANDBY:
        case EKX_CLASSIC_MODE:
        case EKX_TIME_MODE:
        case EKX_LIBRARY_MODE:
        case EKX_SETTINGS_MODE:
        case EKX_ERROR:
        case EKX_SWUPDATE:
            bSuccess = true;
            break;
        default:
            bSuccess = false;
            break;
    }

    if ( bSuccess )
    {
        setEkxMainstate( EkxMainstates::EKX_STANDBY );
    }

    return bSuccess;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool MainStatemachine::tryActivateClassicState()
{
    bool bSuccess;

    switch ( m_nEkxMainstate )
    {
        case EKX_STANDBY:
        case EKX_CLASSIC_MODE:
        case EKX_TIME_MODE:
        case EKX_LIBRARY_MODE:
        case EKX_SETTINGS_MODE:
        case EKX_ERROR:
        case EKX_SWUPDATE:
            bSuccess = true;
            break;
        default:
            bSuccess = false;
            break;
    }

    if ( bSuccess )
    {
        setEkxMainstate( EkxMainstates::EKX_CLASSIC_MODE );
    }

    return bSuccess;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool MainStatemachine::tryActivateTimeModeState()
{
    bool bSuccess;

    switch ( m_nEkxMainstate )
    {
        case EKX_STANDBY:
        case EKX_CLASSIC_MODE:
        case EKX_TIME_MODE:
        case EKX_LIBRARY_MODE:
        case EKX_SETTINGS_MODE:
        case EKX_ERROR:
        case EKX_SWUPDATE:
            bSuccess = true;
            break;
        default:
            bSuccess = false;
            break;
    }

    if ( bSuccess )
    {
        setEkxMainstate( EkxMainstates::EKX_TIME_MODE );
    }

    return bSuccess;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool MainStatemachine::tryActivateLibraryModeState()
{
    bool bSuccess;

    switch ( m_nEkxMainstate )
    {
        case EKX_STANDBY:
        case EKX_CLASSIC_MODE:
        case EKX_TIME_MODE:
        case EKX_LIBRARY_MODE:
        case EKX_SETTINGS_MODE:
        case EKX_ERROR:
        case EKX_SWUPDATE:
            bSuccess = true;
            break;
        default:
            bSuccess = false;
            break;
    }

    if ( bSuccess )
    {
        setEkxMainstate( EkxMainstates::EKX_LIBRARY_MODE );
    }

    return bSuccess;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool MainStatemachine::tryActivateMenuState()
{
    bool bSuccess;

    switch ( m_nEkxMainstate )
    {
        case EKX_STANDBY:
            // standby to menu not allowed
            qInfo() << "setMenuState() illegal transistion to STANDBY rejected";
            bSuccess = false;
            break;

        case EKX_CLASSIC_MODE:
        case EKX_TIME_MODE:
        case EKX_LIBRARY_MODE:
        case EKX_SETTINGS_MODE:
        case EKX_ERROR:
        case EKX_SWUPDATE:
            bSuccess = true;
            break;
        default:
            bSuccess = false;
            break;
    }

    if ( bSuccess )
    {
        setEkxMainstate( EkxMainstates::EKX_SETTINGS_MODE );
    }

    return bSuccess;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool MainStatemachine::tryActivateErrorState()
{
    bool bSuccess;

    switch ( m_nEkxMainstate )
    {
        case EKX_STANDBY:
        case EKX_CLASSIC_MODE:
        case EKX_TIME_MODE:
        case EKX_LIBRARY_MODE:
        case EKX_SETTINGS_MODE:
        case EKX_ERROR:
        case EKX_SWUPDATE:
            bSuccess = true;
            break;
        default:
            bSuccess = false;
            break;
    }
    if ( bSuccess )
    {
        setEkxMainstate( EkxMainstates::EKX_ERROR );
    }

    return bSuccess;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool MainStatemachine::tryActivateSwUpdateState()
{
    bool bSuccess;

    switch ( m_nEkxMainstate )
    {
        case EKX_STANDBY:
        case EKX_CLASSIC_MODE:
        case EKX_TIME_MODE:
        case EKX_LIBRARY_MODE:
        case EKX_SETTINGS_MODE:
        case EKX_ERROR:
        case EKX_SWUPDATE:
            bSuccess = true;
            break;
        default:
            bSuccess = false;
            break;
    }

    if ( bSuccess )
    {
        setEkxMainstate( EkxMainstates::EKX_SWUPDATE );
    }

    return bSuccess;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void MainStatemachine::processSwUpdateStart()
{
    tryActivateEkxMainstate( EkxMainstates::EKX_SWUPDATE );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void MainStatemachine::processSwUpdateStop()
{
    loadAndActivateFileState();

    if ( m_bStartUp )
    {
        m_bStartUp = false;

        // only do it one time after startup
        checkStartSystemTime();
        checkGrindingDiscsStatus();
        checkPduStatus();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void MainStatemachine::processTimeRecipeIndexChanged()
{
    if ( m_nCurrentTimeRecipe != recipeControl()->currentTimeRecipeIndex() )
    {
        m_nCurrentTimeRecipe = recipeControl()->currentTimeRecipeIndex();
        m_timerSaveEkxMainstate.start();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void MainStatemachine::processLibraryRecipeIndexChanged()
{
    if ( m_nCurrentLibraryRecipe != recipeControl()->currentLibraryRecipeIndex() )
    {
        m_nCurrentLibraryRecipe = recipeControl()->currentLibraryRecipeIndex();
        m_timerSaveEkxMainstate.start();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void MainStatemachine::processLibraryGrammageIndexChanged()
{
    if ( m_nCurrentLibraryGrammage != recipeControl()->currentLibraryGrammageIndex() )
    {
        m_nCurrentLibraryGrammage = recipeControl()->currentLibraryGrammageIndex();
        m_timerSaveEkxMainstate.start();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void MainStatemachine::processStandbyChanged( const bool bStandbyActive )
{
    if ( bStandbyActive )
    {
        tryActivateStandbyState();
        m_pSettingsStatemachine->setSettingsState( SettingsStatemachine::SETTING_MAIN );
        disconnect( m_pEspDriver->startStopButtonDriver(), &StartStopButtonDriver::buttonStateChanged, this, &MainStatemachine::processStartStopButton );
        m_timerStandbyCounter.start();
    }
    else
    {
        loadAndActivateFileState();
        connect( m_pEspDriver->startStopButtonDriver(), &StartStopButtonDriver::buttonStateChanged, this, &MainStatemachine::processStartStopButton );
        m_timerStandbyCounter.stop();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void MainStatemachine::processMaxGrindDuration()
{
    qInfo() << "processMaxGrindDuration()";
    setGrindRunning( false );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void MainStatemachine::processSettingsModeExitTimeout()
{
    if ( ekxMainstate() == EkxMainstates::EKX_SETTINGS_MODE )
    {
        qInfo() << "MainStatemachine::processSettingsModeExitTimeout()";

        loadAndActivateFileState();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void MainStatemachine::processUserInput()
{
    // Restart timer only if it is already running.
    if ( m_timerAutoLogout.isActive() )
    {
        m_timerAutoLogout.start();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void MainStatemachine::processStartStopButton( const StartStopButtonDriver::ButtonState nButtonState )
{
    // prevent start event in standby mode (so the first event deactivates standby, the second is then caught)
    if ( !standbyController()->isStandbyActive()
         && m_pSwUpdateController->isIdle()
         && isGrindingAllowed() )
    {
        if ( nButtonState == StartStopButtonDriver::Pressed )
        {
            switch ( m_nEkxMainstate )
            {
                case EKX_CLASSIC_MODE:
                case EKX_LIBRARY_MODE:
                case EKX_TIME_MODE:
                    setGrindRunning( !isGrindRunning() );
                    break;

                case EKX_STANDBY:
                case EKX_SETTINGS_MODE:
                case EKX_ERROR:
                case EKX_SWUPDATE:
                default:
                    //do nothing for the moment
                    break;
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void MainStatemachine::saveEkxMainState()
{
    switch ( m_nEkxMainstate )
    {
        case EKX_CLASSIC_MODE:
        case EKX_TIME_MODE:
        case EKX_LIBRARY_MODE:
        {
            LockableFile file( rootPath() + SAVE_STATE_PATH );

            // take care with chnaging this method! Small files on Linux cause problems with save write/read cylces
            if ( file.open( QIODevice::ReadWrite ) )
            {
                file.resize( 0 );
                QJsonObject latestState;

                latestState[STATE_JSON]             = static_cast<int>( m_nEkxMainstate );
                latestState[STATE_TIME_RECIPE]      = m_nCurrentTimeRecipe;
                latestState[STATE_LIBRARY_RECIPE]   = m_nCurrentLibraryRecipe;
                latestState[STATE_LIBRARY_GRAMMAGE] = m_nCurrentLibraryGrammage;

                QJsonDocument jsonDoc( latestState );
                file.write( jsonDoc.toJson() );
                file.flush();
                file.close();

#ifdef TARGETBUILD
                if ( system( "sync" ) == -1 )
                {
                    qWarning() << "saveEkxMainState(): Failed to sync";
                }
#endif
                m_acycNotificationStateFileWrite.deactivate();
            }
            else
            {
                m_acycNotificationStateFileWrite.activate();
            }
        }
        break;

        default:
            // dont't save these states
            break;
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void MainStatemachine::startOrStopAutoLogoutTimer()
{
    bool bRunning = false;

    if ( !m_bSystemSetupOpen )
    {
        const auto currentUser = m_pSettingsSerializer->currentLoggedInUserRole();
        if ( currentUser == SettingsSerializer::UserRole::StoreOwner )
        {
            bRunning = m_pSettingsSerializer->isUserAutoLogoutEnabled();
        }
        else if ( currentUser == SettingsSerializer::UserRole::Technician )
        {
            bRunning = true;
        }
    }

    if ( bRunning )
    {
        if ( !m_timerAutoLogout.isActive() )
        {
            m_timerAutoLogout.start();
        }
    }
    else
    {
        m_timerAutoLogout.stop();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void MainStatemachine::pushRecoverActionStep( const EkxSqliteTypes::SqliteNotificationType nNotification,
                                              const int                                    nStep )
{
    m_theRecoverActions.append( QPair<EkxSqliteTypes::SqliteNotificationType, int>( nNotification, nStep ) );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void MainStatemachine::processNextRecoverActionStep()
{
    bool bPopNext = true;

    while ( bPopNext
            && !m_pNotificationCenter->isAnyActive()
            && !m_theRecoverActions.empty() )
    {
        QPair<EkxSqliteTypes::SqliteNotificationType, int> pair = m_theRecoverActions.takeLast();

        switch ( pair.first )
        {
            case EkxSqliteTypes::SqliteNotificationType::SqliteNotificationType_SYSTEM_TIME_LOST:
                if ( pair.second == 0 )
                {
                    tryActivateEkxMainstate( EkxMainstates::EKX_SETTINGS_MODE );
                    m_pSettingsStatemachine->setSettingsState( SettingsStatemachine::SETTING_SYSTEM );
                    m_pSettingsStatemachine->setSettingsState( SettingsStatemachine::SETTING_SYSTEM_DATE );
                    pushRecoverActionStep( pair.first, 1 );
                }
                else if ( pair.second == 1 )
                {
                    tryActivateEkxMainstate( EkxMainstates::EKX_SETTINGS_MODE );
                    m_pSettingsStatemachine->setSettingsState( SettingsStatemachine::SETTING_SYSTEM );
                    m_pSettingsStatemachine->setSettingsState( SettingsStatemachine::SETTING_SYSTEM_TIME );
                }
                bPopNext = false;
                break;

            case EkxSqliteTypes::SqliteNotificationType::SqliteNotificationType_SAVE_SETTINGS_FAILED:
                m_pNotificationCenter->removeNotification( pair.first );
                m_pSettingsSerializer->saveSettings();
                break;

            case EkxSqliteTypes::SqliteNotificationType::SqliteNotificationType_ESP_INTERFACE_FAILED:
                m_pNotificationCenter->removeNotification( pair.first );
                m_pEspDriver->swUpdateDriver()->sendCmd( EkxProtocol::Payload::SwUpdateCmd::Reboot );
                break;

            case EkxSqliteTypes::SqliteNotificationType::SqliteNotificationType_MCU_INTERFACE_FAILED:
                m_pNotificationCenter->removeNotification( pair.first );
                m_pEspDriver->espMcuDriver()->mcuDriver()->resetMcu();
                break;

            default:
                // skip all others
                qWarning() << "MainStatemachine::processRecoverAction() skip notification, " << pair.first << "not implemented";
                break;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool MainStatemachine::isGrindingAllowed() const
{
    return m_bGrindingAllowed;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void MainStatemachine::setGrindingAllowed( const bool bAllowed )
{
    if ( m_bGrindingAllowed != bAllowed )
    {
        m_bGrindingAllowed = bAllowed;
        emit grindingAllowedChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool MainStatemachine::isSystemSetupOpen() const
{
    return m_bSystemSetupOpen;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void MainStatemachine::setSystemSetupOpen( bool bOpen )
{
    if ( m_bSystemSetupOpen != bOpen )
    {
        m_bSystemSetupOpen = bOpen;

        m_pNotificationCenter->setSuppressNotifications( bOpen );
        m_pStandbyController->setSuppressSettingsModeExitTimer( bOpen );
        startOrStopAutoLogoutTimer();

        Q_EMIT systemSetupOpenChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void MainStatemachine::setSerialPort( const QString & strSerialPort )
{
    m_pEspDriver->protocolStack()->openPort( strSerialPort );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void MainStatemachine::resetEkxMainstate()
{
    LockableFile file( QString( rootPath() + SAVE_STATE_PATH ) );

    m_nCurrentTimeRecipe    = 0;
    m_nCurrentLibraryRecipe = 0;
    m_nCurrentTimeRecipe    = 0;

    if ( file.open( QIODevice::WriteOnly ) )
    {
        QJsonObject latestState;

        latestState[STATE_JSON]             = static_cast<int>( MainStatemachine::EKX_CLASSIC_MODE );
        latestState[STATE_TIME_RECIPE]      = m_nCurrentTimeRecipe;
        latestState[STATE_LIBRARY_RECIPE]   = m_nCurrentLibraryRecipe;
        latestState[STATE_LIBRARY_GRAMMAGE] = m_nCurrentLibraryGrammage;

        QJsonDocument jsonDoc( latestState );
        file.write( jsonDoc.toJson() );
        file.flush();
        file.close();

#ifdef TARGETBUILD
        int systemStatus = system( "sync" );
        if ( systemStatus == -1 )
        {
            qCritical() << "Failed to fsync";
        }
#endif

    }

}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

DddCouple * MainStatemachine::dddCouple()
{
    return m_pDddCouple;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

EspDriver * MainStatemachine::espDriver()
{
    return m_pEspDriver;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

SettingsSerializer * MainStatemachine::settingsSerializer()
{
    return m_pSettingsSerializer;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

SettingsStatemachine * MainStatemachine::settingsStatemachine()
{
    return m_pSettingsStatemachine;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

StandbyController * MainStatemachine::standbyController()
{
    return m_pStandbyController;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

DeviceInfoCollector * MainStatemachine::deviceInfoCollector()
{
    return m_pDeviceInfoCollector;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

SqliteInterface * MainStatemachine::sqliteInterface()
{
    return m_pSqliteInterface;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

RecipeControl * MainStatemachine::recipeControl()
{
    return m_pRecipeControl;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

NotificationCenter * MainStatemachine::notificationCenter()
{
    return m_pNotificationCenter;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

DisplayController * MainStatemachine::displayController()
{
    return m_pDisplayController;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

SwUpdateController * MainStatemachine::swUpdateController()
{
    return m_pSwUpdateController;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

AgsaControl * MainStatemachine::agsaControl()
{
    return m_pAgsaControl;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

SystemIo::DddDriver * MainStatemachine::dddFineDriver()
{
    return m_pDddFineDriver;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

SystemIo::DddDriver * MainStatemachine::dddCoarseDriver()
{
    return m_pDddCoarseDriver;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

// static
QString MainStatemachine::rootPath()
{
#ifdef TARGETBUILD
    return "";
#else
    return QDir::homePath() + "/EkxData";
#endif
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

//static
bool MainStatemachine::ensureExistingDirectory( const QString & strFilename )
{
    bool bSuccess = true;

    QFileInfo fileInfo( strFilename );
    QDir      dir( fileInfo.path() );
    if ( !dir.exists() )
    {
        bSuccess = dir.mkpath( fileInfo.path() );
    }

    return bSuccess;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool MainStatemachine::isMcuIgnored() const
{
    return m_bIgnoreMcu;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

MainStatemachine::EkxMainstates MainStatemachine::loadAndActivateFileState()
{
    MainStatemachine::EkxMainstates nPersistentMainState = EkxMainstates::EKX_CLASSIC_MODE;
    m_nCurrentTimeRecipe      = 0;
    m_nCurrentLibraryRecipe   = 0;
    m_nCurrentLibraryGrammage = 0;

    QJsonObject jsonFile;
    if ( JsonHelper::readJsonFile( MainStatemachine::rootPath() + SAVE_STATE_PATH, jsonFile ) )
    {
        nPersistentMainState      = static_cast<MainStatemachine::EkxMainstates>( JsonHelper::read( jsonFile, STATE_JSON, 0 ) );
        m_nCurrentTimeRecipe      = JsonHelper::read( jsonFile, STATE_TIME_RECIPE, 0 );
        m_nCurrentLibraryRecipe   = JsonHelper::read( jsonFile, STATE_LIBRARY_RECIPE, 0 );
        m_nCurrentLibraryGrammage = JsonHelper::read( jsonFile, STATE_LIBRARY_GRAMMAGE, 0 );

        m_acycNotificationStateFileCorrupted.deactivate();
    }
    else
    {
        LockableFile::remove( rootPath() + SAVE_STATE_PATH );
        qWarning() << "loadAndActivateFileState(): broken state file removed!";

        m_acycNotificationStateFileCorrupted.activate();
    }

    tryActivateEkxMainstate( nPersistentMainState );

    m_pRecipeControl->setCurrentTimeRecipeIndex( m_nCurrentTimeRecipe );
    m_pRecipeControl->setCurrentLibraryRecipeIndex( m_nCurrentLibraryRecipe );
    m_pRecipeControl->setCurrentLibraryGrammageIndex( m_nCurrentLibraryGrammage );

    m_timerSaveEkxMainstate.stop(); // We just restored it, don't bother saving now.

    return nPersistentMainState;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void MainStatemachine::checkStartSystemTime()
{
    QDateTime epochPlus1;
    epochPlus1.setSecsSinceEpoch( 30 * 24 * 3600 );

    // Suppress error on initial setup since the initial setup tutorial includes date/time setting.
    if ( QDateTime::currentDateTime() < epochPlus1
         && !m_pSettingsSerializer->isInitialSetupDone() )
    {
        m_pNotificationCenter->raiseNotification( EkxSqliteTypes::SqliteNotificationType::SqliteNotificationType_SYSTEM_TIME_LOST );
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void MainStatemachine::checkGrindingDiscsStatus()
{
    if ( m_pSettingsStatemachine->diskUsageTime().isWarningActive() )
    {
        m_pNotificationCenter->raiseNotification( EkxSqliteTypes::SqliteNotificationType::SqliteNotificationType_CHANGE_DISCS_INFO );
    }
    else if ( m_pSettingsStatemachine->diskUsageTime().isPreWarningActive() )
    {
        // do nothing currently
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void MainStatemachine::checkPduStatus()
{
    if ( m_pEspDriver->pduDcHallMotorDriver()->isValid() )
    {
        if ( m_pSettingsSerializer->isPduInstalled()
             && !m_pEspDriver->pduDcHallMotorDriver()->isMotorTestOk() )
        {
            m_pNotificationCenter->raiseNotification( EkxSqliteTypes::SqliteNotificationType::SqliteNotificationType_PDU_UNINSTALLED_AFTER_PRESENCE );
            m_pNotificationCenter->removeNotification( EkxSqliteTypes::SqliteNotificationType::SqliteNotificationType_PDU_INSTALLED_AFTER_ABSENCE );

            m_pSettingsSerializer->setPduInstalled( false );
            m_pSettingsSerializer->saveSettings();
        }
        else if ( !m_pSettingsSerializer->isPduInstalled()
                  && m_pEspDriver->pduDcHallMotorDriver()->isMotorTestOk() )
        {
            m_pNotificationCenter->raiseNotification( EkxSqliteTypes::SqliteNotificationType::SqliteNotificationType_PDU_INSTALLED_AFTER_ABSENCE );
            m_pNotificationCenter->removeNotification( EkxSqliteTypes::SqliteNotificationType::SqliteNotificationType_PDU_UNINSTALLED_AFTER_PRESENCE );

            m_pSettingsSerializer->setPduInstalled( true );

            // First time the PDU is detected, prompt for PDU tutorial.
            if ( !m_pSettingsSerializer->isPduAcknowledged() )
            {
                Q_EMIT pduTutorialRequested();
                // The setting will be set to true when the prompt has actually been shown.
            }

            m_pSettingsSerializer->saveSettings();

        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void MainStatemachine::processRecoverAction( const EkxSqliteTypes::SqliteNotificationType nNotificationId )
{
    pushRecoverActionStep( nNotificationId, 0 );
    processNextRecoverActionStep();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

