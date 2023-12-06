/*!*************************************************************************************************************************************************************
 *
 * @file SocketQueue.h
 * @brief Source file of component unit Socket.
 *
 * This file was developed as part of SocketTask.
 *
 * @author Ultratronik GmbH
 *         Dornierstr. 9
 *         D-82205 Gilching
 *         http://www.ultratronik.de
 * @author Srdjan Nikolic, Entwicklung
 *
 * @copyright Copyright 2020 by Ultratronik GmbH.
 *            All rights reserved.
 *            None of this file or parts of it may be
 *            copied, redistributed or used in any other way
 *            without written approval of Ultratronik GmbH.
 *
 **************************************************************************************************************************************************************/

/*!*************************************************************************************************************************************************************
 *
 **************************************************************************************************************************************************************/

#include "UcdMcuUartQueue.h"

#include "UcdUartQueueTypes.h"
#include "FreeRtosQueueTask.h"

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

UcdMcuUartQueue::UcdMcuUartQueue()
    : UxEspCppLibrary::FreeRtosQueue( 10,
                                      UxEspCppLibrary::FreeRtosQueueTask::minTargetQueueSize( sizeof( UcdMcuUartQueueTypes::message ) ),
                                      "UARTQueue" )
{

}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

UcdMcuUartQueue::~UcdMcuUartQueue()
{
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

BaseType_t UcdMcuUartQueue::sendCtrlUartCommand( const EkxProtocol::DriverId                  u8DriverId,
                                                 const uint32_t                               u32MsgCounter,
                                                 const uint8_t                                u8RepeatCounter,
                                                 EkxProtocol::Payload::MessagePayload * const pMessagePayload )
{
    UcdMcuUartQueueTypes::message message;

    message.u32Type                                   = UcdMcuUartQueueTypes::messageType::CtrlUartCommand;
    message.payload.ctrlUartCommand.m_u8DriverId      = u8DriverId;
    message.payload.ctrlUartCommand.m_u32MsgCounter   = u32MsgCounter;
    message.payload.ctrlUartCommand.m_u8RepeatCounter = u8RepeatCounter;
    message.payload.ctrlUartCommand.m_pMessagePayload = pMessagePayload;

    return send( &message );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

BaseType_t UcdMcuUartQueue::sendAckNackReceived( const uint32_t u32MsgCounter,
                                                 const bool     bSuccess )
{
    UcdMcuUartQueueTypes::message message;

    message.u32Type                                 = UcdMcuUartQueueTypes::messageType::AckNackReceived;
    message.payload.ackNackReceived.m_u32MsgCounter = u32MsgCounter;
    message.payload.ackNackReceived.m_bSuccess      = bSuccess;

    return send( &message );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/
