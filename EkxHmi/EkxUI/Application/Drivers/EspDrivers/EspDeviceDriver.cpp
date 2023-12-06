///////////////////////////////////////////////////////////////////////////////
///
/// @file EspDeviceDriver.cpp
///
/// @brief main application entry point of EspDeviceDriver.
///
/// @author Ultratronik GmbH
///         Dornierstr. 9
///         D-82205 Gilching
///         Germany
///         http://www.ultratronik.de
///
/// @author written by Florian Sonntag, Forschung & Entwicklung, gesser@ultratronik.de
///
/// @date 05.05.2023
///
/// @copyright Copyright 2023 by Hemro International AG
///            Hemro International AG
///            Länggenstrasse 34
///            CH 8184 Bachenbülach
///            Switzerland
///            Homepage: www.hemrogroup.com
///
///////////////////////////////////////////////////////////////////////////////

#include "EspDeviceDriver.h"

#include <QDebug>

#include "EspProtocolStack.h"

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

EspDeviceDriver::EspDeviceDriver( const EkxProtocol::DriverId u8DriverId,
                                  QObject *                   parent )
    : QObject( parent )
    , m_u8DriverId( u8DriverId )
{

}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void EspDeviceDriver::create( EspProtocolStack * const pProtocolStack )
{
    m_pProtocolStack = pProtocolStack;

    m_pProtocolStack->registerDriver( this );

    connect( m_pProtocolStack, &EspProtocolStack::aliveChanged, this, &EspDeviceDriver::processAlive );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

EkxProtocol::DriverId EspDeviceDriver::driverId() const
{
    return m_u8DriverId;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void EspDeviceDriver::processMessageFrame( const EkxProtocol::MessageFrame & frame )
{
    Q_UNUSED( frame );

    // do nothing, all done in overloaded class
    qCritical() << "EspDeviceDriver::processMessageFrame not overloaded";
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void EspDeviceDriver::replyNack( const EkxProtocol::DriverId                  u8DriverId,
                                 const quint32                                u32MsgCounter,
                                 const quint32                                u8RepeatCounter,
                                 const EkxProtocol::Payload::Nack::NackReason u8NackReason )
{
    EkxProtocol::Payload::Nack * pPayload = new EkxProtocol::Payload::Nack();
    pPayload->nackReason().setValue( u8NackReason );

    send( u8DriverId, u32MsgCounter, u8RepeatCounter, pPayload );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void EspDeviceDriver::replyAck( const EkxProtocol::DriverId u8DriverId,
                                const quint32               u32MsgCounter,
                                const quint32               u8RepeatCounter )
{
    EkxProtocol::Payload::Ack * pPayload = new EkxProtocol::Payload::Ack();

    send( u8DriverId, u32MsgCounter, u8RepeatCounter, pPayload );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void EspDeviceDriver::send( const EkxProtocol::DriverId                  u8DriverId,
                            const quint32                                u32MsgCounter,
                            const quint32                                u8RepeatCounter,
                            EkxProtocol::Payload::MessagePayload * const pPayload )
{
    if ( !m_pProtocolStack )
    {
        qWarning() << "EspDeviceDriver::send() protocol stack is null";
    }
    else if ( !pPayload )
    {
        qCritical() << "EspDeviceDriver::send() illegal null pointer";
    }
    else
    {
        m_pProtocolStack->queueMessageFrame( u8DriverId, u32MsgCounter, u8RepeatCounter, pPayload );
//        m_pProtocolStack->sendMessageFrame( u8DriverId, u32MsgCounter, u8RepeatCounter, pPayload );
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void EspDeviceDriver::send( EkxProtocol::Payload::MessagePayload * const pPayload )
{
    if ( m_pProtocolStack )
    {
        send( driverId(), m_pProtocolStack->incMsgCounter(), 0, pPayload );
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void EspDeviceDriver::processAlive( const bool bAlive )
{
    Q_UNUSED( bAlive );

    // all stuf done in overloaded method
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

