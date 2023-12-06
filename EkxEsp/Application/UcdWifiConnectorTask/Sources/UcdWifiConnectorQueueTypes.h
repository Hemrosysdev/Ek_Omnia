/*
 * UcdWifiConnectorQueue.h
 *
 *  Created on: 24.10.2019
 *      Author: gesser
 */

#ifndef UcdWifiConnectorQueueTypes_h
#define UcdWifiConnectorQueueTypes_h

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#include "PluginTaskWifiConnectorTypes.h"

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

namespace UcdWifiConnectorQueueTypes
{
enum class messageType : uint32_t
{
    Alive = 0,
    CtrlUartCommand,
    AckNackReceived,
    ConnectionTimeout
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

struct WifiConfig
{
    char szSsid[UxEspCppLibrary::PluginTaskWifiConnectorTypes::SSID_LENGTH + 1];
    char szPassword[UxEspCppLibrary::PluginTaskWifiConnectorTypes::PW_LENGTH + 1];
    bool bIsAccesspoint;
    bool bOnOff;
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
    WifiConfig      wifiConfig;
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

#endif /* UcdWifiConnectorQueueTypes_h */
