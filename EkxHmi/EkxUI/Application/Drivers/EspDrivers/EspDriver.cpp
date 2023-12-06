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

#include "MainStatemachine.h"
#include "StandbyController.h"

#include "At24c16Driver.h"
#include "EspDcHallMotorDriver.h"
#include "EspMcuDriver.h"
#include "EspProtocolStack.h"
#include "EspStepperMotorDriver.h"
#include "EspStepperMotorDriver.h"
#include "EspSwUpdateDriver.h"
#include "HttpServerDriver.h"
#include "McuDriver.h"
#include "NtcTempDriver.h"
#include "SpotLightDriver.h"
#include "StartStopButtonDriver.h"
#include "StartStopLightDriver.h"
#include "WifiDriver.h"
#include "JsonApi.h"
#include "DeviceInfoCollector.h"

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

EspDriver::EspDriver()
    : m_pProtocolStack( new EspProtocolStack() )
    , m_pEspAt24c16Driver( new At24c16Driver( EkxProtocol::DriverId::EepromDriver ) )
    , m_pPduDcHallMotorDriver( new EspDcHallMotorDriver( EkxProtocol::DriverId::PduDcHallMotorDriver, "PduDcHallMotor" ) )
    , m_pEspMcuDriver( new EspMcuDriver( EkxProtocol::DriverId::McuDriver ) )
    , m_pAgsaStepperMotorDriver( new EspStepperMotorDriver( EkxProtocol::DriverId::AgsaStepperMotorDriver, "AgsaStepperMotor" ) )
    , m_pSwUpdateDriver( new EspSwUpdateDriver( EkxProtocol::DriverId::SwUpdateDriver ) )
    , m_pHttpServerDriver( new HttpServerDriver( EkxProtocol::DriverId::HttpServerDriver ) )
    , m_pNtcTempDriver( new NtcTempDriver( EkxProtocol::DriverId::TempSensorDriver ) )
    , m_pSpotLightDriver( new SpotLightDriver( EkxProtocol::DriverId::SpotLightDriver ) )
    , m_pStartStopButtonDriver( new StartStopButtonDriver( EkxProtocol::DriverId::StartStopBtnDriver ) )
    , m_pStartStopLightDriver( new StartStopLightDriver( EkxProtocol::DriverId::StartStopLightDriver ) )
    , m_pWifiDriver( new WifiDriver( EkxProtocol::DriverId::WifiDriver ) )
    , m_pJsonApi( new JsonApi() )
{
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

EspDriver::~EspDriver()
{
    delete m_pStartStopButtonDriver;
    m_pStartStopButtonDriver = nullptr;

    delete m_pNtcTempDriver;
    m_pNtcTempDriver = nullptr;

    delete m_pWifiDriver;
    m_pWifiDriver = nullptr;

    delete m_pEspAt24c16Driver;
    m_pEspAt24c16Driver = nullptr;

    delete m_pSpotLightDriver;
    m_pSpotLightDriver = nullptr;

    delete m_pStartStopLightDriver;
    m_pStartStopLightDriver = nullptr;

    delete m_pEspMcuDriver;
    m_pEspMcuDriver = nullptr;

    delete m_pSwUpdateDriver;
    m_pSwUpdateDriver = nullptr;

    delete m_pAgsaStepperMotorDriver;
    m_pAgsaStepperMotorDriver = nullptr;

    delete m_pPduDcHallMotorDriver;
    m_pPduDcHallMotorDriver = nullptr;

    delete m_pProtocolStack;
    m_pProtocolStack = nullptr;

    delete m_pHttpServerDriver;
    m_pHttpServerDriver = nullptr;

    delete m_pJsonApi;
    m_pJsonApi = nullptr;

    m_pMainStatemachine = nullptr;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void EspDriver::create( MainStatemachine * pMainStatemachine )
{
    m_pMainStatemachine = pMainStatemachine;

    connect( protocolStack(), &EspProtocolStack::aliveChanged, this, &EspDriver::connectedChanged );

    m_pAgsaStepperMotorDriver->create( protocolStack() );
    m_pPduDcHallMotorDriver->create( protocolStack() );
    m_pEspAt24c16Driver->create( protocolStack() );
    m_pEspMcuDriver->create( pMainStatemachine );
    m_pHttpServerDriver->create( protocolStack(),
                                 jsonApi(),
                                 m_pMainStatemachine->deviceInfoCollector() );
    m_pNtcTempDriver->create( pMainStatemachine );
    m_pSpotLightDriver->create( protocolStack() );
    m_pStartStopButtonDriver->create( protocolStack() );
    m_pStartStopLightDriver->create( protocolStack() );
    m_pSwUpdateDriver->create( protocolStack() );
    m_pWifiDriver->create( pMainStatemachine );
    m_pJsonApi->create( pMainStatemachine );

    m_pSpotLightDriver->processStandbyChanged( m_pMainStatemachine->standbyController()->isStandbyActive() );
    m_pSpotLightDriver->processShowSpotLightChanged( m_pMainStatemachine->settingsSerializer()->showSpotLight() );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

WifiDriver * EspDriver::wifiDriver( void )
{
    return m_pWifiDriver;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

JsonApi * EspDriver::jsonApi()
{
    return m_pJsonApi;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

StartStopButtonDriver * EspDriver::startStopButtonDriver( void )
{
    return m_pStartStopButtonDriver;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

NtcTempDriver * EspDriver::ntcTempDriver()
{
    return m_pNtcTempDriver;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

SpotLightDriver * EspDriver::spotLightDriver( void )
{
    return m_pSpotLightDriver;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

StartStopLightDriver * EspDriver::startStopLightDriver()
{
    return m_pStartStopLightDriver;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

EspMcuDriver * EspDriver::espMcuDriver()
{
    return m_pEspMcuDriver;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

At24c16Driver * EspDriver::at24c16Driver( void )
{
    return m_pEspAt24c16Driver;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

EspSwUpdateDriver * EspDriver::swUpdateDriver( void )
{
    return m_pSwUpdateDriver;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

EspDcHallMotorDriver * EspDriver::pduDcHallMotorDriver()
{
    return m_pPduDcHallMotorDriver;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

EspStepperMotorDriver * EspDriver::agsaStepperMotorDriver()
{
    return m_pAgsaStepperMotorDriver;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

EspProtocolStack * EspDriver::protocolStack()
{
    return m_pProtocolStack;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

HttpServerDriver * EspDriver::httpServerDriver()
{
    return m_pHttpServerDriver;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

const EspProtocolStack * EspDriver::protocolStack() const
{
    return m_pProtocolStack;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool EspDriver::isConnected() const
{
    return protocolStack()->isAlive();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

