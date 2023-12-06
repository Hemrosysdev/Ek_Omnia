/*!*************************************************************************************************************************************************************
 *
 * @file UcdEepromQueueTypes.h
 * @brief Source file of component unit Eeprom-Task.
 *
 * This file was developed as part of SCD  holder of PluginTaskAt24c16.
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

#ifndef UcdEepromQueueTypes_h
#define UcdEepromQueueTypes_h

/***************************************************************************************************************************************************************
 *
 **************************************************************************************************************************************************************/

#include "PluginTaskAt24c16Types.h"

#include "EkxProtocol.h"

/***************************************************************************************************************************************************************
 *
 **************************************************************************************************************************************************************/

namespace UcdEepromQueueTypes
{
enum class messageType : uint32_t
{
    CtrlUartCommand,
    AckNackReceived,
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

/***************************************************************************************************************************************************************
 *
 **************************************************************************************************************************************************************/

#endif /* UcdEepromQueueTypes_h */
