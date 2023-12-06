/*!*************************************************************************************************************************************************************
 *
 * @file UcdHttpServerQueueTypes.h
 * @brief Header file of component unit UcdHttpServerQueue.
 *
 * @author Ultratronik GmbH
 *         Dornierstr. 9
 *         D-82205 Gilching
 *         http://www.ultratronik.de
 *
 * @author Gerd Esser, Research & Development
 *
 * @date 13.05.2023
 *
 * @copyright Copyright 2023 by Ultratronik GmbH.
 *            All rights reserved.
 *            None of this file or parts of it may be
 *            copied, redistributed or used in any other way
 *            without written approval of Ultratronik GmbH.
 *
 **************************************************************************************************************************************************************/

#ifndef UcdHttpServerQueueTypes_H
#define UcdHttpServerQueueTypes_H

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#include <cstdint>

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

namespace UcdHttpServerQueueTypes
{

enum class messageType : uint32_t
{
    Start = 0U,
    Stop,
    CtrlUartCommand,
    AckNackReceived,
    TimeoutListClean,
};

struct CtrlUartCommand
{
    EkxProtocol::DriverId                  m_u8DriverId;
    uint32_t                               m_u32MsgCounter;
    uint8_t                                m_u8RepeatCounter;
    EkxProtocol::Payload::MessagePayload * m_pMessagePayload;
};

struct AckNackReceived
{
    uint32_t m_u32MsgCounter;
    bool     m_bSuccess;
};

union messagePayload
{
    CtrlUartCommand ctrlUartCommand;
    AckNackReceived ackNackReceived;
};

struct message
{
    messageType    u32Type;
    messagePayload payload;
};

const uint32_t u32MsgSize = sizeof( message );

}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#endif /* UcdHttpServerQueueTypes_H */
