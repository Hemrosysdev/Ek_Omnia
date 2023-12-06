/*
 * CtrlUartQueueTypes.h
 *
 *  Created on: 26.04.2023
 *      Author: gesser
 */

#ifndef CtrlUartQueueTypes_H
#define CtrlUartQueueTypes_H

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#include <stdint.h>

#include "FreeRtosQueue.h"
#include "EkxProtocol.h"

class UartControlledDevice;

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

namespace CtrlUartQueueTypes
{

enum class messageType : uint32_t
{
    CtrlUartReply = 0,
    RegisterDriver,
    TimeoutAlive,
    TimeoutMessageQueue,
    TimeoutStatusUpdate,
};

struct RegisterDriverPayload
{
    UartControlledDevice * m_pDevice;
};

struct CtrlUartMessagePayload
{
    EkxProtocol::DriverId                  m_u8DriverId;
    uint32_t                               m_u32MsgCounter;
    uint8_t                                m_u8RepeatCounter;
    EkxProtocol::Payload::MessagePayload * m_pMessagePayload;
};

union messagePayload
{
    CtrlUartMessagePayload ctrlUartReply;
    RegisterDriverPayload  registerDriverPayload;
};

struct message
{
    messageType    u32Type;     //!<  it has to make sure, that this attribute takes 32bit, as it is mapped to a general uint32 attribute.
    messagePayload payload;     //!< generic payload
};

};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#endif /* CtrlUartQueueTypes_H */
