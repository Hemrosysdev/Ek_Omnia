/*!************************************************************************************************************************************************************
 *
 * @file PluginTaskWifiConnector.h
 * 
 * This file was developed as part of UX Extended Eco System
 *
 * @brief Header file of class PluginTaskWifiConnector.
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

#ifndef PluginTaskWifiConnector_h
#define PluginTaskWifiConnector_h

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#include "PluginTask.h"
#include "FreeRtosQueueTimer.h"
#include "PluginTaskWifiConnectorTypes.h"
#include "FreeRtosQueueTask.h"

#include <string>
#include <esp_wifi_types.h>
#include <esp_interface.h>
#include <esp_netif.h>

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

namespace UxEspCppLibrary
{

class PluginTaskWifiConnectorDistributor;
class PluginTaskWifiConnectorQueue;

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class PluginTaskWifiConnector : public PluginTask
{   
public:

    PluginTaskWifiConnector( EspApplication * const pApplication,
                             const int nTaskPriority );

    ~PluginTaskWifiConnector() override;

    bool startupStep( const FreeRtosQueueTask::startupStep_t  nStartupStep ) override;

    void processTaskMessage( void ) override;

    PluginTaskWifiConnectorQueue * pluginTaskWifiConnectorQueue( void );
    const PluginTaskWifiConnectorQueue * pluginTaskWifiConnectorQueue( void ) const;

    PluginTaskWifiConnectorDistributor * pluginTaskWifiConnectorDistributor( void );
    const PluginTaskWifiConnectorDistributor * pluginTaskWifiConnectorDistributor( void ) const;

    static std::string makeMacStr( const uint8_t * pu8Mac );

    void updateActiveMac( void );

    void updateActiveIpv4( void );

private:

    PluginTaskWifiConnector() = delete;

    void processWifiConfig( const PluginTaskWifiConnectorTypes::WifiMode nWifiMode,
                            const std::string & strApSsid,
                            const std::string & strApPassword,
                            const std::string & strApIpv4,
                            const std::string & strStaSsid,
                            const std::string & strStaPassword,
                            const bool bStaDhcp,
                            const std::string & strStaStaticIpv4,
                            const std::string & strStaStaticGateway,
                            const std::string & strStaStaticNetmask,
                            const std::string & strStaStaticDns );

    void processHostname( const std::string & strHostname );

    void processWifiOn();
    void processWifiOff();
    void processSleepOn();
    void processSleepOff();
    void processScanAccessPoints();

    esp_err_t configApMode( void );
    esp_err_t configStaMode( void );

    esp_err_t setStaticIp( const std::string & strStaticIp,
                           const std::string & strNetmask,
                           const std::string & strGatewayIp,
                           const std::string & strDnsServer );
    esp_err_t startDhcpClient( void );

    void initialiseEventHandler( void );

    void startWifi( const bool bModeChanged,
                    const bool bApModeChanged,
                    const bool bStaModeChanged );

    void stopWifi( const UxEspCppLibrary::PluginTaskWifiConnectorTypes::WifiStatus nWifiStatus );

    esp_err_t setCountryCode( const int nChannelNum );

    void connectToStation( void );

    bool disconnectFromStation( void );

    void processScanResults( void );

    void startWifiScan( void );

    static void wifiEventHandler( void * event_handler_arg,
                                  esp_event_base_t event_base,
                                  int32_t event_id,
                                  void * event_data );

    void distributeWifiInfo( void );

private:

    uint8_t             m_u8ConfigMaxConnections { 10 };

    std::string         m_strConfigHostname;

    PluginTaskWifiConnectorTypes::WifiMode  m_nConfigWifiMode { PluginTaskWifiConnectorTypes::WifiMode::Ap };

    std::string         m_strConfigApSsid;

    std::string         m_strConfigApPassword;

    std::string         m_strConfigApIpv4;

    std::string         m_strConfigStaSsid;

    std::string         m_strConfigStaPassword;

    bool                m_bConfigStaDhcp { false };

    std::string         m_strConfigStaStaticIpv4;

    std::string         m_strConfigStaStaticGateway;

    std::string         m_strConfigStaStaticNetmask;

    std::string         m_strConfigStaStaticDns;

    FreeRtosQueueTimer  m_timerConnectionTimeout;

    bool                m_bSleepOn { false };

    bool                m_bWifiOn { false };

    bool                m_bWifiStarted { false };

    esp_netif_t *       m_pWifiAp { nullptr };

    esp_netif_t *       m_pWifiSta { nullptr };

    std::string         m_strApIpv4;

    std::string         m_strStaIpv4;

    std::string         m_strApMac;

    std::string         m_strStaMac;

    int                 m_nChannelNum { 0 };

};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

} // namespace UxEspCppLibrary

#endif /* PluginTaskWifiConnector_h */
