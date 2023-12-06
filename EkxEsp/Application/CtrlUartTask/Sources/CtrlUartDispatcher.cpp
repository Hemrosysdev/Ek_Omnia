/*!*************************************************************************************************************************************************************
 *
 * @file SocketDispatcherTask.cpp
 * @brief Source file of component unit Socket-Dispatcher-Task.
 *
 * This file was developed as part of Socket-Dispatcher.
 *
 * @author Ultratronik GmbH
 *         Dornierstr. 9
 *         D-82205 Gilching
 *         http://www.ultratronik.de
 * @author Srdjan Nikolic, Entwicklung - <optional according to the authors needs>
 *
 * @copyright Copyright 2020 by Ultratronik GmbH.
 *            All rights reserved.
 *            None of this file or parts of it may be
 *            copied, redistributed or used in any other way
 *            without written approval of Ultratronik GmbH.
 *
 **************************************************************************************************************************************************************/

/***************************************************************************************************************************************************************
 *      Includes
 **************************************************************************************************************************************************************/

#include "CtrlUartDispatcher.h"

#include <string>

#include "UartControlledDevice.h"
#include "EspLog.h"
#include "EkxProtocol.h"
#include "CtrlUartTask.h"
#include "CtrlUartQueueTypes.h"
#include "MessageQueue.h"

