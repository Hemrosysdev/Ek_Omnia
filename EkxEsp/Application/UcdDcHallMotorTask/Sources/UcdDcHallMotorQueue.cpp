/*
 * UcdDcHallMotorQueue.cpp
 *
 *  Created on: 06.11.2021
 *      Author: gesser
 */

#include "UcdDcHallMotorQueue.h"

#include "UcdDcHallMotorQueueTypes.h"
#include "FreeRtosQueueTask.h"
#include "EkxProtocol.h"

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

UcdDcHallMotorQueue::UcdDcHallMotorQueue()
    : UxEspCppLibrary::FreeRtosQueue( 100,
                                      UxEspCppLibrary::FreeRtosQueueTask::minTargetQueueSize( sizeof( UcdDcHallMotorQueueTypes::message ) ),
                                      "UcdDcHallMotorQueue" )
{

}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

UcdDcHallMotorQueue::~UcdDcHallMotorQueue()
{
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

BaseType_t UcdDcHallMotorQueue::sendCtrlUartCommand( const EkxProtocol::DriverId                  u8DriverId,
                                                     const uint32_t                               u32MsgCounter,
                                                     const uint8_t                                u8RepeatCounter,
                                                     EkxProtocol::Payload::MessagePayload * const pMessagePayload )
{
    UcdDcHallMotorQueueTypes::message message;

    message.u32Type                                   = UcdDcHallMotorQueueTypes::messageType::CtrlUartCommand;
    message.payload.ctrlUartCommand.m_u8DriverId      = u8DriverId;
    message.payload.ctrlUartCommand.m_u32MsgCounter   = u32MsgCounter;
    message.payload.ctrlUartCommand.m_u8RepeatCounter = u8RepeatCounter;
    message.payload.ctrlUartCommand.m_pMessagePayload = pMessagePayload;

    return send( &message );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

BaseType_t UcdDcHallMotorQueue::sendAckNackReceived( const uint32_t u32MsgCounter,
                                                     const bool     bSuccess )
{
    UcdDcHallMotorQueueTypes::message message;

    message.u32Type                                 = UcdDcHallMotorQueueTypes::messageType::AckNackReceived;
    message.payload.ackNackReceived.m_u32MsgCounter = u32MsgCounter;
    message.payload.ackNackReceived.m_bSuccess      = bSuccess;

    return send( &message );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

BaseType_t UcdDcHallMotorQueue::sendAlive( const bool bAlive )
{
    UcdDcHallMotorQueueTypes::message message;

    message.u32Type                      = UcdDcHallMotorQueueTypes::messageType::Alive;
    message.payload.aliveStatus.m_bAlive = bAlive;

    return send( &message );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/
