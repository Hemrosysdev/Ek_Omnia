/*!*************************************************************************************************************************************************************
 *
 * @file UcdEepromQueue.cpp
 * @brief Source file of component unit UcdPwmOut.
 *
 * This file was developed as part of Spotlight.
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

/*!*************************************************************************************************************************************************************
 *
 **************************************************************************************************************************************************************/

#include "UcdEepromQueue.h"

#include <cstring>

#include "UcdEepromQueueTypes.h"
#include "FreeRtosQueueTask.h"

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

UcdEepromQueue::UcdEepromQueue( const UBaseType_t uRequiredConsumerQueueItemSize )
    : UxEspCppLibrary::FreeRtosQueue( 10,
                                      UxEspCppLibrary::FreeRtosQueueTask::minTargetQueueSize( std::max( uRequiredConsumerQueueItemSize, sizeof( UcdEepromQueueTypes::message ) ) ),
                                      "UcdEepromQueue" )
{
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

UcdEepromQueue::~UcdEepromQueue()
{
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

BaseType_t UcdEepromQueue::sendCtrlUartCommand( const EkxProtocol::DriverId                  u8DriverId,
                                                const uint32_t                               u32MsgCounter,
                                                const uint8_t                                u8RepeatCounter,
                                                EkxProtocol::Payload::MessagePayload * const pMessagePayload )
{
    UcdEepromQueueTypes::message message;

    message.u32Type                                   = UcdEepromQueueTypes::messageType::CtrlUartCommand;
    message.payload.ctrlUartCommand.m_u8DriverId      = u8DriverId;
    message.payload.ctrlUartCommand.m_u32MsgCounter   = u32MsgCounter;
    message.payload.ctrlUartCommand.m_u8RepeatCounter = u8RepeatCounter;
    message.payload.ctrlUartCommand.m_pMessagePayload = pMessagePayload;

    return send( &message );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

BaseType_t UcdEepromQueue::sendAckNackReceived( const uint32_t u32MsgCounter,
                                                const bool     bSuccess )
{
    UcdEepromQueueTypes::message message;

    message.u32Type                                 = UcdEepromQueueTypes::messageType::AckNackReceived;
    message.payload.ackNackReceived.m_u32MsgCounter = u32MsgCounter;
    message.payload.ackNackReceived.m_bSuccess      = bSuccess;

    return send( &message );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/
