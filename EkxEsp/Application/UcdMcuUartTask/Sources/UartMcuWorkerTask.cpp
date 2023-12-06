/*
 * UartWorkerTask.cpp
 *
 *  Created on: 19.11.2020
 *      Author: gesser
 */

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#include "UartMcuWorkerTask.h"

#include <driver/uart.h>

#include "UcdMcuUartTask.h"

//#define DEBUG_UART

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

UartMcuWorkerTask::UartMcuWorkerTask( UcdMcuUartTask *    pUcdUartTask,
                                      const uint32_t      u32StackSize,
                                      const UBaseType_t   uTaskPriority,
                                      const std::string & strTaskName )
    : UxEspCppLibrary::FreeRtosTask( u32StackSize,
                                     uTaskPriority,
                                     strTaskName )
    , m_pUcdUartTask( pUcdUartTask )
{
    vlogInfo( "constructor" );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

UartMcuWorkerTask::~UartMcuWorkerTask()
{
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

bool UartMcuWorkerTask::create( const uart_port_t        nUartNum,
                                const int                nTxPin,
                                const int                nRxPin,
                                const int                nBaudRate,
                                const uart_word_length_t nDataBits,
                                const uart_parity_t      nParity,
                                const uart_stop_bits_t   nStopBits )
{
    vlogInfo( "create(): UART#%d", m_nUartNum );

    bool bSuccess = false;

    m_nUartNum = nUartNum;

    uart_config_t uart_config;

    uart_config.baud_rate           = nBaudRate;
    uart_config.data_bits           = nDataBits;
    uart_config.parity              = nParity;
    uart_config.stop_bits           = nStopBits;
    uart_config.flow_ctrl           = UART_HW_FLOWCTRL_DISABLE;
    uart_config.rx_flow_ctrl_thresh = 0U;
    uart_config.source_clk          = UART_SCLK_APB;

    esp_err_t err = uart_param_config( m_nUartNum,
                                       &uart_config );

    if ( err == ESP_OK )
    {
        err = uart_set_pin( m_nUartNum,
                            nTxPin,
                            nRxPin,
                            UART_PIN_NO_CHANGE,
                            UART_PIN_NO_CHANGE );

        if ( err == ESP_OK )
        {
            err = uart_driver_install( m_nUartNum,
                                       UART_RX_TX_BUFFER_SIZE,    // RX buffer size
                                       0,                         // tx_buffer_size
                                       50,                        // queue_size
                                       &m_hUartReceiveQueue,      // uart_queue handle
                                       0 );                       // intr_alloc_flags

            if ( err == ESP_OK )
            {
                createTask();
                bSuccess = true;
            }
            else
            {
                vlogError( "create: driver install failed err: %d", err );
            }
        }
        else
        {
            vlogError( "create: set pin failed err %d", err );
        }
    }
    else
    {
        vlogError( "create: config failed err %d", err );
    }

    return bSuccess;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void UartMcuWorkerTask::execute( void )
{
    vlogInfo( "execute()" );

    uart_event_t uartEvent;
    uint8_t      tmpBuf[UART_RX_TX_BUFFER_SIZE];

    while ( true )
    {
        //Waiting for UART event.
        BaseType_t xReturn = xQueueReceive( m_hUartReceiveQueue,
                                            ( void * ) &uartEvent,
                                            100 / portTICK_PERIOD_MS );

        if ( xReturn == pdPASS )
        {
            switch ( uartEvent.type )
            {
                case UART_DATA:
                {
                    if ( uartEvent.size <= sizeof( tmpBuf ) )
                    {
                        int n = uart_read_bytes( m_nUartNum, tmpBuf, uartEvent.size, portMAX_DELAY );

                        if ( n > 0 )
                        {
#ifdef DEBUG_UART
                            printf( "Receive: " );
                            for ( int i = 0; i < n; i++ )
                            {
                                printf( "%02x ", ( (char *)tmpBuf )[i] );
                            }
                            printf( "\n" );
#endif

                            m_pUcdUartTask->sendReplyToCtrlUart( tmpBuf, n );
                        }
                    }
                    else
                    {
                        vlogError( "execute: read size too big, skip data" );
                        uart_flush_input( m_nUartNum );
                        xQueueReset( m_hUartReceiveQueue );
                    }
                }
                break;

                //Event of HW FIFO overflow detected
                case UART_FIFO_OVF:
                {
                    vlogError( "execute: event UART_FIFO_OVF" );
                    uart_flush_input( m_nUartNum );
                    xQueueReset( m_hUartReceiveQueue );
                }
                break;

                //Event of UART ring buffer full
                case UART_BUFFER_FULL:
                {
                    vlogError( "execute: event UART_BUFFER_FULL" );
                    uart_flush_input( m_nUartNum );
                    xQueueReset( m_hUartReceiveQueue );
                }
                break;

                //Event of UART RX break detected
                case UART_BREAK:
                {
                    vlogError( "execute: event UART_BREAK" );
                    uart_flush_input( m_nUartNum );
                    xQueueReset( m_hUartReceiveQueue );
                }
                break;

                //Event of UART parity check error
                case UART_PARITY_ERR:
                {
                    vlogError( "execute: event UART_PARITY_ERR" );
                    uart_flush_input( m_nUartNum );
                    xQueueReset( m_hUartReceiveQueue );
                }
                break;

                //Event of UART frame error
                case UART_FRAME_ERR:
                {
                    vlogError( "execute: event UART_FRAME_ERR" );
                    uart_flush_input( m_nUartNum );
                    xQueueReset( m_hUartReceiveQueue );
                }
                break;

                //Others
                default:
                {
                    vlogError( "execute: unknown uart event %d", uartEvent.type );
                    uart_flush_input( m_nUartNum );
                    xQueueReset( m_hUartReceiveQueue );
                }
                break;
            }
        }
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/
