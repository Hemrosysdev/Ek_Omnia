/*!*************************************************************************************************************************************************************
 *
 * @file UcdDiscreteInQueue.cpp
 * @brief Source file of component unit DiscreteIn.
 *
 * This file was developed as part of generic DiscreteIn interface
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


/***************************************************************************************************************************************************************
 *      Includes
 **************************************************************************************************************************************************************/

#include "UcdDiscreteInQueue.h"
#include "UcdDiscreteInQueueTypes.h"

#include "FreeRtosQueueTask.h"
#include "EkxProtocol.h"

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

UcdDiscreteInQueue::UcdDiscreteInQueue()
    : UxEspCppLibrary::FreeRtosQueue( 10,
                                      UxEspCppLibrary::FreeRtosQueueTask::minTargetQueueSize( sizeof( UcdDiscreteInQueueTypes::message ) ),
                                      "UcdDiscreteInQueue" )
{

}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

UcdDiscreteInQueue::~UcdDiscreteInQueue()
{
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

BaseType_t UcdDiscreteInQueue::sendCtrlUartCommand( const EkxProtocol::DriverId                  u8DriverId,
                                                    const uint32_t                               u32MsgCounter,
                                                    const uint8_t                                u8RepeatCounter,
                                                    EkxProtocol::Payload::MessagePayload * const pMessagePayload )
{
    UcdDiscreteInQueueTypes::message message;

    message.u32Type                                   = UcdDiscreteInQueueTypes::messageType::CtrlUartCommand;
    message.payload.ctrlUartCommand.m_u8DriverId      = u8DriverId;
    message.payload.ctrlUartCommand.m_u32MsgCounter   = u32MsgCounter;
    message.payload.ctrlUartCommand.m_u8RepeatCounter = u8RepeatCounter;
    message.payload.ctrlUartCommand.m_pMessagePayload = pMessagePayload;

    return send( &message );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

BaseType_t UcdDiscreteInQueue::sendAckNackReceived( const uint32_t u32MsgCounter,
                                                    const bool     bSuccess )
{
    UcdDiscreteInQueueTypes::message message;

    message.u32Type                                 = UcdDiscreteInQueueTypes::messageType::AckNackReceived;
    message.payload.ackNackReceived.m_u32MsgCounter = u32MsgCounter;
    message.payload.ackNackReceived.m_bSuccess      = bSuccess;

    return send( &message );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/
