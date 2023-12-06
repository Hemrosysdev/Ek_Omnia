/*!*************************************************************************************************************************************************************
 *
 * @file EthernetController.cpp
 * @brief Source file of component unit <title>.
 *
 * This file was developed as part of <component>.
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

/***************************************************************************************************************************************************************
 *      Includes
 **************************************************************************************************************************************************************/

#include "EthernetController.h"

#include <stdint.h>
#include <esp_err.h>
#include <esp_event.h>

#include <esp_event_base.h>

#if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(5, 0, 0)
#include <esp_eth.h>
#else
#include <esp_eth_com.h>
#include <esp_eth_driver.h>
#endif

#include <lwip/ip4_addr.h>
#include "EkxApplication.h"
#include "EspLog.h"

/***************************************************************************************************************************************************************
 *      Static members Initialization
 **************************************************************************************************************************************************************/
EthernetController::LINK_STATE EthernetController::m_linkState = EthernetController::LINK_STATE::UNINITIALIZED;

/***************************************************************************************************************************************************************
 *      Private method implementations
 **************************************************************************************************************************************************************/

void EthernetController::gotIpEventCallback( void*            arg,
                                             esp_event_base_t event_base,
                                             int32_t          event_id,
                                             void*            event_data )
{
    ip_event_got_ip_t *event = (ip_event_got_ip_t *) event_data;

    //const tcpip_adapter_ip_info_t *ip_info = &event->ip_info;

    printf( "Got IP Address " IPSTR "\n", IP2STR( &event->ip_info.ip  ) );

    m_linkState = EthernetController::LINK_STATE::GOT_IP;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void EthernetController::ethEventCallback( void*            arg,
                                           esp_event_base_t event_base,
                                           int32_t          event_id,
                                           void*            event_data )
{
    uint8_t mac_addr[6] = {0};
    /* we can get the ethernet driver handle from event data */
    esp_eth_handle_t eth_handle = *(esp_eth_handle_t *)event_data;

    printf( "Eth Event ID: %d\n", static_cast<int>( event_id ) );
    switch (event_id)
    {
        case ETHERNET_EVENT_CONNECTED:
            esp_eth_ioctl(eth_handle, ETH_CMD_G_MAC_ADDR, mac_addr);
            m_linkState = EthernetController::LINK_STATE::CONNECTED;
            printf( "ethEventCallback(): ETHERNET_EVENT_CONNECTED\n" );
            break;
        case ETHERNET_EVENT_DISCONNECTED:
            m_linkState = EthernetController::LINK_STATE::DISCONNECTED;
            printf( "ethEventCallback(): ETHERNET_EVENT_DISCONNECTED\n" );
            break;
        case ETHERNET_EVENT_START:
            m_linkState = EthernetController::LINK_STATE::STARTED;
            printf( "ethEventCallback(): ETHERNET_EVENT_START\n" );
            break;
        case ETHERNET_EVENT_STOP:
            m_linkState = EthernetController::LINK_STATE::STOPPED;
            printf( "ethEventCallback(): ETHERNET_EVENT_STOP\n" );
            break;
        default:
            m_linkState = EthernetController::LINK_STATE::ERROR;
            printf( "ethEventCallback(): unhandled default\n" );
            break;
    }
}
/***************************************************************************************************************************************************************
 *      Public method implementations
 **************************************************************************************************************************************************************/
/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

EthernetController::EthernetController( const std::string & strName )
: UxEspCppLibrary::EspLog( strName )
{
    logInfo( "EthernetController constructor" );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

EthernetController::~EthernetController()
{
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void EthernetController::init( void )
{
#ifdef ESP32_ETHERNET_KIT
    ESP_ERROR_CHECK(tcpip_adapter_set_default_eth_handlers());
    ESP_ERROR_CHECK(esp_event_handler_register(ETH_EVENT, ESP_EVENT_ANY_ID, &ethEventCallback, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_ETH_GOT_IP, &gotIpEventCallback, NULL));

    eth_mac_config_t mac_config = ETH_MAC_DEFAULT_CONFIG();
    eth_phy_config_t phy_config = ETH_PHY_DEFAULT_CONFIG();
    phy_config.phy_addr = 1UL;
    phy_config.reset_gpio_num = 5;

    mac_config.smi_mdc_gpio_num = 23;
    mac_config.smi_mdio_gpio_num = 18;
    esp_eth_mac_t *mac = esp_eth_mac_new_esp32(&mac_config);

    esp_eth_phy_t *phy = esp_eth_phy_new_ip101(&phy_config);

    esp_eth_config_t config = ETH_DEFAULT_CONFIG(mac, phy);
    esp_eth_handle_t eth_handle = NULL;
    ESP_ERROR_CHECK(esp_eth_driver_install(&config, &eth_handle));
    ESP_ERROR_CHECK(esp_eth_start(eth_handle));
#else
    m_linkState = EthernetController::LINK_STATE::GOT_IP;
#endif
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

EthernetController::LINK_STATE EthernetController::getLinkState( void )
{
    return m_linkState;
}
