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
#include <QtDebug>
#include <QDir>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

#include "MainStatemachine.h"
#include "EspConnectorClient.h"
#include "EkxGlobals.h"
#include "McuDriver.h"
#include "StandbyController.h"
#include "DisplayController.h"
#include "EspDriver.h"
#include "LedPwmDriver.h"
#include "UartDriver.h"
#include "StartStopButtonDriver.h"
#include "SqliteInterface.h"

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#define COM_PATH            "/tmp/esp_connector/"
#define COM_PATH_INPUT      "/tmp/esp_connector/input"
#define COM_PATH_OUTPUT     "/tmp/esp_connector/output"

#define STATE_JSON          "State"
#define STATE_RECIPE        "StateRecipe"

#define SAVE_STATE_PATH      "/config/LastState.json"

int MainStatemachine::m_nEkxMainstatesQTypeId   = qRegisterMetaType<MainStatemachine::EkxMainstates>( "MainStatemachine::EkxMainstates" );
int MainStatemachine::m_nQmlEkxMainstateId      = qmlRegisterUncreatableType<MainStatemachine>( "EkxMainstateEnum", 1, 0, "EkxMainstateEnum", "something went wrong" );

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

MainStatemachine::MainStatemachine( QQmlEngine * pEnigne,
                                    QObject * parent )
    : QObject( parent )
    , m_pRecipeControl( new RecipeControl( pEnigne, this) )
    , m_pEspDriver( new EspDriver( pEnigne, this ) )
    , m_pDddDriver( new DddDriver( pEnigne, this ) )
    , m_pHmiTempDriver( new HmiTempDriver( this ) )
    , m_pDisplayController( new DisplayController( pEnigne, this) )
    , m_pSqliteInterface( new SqliteInterface( pEnigne, this ) )
    , m_pStandbyController( new StandbyController( pEnigne, this ) )
    , m_pDeviceInfoCollector( new DeviceInfoCollector( pEnigne ,this ) )
    , m_pSwUpdateController( new SwUpdateController( pEnigne, this ) )
    , m_pSettingsSerializer( new SettingsSerializer( pEnigne, this ) )
    , m_pSettingsStatemachine( new SettingsStatemachine( pEnigne,
                                                         this,
                                                         m_pSettingsSerializer,
                                                         m_pSqliteInterface ) )
{
    ensureExistingDirectory( rootPath() + SAVE_STATE_PATH );
    ensureExistingDirectory( COM_PATH_INPUT "/dummy" );
    ensureExistingDirectory( COM_PATH_OUTPUT "/dummy" );

#ifdef TARGETBUILD
    m_bIgnoreMcu = QFile( rootPath() + "/config/mcu_ignored" ).exists();
#else
    m_bIgnoreMcu = true;
#endif

    if ( m_bIgnoreMcu )
    {
        qWarning() << "MainStatemachine(): MCU will be ignored";
    }

    pEnigne->rootContext()->setContextProperty( "mainStatemachine", this );

    connect( m_pEspDriver->startStopButtonDriver(), &StartStopButtonDriver::buttonStateChanged, this, &MainStatemachine::processStartStopGrind );
    connect( m_pSwUpdateController, &SwUpdateController::swUpdateStarted, this, &MainStatemachine::processSwUpdateStart );
    connect( m_pSwUpdateController, &SwUpdateController::swUpdateStopped, this, &MainStatemachine::processSwUpdateStop );

    m_pRecipeControl->create();
    m_pSettingsStatemachine->create();
    m_pSettingsStatemachine->setSettingState( SettingsStatemachine::SETTING_MAIN, false );
    m_pSettingsStatemachine->setDeviceInfoDriverInSettingsSerializer( m_pDeviceInfoCollector );
    m_pEspDriver->connectorClient()->create( COM_PATH );
    m_pStandbyController->create();
    m_pDddDriver->create();
    m_pHmiTempDriver->create();
    m_pEspDriver->create();
    m_pSwUpdateController->create();

    m_timerSaveEkxMainstate.setInterval( 5000 );   // 5secs
    connect( &m_timerSaveEkxMainstate, &QTimer::timeout, this, &MainStatemachine::saveEkxMainState );
    connect( m_pStandbyController, &StandbyController::standbyChanged, this, &MainStatemachine::processStandbyChanged );

    // Connections SettingsSerializer <-> DisplayConroller:
    connect( m_pSettingsSerializer, &SettingsSerializer::displayBrightnessPercentChanged, m_pDisplayController, &DisplayController::setBrightnessPercent);
    m_pDisplayController->setBrightnessPercent( m_pSettingsSerializer->getDisplayBrightnessPercent() );

    // Connections StandbyController <-> DisplayConroller:
    connect( m_pStandbyController, &StandbyController::standbyChanged, m_pDisplayController, &DisplayController::setBrightnessDisable );
    m_pDisplayController->setBrightnessDisable( m_pStandbyController->isStandbyActive() );

    // ensure QML layer is consistent
    emit ekxMainstateChanged();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

MainStatemachine::~MainStatemachine()
{
    if( m_pEspDriver->startStopButtonDriver() )
    {
        disconnect( m_pEspDriver->startStopButtonDriver(), &StartStopButtonDriver::buttonStateChanged, this, &MainStatemachine::processStartStopGrind );
    }

    disconnect( m_pSwUpdateController, &SwUpdateController::swUpdateStarted, this, &MainStatemachine::processSwUpdateStart );
    disconnect( m_pSwUpdateController, &SwUpdateController::swUpdateStopped, this, &MainStatemachine::processSwUpdateStop );
    disconnect( &m_timerSaveEkxMainstate, &QTimer::timeout, this, &MainStatemachine::saveEkxMainState );

    delete m_pDisplayController;
    m_pDisplayController = nullptr;

    delete m_pStandbyController;
    m_pStandbyController = nullptr;

    delete m_pDeviceInfoCollector;
    m_pDeviceInfoCollector = nullptr;

    delete m_pDddDriver;
    m_pDddDriver = nullptr;

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

MainStatemachine::EkxMainstates MainStatemachine::ekxMainstate()
{
    return m_nEkxMainstate;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool MainStatemachine::isCoffeeRunning() const
{
    return m_isCoffeeRunning;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void  MainStatemachine::setIsCoffeeRunning( const bool desiredCoffeeIsRunning )
{
    if( m_isCoffeeRunning != desiredCoffeeIsRunning )
    {
        if(desiredCoffeeIsRunning)
        {
            // switch on?
            if(m_isCoffeeRunAllowed)
            {
                if ( m_pEspDriver->uartDriver()->mcuDriver()->startMotor() || m_bIgnoreMcu )
                {
                    m_isCoffeeRunning = desiredCoffeeIsRunning;
                    qInfo() << "setCoffeeRunning(): grinding started";
                    emit isCoffeeRunningChanged();

                    // reactions: (better use signal connections)
                    m_dtStartGrinding = QDateTime::currentDateTime();
                    m_pEspDriver->ledPwmDriver()->setStartStopPwm( LedPwmDriver::StartStopGrinding );
                    m_pEspDriver->ledPwmDriver()->setSpotlightPwm( LedPwmDriver::SpotlightGrinding );
                }
                else
                {
                    qWarning() << "setIsCoffeeRunning(): grinding start failed!";
                }
            }
            else
            {
                qInfo() << "setIsCoffeeRunning(): grinding not allowed!";
            }
        }
        else
        {
            // switch off
            if( m_pEspDriver->uartDriver()->mcuDriver()->stopMotor() || m_bIgnoreMcu )
            {
                m_isCoffeeRunning = desiredCoffeeIsRunning;
                qInfo() << "setCoffeeRunning(): grinding stopped";
                emit isCoffeeRunningChanged();

                // reactions: (better use signal connections)
                quint64 u64GrindTimeMs = m_dtStartGrinding.msecsTo( QDateTime::currentDateTime() );
                m_pEspDriver->ledPwmDriver()->setStartStopPwm( LedPwmDriver::StartStopIdle );
                m_pEspDriver->ledPwmDriver()->setSpotlightPwm( LedPwmDriver::SpotlightIdle );
                sqliteInterface()->addGrind( recipeControl()->currentRecipeUuid(), u64GrindTimeMs / 10 );
            }
            else
            {
                qWarning() << "setCoffeeRunning(): grinding stop failed!";
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

int  MainStatemachine::currentRecipe()
{
    return m_nCurrentRecipe;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void  MainStatemachine::setCurrentRecipe( int newCurrentRecipe )
{
    if( m_nCurrentRecipe != newCurrentRecipe )
    {
        m_nCurrentRecipe = newCurrentRecipe;
        m_timerSaveEkxMainstate.start();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool MainStatemachine::tryActivateEkxMainstate( const MainStatemachine::EkxMainstates nEkxMainstate  )
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
            case EKX_MENU:
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

void MainStatemachine::setEkxMainstate(const MainStatemachine::EkxMainstates nEkxMainstate)
{
    if ( m_nEkxMainstate != nEkxMainstate )
    {
        m_nEkxMainstate = nEkxMainstate;
        emit ekxMainstateChanged();
        m_timerSaveEkxMainstate.start();
        setIsCoffeeRunning( false );
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
        case EKX_MENU:
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
        case EKX_MENU:
        case EKX_ERROR:
        case EKX_SWUPDATE:
            bSuccess = true;
        break;
        default:
            bSuccess = false;
        break;
    }

    if( bSuccess )
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
        case EKX_MENU:
        case EKX_ERROR:
        case EKX_SWUPDATE:
            bSuccess = true;
        break;
        default:
            bSuccess = false;
        break;
    }

    if( bSuccess )
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
        case EKX_MENU:
        case EKX_ERROR:
        case EKX_SWUPDATE:
            bSuccess = true;
        break;
        default:
            bSuccess = false;
        break;
    }

    if( bSuccess )
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
        case EKX_MENU:
        case EKX_ERROR:
        case EKX_SWUPDATE:
            bSuccess = true;
        break;
        default:
            bSuccess = false;
        break;
    }

    if( bSuccess )
    {
        setEkxMainstate( EkxMainstates::EKX_MENU );
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
        case EKX_MENU:
        case EKX_ERROR:
        case EKX_SWUPDATE:
            bSuccess = true;
        break;
        default:
            bSuccess = false;
        break;
    }
    if( bSuccess )
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
        case EKX_MENU:
        case EKX_ERROR:
        case EKX_SWUPDATE:
            bSuccess = true;
        break;
        default:
            bSuccess = false;
        break;
    }

    if( bSuccess )
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
    tryActivateEkxMainstate( loadCurrentState() );
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
        m_pSettingsStatemachine->setSettingState( SettingsStatemachine::SETTING_MAIN );
        disconnect( m_pEspDriver->startStopButtonDriver(), &StartStopButtonDriver::buttonStateChanged, this, &MainStatemachine::processStartStopGrind );
    }
    else
    {
        tryActivateEkxMainstate( loadCurrentState() );
        connect( m_pEspDriver->startStopButtonDriver(), &StartStopButtonDriver::buttonStateChanged, this, &MainStatemachine::processStartStopGrind );
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void MainStatemachine::processStartStopGrind( const StartStopButtonDriver::ButtonState nButtonState )
{
    // prevent start event in standby mode (so the first event deactivates standby, the second is then caught)
    if ( !standbyController()->isStandbyActive()
         && m_pSwUpdateController->isIdle()
         && isCoffeeRunAllowed() )
    {
        if ( nButtonState == StartStopButtonDriver::Pressed )
        {
            switch ( m_nEkxMainstate )
            {
                case EKX_CLASSIC_MODE:
                case EKX_LIBRARY_MODE:
                case EKX_TIME_MODE:
                    setIsCoffeeRunning( !isCoffeeRunning() );
                break;

                case EKX_STANDBY:
                case EKX_MENU:
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
            QFile file( rootPath() + SAVE_STATE_PATH );

            if( m_nEkxMainstate != EKX_TIME_MODE )
            {
                m_nCurrentRecipe = 0;
            }

            if ( file.open( QIODevice::ReadWrite ) )
            {
                file.resize(0);
                QJsonObject latestState;

                latestState[STATE_JSON]   = static_cast<int>( m_nEkxMainstate );
                latestState[STATE_RECIPE] = m_nCurrentRecipe;

                QJsonDocument jsonDoc(latestState);
                file.write( jsonDoc.toJson() );
                file.flush();
                file.close();

#ifdef TARGETBUILD
                if ( system( "sync" ) == -1)
                {
                    qWarning() << "saveEkxMainState(): Failed to sync";
                }
#endif
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

bool MainStatemachine::isCoffeeRunAllowed() const
{
    return( m_isCoffeeRunAllowed );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void MainStatemachine::setIsCoffeeRunAllowed(bool coffeeRunAllowed)
{
    if( m_isCoffeeRunAllowed != coffeeRunAllowed )
    {
        m_isCoffeeRunAllowed = coffeeRunAllowed;
        emit isCoffeeRunAllowedChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void MainStatemachine::resetEkxMainstate()
{
    QFile file( QString( rootPath() + SAVE_STATE_PATH ) );

    m_nCurrentRecipe = 0;

    if ( file.open(QIODevice::WriteOnly ) )
    {
        QJsonObject latestState;

        latestState[STATE_JSON]     = static_cast<int>(MainStatemachine::EKX_CLASSIC_MODE);
        latestState[STATE_RECIPE]   = m_nCurrentRecipe;

        QJsonDocument jsonDoc(latestState);
        file.write( jsonDoc.toJson() );
        file.flush();
        file.close();

#ifdef TARGETBUILD
        int systemStatus = system( "sync" );
        if (systemStatus == -1)
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

DddDriver *MainStatemachine::dddDriver()
{
    return m_pDddDriver;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

EspDriver *MainStatemachine::espDriver()
{
    return m_pEspDriver;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

SettingsStatemachine *MainStatemachine::settingsStatemachine()
{
    return m_pSettingsStatemachine;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

StandbyController *MainStatemachine::standbyController()
{
    return m_pStandbyController;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

DeviceInfoCollector *MainStatemachine::deviceInfoCollector()
{
    return m_pDeviceInfoCollector;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

SqliteInterface *MainStatemachine::sqliteInterface()
{
    return m_pSqliteInterface;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

RecipeControl *MainStatemachine::recipeControl()
{
    return m_pRecipeControl;
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
    return ( QDir::homePath() + "/EkxData" );
#endif
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

//static
bool MainStatemachine::ensureExistingDirectory( const QString &strFilename )
{
    bool bSuccess = true;

    QFileInfo fileInfo( strFilename );
    QDir dir( fileInfo.path() );
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

MainStatemachine::EkxMainstates MainStatemachine::loadCurrentState()
{
    MainStatemachine::EkxMainstates nPersistentMainState = EkxMainstates::EKX_CLASSIC_MODE;

    QFile file( rootPath() + SAVE_STATE_PATH );

    if ( file.open(QIODevice::ReadOnly) )
    {
        QByteArray ba = file.readAll();
        file.close();

        QJsonDocument doc = QJsonDocument::fromJson(ba);
        QJsonObject obj = doc.object();

        if( !doc.isObject() )
        {
            nPersistentMainState = EkxMainstates::EKX_CLASSIC_MODE;
            m_nCurrentRecipe     = 0;

            file.remove();
            qWarning() << "loadCurrentState(): broken state file removed!";
        }
        else
        {
            nPersistentMainState = static_cast<MainStatemachine::EkxMainstates>( obj[STATE_JSON].toInt() );
            m_nCurrentRecipe     = obj[STATE_RECIPE].toInt();
        }
    }

    return nPersistentMainState;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

