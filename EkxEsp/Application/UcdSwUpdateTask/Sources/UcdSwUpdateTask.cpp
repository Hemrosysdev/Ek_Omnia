/*
 * UcdSwUpdateTask.cpp
 *
 *  Created on: 21.10.2019
 *      Author: gesser
 */

#include "UcdSwUpdateTask.h"

#include <cstring>
#include <esp_partition.h>

#include "UcdSwUpdateQueue.h"
#include "UcdSwUpdateQueueTypes.h"

#include "../../build/version.h"
#include "ApplicationGlobals.h"
#include "EkxApplication.h"
#include "EkxProtocol.h"

//#define DUMP_UPDATE_PROCESS

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#define TIMEOUT_ASYNC_TRANSACTION_US    ( 500000 )     // 500ms

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

UcdSwUpdateTask::UcdSwUpdateTask( UxEspCppLibrary::EspApplication * pApplication )
    : UartControlledDevice( "SwUpdate",
                            "SwUpdate",
                            EkxProtocol::DriverId::SwUpdateDriver,
                            pApplication,
                            ApplicationGlobals::c_nSwUpdateTaskStackSize,
                            ApplicationGlobals::c_nSwUpdateTaskPriority,
                            "UcdSwUpdateTask",
                            new UcdSwUpdateQueue() )
    , m_timerAsyncTransaction( taskQueue(),
                               static_cast<int>( UcdSwUpdateQueueTypes::messageType::TimeoutAsyncTransaction ) )
{
    FreeRtosQueueTask::logInfo( "constructor" );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

UcdSwUpdateTask::~UcdSwUpdateTask()
{
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

bool UcdSwUpdateTask::startupStep( const FreeRtosQueueTask::startupStep_t nStartupStep )
{
    bool bContinueStartup = true;

    switch ( nStartupStep )
    {
        case UxEspCppLibrary::FreeRtosQueueTask::startupStep_t::startupStep1:
        {
            registerDriver();
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

void UcdSwUpdateTask::processTaskMessage( void )
{
    const UcdSwUpdateQueueTypes::message * pMessage = reinterpret_cast<const UcdSwUpdateQueueTypes::message *>( receiveMsg() );

    if ( pMessage )
    {
        switch ( pMessage->u32Type )
        {
            case UcdSwUpdateQueueTypes::messageType::CtrlUartCommand:
            {
                processBaseCtrlUartCommand( pMessage->payload.ctrlUartCommand.m_u8DriverId,
                                            pMessage->payload.ctrlUartCommand.m_u32MsgCounter,
                                            pMessage->payload.ctrlUartCommand.m_u8RepeatCounter,
                                            pMessage->payload.ctrlUartCommand.m_pMessagePayload );
            }
            break;

            case UcdSwUpdateQueueTypes::messageType::AckNackReceived:
            {
                processAckNackReceived( pMessage->payload.ackNackReceived.m_u32MsgCounter,
                                        pMessage->payload.ackNackReceived.m_bSuccess );
            }
            break;

            case UcdSwUpdateQueueTypes::messageType::TimeoutAsyncTransaction:
            {
                processTimeoutAsyncTransaction();
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

UcdSwUpdateQueue * UcdSwUpdateTask::swUpdateQueue( void )
{
    return dynamic_cast<UcdSwUpdateQueue *>( taskQueue() );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

const UcdSwUpdateQueue * UcdSwUpdateTask::swUpdateQueue( void ) const
{
    return dynamic_cast<const UcdSwUpdateQueue *>( taskQueue() );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void UcdSwUpdateTask::reportDriverStatus( const bool bFirst )
{
    //vlogInfo( "reportDriverStatus" );

    EkxProtocol::Payload::SwUpdateStatus * pPayload = new EkxProtocol::Payload::SwUpdateStatus();
    pPayload->valid().setValue( true );
    pPayload->success().setValue( m_bSuccess );
    pPayload->transactionId().setValue( m_u32TransactionId );
    pPayload->cmd().setValue( m_u8Command );
    pPayload->swVersionNo().setValue( VERSION_NO );

    sendCtrlUartReply( pPayload, bFirst );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void UcdSwUpdateTask::systemRestart()
{
    vTaskDelay( 2000 / portTICK_PERIOD_MS );
    esp_restart();
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void UcdSwUpdateTask::processTimeoutAsyncTransaction()
{
    esp_err_t nEspError = ESP_OK;

#ifdef DUMP_UPDATE_PROCESS
    vlogInfo( "processTimeoutAsyncTransaction: cmd %d tid %d size %d", static_cast<int>( m_u8Command ), m_u32TransactionId, m_u32DataSize );
#endif

    switch ( m_u8Command )
    {
        case EkxProtocol::Payload::SwUpdateCmd::Start:
        {
            m_pOtaHandle  = 0;
            m_u32DataSize = 0;

            const esp_partition_t * pUpdatePartition = esp_ota_get_next_update_partition( NULL );
            vlogInfo( "processTimeoutAsyncTransaction: Writing to partition subtype %d at offset 0x%x",
                      pUpdatePartition->subtype,
                      pUpdatePartition->address );
            nEspError        = esp_ota_begin( pUpdatePartition, OTA_SIZE_UNKNOWN, &m_pOtaHandle );
            m_u32LastChunkNo = -1;

            if ( nEspError != ESP_OK )
            {
                vlogError( "processTimeoutAsyncTransaction: esp_ota_begin failed (%s)\n", esp_err_to_name( nEspError ) );
            }
            else
            {
                m_bSuccess = true;
            }
        }
        break;

        case EkxProtocol::Payload::SwUpdateCmd::Finish:
        {
            vlogInfo( "processTimeoutAsyncTransaction: Receiving ESP SW update package completed - size %d, chunks %d", m_u32DataSize, m_u32LastChunkNo );

            nEspError = esp_ota_end( m_pOtaHandle );

            if ( nEspError != ESP_OK )
            {
                vlogError( "processTimeoutAsyncTransaction: esp_ota_end failed (%s)!", esp_err_to_name( nEspError ) );
            }

            if ( nEspError == ESP_OK )
            {
                const esp_partition_t * pUpdatePartition = esp_ota_get_next_update_partition( NULL );
                nEspError = esp_ota_set_boot_partition( pUpdatePartition );

                if ( nEspError != ESP_OK )
                {
                    vlogError( "processTimeoutAsyncTransaction: esp_ota_set_boot_partition failed (%s)!", esp_err_to_name( nEspError ) );
                }
            }

            if ( nEspError == ESP_OK )
            {
                m_bSuccess = true;
            }
        }
        break;

        case EkxProtocol::Payload::SwUpdateCmd::Reboot:
        {
            vlogWarning( "processTimeoutAsyncTransaction: receive reboot command" );
            m_bSuccess = true;

            reportDriverStatus( true );

            vTaskDelay( 2000 / portTICK_PERIOD_MS );

            esp_restart();
        }
        break;

        default:
        {
            vlogError( "processAsyncTransaction: process illegal command %d", static_cast<int>( m_u8Command ) );
            m_bSuccess = false;
        }
        break;
    }

    reportDriverStatus( true );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

bool UcdSwUpdateTask::queueCtrlUartCommand( const EkxProtocol::DriverId                  u8DriverId,
                                            const uint32_t                               u32MsgCounter,
                                            const uint8_t                                u8RepeatCounter,
                                            EkxProtocol::Payload::MessagePayload * const pMessagePayload )
{
    return swUpdateQueue()->sendCtrlUartCommand( u8DriverId, u32MsgCounter, u8RepeatCounter, pMessagePayload ) == pdPASS;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void UcdSwUpdateTask::processCtrlUartCommand( const EkxProtocol::DriverId                        u8DriverId,
                                              const uint32_t                                     u32MsgCounter,
                                              const uint8_t                                      u8RepeatCounter,
                                              const EkxProtocol::Payload::MessagePayload * const pMessagePayload )
{
    const EkxProtocol::Payload::SwUpdateCommand * const pPayload = dynamic_cast<const EkxProtocol::Payload::SwUpdateCommand *>( pMessagePayload );

    if ( pPayload )
    {
        processTransaction( u32MsgCounter, u8RepeatCounter, pPayload );
    }
    else
    {
        vlogError( "processCtrlUartCommand: cant't process data; unknown command" );
        sendCtrlUartReplyNack( u32MsgCounter, u8RepeatCounter, EkxProtocol::Payload::Nack::NackReason::UnknownDriverCommand );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void UcdSwUpdateTask::processTransaction( const uint32_t                                      u32MsgCounter,
                                          const uint8_t                                       u8RepeatCounter,
                                          const EkxProtocol::Payload::SwUpdateCommand * const pCommand )
{
    esp_err_t nEspError = ESP_OK;

    m_u8Command        = pCommand->cmd().value();
    m_u32TransactionId = pCommand->transactionId().value();
    m_bSuccess         = false;
    m_u32ChunkNo       = pCommand->chunkNo().value();

#ifdef DUMP_UPDATE_PROCESS
    vlogInfo( "processTransaction: cmd %d tid %d size %d", static_cast<int>( m_u8Command ), m_u32TransactionId, pCommand->chunkData().size() );
#endif

    switch ( m_u8Command )
    {
        case EkxProtocol::Payload::SwUpdateCmd::Idle:
        {
            sendCtrlUartReplyAck( u32MsgCounter, u8RepeatCounter );
            m_bSuccess = true;
            reportDriverStatus( true );
        }
        break;

        case EkxProtocol::Payload::SwUpdateCmd::Start:
        {
            m_timerAsyncTransaction.startOnce( TIMEOUT_ASYNC_TRANSACTION_US );
            sendCtrlUartReplyAck( u32MsgCounter, u8RepeatCounter );
        }
        break;

        case EkxProtocol::Payload::SwUpdateCmd::Data:
        {
            sendCtrlUartReplyAck( u32MsgCounter, u8RepeatCounter );

            if ( m_u32LastChunkNo == m_u32ChunkNo )
            {
                vlogWarning( "processTransaction: skip repeated chunk no %d", m_u32ChunkNo );
                m_bSuccess = true;
            }
            else if ( m_u32LastChunkNo + 1 != m_u32ChunkNo )
            {
                vlogWarning( "processTransaction: receive chunk no. %d out of order, expected %d", m_u32ChunkNo, m_u32LastChunkNo + 1 );
                m_bSuccess = false;
            }
            else
            {
                m_u32LastChunkNo = m_u32ChunkNo;

                nEspError = esp_ota_write( m_pOtaHandle,
                                           pCommand->chunkData().value().data(),
                                           pCommand->chunkData().value().size() );
                m_u32DataSize += pCommand->chunkData().value().size();

                if ( nEspError != ESP_OK )
                {
                    vlogError( "processTransaction: esp_ota_write failed (%s)\n", esp_err_to_name( nEspError ) );
                }
                else
                {
                    m_bSuccess = true;
                }
            }
            reportDriverStatus( true );
        }
        break;

        case EkxProtocol::Payload::SwUpdateCmd::Finish:
        {
            sendCtrlUartReplyAck( u32MsgCounter, u8RepeatCounter );
            m_timerAsyncTransaction.startOnce( TIMEOUT_ASYNC_TRANSACTION_US );
        }
        break;

        case EkxProtocol::Payload::SwUpdateCmd::Abort:
        {
            sendCtrlUartReplyAck( u32MsgCounter, u8RepeatCounter );
            vlogWarning( "processTransaction: receive abort command" );
            m_bSuccess = true;
            reportDriverStatus( true );
        }
        break;

        case EkxProtocol::Payload::SwUpdateCmd::Reboot:
        {
            sendCtrlUartReplyAck( u32MsgCounter, u8RepeatCounter );
            m_timerAsyncTransaction.startOnce( TIMEOUT_ASYNC_TRANSACTION_US );
        }
        break;

        default:
        {
            sendCtrlUartReplyNack( u32MsgCounter, u8RepeatCounter, EkxProtocol::Payload::Nack::NackReason::WrongParameter );
            vlogError( "processTransaction: receive illegal command %d", static_cast<int>( pCommand->cmd().value() ) );
            m_bSuccess = false;
            reportDriverStatus( true );
        }
        break;
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

bool UcdSwUpdateTask::queueAckNackReceived( const uint32_t u32MsgCounter,
                                            const bool     bSuccess )
{
    return swUpdateQueue()->sendAckNackReceived( u32MsgCounter, bSuccess ) == pdPASS;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/
