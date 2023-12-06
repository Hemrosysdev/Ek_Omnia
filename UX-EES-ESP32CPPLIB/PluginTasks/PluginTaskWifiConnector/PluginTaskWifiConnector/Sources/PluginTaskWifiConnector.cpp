/*!************************************************************************************************************************************************************
 *
 * @file PluginTaskWifiConnector.cpp
 * 
 * This file was developed as part of UX Extended Eco System
 *
 * @brief Implementation file of class PluginTaskWifiConnector.
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

#include "PluginTaskWifiConnector.h"
#include "PluginTaskWifiConnectorQueue.h"
#include "PluginTaskWifiConnectorQueueTypes.h"
#include "PluginTaskWifiConnectorDistributor.h"
#include "UxLibCommon.h"

#include <esp_wifi_types.h>
#include <esp_wifi.h>
#include <cstring>
#include <lwip/inet.h>
#include <lwip/dns.h>
#include <esp_idf_version.h>

#define CONNECTION_TIMEOUT   30000000  // 30 secs

#define REDUCED_CHANNEL_NUM   11
#define MAX_CHANNEL_NUM       14

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

namespace UxEspCppLibrary
{

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

PluginTaskWifiConnector::PluginTaskWifiConnector( EspApplication * const pApplication,
                                                  const int nTaskPriority )
                                : PluginTask( pApplication,
                                              4096,
                                              nTaskPriority,
                                              "PluginTaskWifiConnector",
                                              new PluginTaskWifiConnectorQueue(),
                                              new PluginTaskWifiConnectorDistributor(),
                                              sizeof( PluginTaskWifiConnectorDistributorQueueTypes::message_t ) )
                                              , m_timerConnectionTimeout( taskQueue(),
                                                                          static_cast<uint32_t>( PluginTaskWifiConnectorQueueTypes::messageType::ConnectionTimeout ) )
                                                                          {
    logInfo( "PluginTaskWifiConnector constructor" );
                                                                          }

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

PluginTaskWifiConnector::~PluginTaskWifiConnector()
{
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void PluginTaskWifiConnector::processTaskMessage( void )
{
    const PluginTaskWifiConnectorQueueTypes::message * pMessage = reinterpret_cast<const PluginTaskWifiConnectorQueueTypes::message *>( receiveMsg() );

    if ( pMessage )
    {
        switch ( pMessage->u32Type )
        {
            case PluginTaskWifiConnectorQueueTypes::messageType::WifiConfig:
            {
                processWifiConfig( pMessage->payload.wifiConfigPayload.nWifiMode,
                                   pMessage->payload.wifiConfigPayload.pszApSsid,
                                   pMessage->payload.wifiConfigPayload.pszApPassword,
                                   pMessage->payload.wifiConfigPayload.pszApIpv4,
                                   pMessage->payload.wifiConfigPayload.pszStaSsid,
                                   pMessage->payload.wifiConfigPayload.pszStaPassword,
                                   pMessage->payload.wifiConfigPayload.bStaDhcp,
                                   pMessage->payload.wifiConfigPayload.pszStaStaticIpv4,
                                   pMessage->payload.wifiConfigPayload.pszStaStaticGateway,
                                   pMessage->payload.wifiConfigPayload.pszStaStaticNetmask,
                                   pMessage->payload.wifiConfigPayload.pszStaStaticDns );
            }
            break;

            case PluginTaskWifiConnectorQueueTypes::messageType::Hostname:
            {
                processHostname( pMessage->payload.hostnamePayload.pszHostname );
            }
            break;

            case PluginTaskWifiConnectorQueueTypes::messageType::WifiOn:
            {
                processWifiOn();
            }
            break;

            case PluginTaskWifiConnectorQueueTypes::messageType::WifiOff:
            {
                processWifiOff();
            }
            break;

            case PluginTaskWifiConnectorQueueTypes::messageType::SleepOn:
            {
                processSleepOn();
            }
            break;

            case PluginTaskWifiConnectorQueueTypes::messageType::SleepOff:
            {
                processSleepOff();
            }
            break;

            case PluginTaskWifiConnectorQueueTypes::messageType::ScanAccessPoints:
            {
                processScanAccessPoints();
            }
            break;

            case PluginTaskWifiConnectorQueueTypes::messageType::ConnectionTimeout:
            {
                logWarning( "processTaskMessage: connection timeout of 60 seconds, proceed trying to connect" );
                stopWifi( PluginTaskWifiConnectorTypes::WifiStatus::Failed );
            }
            break;

            default:
            {
                vlogError( "processTaskMessage: receive illegal message type %d",
                           pMessage->u32Type );
            }
            break;
        }
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

bool PluginTaskWifiConnector::startupStep( const FreeRtosQueueTask::startupStep_t nStartupStep )
{
    bool bContinueStartup = true;

    switch ( nStartupStep )
    {
        case FreeRtosQueueTask::startupStep_t::startupStep1:
        {
            logInfo( "startupStep1" );

            initialiseEventHandler();
        }
        break;

        default:
        {
            // do nothing
        }
        break;
    }

    return bContinueStartup;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

PluginTaskWifiConnectorQueue * PluginTaskWifiConnector::pluginTaskWifiConnectorQueue( void )
{
    return dynamic_cast<PluginTaskWifiConnectorQueue *>( taskQueue() );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

const PluginTaskWifiConnectorQueue * PluginTaskWifiConnector::pluginTaskWifiConnectorQueue( void ) const
{
    return dynamic_cast<const PluginTaskWifiConnectorQueue *>( taskQueue() );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

PluginTaskWifiConnectorDistributor * PluginTaskWifiConnector::pluginTaskWifiConnectorDistributor( void )
{
    return dynamic_cast<PluginTaskWifiConnectorDistributor*>( distributor() );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

const PluginTaskWifiConnectorDistributor * PluginTaskWifiConnector::pluginTaskWifiConnectorDistributor( void ) const
{
    return dynamic_cast<const PluginTaskWifiConnectorDistributor*>( distributor() );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void PluginTaskWifiConnector::initialiseEventHandler( void )
{
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init( &cfg ) );

    ESP_ERROR_CHECK( esp_wifi_set_storage( WIFI_STORAGE_RAM ) );
    ESP_ERROR_CHECK( esp_wifi_set_ps( WIFI_PS_NONE ) );

    m_pWifiAp  = esp_netif_create_default_wifi_ap();
    m_pWifiSta = esp_netif_create_default_wifi_sta();

    ESP_ERROR_CHECK( esp_event_handler_register( WIFI_EVENT, ESP_EVENT_ANY_ID, wifiEventHandler, this ) );
    ESP_ERROR_CHECK( esp_event_handler_register( IP_EVENT, IP_EVENT_STA_GOT_IP, wifiEventHandler, this ) );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

esp_err_t PluginTaskWifiConnector::setCountryCode( const int nChannelNum )
{
    esp_err_t nEspError = ESP_OK;

    // United States Indoor use

    if ( m_nChannelNum != nChannelNum )
    {
        m_nChannelNum = nChannelNum;

        // test case: select fix channel 13 on Fritz.Box and try to connect in STA mode
#if ESP_IDF_VERSION > ESP_IDF_VERSION_VAL( 4, 2, 0 )
        nEspError = esp_wifi_set_country_code( "US", true );
#else
        wifi_country_t country_config;
        country_config.cc[0]  = 'U';
        country_config.cc[1]  = 'S';
        country_config.cc[2]  = 'I';
        country_config.schan  = 1;
        country_config.nchan  = m_nChannelNum;
        country_config.policy = WIFI_COUNTRY_POLICY_AUTO;

        nEspError = esp_wifi_set_country( &country_config );
#endif

        if ( nEspError == ESP_OK )
        {
            vlogInfo( "setCountryCode: esp_wifi_set_country_code passed" );
        }
        else
        {
            vlogError( "setCountryCode: esp_wifi_set_country_code failed %d (%s)", nEspError, esp_err_to_name( nEspError ) );
        }
    }

    return nEspError;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void PluginTaskWifiConnector::processWifiOn( void )
{
    if ( m_bWifiOn != true )
    {
        m_bWifiOn = true;
        startWifi( true, true, true );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void PluginTaskWifiConnector::processWifiOff( void )
{
    if ( m_bWifiOn != false )
    {
        m_bWifiOn = false;
        stopWifi( PluginTaskWifiConnectorTypes::WifiStatus::Off );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void PluginTaskWifiConnector::processSleepOn( void )
{
    vlogInfo( "processSleepOn" );

    if ( m_bSleepOn != true )
    {
        m_bSleepOn = true;
        stopWifi( PluginTaskWifiConnectorTypes::WifiStatus::Off );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void PluginTaskWifiConnector::processSleepOff( void )
{
    vlogInfo( "processSleepOff" );

    if ( m_bSleepOn != false )
    {
        m_bSleepOn = false;
        startWifi( true, true, true );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void PluginTaskWifiConnector::processScanAccessPoints()
{
    startWifiScan();
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void PluginTaskWifiConnector::processWifiConfig( const PluginTaskWifiConnectorTypes::WifiMode nWifiMode,
                                                 const std::string & strApSsid,
                                                 const std::string & strApPassword,
                                                 const std::string & strApIpv4,
                                                 const std::string & strStaSsid,
                                                 const std::string & strStaPassword,
                                                 const bool bStaDhcp,
                                                 const std::string & strStaStaticIpv4,
                                                 const std::string & strStaStaticGateway,
                                                 const std::string & strStaStaticNetmask,
                                                 const std::string & strStaStaticDns )
{
    logInfo( "processWifiConfig()" );

    m_u8ConfigMaxConnections    = 10;

    bool bModeChanged = false;
    bool bApModeChanged = false;
    bool bStaModeChanged = false;

    if ( m_nConfigWifiMode != nWifiMode )
    {
        m_nConfigWifiMode = nWifiMode;
        bModeChanged = true;
    }
    if ( m_strConfigApSsid != strApSsid )
    {
        m_strConfigApSsid = strApSsid;
        bApModeChanged = true;
    }
    if ( m_strConfigApPassword != strApPassword )
    {
        m_strConfigApPassword = strApPassword;
        bApModeChanged = true;
    }
    if ( m_strConfigApIpv4 != strApIpv4 )
    {
        m_strConfigApIpv4 = strApIpv4;
        bApModeChanged = true;
    }
    if ( m_strConfigStaSsid != strStaSsid )
    {
        m_strConfigStaSsid = strStaSsid;
        bStaModeChanged = true;
    }
    if ( m_strConfigStaPassword != strStaPassword )
    {
        m_strConfigStaPassword = strStaPassword;
        bStaModeChanged = true;
    }
    if ( m_bConfigStaDhcp != bStaDhcp )
    {
        m_bConfigStaDhcp = bStaDhcp;
        bStaModeChanged = true;
    }
    if ( m_strConfigStaStaticIpv4 != strStaStaticIpv4 )
    {
        m_strConfigStaStaticIpv4 = strStaStaticIpv4;
        bStaModeChanged = true;
    }
    if ( m_strConfigStaStaticGateway != strStaStaticGateway )
    {
        m_strConfigStaStaticGateway = strStaStaticGateway;
        bStaModeChanged = true;
    }
    if ( m_strConfigStaStaticNetmask != strStaStaticNetmask )
    {
        m_strConfigStaStaticNetmask = strStaStaticNetmask;
        bStaModeChanged = true;
    }
    if ( m_strConfigStaStaticDns != strStaStaticDns )
    {
        m_strConfigStaStaticDns = strStaStaticDns;
        bStaModeChanged = true;
    }

    startWifi( bModeChanged, bApModeChanged, bStaModeChanged );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void PluginTaskWifiConnector::startWifi( const bool bModeChanged,
                                         const bool bApModeChanged,
                                         const bool bStaModeChanged )
{
    logInfo( "startWifi()" );

    esp_err_t nEspError = ESP_OK;

    if ( bModeChanged )
    {
        wifi_mode_t nMode = WIFI_MODE_NULL;
        switch ( m_nConfigWifiMode )
        {
            case UxEspCppLibrary::PluginTaskWifiConnectorTypes::WifiMode::Ap:
            {
                nMode = WIFI_MODE_AP;
                ESP_ERROR_CHECK( setCountryCode( REDUCED_CHANNEL_NUM ) );
            }
            break;

            case UxEspCppLibrary::PluginTaskWifiConnectorTypes::WifiMode::Sta:
            {
                nMode = WIFI_MODE_STA;
                ESP_ERROR_CHECK( setCountryCode( MAX_CHANNEL_NUM ) );
            }
            break;

            case UxEspCppLibrary::PluginTaskWifiConnectorTypes::WifiMode::Both:
            {
                nMode = WIFI_MODE_APSTA;
                ESP_ERROR_CHECK( setCountryCode( MAX_CHANNEL_NUM ) );
            }
            break;
        }

        nEspError = esp_wifi_set_mode( nMode );

        if ( nEspError != ESP_OK )
        {
            vlogError( "startWifi: esp_wifi_set_mode failed (%s)", esp_err_to_name( nEspError ) );
        }
    }

    if ( ( nEspError == ESP_OK )
                    && ( ( m_nConfigWifiMode == UxEspCppLibrary::PluginTaskWifiConnectorTypes::WifiMode::Ap )
                                    || ( m_nConfigWifiMode == UxEspCppLibrary::PluginTaskWifiConnectorTypes::WifiMode::Both ) )
    /* && bApModeChanged */)
    {
        nEspError = configApMode();
    }

    if ( ( nEspError == ESP_OK )
                    && ( ( m_nConfigWifiMode == UxEspCppLibrary::PluginTaskWifiConnectorTypes::WifiMode::Sta )
                                    || ( m_nConfigWifiMode == UxEspCppLibrary::PluginTaskWifiConnectorTypes::WifiMode::Both ) )
    /*&& bStaModeChanged*/ )
    {
        nEspError = configStaMode();
    }

    if ( ( nEspError == ESP_OK ) && ( bApModeChanged || bStaModeChanged ) )
    {
        processHostname( m_strConfigHostname );
    }

    if ( m_bWifiOn && !m_bSleepOn )
    {
        if ( /*!m_bWifiStarted && */( nEspError == ESP_OK ) && ( bModeChanged || bApModeChanged || bStaModeChanged ) )
        {
            m_timerConnectionTimeout.startOnce( CONNECTION_TIMEOUT );
            pluginTaskWifiConnectorDistributor()->sendWifiStatus( PluginTaskWifiConnectorTypes::WifiStatus::Connecting );

            vlogInfo( "startWifi: esp_wifi_start" );
            nEspError = esp_wifi_start();

            if ( nEspError == ESP_OK )
            {
                m_bWifiStarted = true;
            }
            else
            {
                vlogError( "startWifi: esp_wifi_start failed (%s)", esp_err_to_name( nEspError ) );
            }
        }

        if ( nEspError == ESP_OK )
        {
            vlogInfo( "startWifi: wifi successfully started" );
        }
    }
    else
    {
        nEspError = ESP_OK;    // we stop WiFi - no need to signal any error
        stopWifi( PluginTaskWifiConnectorTypes::WifiStatus::Off );
    }

    if ( nEspError != ESP_OK )
    {
        vlogError( "startWifi: stop wifi after previous error" );
        stopWifi( PluginTaskWifiConnectorTypes::WifiStatus::Failed );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void PluginTaskWifiConnector::stopWifi( const UxEspCppLibrary::PluginTaskWifiConnectorTypes::WifiStatus nWifiStatus )
{
    logInfo( "stopWifi()" );

    m_timerConnectionTimeout.stop();

    vTaskDelay( 1000 / portTICK_PERIOD_MS );

    wifi_mode_t wifiMode;

    esp_err_t nEspError = esp_wifi_get_mode( &wifiMode );

    if ( nEspError != ESP_OK )
    {
        vlogWarning( "stopWifi: esp_wifi_get_mode failed %d (%s)", nEspError, esp_err_to_name( nEspError ) );
    }
    else
    {
        if ( wifiMode == WIFI_MODE_STA || wifiMode == WIFI_MODE_APSTA )
        {
            disconnectFromStation();
        }

        nEspError = esp_wifi_stop();

        if ( nEspError !=  ESP_OK)
        {
            vlogWarning( "stopWifi: esp_wifi_stop failed %d (%s)", nEspError, esp_err_to_name( nEspError ) );
        }

        m_bWifiStarted = false;
    }

    pluginTaskWifiConnectorDistributor()->sendWifiStatus( nWifiStatus );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

esp_err_t PluginTaskWifiConnector::configApMode()
{
    vlogInfo( "configApMode()" );

    esp_err_t nEspError = ESP_OK;

    in_addr_t apAddr = inet_addr( m_strConfigApIpv4.c_str() );

    esp_netif_ip_info_t ipInfo;
    nEspError = esp_netif_get_ip_info( m_pWifiAp, &ipInfo );

    if ( nEspError != ESP_OK || ipInfo.ip.addr != apAddr )
    {
        vlogInfo( "configApMode: configure access point on IP %s", m_strConfigApIpv4.c_str() );

        esp_netif_dhcps_stop( m_pWifiAp );

        ipInfo.ip.addr = apAddr;
        ipInfo.gw.addr = apAddr;
        IP4_ADDR( &ipInfo.netmask, 255,255,255,0 );

        nEspError = esp_netif_set_ip_info( m_pWifiAp, &ipInfo );

        if ( nEspError != ESP_OK )
        {
            vlogError( "configApMode: esp_netif_set_ip_info failed (%s)", esp_err_to_name( nEspError ) );
        }

        if ( nEspError == ESP_OK )
        {
            nEspError = esp_netif_dhcps_start( m_pWifiAp );

            if ( nEspError != ESP_OK )
            {
                vlogError( "configApMode: esp_netif_dhcps_start failed (%s)", esp_err_to_name( nEspError ) );
            }
        }
    }

    if ( nEspError == ESP_OK )
    {
        wifi_config_t wifi_config;
        std::memset( &wifi_config, 0, sizeof( wifi_config ) );

        nEspError = esp_wifi_get_config( WIFI_IF_AP, &wifi_config );

        if ( ( m_strConfigApSsid == reinterpret_cast<char*>( wifi_config.ap.ssid ) )
                        && ( m_strConfigApPassword == reinterpret_cast<char*>( wifi_config.ap.password ) )  )
        {
            // do nothing, no change
            vlogInfo( "configApMode: skip new wifi AP config, no change" );
        }
        else
        {
            wifi_config.ap.ssid_len        = m_strConfigApSsid.size();
            wifi_config.ap.authmode        = WIFI_AUTH_WPA_WPA2_PSK;
            wifi_config.ap.max_connection  = m_u8ConfigMaxConnections;
            wifi_config.ap.channel         = 0;
            wifi_config.ap.beacon_interval = 100;

            UxLibCommon::saveStringCopy( reinterpret_cast<char*>( wifi_config.ap.ssid ),
                                         m_strConfigApSsid.c_str(),
                                         sizeof( wifi_config.ap.ssid ) );
            UxLibCommon::saveStringCopy( reinterpret_cast<char*>( wifi_config.ap.password ),
                                         m_strConfigApPassword.c_str(),
                                         sizeof( wifi_config.ap.password ) );

            if ( m_strConfigApPassword.empty() )
            {
                wifi_config.ap.authmode = WIFI_AUTH_OPEN;
            }

            vlogInfo( "configApMode: set new wifi AP config" );
            nEspError = esp_wifi_set_config( WIFI_IF_AP, &wifi_config );

            if ( nEspError != ESP_OK )
            {
                vlogError( "configApMode: esp_wifi_set_config failed, %s", esp_err_to_name( nEspError ) );
            }
        }
    }

    return nEspError;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

esp_err_t PluginTaskWifiConnector::configStaMode()
{
    vlogInfo( "configStaMode()" );

    wifi_config_t wifi_config;
    std::memset( &wifi_config, 0, sizeof( wifi_config ) );

    UxLibCommon::saveStringCopy( reinterpret_cast<char*>( wifi_config.sta.ssid ),
                                 m_strConfigStaSsid.c_str(),
                                 sizeof( wifi_config.sta.ssid ) );
    UxLibCommon::saveStringCopy( reinterpret_cast<char*>( wifi_config.sta.password ),
                                 m_strConfigStaPassword.c_str(),
                                 sizeof( wifi_config.sta.password ) );

    wifi_config.sta.scan_method        = WIFI_FAST_SCAN;
    wifi_config.sta.sort_method        = WIFI_CONNECT_AP_BY_SIGNAL;
    wifi_config.sta.threshold.rssi     = 0;
    wifi_config.sta.threshold.authmode = WIFI_AUTH_OPEN;
    wifi_config.sta.bssid_set          = false;

    if ( m_bConfigStaDhcp )
    {
        startDhcpClient();
    }
    else
    {
        setStaticIp( m_strConfigStaStaticIpv4,
                     m_strConfigStaStaticNetmask,
                     m_strConfigStaStaticGateway,
                     m_strConfigStaStaticDns );
    }

    esp_err_t nEspError = esp_wifi_set_config( WIFI_IF_STA, &wifi_config );

    if ( nEspError != ESP_OK )
    {
        vlogError( "configStaMode: esp_wifi_set_config failed, %s", esp_err_to_name( nEspError ) );
    }

    // connect to station is usually done upon STA_START event
    // but at the first time when switching the mode from AP to AP/STA, this event
    // is too early and in front of the STA config, so the connect fails and must be done here after config
    if ( nEspError == ESP_OK )
    {
        connectToStation();
    }

    return nEspError;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

esp_err_t PluginTaskWifiConnector::startDhcpClient( void )
{
    esp_err_t nEspError = ESP_FAIL;

    if ( m_pWifiSta != nullptr )
    {
        esp_netif_dhcp_status_t nStatus;

        if ( esp_netif_dhcpc_get_status( m_pWifiSta, &nStatus ) == ESP_OK )
        {
            if ( nStatus == ESP_NETIF_DHCP_STARTED && !m_bConfigStaDhcp )
            {
                vlogInfo( "startDhcpClient: stop client" );
                nEspError = esp_netif_dhcpc_stop( m_pWifiSta );
            }
            else if ( nStatus != ESP_NETIF_DHCP_STARTED && m_bConfigStaDhcp )
            {
                vlogInfo( "startDhcpClient: start client" );
                nEspError = esp_netif_dhcpc_start( m_pWifiSta );
            }
            else
            {
                nEspError = ESP_OK;
            }

            if ( nEspError != ESP_OK )
            {
                vlogError( "startDhcpClient: esp_netif_dhcpc_start/stop failed (%s)", esp_err_to_name( nEspError ) );
            }
        }
    }

    return nEspError;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

esp_err_t PluginTaskWifiConnector::setStaticIp( const std::string & strStaticIp,
                                                const std::string & strNetmask,
                                                const std::string & strGatewayIp,
                                                const std::string & strDnsServer )
{
    esp_err_t nEspError = esp_netif_dhcpc_stop( m_pWifiSta );

    if ( nEspError != ESP_OK )
    {
        vlogWarning( "setStaticIp: esp_netif_dhcpc_stop failed (%s)", esp_err_to_name( nEspError ) );
    }

    esp_netif_ip_info_t ipInfo;
    memset( &ipInfo, 0, sizeof( esp_netif_ip_info_t ) );

    ipInfo.ip.addr      = esp_ip4addr_aton( strStaticIp.c_str() );
    ipInfo.gw.addr      = esp_ip4addr_aton( strGatewayIp.c_str() );
    ipInfo.netmask.addr = esp_ip4addr_aton( strNetmask.c_str() );

    nEspError = esp_netif_set_ip_info( m_pWifiSta, &ipInfo );

    if ( nEspError != ESP_OK )
    {
        vlogError( "setStaticIp: esp_netif_set_ip_info failed (%s)", esp_err_to_name( nEspError ) );
    }

    if ( strDnsServer.empty() )
    {
        dns_setserver( 0, NULL );
    }
    else
    {
        ip_addr_t dnsServer;
        memset( &dnsServer, 0, sizeof( ip_addr_t ) );

        dnsServer.type = IPADDR_TYPE_V4;
        dnsServer.u_addr.ip4.addr = esp_ip4addr_aton( strDnsServer.c_str() );
        dns_setserver( 0, &dnsServer );
    }

    return nEspError;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void PluginTaskWifiConnector::connectToStation( void )
{
    if ( m_bWifiOn
                    && !m_bSleepOn
                    && ( ( m_nConfigWifiMode == UxEspCppLibrary::PluginTaskWifiConnectorTypes::WifiMode::Sta )
                                    || ( m_nConfigWifiMode == UxEspCppLibrary::PluginTaskWifiConnectorTypes::WifiMode::Both ) ) )
    {
        esp_err_t nEspError = esp_wifi_connect();

        switch ( nEspError )
        {
            case ESP_OK:
            {
                logInfo( "connectToStation: passed" );
                pluginTaskWifiConnectorDistributor()->sendWifiStatus( PluginTaskWifiConnectorTypes::WifiStatus::Connecting );
            }
            break;

            case ESP_ERR_WIFI_NOT_STARTED:
            {
                wifi_config_t config;
                esp_wifi_get_config( WIFI_IF_STA, &config );
                vlogWarning( "connectToStation: esp_wifi_connect failed %d (%s) to station <%s>", nEspError, esp_err_to_name( nEspError ), reinterpret_cast<char*>( config.sta.ssid ) );
            }
            break;

            default:
            {
                wifi_config_t config;
                esp_wifi_get_config( WIFI_IF_STA, &config );
                vlogError( "connectToStation: esp_wifi_connect failed %d (%s) to station <%s>", nEspError, esp_err_to_name( nEspError ), reinterpret_cast<char*>( config.sta.ssid ) );
                pluginTaskWifiConnectorDistributor()->sendWifiStatus( PluginTaskWifiConnectorTypes::WifiStatus::Failed );
            }
            break;
        }
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

bool PluginTaskWifiConnector::disconnectFromStation( void )
{
    bool result = false;

    if ( m_pWifiSta )
    {
        esp_err_t nEspError = esp_wifi_disconnect();

        if ( nEspError ==  ESP_OK )
        {
            logInfo( "disconnectFromStation: passed" );
            result = true;
        }
        else
        {
            vlogWarning( "disconnectFromStation: esp_wifi_disconnect failed %d (%s)", nEspError, esp_err_to_name( nEspError ) );
        }
    }

    return result;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

// static
void PluginTaskWifiConnector::wifiEventHandler( void * event_handler_arg,
                                                esp_event_base_t event_base,
                                                int32_t event_id,
                                                void * event_data )
{
    PluginTaskWifiConnector * pThis = reinterpret_cast<PluginTaskWifiConnector *>( event_handler_arg );

    if ( pThis )
    {
        switch ( event_id )
        {
            case WIFI_EVENT_AP_START:
            {
                pThis->logInfo( "wifiEventHandler: WIFI_EVENT_AP_START" );

                pThis->m_timerConnectionTimeout.stop();

                pThis->pluginTaskWifiConnectorDistributor()->sendWifiStatus( PluginTaskWifiConnectorTypes::WifiStatus::Connected );
                pThis->pluginTaskWifiConnectorDistributor()->sendWifiEventApStart();
            }
            break;

            case WIFI_EVENT_AP_STOP:
            {
                pThis->logInfo( "wifiEventHandler: WIFI_EVENT_AP_STOP" );

                pThis->pluginTaskWifiConnectorDistributor()->sendWifiEventApStop();

                // special case: switching back from AP/STA into STA mode
                if ( pThis->m_nConfigWifiMode == PluginTaskWifiConnectorTypes::WifiMode::Sta )
                {
                    pThis->m_timerConnectionTimeout.stop();
                    pThis->pluginTaskWifiConnectorDistributor()->sendWifiStatus( PluginTaskWifiConnectorTypes::WifiStatus::Connected );
                }
            }
            break;

            case WIFI_EVENT_AP_STACONNECTED:
            {
                wifi_event_ap_staconnected_t * const event = reinterpret_cast<wifi_event_ap_staconnected_t*>( event_data );
                std::string strMac = PluginTaskWifiConnector::makeMacStr( event->mac );

                pThis->vlogInfo( "wifiEventHandler: WIFI_EVENT_AP_STACONNECTED station %s join, AID=%d",
                                 strMac.c_str(),
                                 event->aid );

                pThis->pluginTaskWifiConnectorDistributor()->sendWifiEventApStaConnected( strMac );
            }
            break;

            case WIFI_EVENT_AP_STADISCONNECTED:
            {
                wifi_event_ap_stadisconnected_t * const event = reinterpret_cast<wifi_event_ap_stadisconnected_t*>( event_data );
                std::string strMac = PluginTaskWifiConnector::makeMacStr( event->mac );

                pThis->vlogInfo( "wifiEventHandler: WIFI_EVENT_AP_STADISCONNECTED station %s leave, AID=%d",
                                 strMac.c_str(),
                                 event->aid );

                pThis->pluginTaskWifiConnectorDistributor()->sendWifiEventApStaDisconnected( strMac );
            }
            break;

            case WIFI_EVENT_STA_START:
            {
                pThis->logInfo( "wifiEventHandler: WIFI_EVENT_STA_START" );
                pThis->connectToStation();

                pThis->pluginTaskWifiConnectorDistributor()->sendWifiEventStaStart();
            }
            break;

            case WIFI_EVENT_STA_STOP:
            {
                pThis->logInfo( "wifiEventHandler: WIFI_EVENT_STA_STOP" );

                pThis->pluginTaskWifiConnectorDistributor()->sendWifiEventStaStop();
            }
            break;

            case WIFI_EVENT_STA_CONNECTED:
            {
                wifi_event_sta_connected_t * const event = reinterpret_cast<wifi_event_sta_connected_t*>( event_data );

                pThis->vlogInfo( "wifiEventHandler: WIFI_EVENT_STA_CONNECTED to SSID=%s channel=%d",
                                 event->ssid,
                                 event->channel );

                pThis->pluginTaskWifiConnectorDistributor()->sendWifiEventStaConnected();
            }
            break;

            case WIFI_EVENT_STA_DISCONNECTED:
            {
                wifi_event_sta_disconnected_t * const event = reinterpret_cast<wifi_event_sta_disconnected_t*>( event_data );

                pThis->vlogInfo( "wifiEventHandler: WIFI_EVENT_STA_DISCONNECTED - SSID %s reason %u / reconnect ...",
                                 event->ssid,
                                 event->reason );

                pThis->pluginTaskWifiConnectorDistributor()->sendWifiEventStaDisconnected();

                pThis->connectToStation();
            }
            break;

            case WIFI_EVENT_SCAN_DONE:
            {
                pThis->vlogInfo( "wifiEventHandler: WIFI_EVENT_SCAN_DONE");
                pThis->pluginTaskWifiConnectorDistributor()->sendWifiEventScanDone();
                pThis->processScanResults();
            }
            break;

            case IP_EVENT_STA_GOT_IP:
            {
                ip_event_got_ip_t * const event = reinterpret_cast<ip_event_got_ip_t*>( event_data );

                pThis->vlogInfo( "wifiEventHandler: got ip " IPSTR, IP2STR( &event->ip_info.ip ) );

                char szIp[50];
                sprintf( szIp, IPSTR, IP2STR( &event->ip_info.ip ) );

                pThis->pluginTaskWifiConnectorDistributor()->sendWifiEventStaGotIp( szIp );

                pThis->pluginTaskWifiConnectorDistributor()->sendWifiStatus( PluginTaskWifiConnectorTypes::WifiStatus::Connected );
                pThis->m_timerConnectionTimeout.stop();
            }
            break;

            default:
            {
                pThis->vlogInfo( "wifiEventHandler: %d (unhandled)", event_id );
            }
            break;
        }
    }

    pThis->distributeWifiInfo();
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void PluginTaskWifiConnector::processScanResults()
{
    uint16_t u16Count = 0;

    if ( esp_wifi_scan_get_ap_num( &u16Count ) == ESP_OK )
    {
        if ( u16Count > 0 )
        {
            wifi_ap_record_t * const pApRecordList = reinterpret_cast<wifi_ap_record_t *>( malloc( u16Count * sizeof( wifi_ap_record_t ) ) );

            if ( pApRecordList )
            {
                esp_err_t nEspError = esp_wifi_scan_get_ap_records( &u16Count, pApRecordList );

                if ( nEspError == ESP_OK )
                {
                    vlogInfo( "processScanResults: esp_wifi_scan_get_ap_records succeeded, found %d AP", u16Count );
                    //                    for ( int i = 0; i < u16Count; i++ )
                    //                    {
                    //                        printf( "apRecordList[%d]=%s\n", i, pApRecordList[i].ssid );
                    //                    }

                    // pApRecordList will be freed by receiver
                    pluginTaskWifiConnectorDistributor()->sendWifiScanResults( true, u16Count, pApRecordList );
                }
                else
                {
                    vlogError( "processScanResults: esp_wifi_scan_get_ap_records failed, %s", esp_err_to_name( nEspError ) );
                    free( pApRecordList );
                    pluginTaskWifiConnectorDistributor()->sendWifiScanResults( false, 0,  nullptr );
                }
            }
            else
            {
                logError( "processScanResults: memory allocate failed" );
                pluginTaskWifiConnectorDistributor()->sendWifiScanResults( false, 0,  nullptr );
            }
        }
        else
        {
            logInfo( "processScanResults: Accesspoints count is 0" );
            pluginTaskWifiConnectorDistributor()->sendWifiScanResults( true, 0,  nullptr );
        }
    }
    else
    {
        logError("processScanResults: esp_wifi_scan_get_ap_num failed");
        pluginTaskWifiConnectorDistributor()->sendWifiScanResults( false, 0,  nullptr );
    }

    esp_wifi_scan_stop();

    if ( m_nConfigWifiMode == PluginTaskWifiConnectorTypes::WifiMode::Ap )
    {
        esp_wifi_set_mode( WIFI_MODE_AP );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void PluginTaskWifiConnector::startWifiScan( void )
{
    if ( m_bWifiOn )
    {
        if ( m_nConfigWifiMode == PluginTaskWifiConnectorTypes::WifiMode::Ap )
        {
            // if any STA credentials have been set before, clear them, because they might be wrong and then transition
            // won't work
            if ( !m_strConfigStaSsid.empty() || !m_strConfigStaPassword.empty() )
            {
                wifi_config_t wifi_config;
                std::memset( &wifi_config, 0, sizeof( wifi_config ) );

                wifi_config.sta.scan_method        = WIFI_FAST_SCAN;
                wifi_config.sta.sort_method        = WIFI_CONNECT_AP_BY_SIGNAL;
                wifi_config.sta.threshold.rssi     = 0;
                wifi_config.sta.threshold.authmode = WIFI_AUTH_OPEN;
                wifi_config.sta.bssid_set          = false;

                esp_err_t nEspError = esp_wifi_set_config( WIFI_IF_STA, &wifi_config );

                if ( nEspError != ESP_OK )
                {
                    vlogError( "startWifiScan: esp_wifi_set_config failed, %s", esp_err_to_name( nEspError ) );
                }
            }

            esp_wifi_set_mode( WIFI_MODE_APSTA );
        }

        //esp_wifi_start();
        esp_err_t nEspError = esp_wifi_scan_start( nullptr, true );

        if ( nEspError != ESP_OK )
        {
            vlogError( "startWifiScan: esp_wifi_scan_start failed (%s)", esp_err_to_name( nEspError ) );
            pluginTaskWifiConnectorDistributor()->sendWifiEventScanDone();
            pluginTaskWifiConnectorDistributor()->sendWifiScanResults( false, 0, nullptr );
        }
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

//static
std::string PluginTaskWifiConnector::makeMacStr( const uint8_t * pu8Mac )
{
    char szMacStr[20] = { 0 };

    if ( pu8Mac != nullptr )
    {
        sprintf( szMacStr, MACSTR, MAC2STR( pu8Mac ) );
    }

    return szMacStr;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void PluginTaskWifiConnector::updateActiveMac( void )
{
    //m_strActiveMac = "";

    if ( m_pWifiAp )
    {
        uint8_t u8Mac[20];
        esp_err_t nEspError = esp_netif_get_mac( m_pWifiAp, u8Mac );

        if ( nEspError != ESP_OK )
        {
            m_strApMac = "";
            vlogError( "updateActiveMac: esp_netif_get_mac AP failed (%s)", esp_err_to_name( nEspError ) );
        }
        else if ( u8Mac[0] != 0 )
        {
            m_strApMac = makeMacStr( u8Mac );
        }
        else
        {
            m_strApMac = "";
        }
    }

    if ( m_pWifiSta )
    {
        uint8_t u8Mac[20];
        esp_err_t nEspError = esp_netif_get_mac( m_pWifiSta, u8Mac );

        if ( nEspError != ESP_OK )
        {
            m_strStaMac = "";
            vlogError( "updateActiveMac: esp_netif_get_mac STA failed (%s)", esp_err_to_name( nEspError ) );
        }
        else if ( u8Mac[0] != 0 )
        {
            m_strStaMac = makeMacStr( u8Mac );
        }
        else
        {
            m_strStaMac = "";
        }
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void PluginTaskWifiConnector::updateActiveIpv4( void )
{
    m_strApIpv4 = "";
    m_strStaIpv4 = "";

    if ( m_pWifiAp )
    {
        esp_netif_ip_info_t ipInfo;
        esp_err_t nEspError = esp_netif_get_ip_info( m_pWifiAp, &ipInfo );

        if ( nEspError != ESP_OK )
        {
            vlogError( "updateActiveIpv4: esp_netif_get_ip_info failed (%s)", esp_err_to_name( nEspError ) );
        }
        else if ( ipInfo.ip.addr != 0 )
        {
            char szIp[50];
            sprintf( szIp, IPSTR, IP2STR( &ipInfo.ip ) );

            m_strApIpv4 = szIp;
        }
    }

    if ( m_pWifiSta )
    {
        esp_netif_ip_info_t ipInfo;
        esp_err_t nEspError = esp_netif_get_ip_info( m_pWifiSta, &ipInfo );

        if ( nEspError != ESP_OK )
        {
            vlogError( "updateActiveIpv4: esp_netif_get_ip_info failed (%s)", esp_err_to_name( nEspError ) );
        }
        else if ( ipInfo.ip.addr != 0 )
        {
            char szIp[50];
            sprintf( szIp, IPSTR, IP2STR( &ipInfo.ip ) );

            m_strStaIpv4 = szIp;
        }
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void PluginTaskWifiConnector::processHostname( const std::string & strHostname )
{
    m_strConfigHostname = strHostname;

    if ( m_pWifiSta && !m_strConfigHostname.empty() )
    {
        esp_err_t nEspError = esp_netif_set_hostname( m_pWifiSta, m_strConfigHostname.c_str() );

        if ( nEspError != ESP_OK )
        {
            vlogError( "processHostname: esp_netif_set_hostname STA failed (%s)", esp_err_to_name( nEspError ) );
        }
        else
        {
            vlogInfo( "processHostname STA: %s", m_strConfigHostname.c_str() );
        }
    }

    if ( m_pWifiAp && !m_strConfigHostname.empty() )
    {
        esp_err_t nEspError = esp_netif_set_hostname( m_pWifiAp, m_strConfigHostname.c_str() );

        if ( nEspError != ESP_OK )
        {
            vlogError( "processHostname: esp_netif_set_hostname AP failed (%s)", esp_err_to_name( nEspError ) );
        }
        else
        {
            vlogInfo( "processHostname AP: %s", m_strConfigHostname.c_str() );
        }
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void PluginTaskWifiConnector::distributeWifiInfo( void )
{
    updateActiveMac();
    updateActiveIpv4();
    pluginTaskWifiConnectorDistributor()->sendWifiInfo( m_strApIpv4,
                                                        m_strApMac,
                                                        m_strStaIpv4,
                                                        m_strStaMac );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

} // namespace UxEspCppLibrary

