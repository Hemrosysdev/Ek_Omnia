/*
 * HttpServerQueue.cpp
 *
 *  Created on: 24.10.2019
 *      Author: gesser
 */

#include <string.h>

#include "UcdHttpServerQueue.h"
#include "UcdHttpServerQueueTypes.h"
#include "FreeRtosQueueTask.h"

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

UcdHttpServerQueue::UcdHttpServerQueue()
    : UxEspCppLibrary::FreeRtosQueue( 10,
                                      UxEspCppLibrary::FreeRtosQueueTask::minTargetQueueSize( sizeof( UcdHttpServerQueueTypes::message ) ),
                                      "HttpServerQueue" )
{

}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

UcdHttpServerQueue::~UcdHttpServerQueue()
{
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

BaseType_t UcdHttpServerQueue::sendServerStart( void )
{
    return sendSimpleEvent( static_cast<uint32_t>( UcdHttpServerQueueTypes::messageType::Start ) );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

BaseType_t UcdHttpServerQueue::sendServerStop()
{
    return sendSimpleEvent( static_cast<uint32_t>( UcdHttpServerQueueTypes::messageType::Stop ) );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

BaseType_t UcdHttpServerQueue::sendCtrlUartCommand( const EkxProtocol::DriverId                  u8DriverId,
                                                    const uint32_t                               u32MsgCounter,
                                                    const uint8_t                                u8RepeatCounter,
                                                    EkxProtocol::Payload::MessagePayload * const pMessagePayload )
{
    UcdHttpServerQueueTypes::message message;

    message.u32Type                                   = UcdHttpServerQueueTypes::messageType::CtrlUartCommand;
    message.payload.ctrlUartCommand.m_u8DriverId      = u8DriverId;
    message.payload.ctrlUartCommand.m_u32MsgCounter   = u32MsgCounter;
    message.payload.ctrlUartCommand.m_u8RepeatCounter = u8RepeatCounter;
    message.payload.ctrlUartCommand.m_pMessagePayload = pMessagePayload;

    return send( &message );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

BaseType_t UcdHttpServerQueue::sendAckNackReceived( const uint32_t u32MsgCounter,
                                                    const bool     bSuccess )
{
    UcdHttpServerQueueTypes::message message;

    message.u32Type                                 = UcdHttpServerQueueTypes::messageType::AckNackReceived;
    message.payload.ackNackReceived.m_u32MsgCounter = u32MsgCounter;
    message.payload.ackNackReceived.m_bSuccess      = bSuccess;

    return send( &message );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/
