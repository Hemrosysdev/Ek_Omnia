/*
 * UcdWifiConnectorTask.h
 *
 *  Created on: 24.10.2019
 *      Author: gesser
 */

#ifndef WIFI_CONNECTOR_TASK_H
#define WIFI_CONNECTOR_TASK_H

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#include "UartControlledDevice.h"

#include <string>
#include <esp_log.h>
#include <esp_system.h>
#include <esp_wifi.h>
#include <lwip/err.h>
#include <lwip/sys.h>

#include "FreeRtosQueueTimer.h"
#include "EspLog.h"
#include "UcdWifiConnectorQueue.h"
#include "PluginTaskWifiConnectorTypes.h"

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class UcdWifiConnectorTask : public UartControlledDevice
{

public:

    UcdWifiConnectorTask( UxEspCppLibrary::EspApplication * pApplication );

    ~UcdWifiConnectorTask() override;

    bool startupStep( const FreeRtosQueueTask::startupStep_t nStartupStep );

    void processTaskMessage( void );

    UcdWifiConnectorQueue * wifiConnectorQueue( void );

    const UcdWifiConnectorQueue * wifiConnectorQueue( void ) const;

    bool queueCtrlUartCommand( const EkxProtocol::DriverId                  u8DriverId,
                               const uint32_t                               u32MsgCounter,
                               const uint8_t                                u8RepeatCounter,
                               EkxProtocol::Payload::MessagePayload * const pMessagePayload ) override;

    bool queueAckNackReceived( const uint32_t u32MsgCounter,
                               const bool     bSuccess ) override;

    bool queueAlive( const bool bAlive ) override;

private:

    UcdWifiConnectorTask() = delete;

    void processAlive( const bool bAlive ) override;

    void setWifiConfig( const bool                                                    bActive,
                        const UxEspCppLibrary::PluginTaskWifiConnectorTypes::WifiMode nWifiMode,
                        const std::string &                                           strApSsid,
                        const std::string &                                           strApPassword,
                        const std::string &                                           strApIp,
                        const std::string &                                           strStaSsid,
                        const std::string &                                           strStaPassword,
                        const bool                                                    bStaDhcp,
                        const std::string &                                           strStaStaticIp,
                        const std::string &                                           strStaStaticGateway,
                        const std::string &                                           strStaStaticNetmask,
                        const std::string &                                           strStaStaticDns );

    void processCtrlUartCommand( const EkxProtocol::DriverId                        u8DriverId,
                                 const uint32_t                                     u32MsgCounter,
                                 const uint8_t                                      u8RepeatCounter,
                                 const EkxProtocol::Payload::MessagePayload * const pMessagePayload );

    void reportDriverStatus( const bool bFirst ) override;

private:

    UxEspCppLibrary::FreeRtosQueueTimer m_connectionTimeout;

    bool                                m_bIsAccesspoint { true };
    bool                                m_bActive { false };

};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#endif /* WIFI_CONNECTOR_TASK_H */