namespace CtrlUart
{

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

CtrlUartDispatcher::CtrlUartDispatcher( CtrlUartTask * pCtrlUartTask )
    : UxEspCppLibrary::EspLog( "CtrlUartDispatcher" )
    , m_pCtrlUartTask( pCtrlUartTask )
    , m_timerAlive( m_pCtrlUartTask->taskQueue(),
                    static_cast<int>( CtrlUartQueueTypes::messageType::TimeoutAlive ) )
{
    logInfo( "constructor" );

    m_timerAlive.setIntervall( c_nAliveTimeoutUs );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

CtrlUartDispatcher::~CtrlUartDispatcher()
{
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void CtrlUartDispatcher::registerDriver( UartControlledDevice * const pDevice )
{
    if ( pDevice )
    {
        EkxProtocol::DriverId u8DriverId = pDevice->driverId();

        m_theDevices[u8DriverId] = pDevice;
    }
    else
    {
        vlogError( "registerDriver: null" );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void CtrlUartDispatcher::unregisterDriver( const UartControlledDevice * const pDevice )
{
    deviceMap::iterator i;
    for ( i = m_theDevices.begin(); i != m_theDevices.end(); i++ )
    {
        if ( i->second == pDevice )
        {
            m_theDevices.erase( i );
            break;
        }
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

UartControlledDevice * CtrlUartDispatcher::device( const EkxProtocol::DriverId u8DriverId )
{
    if ( m_theDevices.find( u8DriverId ) == m_theDevices.end() )
    {
        return nullptr;
    }

    return m_theDevices[u8DriverId];
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void CtrlUartDispatcher::dispatchRequestDriverStatus( void )
{
    deviceMap::iterator i;
    for ( i = m_theDevices.begin(); i != m_theDevices.end(); i++ )
    {
        i->second->queueRequestDriverStatus();
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void CtrlUartDispatcher::dispatchCtrlUartCommand( const EkxProtocol::Deserializer::ParseResult nResult,
                                                  EkxProtocol::MessageFrame &                  frame )
{
    uint32_t              u32MsgCounter   = frame.msgCounter().value();
    uint8_t               u8RepeatCounter = frame.repeatCounter().value();
    EkxProtocol::DriverId u8DriverId      = frame.driverId().value();

    if ( nResult == EkxProtocol::Deserializer::ParseResult::WrongCrc )
    {
        replyNack( u8DriverId,
                   u32MsgCounter,
                   u8RepeatCounter,
                   EkxProtocol::Payload::Nack::NackReason::WrongCrc );
    }
    else
    {
        switch ( frame.payloadRef().payloadType() )
        {
            case EkxProtocol::Payload::PayloadType::Ack:
            {
                m_pCtrlUartTask->messageQueue()->processAck( u32MsgCounter );
            }
            break;

            case EkxProtocol::Payload::PayloadType::Nack:
            {
                const EkxProtocol::Payload::Nack * const pNack = dynamic_cast<const EkxProtocol::Payload::Nack *>( frame.payloadRef().value() );

                if ( pNack )
                {
                    m_pCtrlUartTask->messageQueue()->processNack( u32MsgCounter, pNack->nackReason().value() );
                }
            }
            break;

            default:
            {
                const EkxProtocol::Payload::MessagePayload * const pMessagePayload = frame.payloadRef().value();

                if ( !pMessagePayload )
                {
                    replyNack( u8DriverId, u32MsgCounter, u8RepeatCounter, EkxProtocol::Payload::Nack::NackReason::PayloadBroken );
                }
                else
                {
                    UartControlledDevice * pDevice = device( u8DriverId );

                    if ( u8DriverId == EkxProtocol::DriverId::SystemDriver )
                    {
                        processSystemCommand( u8DriverId,
                                              u32MsgCounter,
                                              u8RepeatCounter,
                                              pMessagePayload );
                    }
                    else if ( isAlive() )
                    {
                        if ( u8DriverId == EkxProtocol::DriverId::Broadcast )
                        {
                            processBroadcastCommand( u32MsgCounter,
                                                     u8RepeatCounter,
                                                     pMessagePayload );
                        }
                        else if ( !pDevice )
                        {
                            replyNack( u8DriverId, u32MsgCounter, u8RepeatCounter, EkxProtocol::Payload::Nack::NackReason::DriverNotFound );
                            vlogError( "dispatchCtrlUartCommand() device not found for driver ID %d", static_cast<int>( u8DriverId ) );
                        }
                        else
                        {
                            pDevice->queueCtrlUartCommand( u8DriverId,
                                                           u32MsgCounter,
                                                           u8RepeatCounter,
                                                           pMessagePayload->clone() );
                        }
                    }
                }
            }
            break;
        }
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void CtrlUartDispatcher::processBroadcastCommand( const uint32_t                                     u32MsgCounter,
                                                  const uint8_t                                      u8RepeatCounter,
                                                  const EkxProtocol::Payload::MessagePayload * const pMessagePayload )
{
    if ( pMessagePayload )
    {
        switch ( pMessagePayload->payloadType() )
        {
            case EkxProtocol::Payload::PayloadType::RequestDriverStatus:
                replyAck( EkxProtocol::DriverId::Broadcast, u32MsgCounter, u8RepeatCounter );

                dispatchRequestDriverStatus();
                break;

            default:
                replyNack( EkxProtocol::DriverId::Broadcast, u32MsgCounter, u8RepeatCounter, EkxProtocol::Payload::Nack::NackReason::NoBroadcastCommand );
                break;
        }
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void CtrlUartDispatcher::replyAck( const EkxProtocol::DriverId u8DriverId,
                                   const uint32_t              u32MsgCounter,
                                   const uint8_t               u8RepeatCounter )
{
    EkxProtocol::Payload::Ack * pPayload = new EkxProtocol::Payload::Ack();

    m_pCtrlUartTask->processReplyMessage( u8DriverId, u32MsgCounter, pPayload );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void CtrlUartDispatcher::replyPong( const uint32_t u32MsgCounter,
                                    const uint8_t  u8RepeatCounter )
{
    EkxProtocol::Payload::Pong * pPayload = new EkxProtocol::Payload::Pong();

    m_pCtrlUartTask->processReplyMessage( EkxProtocol::DriverId::SystemDriver, u32MsgCounter, pPayload );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void CtrlUartDispatcher::processSystemCommand( const EkxProtocol::DriverId                        u8DriverId,
                                               const uint32_t                                     u32MsgCounter,
                                               const uint8_t                                      u8RepeatCounter,
                                               const EkxProtocol::Payload::MessagePayload * const pMessagePayload )
{
    if ( pMessagePayload )
    {
        switch ( pMessagePayload->payloadType() )
        {
            case EkxProtocol::Payload::PayloadType::Reset:
                if ( isAlive() )
                {
                    replyAck( u8DriverId, u32MsgCounter, u8RepeatCounter );

                    vTaskDelay( 2000 / portTICK_PERIOD_MS );

                    esp_restart();
                }
                break;

            case EkxProtocol::Payload::PayloadType::Ping:
                replyPong( u32MsgCounter, u8RepeatCounter );

                setAlive( true );
                break;

            case EkxProtocol::Payload::PayloadType::Ack:
                // do nothing
                break;

            case EkxProtocol::Payload::PayloadType::Nack:
                vlogWarning( "processSystemCommand() receive Nack" );
                // do nothing
                break;

            default:
                replyNack( u8DriverId, u32MsgCounter, u8RepeatCounter, EkxProtocol::Payload::Nack::NackReason::UnknownDriverCommand );
                break;
        }
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void CtrlUartDispatcher::replyNack( const EkxProtocol::DriverId                  u8DriverId,
                                    const uint32_t                               u32MsgCounter,
                                    const uint8_t                                u8RepeatCounter,
                                    const EkxProtocol::Payload::Nack::NackReason u8NackReason )
{
    EkxProtocol::Payload::Nack * pPayload = new EkxProtocol::Payload::Nack();
    pPayload->nackReason().setValue( u8NackReason );

    m_pCtrlUartTask->processReplyMessage( u8DriverId, u32MsgCounter, pPayload );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void CtrlUartDispatcher::setAlive( const bool bAlive )
{
    if ( m_bAlive != bAlive )
    {
        m_bAlive = bAlive;

        if ( m_bAlive )
        {
            vlogInfo( "setAlive() become alive" );
        }
        else
        {
            vlogError( "setAlive() died" );
        }

        dispatchAlive( m_bAlive );
    }

    if ( bAlive )
    {
        m_timerAlive.startOnce();
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void CtrlUartDispatcher::dispatchAlive( const bool bAlive )
{
    deviceMap::iterator i;
    for ( i = m_theDevices.begin(); i != m_theDevices.end(); i++ )
    {
        i->second->queueAlive( bAlive );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

bool CtrlUartDispatcher::isAlive() const
{
    return m_bAlive;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void CtrlUartDispatcher::dispatchMessageProcessed( const EkxProtocol::DriverId u8DriverId,
                                                   const uint32_t              u32MsgCounter,
                                                   const bool                  bSuccess )
{
    UartControlledDevice * const pDevice = device( u8DriverId );

    if ( pDevice )
    {
        pDevice->queueAckNackReceived( u32MsgCounter, bSuccess );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

} // namespace CtrlUart
