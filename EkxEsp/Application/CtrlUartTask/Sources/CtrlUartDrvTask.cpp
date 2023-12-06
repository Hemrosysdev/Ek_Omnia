/*!*****************************************************************************
*
* @file CtrlUartDrvTask.c
*
* @brief Implementation file of class CtrlUartDrvTask.
*
* @author Ultratronik GmbH
*         Dornierstr. 9
*         D-82205 Gilching
*         www.ultratronik.de
*
* @author Gerd Esser, Forschung & Entwicklung
*
* @created 28.11.2022
*
* @copyright Copyright 2022 by Hemro International AG
*             Hemro International AG
*             Länggenstrasse 34
*             CH 8184 Bachenbülach
*             Switzerland
*             Homepage: www.hemrogroup.com
*
*******************************************************************************/

/*##*************************************************************************************************************************************************************
 *  Includes
 **************************************************************************************************************************************************************/

#include "CtrlUartDrvTask.h"

#include "CtrlUartTask.h"
#include "FreeRtosTask.h"
#include "ApplicationGlobals.h"
#include "ApplicationHelper.h"
#include "EkxProtocol.h"

#include <functional>
#include <algorithm>
#include <freertos/queue.h>
#include <rom/crc.h>

//#define CONFIG_DEBUG_CTRL_UART_DATA

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

