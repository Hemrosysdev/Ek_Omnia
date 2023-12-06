/*!*************************************************************************************************************************************************************
 *
 * @file UcdDiscreteInQueueTypes.h
 * @brief Source file of component unit DiscreteInTask.
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

#ifndef UcdDiscreteInQueueTypes_h
#define UcdDiscreteInQueueTypes_h

/***************************************************************************************************************************************************************
 *      Includes
 **************************************************************************************************************************************************************/

#include <cstdint>
#include "EkxProtocol.h"

/***************************************************************************************************************************************************************
 *      Declarations
 **************************************************************************************************************************************************************/

namespace UcdDiscreteInQueueTypes
{
enum class messageType : uint32_t
{
    CtrlUartCommand = 0,
    AckNackReceived,
    DebounceTimeout,
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

#endif /* UcdDiscreteInQueueTypes_h */
