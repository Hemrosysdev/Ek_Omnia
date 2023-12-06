/*
 * UcdWifiConnectorQueue.cpp
 *
 *  Created on: 24.10.2019
 *      Author: gesser
 */

#include "UcdWifiConnectorQueue.h"

#include <string>

#include "UcdWifiConnectorQueueTypes.h"
#include "FreeRtosQueueTask.h"
#include "EkxProtocol.h"

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

UcdWifiConnectorQueue::UcdWifiConnectorQueue()
    : UxEspCppLibrary::FreeRtosQueue( 10,
                                      UxEspCppLibrary::FreeRtosQueueTask::minTargetQueueSize( sizeof( UcdWifiConnectorQueueTypes::message ) ),
                                      "UcdWifiConnectorQueue" )
{
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

UcdWifiConnectorQueue::~UcdWifiConnectorQueue()
{
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

BaseType_t UcdWifiConnectorQueue::sendCtrlUartCommand( const EkxProtocol::DriverId                  u8DriverId,
                                                       const uint32_t                               u32MsgCounter,
                                                       const uint8_t                                u8RepeatCounter,
                                                       EkxProtocol::Payload::MessagePayload * const pMessagePayload )
{
    UcdWifiConnectorQueueTypes::message message;

    message.u32Type                                   = UcdWifiConnectorQueueTypes::messageType::CtrlUartCommand;
    message.payload.ctrlUartCommand.m_u8DriverId      = u8DriverId;
    message.payload.ctrlUartCommand.m_u32MsgCounter   = u32MsgCounter;
    message.payload.ctrlUartCommand.m_u8RepeatCounter = u8RepeatCounter;
    message.payload.ctrlUartCommand.m_pMessagePayload = pMessagePayload;

    return send( &message );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

BaseType_t UcdWifiConnectorQueue::sendAckNackReceived( const uint32_t u32MsgCounter,
                                                       const bool     bSuccess )
{
    UcdWifiConnectorQueueTypes::message message;

    message.u32Type                                 = UcdWifiConnectorQueueTypes::messageType::AckNackReceived;
    message.payload.ackNackReceived.m_u32MsgCounter = u32MsgCounter;
    message.payload.ackNackReceived.m_bSuccess      = bSuccess;

    return send( &message );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

BaseType_t UcdWifiConnectorQueue::sendAlive( const bool bAlive )
{
    UcdWifiConnectorQueueTypes::message message;

    message.u32Type                      = UcdWifiConnectorQueueTypes::messageType::Alive;
    message.payload.aliveStatus.m_bAlive = bAlive;

    return send( &message );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

