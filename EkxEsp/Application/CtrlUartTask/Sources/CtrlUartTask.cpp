/*!*****************************************************************************
*
* @file CtrlUartTask.c
*
* @brief Implementation file of class CtrlUartTask.
*
* @author Ultratronik GmbH
*         Dornierstr. 9
*         D-82205 Gilching
*         www.ultratronik.de
*
* @author Gerd Esser, Forschung & Entwicklung
*
* @created 28.11.2022
*
* @copyright Copyright 2022 by Hemro International AG
*             Hemro International AG
*             Länggenstrasse 34
*             CH 8184 Bachenbülach
*             Switzerland
*             Homepage: www.hemrogroup.com
*
*******************************************************************************/

/**************************************************************************************************************************************************************
*  Includes
**************************************************************************************************************************************************************/

#include "CtrlUartTask.h"

#include <cstring>
#include <rom/crc.h>

#include "CtrlUartQueue.h"
#include "CtrlUartQueueTypes.h"
#include "ApplicationGlobals.h"
#include "CtrlUartDrvTask.h"
#include "CtrlUartDispatcher.h"
#include "EkxProtocol.h"
#include "MessageQueue.h"
#include "SystemObserver.h"

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

namespace CtrlUart
{

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

CtrlUartTask::CtrlUartTask( UxEspCppLibrary::EspApplication * const pApplication )
    : FreeRtosQueueTask( pApplication,
                         ApplicationGlobals::c_nCtrlUartTaskStackSize,
                         ApplicationGlobals::c_nCtrlUartTaskPriority,
                         "CtrlUartTask",
                         new CtrlUartQueue )
    , m_pUartDrvTask( new CtrlUartDrvTask( this ) )
    , m_pDispatcher( new CtrlUartDispatcher( this ) )
    , m_pMessageQueue( new MessageQueue( this ) )
    , m_timerMessageQueue( taskQueue(),
                           static_cast<int>( CtrlUartQueueTypes::messageType::TimeoutMessageQueue ) )
                           , m_timerStatusUpdate( taskQueue(),
                                                  static_cast<int>( CtrlUartQueueTypes::messageType::TimeoutStatusUpdate ) )
                           , m_pSystemObserver( new SystemObserver( this ) )
{
    logInfo( "constructor" );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

CtrlUartTask::~CtrlUartTask()
{
    delete m_pSystemObserver;
    m_pSystemObserver = nullptr;

    delete m_pMessageQueue;
    m_pMessageQueue = nullptr;

    delete m_pUartDrvTask;
    m_pUartDrvTask = nullptr;

    delete m_pDispatcher;
    m_pDispatcher = nullptr;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

bool CtrlUartTask::startupStep( const UxEspCppLibrary::FreeRtosQueueTask::startupStep_t nStartupStep )
{
    const bool bContinueStartup = true;

    switch ( nStartupStep )
    {
        case FreeRtosQueueTask::startupStep_t::startupStep1:
            logInfo( "startupStep1" );

            m_pUartDrvTask->createTask();

            m_timerMessageQueue.startPeriodic( 20000 );  // 20ms
            m_timerStatusUpdate.startPeriodic( 5000000 );  // 5sec
            break;

        default:
            // do nothing
            break;
    }

    return bContinueStartup;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void CtrlUartTask::processTaskMessage( void )
{
    const CtrlUartQueueTypes::message * pMessage = reinterpret_cast<const CtrlUartQueueTypes::message *>( receiveMsg() );

    if ( pMessage )
    {
        switch ( pMessage->u32Type )
        {
            case CtrlUartQueueTypes::messageType::CtrlUartReply:
            {
                m_pMessageQueue->enqueuMessage( pMessage->payload.ctrlUartReply.m_u8DriverId,
                                                pMessage->payload.ctrlUartReply.m_u32MsgCounter,
                                                pMessage->payload.ctrlUartReply.m_pMessagePayload );
            }
            break;

            case CtrlUartQueueTypes::messageType::RegisterDriver:
            {
                m_pDispatcher->registerDriver( pMessage->payload.registerDriverPayload.m_pDevice );
            }
            break;

            case CtrlUartQueueTypes::messageType::TimeoutAlive:
            {
                m_pDispatcher->setAlive( false );
            }
            break;

            case CtrlUartQueueTypes::messageType::TimeoutMessageQueue:
            {
                m_pMessageQueue->timeoutRun();
            }
            break;

            case CtrlUartQueueTypes::messageType::TimeoutStatusUpdate:
            {
                m_pSystemObserver->statusUpdate();
            }
            break;

            default:
            {
                vlogError( "processTaskMessage: receive illegal message type %d",
                           pMessage->u32Type );
            }
            break;
        }
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

CtrlUartQueue * CtrlUartTask::ctrlUartQueue( void )
{
    return reinterpret_cast<CtrlUartQueue *>( taskQueue() );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

const CtrlUartQueue * CtrlUartTask::ctrlUartQueue( void ) const
{
    return reinterpret_cast<const CtrlUartQueue *>( taskQueue() );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void CtrlUartTask::processReceivedUartFrame( const EkxProtocol::Deserializer::ParseResult nResult,
                                             EkxProtocol::MessageFrame &                  frame )
{
    m_pSystemObserver->incMessageNumRx();
    m_pDispatcher->dispatchCtrlUartCommand( nResult, frame );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void CtrlUartTask::processReplyMessage( const EkxProtocol::DriverId                  u8DriverId,
                                        const uint32_t                               u32MsgCounter,
                                        EkxProtocol::Payload::MessagePayload * const pMessagePayload )
{
    m_pMessageQueue->enqueuMessage( u8DriverId, u32MsgCounter, pMessagePayload );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

CtrlUartDrvTask * CtrlUartTask::uartDrvTask()
{
    return m_pUartDrvTask;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

CtrlUartDispatcher * CtrlUartTask::dispatcher()
{
    return m_pDispatcher;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

MessageQueue * CtrlUartTask::messageQueue()
{
    return m_pMessageQueue;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

SystemObserver* CtrlUartTask::systemObserver()
{
    return m_pSystemObserver;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

uint32_t CtrlUartTask::incSystemMessageCounter()
{
    return ++m_u32SystemMessageCounter;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

} /* namespace CtrlUart */

