/*
 * MessageQueue.cpp
 *
 *  Created on: 12.05.2023
 *      Author: gesser
 */

#include "MessageQueue.h"

#include <rom/crc.h>

#include "CtrlUartDispatcher.h"
#include "CtrlUartDrvTask.h"
#include "CtrlUartTask.h"
#include "ApplicationHelper.h"
#include "SystemObserver.h"

namespace CtrlUart
{

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

MessageQueue::MessageQueue( CtrlUartTask * const pCtrlUartTask )
    : UxEspCppLibrary::EspLog( "MessageQueue" )
    , m_pCtrlUartTask( pCtrlUartTask )
{
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

MessageQueue::~MessageQueue()
{
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void MessageQueue::processAck( const uint32_t u32MsgCounter )
{
    std::lock_guard<std::mutex> lock( m_mutexMsgQueue );

    for ( MsgQueue::iterator it = m_theMsgQueue.begin(); it != m_theMsgQueue.end(); it++ )
    {
        EkxProtocol::MessageFrame * const pFrame = it->m_pFrame;

        if ( pFrame->msgCounter().value() == u32MsgCounter )
        {
            m_theMsgQueue.erase( it );
            m_pCtrlUartTask->dispatcher()->dispatchMessageProcessed( pFrame->driverId().value(), u32MsgCounter, true );
            delete pFrame;
            break;
        }
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void MessageQueue::processNack( const uint32_t                               u32MsgCounter,
                                const EkxProtocol::Payload::Nack::NackReason u8NackReason )
{
    std::lock_guard<std::mutex> lock( m_mutexMsgQueue );

    for ( MsgQueue::iterator it = m_theMsgQueue.begin(); it != m_theMsgQueue.end(); it++ )
    {
        EkxProtocol::MessageFrame * const pFrame = it->m_pFrame;

        if ( pFrame->msgCounter().value() == u32MsgCounter )
        {
            pFrame->repeatCounter().setValue( pFrame->repeatCounter().value() + 1 );

            if ( pFrame->repeatCounter().value() > c_u8MaxRepeatCounter )
            {
                vlogError( "processNack() remove message, give up after final nack %u %d reason %d", u32MsgCounter, pFrame->repeatCounter().value() - 1, static_cast<int>( u8NackReason ) );
                m_theMsgQueue.erase( it );
                m_pCtrlUartTask->dispatcher()->dispatchMessageProcessed( pFrame->driverId().value(), u32MsgCounter, false );
                delete pFrame;
            }
            else
            {
                vlogWarning( "processNack %u %d reason %d", u32MsgCounter, pFrame->repeatCounter().value() - 1, static_cast<int>( u8NackReason ) );
            }

            break;
        }
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void MessageQueue::enqueuMessage( const EkxProtocol::DriverId                  u8DriverId,
                                  const uint32_t                               u32MsgCounter,
                                  EkxProtocol::Payload::MessagePayload * const pMessagePayload )
{
    if ( pMessagePayload )
    {
        bool bPong = ( u8DriverId == EkxProtocol::DriverId::SystemDriver
                       && pMessagePayload->payloadType() == EkxProtocol::Payload::PayloadType::Pong );
        bool bNoEnqueue = ( bPong
                            || ( u8DriverId == EkxProtocol::DriverId::Broadcast )
                            || ( pMessagePayload->payloadType() == EkxProtocol::Payload::PayloadType::Ack )
                            || ( pMessagePayload->payloadType() == EkxProtocol::Payload::PayloadType::Nack ) );

        if ( m_pCtrlUartTask->dispatcher()->isAlive()
             || bPong )
        {
            std::lock_guard<std::mutex> lock( m_mutexMsgQueue );

            EkxProtocol::MessageFrame * pFrame = new EkxProtocol::MessageFrame();

            pFrame->driverId().setValue( u8DriverId );
            pFrame->msgCounter().setValue( u32MsgCounter );
            pFrame->repeatCounter().setValue( 0 );
            pFrame->payloadRef().setValue( pMessagePayload );     // eats the pointer

            if ( !bNoEnqueue )
            {
                m_theMsgQueue.push_back( MsgQueueInfo{ std::chrono::steady_clock::now(), pFrame } );
            }

            sendFrame( pFrame );

            if ( bNoEnqueue )
            {
                delete pFrame;
            }
        }
        else
        {
            delete pMessagePayload;
        }
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void MessageQueue::timeoutRun()
{
    std::chrono::steady_clock::time_point tNow = std::chrono::steady_clock::now();

    std::lock_guard<std::mutex> lock( m_mutexMsgQueue );

    for ( int i = m_theMsgQueue.size() - 1; i >= 0; i-- )
    {
        EkxProtocol::MessageFrame * const pFrame = m_theMsgQueue[i].m_pFrame;

        if ( std::chrono::duration_cast<std::chrono::microseconds>( tNow - m_theMsgQueue[i].m_tLastSend ).count() > c_u32MaxRepeatTimeUs )
        {
            //pFrame->serialize();
            //ApplicationGlobals::ApplicationHelper::dumpHexData( "timeout Msg", reinterpret_cast<const uint8_t*>( pFrame->serializedData().data() ), pFrame->serializedData().size() );

            pFrame->repeatCounter().setValue( pFrame->repeatCounter().value() + 1 );
            m_theMsgQueue[i].m_tLastSend = tNow;

            if ( pFrame->repeatCounter().value() > c_u8MaxRepeatCounter )
            {
                vlogError( "timeoutRun() give-up repeat %u %d", pFrame->msgCounter().value(), static_cast<int>( pFrame->repeatCounter().value() ) - 1 );
                m_pCtrlUartTask->systemObserver()->incMessageNumTxTimeouts();
                m_theMsgQueue.erase( m_theMsgQueue.begin() + i );
                m_pCtrlUartTask->dispatcher()->dispatchMessageProcessed( pFrame->driverId().value(),
                                                                         pFrame->msgCounter().value(),
                                                                         false );
                delete pFrame;
            }
            else
            {
                vlogWarning( "timeoutRun() repeat %u %d", pFrame->msgCounter().value(), static_cast<int>( pFrame->repeatCounter().value() ) - 1 );
                m_pCtrlUartTask->systemObserver()->incMessageNumTxTimeouts();
                sendFrame( pFrame );
            }
        }
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void MessageQueue::sendFrame( EkxProtocol::MessageFrame * const pFrame )
{
    m_pCtrlUartTask->systemObserver()->incMessageNumTx();

    if ( pFrame->payloadRef().payloadType() == EkxProtocol::Payload::PayloadType::Nack )
    {
        m_pCtrlUartTask->systemObserver()->incMessageNumRxErrors();
    }

    auto lambdaCrc32 = []( const std::string & strData, EkxProtocol::MessageFrame & message )
                       {
                           uint32_t u32Crc32 = crc32_be( 0x0, reinterpret_cast<const uint8_t *>( strData.data() ), strData.size() );
                           message.crc32().setValue( u32Crc32 );
                       };

    EkxProtocol::Serializer serializer( lambdaCrc32 );
    serializer.serialize( *pFrame );

    m_pCtrlUartTask->uartDrvTask()->writeData( serializer.serializedData() );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

} // namespace CtrlUart