namespace CtrlUart
{

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

CtrlUartDrvTask::CtrlUartDrvTask( CtrlUartTask * const pUartTask )
    : UxEspCppLibrary::FreeRtosTask( ApplicationGlobals::c_nCtrlUartDrvTaskStackSize,
                                     ApplicationGlobals::c_nCtrlUartDrvTaskPriority,
                                     "CtrlUartDrvTask" )
    , m_pUartTask( pUartTask )
    , m_nUartNum( ApplicationGlobals::c_nCtrlUartNum )
{
    auto lambdaCrc32 = []( const std::string & strData, EkxProtocol::MessageFrame & message )
                       {
                           uint32_t u32Crc32 = crc32_be( 0x0, reinterpret_cast<const uint8_t *>( strData.data() ), strData.size() );
                           message.crc32().setValue( u32Crc32 );
                       };

    m_pDeserializer = new EkxProtocol::Deserializer( lambdaCrc32 );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

CtrlUartDrvTask::~CtrlUartDrvTask()
{
    m_pUartTask = nullptr;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

esp_err_t CtrlUartDrvTask::setupUartDriver( const uart_config_t * const pUartConfig )
{
    esp_err_t nEspError = ESP_OK;

    vlogInfo( "setupUartDriver(): free heap size %d", esp_get_free_heap_size() );

    nEspError = uart_driver_delete( m_nUartNum );
    if ( nEspError != ESP_OK )
    {
        // this is raised within run time, so no possibility to commit this error
        vlogError( "setupUartDriver(): uart_driver_delete failed (%s)", esp_err_to_name( nEspError ) );
    }

    nEspError = uart_param_config( m_nUartNum, pUartConfig );
    if ( nEspError != ESP_OK )
    {
        // this is raised within run time, so no possibility to commit this error
        vlogError( "setupUartDriver(): uart_param_config failed (%s)", esp_err_to_name( nEspError ) );
    }

    nEspError = uart_set_pin( m_nUartNum,
                              ApplicationGlobals::c_nGpioCtrlUartTx,
                              ApplicationGlobals::c_nGpioCtrlUartRx,
                              UART_PIN_NO_CHANGE,
                              UART_PIN_NO_CHANGE );
    if ( nEspError != ESP_OK )
    {
        // this is raised within run time, so no possibility to commit this error
        vlogError( "setupUartDriver(): uart_set_pin failed (%s)", esp_err_to_name( nEspError ) );
    }

    // We won't use a buffer for sending data.
    nEspError = uart_driver_install( m_nUartNum,
                                     c_nCtrlUartRxBufSize,
                                     c_nCtrlUartTxBufSize,
                                     c_nCtrlUartQueueSize,
                                     &m_hUartQueue,
                                     0 );
    if ( nEspError != ESP_OK )
    {
        // this is raised within run time, so no possibility to commit this error
        vlogError( "setupUartDriver(): uart_driver_install failed (%s)", esp_err_to_name( nEspError ) );
    }

    vlogInfo( "setupUartDriver(): post, free heap size %d", esp_get_free_heap_size() );

    return nEspError;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

esp_err_t CtrlUartDrvTask::init( void )
{
    const uart_config_t uartConfig =
    {
        .baud_rate           = c_nCtrlUartBaudRate,
        .data_bits           = UART_DATA_8_BITS,
        .parity              = UART_PARITY_DISABLE,
        .stop_bits           = UART_STOP_BITS_1,
        .flow_ctrl           = c_nCtrlUartFlowCtrl,
        .rx_flow_ctrl_thresh = c_nCtrlUartFlowCtrlTh,
        .source_clk          = UART_SCLK_APB
    };

    const esp_err_t nEspError = setupUartDriver( &uartConfig );

    if ( nEspError != ESP_OK )
    {
        // this is raised within run time, so no possibility to commit this error
        vlogError( "init(): setupUartDriver failed (%s)", esp_err_to_name( nEspError ) );
    }

    return nEspError;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

QueueHandle_t CtrlUartDrvTask::uartQueue()               // NOSONAR void * type is required by Espressif IDF
{
    return m_hUartQueue;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

esp_err_t CtrlUartDrvTask::writeData( const uint8_t * const pData,
                                      const size_t          u32DataSize )
{
    esp_err_t nEspError = ESP_FAIL;

    if ( m_hUartQueue != nullptr )
    {
#if defined( CONFIG_DEBUG_CTRL_UART_DATA )
        ApplicationGlobals::ApplicationHelper::dumpHexData( "writeData", pData, u32DataSize );
#endif

        nEspError = uart_wait_tx_done( m_nUartNum, c_nMaxWaitTxDoneTicks );

        // if buffer was empty (as uart_wait_tx_done reports before)
        if ( nEspError == ESP_OK )
        {
            m_u32AssumedUartTxBufferPos = 0;
        }
        else
        {
            vlogError( "writeData() uart_wait_tx_done failed %s", esp_err_to_name( nEspError ) );
        }

        if ( ( nEspError != ESP_OK )
             && ( m_u32AssumedUartTxBufferPos > 0 )
             && ( ( m_u32AssumedUartTxBufferPos + u32DataSize ) > c_nUartTxBufThreshold ) )
        {
            vlogWarning( "writeData(): error TX buffer not empty" );
        }
        else
        {
#if 0
            size_t u32TotalWrittenBytes = 0;

            while ( u32TotalWrittenBytes < u32DataSize )
            {
                size_t u32DataToWrite = std::min( 50U, u32DataSize - u32TotalWrittenBytes );

                int nWrittenBytes = uart_write_bytes( m_nUartNum,
                                                      reinterpret_cast<const char *>( pData ) + u32TotalWrittenBytes,
                                                      u32DataToWrite );

                if ( nWrittenBytes < 0 )
                {
                    vlogWarning( "writeData(): error while writing, code %d", nWrittenBytes );
                    break;
                }
                else
                {
                    if ( static_cast<size_t>( nWrittenBytes ) != u32DataToWrite )
                    {
                        vlogWarning( "writeData(): error writing chunk data size, %d != %d", nWrittenBytes, u32DataToWrite );
                    }

                    u32TotalWrittenBytes += static_cast<uint32_t>( nWrittenBytes );
                }

                vTaskDelay( 1 );
            }

            if ( u32TotalWrittenBytes != u32DataSize )
            {
                vlogWarning( "writeData(): error writing data size, %d != %d", u32TotalWrittenBytes, u32DataSize );
            }

            m_u32AssumedUartTxBufferPos += u32TotalWrittenBytes;
#else
            int nWrittenBytes = uart_write_bytes( m_nUartNum,
                                                  reinterpret_cast<const char *>( pData ),
                                                  u32DataSize );

            if ( nWrittenBytes < 0 )
            {
                vlogWarning( "writeData(): error while writing, code %d", nWrittenBytes );
            }
            else
            {
                if ( static_cast<size_t>( nWrittenBytes ) != u32DataSize )
                {
                    vlogWarning( "writeData(): error writing data size, %d != %d", nWrittenBytes, u32DataSize );
                }

                m_u32AssumedUartTxBufferPos += static_cast<uint32_t>( nWrittenBytes );
            }
#endif
        }
    }

    return nEspError;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

esp_err_t CtrlUartDrvTask::writeData( const std::string & strData )
{
    esp_err_t nEspError = writeData( reinterpret_cast<const uint8_t *>( strData.data() ), strData.size() );

    if ( nEspError != ESP_OK )
    {
        vlogError( "writeData() error while writing data %s", esp_err_to_name( nEspError ) );
    }

    return nEspError;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void CtrlUartDrvTask::clearUartBuffer()
{
    vlogInfo( "clearUartBuffer" );
    uart_flush_input( m_nUartNum );
    xQueueReset( m_hUartQueue );
    m_pDeserializer->flushStreamData();
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void CtrlUartDrvTask::execute()
{
    ESP_ERROR_CHECK( init() );

    uart_event_t uartEvent;

    while ( true )
    {
        if ( m_hUartQueue == nullptr )
        {
            vTaskDelay( c_nMessageTimeout5Ms / portTICK_PERIOD_MS );
        }
        else
        {
            // Waiting for UART event.
            const BaseType_t xReturn = xQueueReceive( m_hUartQueue,
                                                      reinterpret_cast<void *>( &uartEvent ),
                                                      c_nMessageTimeout100Ms / portTICK_PERIOD_MS );

            // pdFALSE indicates (also) timeout condition
            if ( xReturn == pdFALSE )
            {
                processTimedOutData();
            }
            else
            {
                processUartEvent( uartEvent );
            }
        }
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void CtrlUartDrvTask::processTimedOutData()
{
    if ( m_pDeserializer->hasData() )
    {
        m_pDeserializer->flushStreamData();
        vlogWarning( "processTimedOutData()" );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

bool CtrlUartDrvTask::processUartEvent( const uart_event_t & uartEvent )
{
    bool bDataRead = false;

    switch ( uartEvent.type )
    {
        // Event of UART receving data
        // We'd better handler data event fast, there would be much more data events than
        // other types of events. If we take too much time on data event, the queue might
        // be full.
        case UART_DATA:
        {
            std::string data( uartEvent.size, 0 );

            const int nReadBytes = uart_read_bytes( m_nUartNum,
                                                    const_cast<char *>( data.data() ),
                                                    uartEvent.size,
                                                    portMAX_DELAY );

            if ( nReadBytes != uartEvent.size )
            {
                vlogWarning( "processUartEvent(): read bytes not equal to event size" );
                clearUartBuffer();
            }
            else
            {
                m_pDeserializer->addStreamData( reinterpret_cast<const uint8_t *>( data.data() ), nReadBytes );

                while ( true )
                {
                    EkxProtocol::Deserializer::ParseResult result = m_pDeserializer->parseData();

                    if ( ( result == EkxProtocol::Deserializer::ParseResult::Success )
                         || ( result == EkxProtocol::Deserializer::ParseResult::WrongCrc ) )
                    {
#if defined( CONFIG_DEBUG_CTRL_UART_DATA )
                        std::string strData = m_pDeserializer->messageFrame().serializedData();
                        ApplicationGlobals::ApplicationHelper::dumpHexData( "receive frame", reinterpret_cast<const uint8_t *>( strData.data() ), strData.size() );
#endif
                        m_pUartTask->processReceivedUartFrame( result, m_pDeserializer->messageFrame() );
                    }
                    else
                    {
                        break;
                    }
                }

                bDataRead = true;
            }
        }
        break;

        //Event of HW FIFO overflow detected
        case UART_FIFO_OVF:
            // If fifo overflow happened, you should consider adding flow control for your application.
            // The ISR has already reset the rx FIFO,
            // As an example, we directly flush the rx buffer here in order to read more data.
            vlogWarning( "processUartEvent(): hw fifo overflow" );
            clearUartBuffer();
            break;

        //Event of UART ring buffer full
        case UART_BUFFER_FULL:
            // If buffer full happened, you should consider increasing your buffer size
            // As an example, we directly flush the rx buffer here in order to read more data.
            vlogError( "processUartEvent(): ring buffer full, flushing buffer" );
            clearUartBuffer();
            break;

        //Event of UART RX break detected
        case UART_BREAK:
            vlogWarning( "processUartEvent(): uart rx break" );
            break;

        //Event of UART TX break detected
        case UART_DATA_BREAK:
            vlogWarning( "processUartEvent(): uart tx break" );
            break;

        //Event of UART parity check error
        case UART_PARITY_ERR:
            vlogWarning( "processUartEvent(): uart parity error" );
            break;

        //Event of UART frame error
        case UART_FRAME_ERR:
            vlogWarning( "processUartEvent(): uart frame error" );
            break;

        //Event of UART parity check error
        case UART_PATTERN_DET:
            vlogWarning( "processUartEvent(): uart pattern detected" );
            break;

        //Others
        default:
            vlogError( "processUartEvent(): unhandled uart event type: %d", uartEvent.type );
            break;
    }

    return bDataRead;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

} /* namespace CtrlUart */
