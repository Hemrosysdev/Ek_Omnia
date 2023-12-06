/*!*****************************************************************************
*
* @file CtrlUartQueue.c
*
* @brief Implementation file of class CtrlUartQueue.
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

/*##*************************************************************************************************************************************************************
 *  Includes
 **************************************************************************************************************************************************************/

#include "CtrlUartQueue.h"
#include "CtrlUartQueueTypes.h"
#include "FreeRtosQueueTask.h"

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

namespace CtrlUart
{

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

CtrlUartQueue::CtrlUartQueue()
    : UxEspCppLibrary::FreeRtosQueue( c_nCtrlUartQueueLen,
                                      UxEspCppLibrary::FreeRtosQueueTask::minTargetQueueSize( sizeof( CtrlUartQueueTypes::message ) ),
                                      "CtrlUartQueue" )
{
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

CtrlUartQueue::~CtrlUartQueue()
{
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

BaseType_t CtrlUartQueue::sendMessagePayload( const EkxProtocol::DriverId                  u8DriverId,
                                              const uint32_t                               u32MsgCounter,
                                              const uint8_t                                u8RepeatCounter,
                                              EkxProtocol::Payload::MessagePayload * const pMessagePayload )
{
    CtrlUartQueueTypes::message message;

    message.u32Type = CtrlUartQueueTypes::messageType::CtrlUartReply;

    message.payload.ctrlUartReply.m_u8DriverId      = u8DriverId;
    message.payload.ctrlUartReply.m_u32MsgCounter   = u32MsgCounter;
    message.payload.ctrlUartReply.m_u8RepeatCounter = u8RepeatCounter;
    message.payload.ctrlUartReply.m_pMessagePayload = pMessagePayload;

    return send( &message );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

BaseType_t CtrlUartQueue::sendRegisterDriver( UartControlledDevice * const pDriver )
{
    CtrlUartQueueTypes::message message;

    message.u32Type = CtrlUartQueueTypes::messageType::RegisterDriver;

    message.payload.registerDriverPayload.m_pDevice = pDriver;

    return send( &message );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

} /* namespace CtrlUart */

