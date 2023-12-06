/*!************************************************************************************************************************************************************
 *
 * @file PluginTaskWifiConnectorQueueTypes.h
 *
 * This file was developed as part of UX Extended Eco System
 *
 * @brief Header file of class PluginTaskWifiConnectorQueueTypes.
 *
 * @author Ultratronik GmbH
 *         Dornierstr. 9
 *         D-82205 Gilching
 *         http://www.ultratronik.de
 *
 * @author written by Gerd Esser, Research & Development, gesser@ultratronik.de
 *
 * @date 17.03.2021
 *
 * @copyright Copyright 2021 by Ultratronik GmbH.
 *
 * This file and/or parts of it are subject to Ultratronik´s software license terms (SoLiT, Version 1.16.2).
 * With the use of this software you accept the SoLiT. Without written approval of Ultratronik GmbH this
 * software may not be copied, redistributed or used in any other way than stated in the conditions of the
 * SoLiT.
 *
 *************************************************************************************************************************************************************/

#ifndef PluginTaskWifiConnectorQueueTypes_h
#define PluginTaskWifiConnectorQueueTypes_h

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#include <stdint.h>
#include "PluginTaskWifiConnectorTypes.h"

#include <esp_interface.h>
#include <esp_wifi_types.h>

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

namespace UxEspCppLibrary
{

namespace PluginTaskWifiConnectorQueueTypes
{

enum class messageType : uint32_t
{
    WifiConfig = 0U,
    WifiOn,
    WifiOff,
    SleepOn,
    SleepOff,
    ScanAccessPoints,
    ConnectionTimeout,
    Hostname,
    WifiEventApStart,
    WifiEventApStop,
    WifiEventApStaConnected,
    WifiEventApStaDisconnected,
    WifiEventStaStart,
    WifiEventStaStop,
    WifiEventStaConnected,
    WifiEventStaDisconnected,
    WifiEventScanDone,
    WifiEventStaGotIp
};

struct wifiConfigPayload_t
{
    UxEspCppLibrary::PluginTaskWifiConnectorTypes::WifiMode nWifiMode;
    char                                                    pszApSsid[PluginTaskWifiConnectorTypes::SSID_BUF_LENGTH];
    char                                                    pszApPassword[PluginTaskWifiConnectorTypes::PW_BUF_LENGTH];
    char                                                    pszApIpv4[PluginTaskWifiConnectorTypes::IP4_BUF_LENGTH];
    char                                                    pszStaSsid[PluginTaskWifiConnectorTypes::SSID_BUF_LENGTH];
    char                                                    pszStaPassword[PluginTaskWifiConnectorTypes::PW_BUF_LENGTH];
    bool                                                    bStaDhcp;
    char                                                    pszStaStaticIpv4[PluginTaskWifiConnectorTypes::IP4_BUF_LENGTH];
    char                                                    pszStaStaticGateway[PluginTaskWifiConnectorTypes::IP4_BUF_LENGTH];
    char                                                    pszStaStaticNetmask[PluginTaskWifiConnectorTypes::IP4_BUF_LENGTH];
    char                                                    pszStaStaticDns[PluginTaskWifiConnectorTypes::IP4_BUF_LENGTH];
};

struct hostnamePayload_t
{
    char pszHostname[PluginTaskWifiConnectorTypes::HOSTNAME_BUF_LENGTH];
};

union messagePayload
{
    wifiConfigPayload_t wifiConfigPayload;
    hostnamePayload_t   hostnamePayload;
};

struct message
{
    messageType    u32Type;
    messagePayload payload;
};
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

namespace PluginTaskWifiConnectorDistributorQueueTypes
{
const uint32_t c_u32PluginTaskWifiConnectorOffset = 0x12400000UL;
const uint32_t c_u32MaxMacLen                     = 20;

enum class messageType_t : uint32_t
{
    Status = c_u32PluginTaskWifiConnectorOffset,
    WifiEventApStart,
    WifiEventApStop,
    WifiEventApStaConnected,
    WifiEventApStaDisconnected,
    WifiEventStaStart,
    WifiEventStaStop,
    WifiEventStaConnected,
    WifiEventStaDisconnected,
    WifiEventStaGotIp,
    WifiEventScanDone,
    WifiInfo,
    WifiScanResults,
};

struct statusPayload_t
{
    PluginTaskWifiConnectorTypes::WifiStatus m_nStatus;
};

struct apStaConnectedPayload_t
{
    char m_szMac[c_u32MaxMacLen];
};

struct apStaDisconnectedPayload_t
{
    char m_szMac[c_u32MaxMacLen];
};

struct staGotIpPayload_t
{
    char m_szIp[PluginTaskWifiConnectorTypes::IP4_BUF_LENGTH];
};

struct infoPayload_t
{
    char m_szApIpv4[PluginTaskWifiConnectorTypes::IP4_BUF_LENGTH];
    char m_szApMac[PluginTaskWifiConnectorTypes::MAC_BUF_LENGTH];
    char m_szStaIpv4[PluginTaskWifiConnectorTypes::IP4_BUF_LENGTH];
    char m_szStaMac[PluginTaskWifiConnectorTypes::MAC_BUF_LENGTH];
};

struct scanResults_t
{
    bool               m_bSuccess;
    uint16_t           m_u16Number;
    wifi_ap_record_t * m_pApRecordList;
};

union messagePayload_t
{
    statusPayload_t            statusPayload;
    apStaConnectedPayload_t    apStaConnectedPayload;
    apStaDisconnectedPayload_t apStaDisconnectedPayload;
    staGotIpPayload_t          staGotIpPayload;
    infoPayload_t              infoPayload;
    scanResults_t              scanResultsPayload;
};

struct message_t
{
    messageType_t    u32Type;
    messagePayload_t payload;
};

}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

} // namespace UxEspCppLibrary

#endif /* PluginTaskWifiConnectorQueueTypes_h */
