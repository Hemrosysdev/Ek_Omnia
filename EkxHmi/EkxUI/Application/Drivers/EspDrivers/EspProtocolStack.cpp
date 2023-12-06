///////////////////////////////////////////////////////////////////////////////
///
/// @file EspProtocolStack.h
///
/// @brief Header file of class EspProtocolStack.
///
/// @author Ultratronik GmbH \n
///         Dornierstr. 9 \n
///         D-82205 Gilching \n
///         Germany
///         http://www.ultratronik.de
///
/// @author written by Gerd Esser, Forschung & Entwicklung, gesser@ultratronik.de
///
/// @date 04.05.2023
///
/// @copyright Copyright 2023 by Hemro International AG \n
///            Hemro International AG \n
///            Länggenstrasse 34 \n
///            CH 8184 Bachenbülach \n
///            Switzerland \n
///            Homepage: www.hemrogroup.com
///
///////////////////////////////////////////////////////////////////////////////

#include "EspProtocolStack.h"

#include <QDebug>
#include <QTimer>

#include "EkxProtocol.h"
#include "EspDeviceDriver.h"

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

EspProtocolStack::EspProtocolStack()
{
    connect( &m_timerAlive, &QTimer::timeout, this, &EspProtocolStack::processTimeoutAlive );
    connect( &m_timerSendPing, &QTimer::timeout, this, &EspProtocolStack::processTimeoutSendPing );
    connect( this, &EspProtocolStack::aliveChanged, this, &EspProtocolStack::processAliveChanged );
    connect( &m_timerQueueObserver, &QTimer::timeout, this, &EspProtocolStack::processTimeoutQueueObserver );

    m_timerAlive.setInterval( 1600 );
    m_timerAlive.setSingleShot( true );

    m_timerSendPing.setInterval( 500 );
    m_timerSendPing.setSingleShot( false );
    m_timerSendPing.start();

    m_timerQueueObserver.setInterval( 50 );
    m_timerQueueObserver.setSingleShot( false );
    m_timerQueueObserver.start();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

EspProtocolStack::~EspProtocolStack()
{
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool EspProtocolStack::isAlive() const
{
    return m_bAlive;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool EspProtocolStack::queueMessageFrame( const EkxProtocol::DriverId                  u8DriverId,
                                          const quint32 u32MsgCounter,
                                          const quint8 u8RepeatCounter,
                                          EkxProtocol::Payload::MessagePayload * const pPayload )
{
    bool bSuccess = false;

    bool bForce = ( pPayload->payloadType() == EkxProtocol::Payload::PayloadType::Ping );

    if ( isOpen()
         && ( isAlive()
              || bForce ) )
    {
        EkxProtocol::MessageFrame * pFrame = new EkxProtocol::MessageFrame();

        pFrame->driverId().setValue( u8DriverId );
        pFrame->msgCounter().setValue( u32MsgCounter );
        pFrame->repeatCounter().setValue( u8RepeatCounter );
        pFrame->payloadRef().setValue( pPayload );     // eats the pointer

        m_theMsgQueue << QPair( 0, pFrame );

        processTimeoutQueueObserver();

        bSuccess = true;
    }
    else
    {
        delete pPayload;
    }

    return bSuccess;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void EspProtocolStack::registerDriver( EspDeviceDriver * pDriver )
{
    if ( pDriver )
    {
        m_driverMap[pDriver->driverId()] = pDriver;
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void EspProtocolStack::setAlive( const bool bAlive )
{
    if ( m_bAlive != bAlive )
    {
        m_bAlive = bAlive;

        if ( m_bAlive )
        {
            QTimer::singleShot( 500, this, &EspProtocolStack::delayedAliveChanged );
        }
        else
        {
            qCritical() << "setAlive() died";
            emit aliveChanged( m_bAlive );
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void EspProtocolStack::delayedAliveChanged()
{
    qInfo() << "delayedAliveChanged()";
    emit aliveChanged( m_bAlive );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void EspProtocolStack::processTimeoutAlive()
{
    setAlive( false );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void EspProtocolStack::processTimeoutSendPing()
{
    // ping doesn't need to be queued
    queueMessageFrame( EkxProtocol::DriverId::SystemDriver,
                       incMsgCounter(),
                       0,
                       new EkxProtocol::Payload::Ping() );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void EspProtocolStack::processAliveChanged( const bool bAlive )
{
    if ( bAlive )
    {
        queueMessageFrame( EkxProtocol::DriverId::Broadcast, incMsgCounter(), 0, new EkxProtocol::Payload::RequestDriverStatus() );
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void EspProtocolStack::processMessageFrame( const EkxProtocol::MessageFrame & frame,
                                            const EkxProtocol::Deserializer::ParseResult nResult )
{
    if ( nResult == EkxProtocol::Deserializer::ParseResult::WrongCrc )
    {
//        frame.serialize();
//        dumpHexData( "Wrong CRC msg", frame.serializedData() );

        EkxProtocol::Payload::Nack * pPayload = new EkxProtocol::Payload::Nack();
        pPayload->nackReason().setValue( EkxProtocol::Payload::Nack::NackReason::WrongCrc );

        queueMessageFrame( frame.driverId().value(),
                           frame.msgCounter().value(),
                           frame.repeatCounter().value(),
                           pPayload );
    }
    else
    {
        switch ( frame.payloadRef().payloadType() )
        {
            case EkxProtocol::Payload::PayloadType::Pong:
                setAlive( true );
                m_timerAlive.start();
            break;

            case EkxProtocol::Payload::PayloadType::Ack:
                processAckedFrame( frame.driverId().value(),
                                   frame.msgCounter().value() );
            break;

            case EkxProtocol::Payload::PayloadType::Nack:
                processNackedFrame( frame );
            break;

            default:
                //qInfo() << "receive message" << EkxProtocol::Payload::payloadTypeString( frame.payloadRef().payloadType() ) << frame.msgCounter().value() << frame.repeatCounter().value();

                if ( m_driverMap.contains( frame.driverId().value() ) )
                {
                    m_driverMap[frame.driverId().value()]->processMessageFrame( frame );
                }
            break;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

quint32 EspProtocolStack::incMsgCounter()
{
    return ++m_u32MsgCounter;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void EspProtocolStack::processTimeoutQueueObserver()
{
    foreach( queuePair pair, m_theMsgQueue )
    {
        EkxProtocol::MessageFrame * const pFrame = pair.second;

        if ( pair.first == 0 )
        {
            const EkxProtocol::Payload::MessagePayload * const pPayload = pFrame->payloadRef().value();

            if ( ( pPayload->payloadType() == EkxProtocol::Payload::PayloadType::Ack )
                 || ( pPayload->payloadType() == EkxProtocol::Payload::PayloadType::Nack )
                 || ( pPayload->payloadType() == EkxProtocol::Payload::PayloadType::Ping )
                 || ( pFrame->driverId().value() == EkxProtocol::DriverId::Broadcast ) )
            {
                sendMessageFrame( pFrame );
                m_theMsgQueue.removeAll( pair );
                delete pFrame;
            }
            else if ( pFrame->repeatCounter().value() < c_u8MaxRepeatCounter )
            {
                sendMessageFrame( pFrame );

                pFrame->repeatCounter().setValue( pFrame->repeatCounter().value() + 1 );

                pair.first++;
            }
            else
            {
                qWarning() << "EspProtocolStack::processTimeoutQueueObserver() Remove message due to timeout" << pFrame->msgCounter().value() << pFrame->repeatCounter().value();
                m_theMsgQueue.removeAll( pair );
                frameNacked( pFrame->msgCounter().value(), EkxProtocol::Payload::Nack::NackReason::Timeout );
                delete pFrame;
            }
        }
        else if ( pair.first >= c_nMaxQueueTicks )
        {
            pair.first = 0;
        }
        else
        {
            pair.first++;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void EspProtocolStack::processAckedFrame( const EkxProtocol::DriverId u8DriverId,
                                          const quint32 u32MsgCounter )
{
    foreach( queuePair pair, m_theMsgQueue )
    {
        EkxProtocol::MessageFrame * pFrame = pair.second;

        if ( pFrame->driverId().value() == u8DriverId
             && pFrame->msgCounter().value() == u32MsgCounter )
        {
            m_theMsgQueue.removeAll( pair );
            delete pFrame;

            frameAcked( u32MsgCounter );
            break;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void EspProtocolStack::processNackedFrame( const EkxProtocol::MessageFrame & frame )
{
    const EkxProtocol::Payload::Nack * pPayload = dynamic_cast<const EkxProtocol::Payload::Nack *>( frame.payloadRef().value() );

    if ( pPayload )
    {
        foreach( queuePair pair, m_theMsgQueue )
        {
            EkxProtocol::MessageFrame * pFrame = pair.second;

            if ( pFrame->driverId().value() == frame.driverId().value()
                 && pFrame->msgCounter().value() == frame.msgCounter().value() )
            {
                pair.first = 0;
                processTimeoutQueueObserver();
//                if ( pFrame->repeatCounter().value() < c_u8MaxRepeatCounter )
//                {
//                    pFrame->repeatCounter().setValue( pFrame->repeatCounter().value() + 1 );
//                }
//                else
//                {
//                    qWarning() << "msg removed by nack" << static_cast<int>( pFrame->driverId().value() ) << pFrame->msgCounter().value() << pFrame->repeatCounter().value();

//                    m_theMsgQueue.removeAll( pair );
//                    delete pFrame;

//                    frameNacked( frame.msgCounter().value(), pPayload->nackReason().value() );
//                }
                break;
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void EspProtocolStack::frameAcked( const quint32 u32MsgCounter )
{
    Q_UNUSED( u32MsgCounter );

    // all stuff done in overloaded function
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void EspProtocolStack::frameNacked( const quint32                                u32MsgCounter,
                                    const EkxProtocol::Payload::Nack::NackReason u8NackReason )
{
    Q_UNUSED( u32MsgCounter );
    Q_UNUSED( u8NackReason );

    // all stuff done in overloaded function
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

