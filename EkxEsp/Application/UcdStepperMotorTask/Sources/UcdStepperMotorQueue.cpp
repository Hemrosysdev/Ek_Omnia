/*
 * UcdStepperMotorQueue.cpp
 *
 *  Created on: 31.10.2021
 *      Author: gesser
 */

#include "UcdStepperMotorQueue.h"

#include <cstring>

#include "UcdStepperMotorQueueTypes.h"
#include "FreeRtosQueueTask.h"

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

UcdStepperMotorQueue::UcdStepperMotorQueue()
    : UxEspCppLibrary::FreeRtosQueue( 10,
                                      UxEspCppLibrary::FreeRtosQueueTask::minTargetQueueSize( sizeof( UcdStepperMotorQueueTypes::message ) ),
                                      "UcdStepperMotorQueue" )
{

}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

UcdStepperMotorQueue::~UcdStepperMotorQueue()
{
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

BaseType_t UcdStepperMotorQueue::sendCtrlUartCommand( const EkxProtocol::DriverId                  u8DriverId,
                                                      const uint32_t                               u32MsgCounter,
                                                      const uint8_t                                u8RepeatCounter,
                                                      EkxProtocol::Payload::MessagePayload * const pMessagePayload )
{
    UcdStepperMotorQueueTypes::message message;

    message.u32Type                                   = UcdStepperMotorQueueTypes::messageType::CtrlUartCommand;
    message.payload.ctrlUartCommand.m_u8DriverId      = u8DriverId;
    message.payload.ctrlUartCommand.m_u32MsgCounter   = u32MsgCounter;
    message.payload.ctrlUartCommand.m_u8RepeatCounter = u8RepeatCounter;
    message.payload.ctrlUartCommand.m_pMessagePayload = pMessagePayload;

    return send( &message );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

BaseType_t UcdStepperMotorQueue::sendAckNackReceived( const uint32_t u32MsgCounter,
                                                      const bool     bSuccess )
{
    UcdStepperMotorQueueTypes::message message;

    message.u32Type                                 = UcdStepperMotorQueueTypes::messageType::AckNackReceived;
    message.payload.ackNackReceived.m_u32MsgCounter = u32MsgCounter;
    message.payload.ackNackReceived.m_bSuccess      = bSuccess;

    return send( &message );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

BaseType_t UcdStepperMotorQueue::sendAlive( const bool bAlive )
{
    UcdStepperMotorQueueTypes::message message;

    message.u32Type                      = UcdStepperMotorQueueTypes::messageType::Alive;
    message.payload.aliveStatus.m_bAlive = bAlive;

    return send( &message );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/
