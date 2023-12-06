///////////////////////////////////////////////////////////////////////////////
///
/// @file EspDriver.cpp
///
/// @brief main application entry point of EspDriver.
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

#include "EspDriver.h"

#include <QDebug>
#include <QCoreApplication>
#include <QQmlEngine>

#include "MainStatemachine.h"
#include "StandbyController.h"
#include "SettingsStatemachine.h"
#include "StartStopButtonDriver.h"
#include "EspConnectorClient.h"

#include "NtcTempDriver.h"
#include "WifiDriver.h"
#include "At24c16Driver.h"
#include "LedPwmDriver.h"
#include "UartDriver.h"
#include "EspSwUpdateInDriver.h"
#include "EspSwUpdateOutDriver.h"

#include "EspAdcIn.h"
#include "EspDiscreteIn.h"
#include "EspAt24c16In.h"
#include "EspPwmOut.h"
#include "EspWifiIn.h"
#include "EspWifiOut.h"
#include "EspSwUpdateIn.h"
#include "EspSwUpdateOut.h"
#include "EspUartIn.h"

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

EspDriver::EspDriver( QQmlEngine* pEngine,
                      MainStatemachine * pMainStatemachine )
    : QObject( pMainStatemachine )
    , m_pMainStatemachine( pMainStatemachine )
    , m_pConnectorClient( new EspConnectorClient() )
    , m_pStartStopButtonDriver ( new StartStopButtonDriver( this ) )
    , m_pNtcTempDriver( new NtcTempDriver( pEngine, pMainStatemachine ) )
    , m_pWifiDriver ( new WifiDriver( pEngine, pMainStatemachine ) )
    , m_pEspAt24c16Driver( new At24c16Driver( pEngine, this ) )
    , m_pLedPwmDriver( new LedPwmDriver( this ) )
    , m_pUartDriver( new UartDriver(pMainStatemachine,this) )
    , m_pSwUpdateInDriver( new EspSwUpdateInDriver( this ) )
    , m_pSwUpdateOutDriver( new EspSwUpdateOutDriver( this ) )
{
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

EspDriver::~EspDriver()
{
    delete m_pConnectorClient;
    m_pConnectorClient = nullptr;

    delete m_pStartStopButtonDriver;
    m_pStartStopButtonDriver = nullptr;

    delete m_pNtcTempDriver;
    m_pNtcTempDriver = nullptr;

    delete m_pWifiDriver;
    m_pWifiDriver = nullptr;

    delete m_pEspAt24c16Driver;
    m_pEspAt24c16Driver = nullptr;

    delete m_pLedPwmDriver;
    m_pLedPwmDriver = nullptr;

    delete m_pUartDriver;
    m_pUartDriver = nullptr;

    delete m_pSwUpdateInDriver;
    m_pSwUpdateInDriver = nullptr;

    delete m_pSwUpdateOutDriver;
    m_pSwUpdateOutDriver = nullptr;

    m_pMainStatemachine = nullptr;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void EspDriver::create()
{
    connect( m_pConnectorClient, &EspConnectorClient::connectedChanged, this, &EspDriver::processConnectChanged );
    connect( m_pMainStatemachine->standbyController(), &StandbyController::standbyChanged, m_pLedPwmDriver, &LedPwmDriver::processStandbyChanged );

    m_pWifiDriver->create();
    m_pNtcTempDriver->create();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

EspConnectorClient * EspDriver::connectorClient( void )
{
    return m_pConnectorClient;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

WifiDriver* EspDriver::wifiDriver( void )
{
    return m_pWifiDriver;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

StartStopButtonDriver* EspDriver::startStopButtonDriver( void )
{
    return m_pStartStopButtonDriver;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

NtcTempDriver *EspDriver::ntcTempDriver()
{
    return m_pNtcTempDriver;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

LedPwmDriver* EspDriver::ledPwmDriver( void )
{
    return m_pLedPwmDriver;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

UartDriver *EspDriver::uartDriver()
{
    return m_pUartDriver;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

At24c16Driver *EspDriver::at24c16Driver( void )
{
    return m_pEspAt24c16Driver;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

EspSwUpdateInDriver* EspDriver::swUpdateInDriver( void )
{
    return m_pSwUpdateInDriver;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

EspSwUpdateOutDriver* EspDriver::swUpdateOutDriver( void )
{
    return m_pSwUpdateOutDriver;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void EspDriver::processConnectChanged()
{
    if ( connectorClient()->isConnected() )
    {
        EspDiscreteIn*  pStartStopButtonIn  = dynamic_cast<EspDiscreteIn*>( (*connectorClient())["StartStopBtnIn"] );
        EspAdcIn*       pNtcSensorIn        = dynamic_cast<EspAdcIn*>( (*connectorClient())["NtcSensorIn"] );
        EspWifiIn*      pWifiIn             = dynamic_cast<EspWifiIn*>( (*connectorClient())["WifiIn"] );
        EspWifiOut*     pWifiOut            = dynamic_cast<EspWifiOut*>( (*connectorClient())["WifiOut"] );
        EspAt24c16In*   pAt24c16In          = dynamic_cast<EspAt24c16In*>( (*connectorClient())["At24c16In"] );
        EspPwmOut*      pStartStopLedOut    = dynamic_cast<EspPwmOut*>( (*connectorClient())["StartStopLightOut"] );
        EspPwmOut*      pSpotLightOut       = dynamic_cast<EspPwmOut*>( (*connectorClient())["SpotLightOut"] );
        EspUartIn*      pMcuControlIn       = dynamic_cast<EspUartIn*>( (*connectorClient())["MotorControlIn"] );
        EspSwUpdateIn*  pSwUpdateIn         = dynamic_cast<EspSwUpdateIn*>( (*connectorClient())["SwUpdateIn"] );
        EspSwUpdateOut* pSwUpdateOut        = dynamic_cast<EspSwUpdateOut*>( (*connectorClient())["SwUpdateOut"] );

        if ( pStartStopButtonIn )
        {
            m_pStartStopButtonDriver->connectInterface( pStartStopButtonIn) ;
        }
        else
        {
            qCritical() << "processConnectChanged: failed to connect to StartStopBtnIn";
        }

        if( pNtcSensorIn )
        {
            m_pNtcTempDriver->connectInterface( pNtcSensorIn );
        }
        else
        {
            qCritical() << "processConnectChanged: failed to connect to NtcSensorIn";
        }

        if( pWifiIn )
        {
            m_pWifiDriver->connectInInterface( pWifiIn );
        }
        else
        {
            qCritical() << "processConnectChanged: failed to connect to WifiIn";
        }

        if( pWifiOut )
        {
            m_pWifiDriver->connectOutInterface( pWifiOut );
            m_pMainStatemachine->settingsStatemachine()->setWifiDriverInSettingsSerializer( m_pWifiDriver );
        }
        else
        {
            qCritical() << "processConnectChanged: failed to connect to WifiOut";
        }

        if( pAt24c16In )
        {
            m_pEspAt24c16Driver->connectInterface( pAt24c16In );
        }
        else
        {
            qCritical() << "processConnectChanged: failed to connect to At24c16In";
        }

        if( pStartStopLedOut )
        {

            m_pLedPwmDriver->connectStartStopInterface( pStartStopLedOut );
        }
        else
        {
            qCritical() << "processConnectChanged: failed to connect to StartStopLedOut";
        }

        if( pSpotLightOut )
        {

            m_pLedPwmDriver->connectSpotlightInterface( pSpotLightOut );
        }
        else
        {
            qCritical() << "processConnectChanged: failed to connect to SpotLightOut";
        }

        if( pMcuControlIn )
        {
            m_pUartDriver->connectMcuInterface( pMcuControlIn );
        }
        else
        {
            qCritical() << "processConnectChanged: failed to connect to MotorControlIn";
        }

        if( pSwUpdateIn )
        {
            m_pSwUpdateInDriver->connectInterface( pSwUpdateIn );
        }
        else
        {
            qCritical() << "processConnectChanged: failed to connect to SwUpdateIn";
        }

        if( pSwUpdateOut )
        {
            m_pSwUpdateOutDriver->connectInterface( pSwUpdateOut );
        }
        else
        {
            qCritical() << "processConnectChanged: failed to connect to SwUpdateOut";
        }

    }
    else
    {
        // object already destroyed, clear your local pointer copies
        m_pStartStopButtonDriver->disconnectInterface();
        m_pNtcTempDriver->disconnectInterface();
        m_pWifiDriver->disconnectInInterface();
        m_pMainStatemachine->settingsStatemachine()->setWifiDriverInSettingsSerializer( nullptr );
        m_pWifiDriver->disconnectOutInterface();
        m_pEspAt24c16Driver->disconnectInterface();
        m_pLedPwmDriver->disconnectStartStopInterface();
        m_pLedPwmDriver->disconnectSpotlightInterface();
        m_pUartDriver->disconnectMcuInterface();
        m_pSwUpdateInDriver->disconnectInterface();
        m_pSwUpdateOutDriver->disconnectInterface();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

