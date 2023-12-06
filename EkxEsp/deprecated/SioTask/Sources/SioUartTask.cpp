/*
 * SioUartTask.cpp
 *
 *  Created on: 24.06.2020
 *      Author: gesser
 */

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#include "SioUartTask.h"

#include <lwip/sio.h>
#include <netif/slipif.h>
#include <string.h>
#include <lwip/ip.h>
#include <lwip/tcpip.h>
#include <lwip/inet.h>
#include <esp_mac.h>

#include "EkxApplication.h"
#include "ApplicationGlobals.h"

#define BUF_SIZE   2048

//#define DUMP_IP_INPUT
//#define DUMP_JSON

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

SioUartTask::SioUartTask( void )
: UxEspCppLibrary::FreeRtosTask( 4096,
                                 SIO_UART_TASK_PRIORITY,
                                 "SioUartTask" )
{
    memset( &m_netifSlip, 0, sizeof( m_netifSlip ) );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

SioUartTask::~SioUartTask()
{
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void SioUartTask::init( void )
{
    ip4_addr_t ip4Addr;         /**< Interface IPV4 address */
    ip4_addr_t ip4Netmask;      /**< Interface IPV4 netmask */
    ip4_addr_t ip4Gateway;      /**< Interface IPV4 gateway address */
    ip4_addr_t ip4ReceiverAddr;

    ip4Addr.addr         = inet_addr( SLIP_LOCAL_IP );
    ip4ReceiverAddr.addr = inet_addr( SLIP_REMOTE_IP );
    IP4_ADDR( &ip4Gateway, 192, 168, 200, 1 );
    IP4_ADDR( &ip4Netmask, 255, 255, 255, 0 );

    uart_port_t nUcdUartNum = SIO_UART_NUM;

    uart_config_t uart_config =
    {
                    .baud_rate           = 921600,
                    .data_bits           = UART_DATA_8_BITS,
                    .parity              = UART_PARITY_DISABLE,
                    .stop_bits           = UART_STOP_BITS_1,
                    .flow_ctrl           = UART_HW_FLOWCTRL_DISABLE,
                    .rx_flow_ctrl_thresh = 0,
#if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(5, 0, 0)
                    .use_ref_tick        = false
#else
                    .source_clk          = UART_SCLK_DEFAULT
#endif
    };
    ESP_ERROR_CHECK( uart_param_config( nUcdUartNum, &uart_config ) );

    ESP_ERROR_CHECK( uart_set_pin( nUcdUartNum,
                                   GPIO_SIO_TX,
                                   GPIO_SIO_RX,
                                   UART_PIN_NO_CHANGE,
                                   UART_PIN_NO_CHANGE ) );

    //Install UART driver, and get the queue.
    ESP_ERROR_CHECK( uart_driver_install( nUcdUartNum,
                                          2 * UART_FIFO_LEN,
                                          0,            // don't increase TX buffer! will slow down data rate
                                          0,
                                          NULL,
                                          0 ) );

    ESP_ERROR_CHECK( esp_efuse_mac_get_default( m_netifSlip.hwaddr ) );
    m_netifSlip.hwaddr_len = 6;

    if ( !netif_add( &m_netifSlip,
                     &ip4Addr,
                     &ip4Netmask,
                     &ip4Gateway,
                     ( void * ) nUcdUartNum,
                     slipif_init,
#ifdef DUMP_IP_INPUT
                     ipInput
#else
                     ip_input
#endif
    ) )
    {
        vlogError( "netif_add returns illegal NULL" );
        ESP_ERROR_CHECK( true );
    }

    netif_set_link_up( &m_netifSlip );
    netif_set_up( &m_netifSlip );

    vlogInfo( "Start SLIP interface on IP address %d.%d.%d.%d", IP2STR( &ip4Addr ) );

    ekxApp().wifiConnectorTask().natRouter().setInternalInterface( &m_netifSlip, ip4ReceiverAddr );

    // this doesn't make any sense here
    // it is just implemented to keep the file SioUart
    // is use. Otherwise with idf.py the functions will be
    // eliminated because usage and declaration is within lwip, not this component
    sio_send( ( unsigned char ) 0x20, (sio_fd_t) nUcdUartNum );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void SioUartTask::execute( void )
{
    for ( ;; )
    {
        slipif_poll( &m_netifSlip );
        taskYIELD();//vTaskDelay( 0 );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

struct netif * SioUartTask::netif( void )
{
    return &m_netifSlip;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

// static
err_t SioUartTask::ipInput( struct pbuf *p,
                            struct netif *inp )
{
    // skip 47 (is alive)
    if ( p->len != 47 )
    {
#ifdef DUMP_JSON
        (( char *) p->payload)[p->len] = 0;
        ESP_LOGI( __FUNCTION__, "ipInput %d %s", p->len, (char*) p->payload );
#endif

        ESP_LOGI( __FUNCTION__, "ipInput %d", p->len );
    }

    err_t nError = ip_input( p, inp );

    return nError;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/
