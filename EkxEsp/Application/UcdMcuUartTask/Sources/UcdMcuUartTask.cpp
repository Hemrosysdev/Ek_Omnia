/*!*************************************************************************************************************************************************************
 *
 * @file UcdUartTask.cpp
 * @brief Source file of component unit UcdUartTask.
 *
 * This file was developed as part of svcUcdUart.
 *
 * @author Ultratronik GmbH
 *         Dornierstr. 9
 *         D-82205 Gilching
 *         http://www.ultratronik.de
 * @author Srdjan Nikolic, Entwicklung
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

#include "UcdMcuUartTask.h"

#include <cstring>
#include <driver/uart.h>
#include "UcdMcuUartQueue.h"

#include "UcdUartQueueTypes.h"
#include "EkxProtocol.h"
#include "EkxApplication.h"
#include "CtrlUartQueue.h"
#include "ApplicationGlobals.h"
#include "UartMcuWorkerTask.h"

/*!*************************************************************************************************************************************************************
 *
 **************************************************************************************************************************************************************/

//#define DEBUG_UART

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

UcdMcuUartTask::UcdMcuUartTask( UxEspCppLibrary::EspApplication * pApplication,
                                const uart_port_t                 nUcdUartNum,
                                const int                         nTxPin,
                                const int                         nRxPin,
                                const EkxProtocol::DriverId       u8DriverId,
                                const std::string &               strUcdName,
                                const std::string &               strTaskName,
                                const int                         nTaskStackSize,
                                const UBaseType_t                 uTaskPriority )
    : UartControlledDevice( strUcdName,
                            "Uart",
                            u8DriverId,
                            pApplication,
                            nTaskStackSize,
                            uTaskPriority,
                            strTaskName,
                            new UcdMcuUartQueue() )
    , m_nUcdUartNum( nUcdUartNum )
    , m_nTxPin( nTxPin )
    , m_nRxPin( nRxPin )
    , m_pUartWorker( new UartMcuWorkerTask( this, nTaskStackSize, uTaskPriority, strTaskName + "Worker" ) )
{
    logInfo( "constructor" );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

UcdMcuUartTask::~UcdMcuUartTask()
{
    delete m_pUartWorker;
    m_pUartWorker = nullptr;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

bool UcdMcuUartTask::startupStep( const FreeRtosQueueTask::startupStep_t nStartupStep )
{
    bool bContinueStartup = true;

    switch ( nStartupStep )
    {
        case UxEspCppLibrary::FreeRtosQueueTask::startupStep_t::startupStep1:
        {
            registerDriver();

            initUcdUart( 115200,
                         UART_DATA_8_BITS,
                         UART_PARITY_DISABLE,
                         UART_STOP_BITS_1 );
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

void UcdMcuUartTask::processTaskMessage( void )
{
    const UcdMcuUartQueueTypes::message * pMessage = reinterpret_cast<const UcdMcuUartQueueTypes::message *>( receiveMsg() );

    if ( pMessage )
    {
        switch ( pMessage->u32Type )
        {
            case UcdMcuUartQueueTypes::messageType::CtrlUartCommand:
            {
                processBaseCtrlUartCommand( pMessage->payload.ctrlUartCommand.m_u8DriverId,
                                            pMessage->payload.ctrlUartCommand.m_u32MsgCounter,
                                            pMessage->payload.ctrlUartCommand.m_u8RepeatCounter,
                                            pMessage->payload.ctrlUartCommand.m_pMessagePayload );
            }
            break;

            case UcdMcuUartQueueTypes::messageType::AckNackReceived:
            {
                processAckNackReceived( pMessage->payload.ackNackReceived.m_u32MsgCounter,
                                        pMessage->payload.ackNackReceived.m_bSuccess );
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

UcdMcuUartQueue * UcdMcuUartTask::mcuUartQueue( void )
{
    return dynamic_cast<UcdMcuUartQueue *>( taskQueue() );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

const UcdMcuUartQueue * UcdMcuUartTask::uartQueue( void ) const
{
    return dynamic_cast<const UcdMcuUartQueue *>( taskQueue() );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void UcdMcuUartTask::initUcdUart( const int                nBaudRate,
                                  const uart_word_length_t nDataBits,
                                  const uart_parity_t      nParity,
                                  const uart_stop_bits_t   nStopBits )
{
    if ( !m_pUartWorker->create( m_nUcdUartNum,
                                 m_nTxPin,
                                 m_nRxPin,
                                 nBaudRate,
                                 nDataBits,
                                 nParity,
                                 nStopBits ) )
    {
        vlogError( "initUcdUart: UART worker create failed" );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void UcdMcuUartTask::sendReplyToCtrlUart( const uint8_t * const pData,
                                          const uint32_t        u32Size )
{
    std::string strData( u32Size, 0 );
    std::memcpy( const_cast<char *>( strData.data() ), pData, u32Size );

    EkxProtocol::Payload::McuReply * pPayload = new EkxProtocol::Payload::McuReply();
    pPayload->reply().setValue( strData );

    sendCtrlUartReply( pPayload, true );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

bool UcdMcuUartTask::queueCtrlUartCommand( const EkxProtocol::DriverId                  u8DriverId,
                                           const uint32_t                               u32MsgCounter,
                                           const uint8_t                                u8RepeatCounter,
                                           EkxProtocol::Payload::MessagePayload * const pMessagePayload )
{
    return mcuUartQueue()->sendCtrlUartCommand( u8DriverId, u32MsgCounter, u8RepeatCounter, pMessagePayload ) == pdPASS;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void UcdMcuUartTask::writeMcuData( const std::string & strData )
{
#ifdef DEBUG_UART
    printf( "Write: size %d", strData.size() );
    for ( size_t i = 0; i < strData.size(); i++ )
    {
        printf( "%02x ", static_cast<int>( strData.data()[i] ) );
    }
    printf( "\n" );
#endif

    const int txBytes = uart_write_bytes( m_nUcdUartNum, strData.data(), strData.size() );

    ( void ) txBytes;   // usage of unused variable
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void UcdMcuUartTask::processCtrlUartCommand( const EkxProtocol::DriverId                        u8DriverId,
                                             const uint32_t                                     u32MsgCounter,
                                             const uint8_t                                      u8RepeatCounter,
                                             const EkxProtocol::Payload::MessagePayload * const pMessagePayload )
{
    const EkxProtocol::Payload::McuCommand * const pPayload = dynamic_cast<const EkxProtocol::Payload::McuCommand *>( pMessagePayload );

    if ( pPayload )
    {
        sendCtrlUartReplyAck( u32MsgCounter, u8RepeatCounter );
        writeMcuData( pPayload->command().value() );
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

bool UcdMcuUartTask::queueAckNackReceived( const uint32_t u32MsgCounter,
                                           const bool     bSuccess )
{
    return mcuUartQueue()->sendAckNackReceived( u32MsgCounter, bSuccess ) == pdPASS;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/
