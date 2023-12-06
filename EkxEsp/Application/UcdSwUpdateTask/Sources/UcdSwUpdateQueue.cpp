/*
 * UcdSwUpdateQueue.cpp
 *
 *  Created on: 24.10.2019
 *      Author: gesser
 */

#include "UcdSwUpdateQueue.h"

#include "UcdSwUpdateQueueTypes.h"
#include "FreeRtosQueueTask.h"
#include "EkxProtocol.h"

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

UcdSwUpdateQueue::UcdSwUpdateQueue()
    : UxEspCppLibrary::FreeRtosQueue( 10,
                                      UxEspCppLibrary::FreeRtosQueueTask::minTargetQueueSize( sizeof( UcdSwUpdateQueueTypes::message ) ),
                                      "UcdSwUpdateQueue" )
{

}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

UcdSwUpdateQueue::~UcdSwUpdateQueue()
{
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

BaseType_t UcdSwUpdateQueue::sendCtrlUartCommand( const EkxProtocol::DriverId                  u8DriverId,
                                                  const uint32_t                               u32MsgCounter,
                                                  const uint8_t                                u8RepeatCounter,
                                                  EkxProtocol::Payload::MessagePayload * const pMessagePayload )
{
    UcdSwUpdateQueueTypes::message message;

    message.u32Type                                   = UcdSwUpdateQueueTypes::messageType::CtrlUartCommand;
    message.payload.ctrlUartCommand.m_u8DriverId      = u8DriverId;
    message.payload.ctrlUartCommand.m_u32MsgCounter   = u32MsgCounter;
    message.payload.ctrlUartCommand.m_u8RepeatCounter = u8RepeatCounter;
    message.payload.ctrlUartCommand.m_pMessagePayload = pMessagePayload;

    return send( &message );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

BaseType_t UcdSwUpdateQueue::sendAckNackReceived( const uint32_t u32MsgCounter,
                                                  const bool     bSuccess )
{
    UcdSwUpdateQueueTypes::message message;

    message.u32Type                                 = UcdSwUpdateQueueTypes::messageType::AckNackReceived;
    message.payload.ackNackReceived.m_u32MsgCounter = u32MsgCounter;
    message.payload.ackNackReceived.m_bSuccess      = bSuccess;

    return send( &message );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/
