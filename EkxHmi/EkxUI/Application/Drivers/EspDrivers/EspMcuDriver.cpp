///////////////////////////////////////////////////////////////////////////////
///
/// @file EspMcuDriver.cpp
///
/// @brief main application entry point of EspMcuDriver.
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

#include "EspMcuDriver.h"

#include <QDebug>

#include "EkxGlobals.h"
#include "McuDriver.h"
#include "MainStatemachine.h"
#include "SettingsSerializer.h"
#include "EspDriver.h"
#include "UpppMcu.h"

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

EspMcuDriver::EspMcuDriver( const EkxProtocol::DriverId u8DriverId )
    : EspDeviceDriver( u8DriverId )
    , m_pMcuDriver( new McuDriver() )
{
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

EspMcuDriver::~EspMcuDriver()
{
    delete m_pMcuDriver;
    m_pMcuDriver = nullptr;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void EspMcuDriver::create( MainStatemachine * pMainStatemachine )
{
    m_pMainStatemachine = pMainStatemachine;

    EspDeviceDriver::create( m_pMainStatemachine->espDriver()->protocolStack() );

    m_pMcuDriver->open();

    connect( m_pMcuDriver->upppMcu(), &UpppCore::streamOutput, this, &EspMcuDriver::processMcuStreamData );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

McuDriver * EspMcuDriver::mcuDriver()
{
    return m_pMcuDriver;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void EspMcuDriver::processMessageFrame( const EkxProtocol::MessageFrame & frame )
{
    const EkxProtocol::Payload::McuReply * pPayload = dynamic_cast<const EkxProtocol::Payload::McuReply *>( frame.payloadRef().value() );

    if ( pPayload )
    {
        replyAck( frame.driverId().value(),
                  frame.msgCounter().value(),
                  frame.repeatCounter().value() );

        m_pMcuDriver->upppMcu()->inputFromStream( QByteArray( pPayload->reply().value().data(),
                                                              pPayload->reply().value().size() ) );
    }
    else
    {
        replyNack( frame.driverId().value(),
                   frame.msgCounter().value(),
                   frame.repeatCounter().value(),
                   EkxProtocol::Payload::Nack::NackReason::PayloadBroken );
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void EspMcuDriver::processAlive( const bool bAlive )
{
    Q_UNUSED( bAlive );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void EspMcuDriver::processMcuStreamData( const QByteArray & array )
{
    EkxProtocol::Payload::McuCommand * pPayload = new EkxProtocol::Payload::McuCommand();

    pPayload->command().setValue( array.toStdString() );

    send( pPayload );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void EspMcuDriver::processMcuConfigChanged()
{
    mcuDriver()->setMcuConfig( m_pMainStatemachine->settingsSerializer()->mcuMaxMotorSpeed(),
                               m_pMainStatemachine->settingsSerializer()->mcuNominalMotorSpeed(),
                               m_pMainStatemachine->settingsSerializer()->mcuAccelerationTime(),
                               m_pMainStatemachine->settingsSerializer()->mcuDecelerationTime() );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////
