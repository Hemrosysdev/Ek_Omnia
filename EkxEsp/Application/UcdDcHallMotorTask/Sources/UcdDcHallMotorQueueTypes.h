/*!*************************************************************************************************************************************************************
 *
 * @file UcdDcHallMotorQueueTypes.h
 * @brief Header file of component unit SW Update.
 *
 * @author Ultratronik GmbH
 *         Dornierstr. 9
 *         D-82205 Gilching
 *         http://www.ultratronik.de
 *
 * @author Gerd Esser, Research & Development
 *
 * @date 06.11.2021
 *
 * @copyright Copyright 2021 by Ultratronik GmbH.
 *            All rights reserved.
 *            None of this file or parts of it may be
 *            copied, redistributed or used in any other way
 *            without written approval of Ultratronik GmbH.
 *
 **************************************************************************************************************************************************************/

#ifndef UcdDcHallMotorQueueTypes_H
#define UcdDcHallMotorQueueTypes_H

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#include <cstdint>

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

namespace UcdDcHallMotorQueueTypes
{
enum class messageType : uint32_t
{
    Alive,
    CtrlUartCommand,
    AckNackReceived,
    IsrEncoderA,
    IsrFaultN,
    TimeoutReportOvershootSteps,
    TimeoutMotorTest
};

struct CtrlUartCommand
{
    EkxProtocol::DriverId                  m_u8DriverId;
    uint32_t                               m_u32MsgCounter;
    uint8_t                                m_u8RepeatCounter;
    EkxProtocol::Payload::MessagePayload * m_pMessagePayload;
};

struct AliveStatus
{
    bool m_bAlive;
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
    AliveStatus     aliveStatus;
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

#endif /* UcdDcHallMotorQueueTypes_H */
