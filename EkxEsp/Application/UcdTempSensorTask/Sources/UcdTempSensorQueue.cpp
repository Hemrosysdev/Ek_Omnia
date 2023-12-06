/*!*************************************************************************************************************************************************************
 *
 * @file UcdTempSensorQueue.cpp
 * @brief Source file of component unit <title>.
 *
 * This file was developed as part of <component>.
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

/*!**************************************************************************************************************************************************************
 *
 **************************************************************************************************************************************************************/

#include "UcdTempSensorQueue.h"

#include "UcdTempSensorQueueTypes.h"
#include "FreeRtosQueueTask.h"

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

UcdTempSensorQueue::UcdTempSensorQueue()
    : UxEspCppLibrary::FreeRtosQueue( 10,
                                      UxEspCppLibrary::FreeRtosQueueTask::minTargetQueueSize( sizeof( UcdTempSensorQueueTypes::message ) ),
                                      "UcdTempSensorQueue" )
{
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

UcdTempSensorQueue::~UcdTempSensorQueue()
{
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

BaseType_t UcdTempSensorQueue::sendCtrlUartCommand( const EkxProtocol::DriverId                  u8DriverId,
                                                    const uint32_t                               u32MsgCounter,
                                                    const uint8_t                                u8RepeatCounter,
                                                    EkxProtocol::Payload::MessagePayload * const pMessagePayload )
{
    UcdTempSensorQueueTypes::message message;

    message.u32Type                                   = UcdTempSensorQueueTypes::messageType::CtrlUartCommand;
    message.payload.ctrlUartCommand.m_u8DriverId      = u8DriverId;
    message.payload.ctrlUartCommand.m_u32MsgCounter   = u32MsgCounter;
    message.payload.ctrlUartCommand.m_u8RepeatCounter = u8RepeatCounter;
    message.payload.ctrlUartCommand.m_pMessagePayload = pMessagePayload;

    return send( &message );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

BaseType_t UcdTempSensorQueue::sendAckNackReceived( const uint32_t u32MsgCounter,
                                                    const bool     bSuccess )
{
    UcdTempSensorQueueTypes::message message;

    message.u32Type                                 = UcdTempSensorQueueTypes::messageType::AckNackReceived;
    message.payload.ackNackReceived.m_u32MsgCounter = u32MsgCounter;
    message.payload.ackNackReceived.m_bSuccess      = bSuccess;

    return send( &message );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/
