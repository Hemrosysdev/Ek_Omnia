/*!*************************************************************************************************************************************************************
 *
 * @file UcdEepromTask.cpp
 * @brief Source file of component unit Eeprom-Task.
 *
 * This file was developed as part of  SCD  holder of PluginTaskAt24c16.
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

#include "UcdEepromTask.h"

#include "PluginTaskAt24c16.h"
#include "PluginTaskAt24c16Queue.h"

#include <string.h>
#include "UcdEepromQueue.h"
#include "UcdEepromQueueTypes.h"
#include "EkxApplication.h"
#include "EkxProtocol.h"

/***************************************************************************************************************************************************************
 *
 **************************************************************************************************************************************************************/

UcdEepromTask::UcdEepromTask( UxEspCppLibrary::EspApplication *    pApplication,
                              UxEspCppLibrary::PluginTaskAt24c16 * pPluginTaskAt24c16,
                              const EkxProtocol::DriverId          u8DriverId,
                              const int                            nTaskStackSize,
                              const UBaseType_t                    uTaskPriority )
    : UartControlledDevice( "At24c16",
                            "At24c16",
                            u8DriverId,
                            pApplication,
                            nTaskStackSize,
                            uTaskPriority,
                            "UcdEepromTask",
                            new UcdEepromQueue( pPluginTaskAt24c16->requiredConsumerQueueItemSize() ) )
    , UxEspCppLibrary::PluginTaskAt24c16Consumer( pPluginTaskAt24c16,
                                                  this->taskQueue() )
    , m_bValid( false )
    , m_strProductNo( "Uninitialized" )
    , m_strSerialNo( "XXXXXX" )
{

}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

UcdEepromTask::~UcdEepromTask()
{

}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

bool UcdEepromTask::startupStep( const FreeRtosQueueTask::startupStep_t nStartupStep )
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
        }
        break;
    }

    return bContinueStartup;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void UcdEepromTask::processTaskMessage( void )
{
    const uint8_t * pMsg = receiveMsg();

    if ( !PluginTaskAt24c16Consumer::processTaskMessage( pMsg ) )
    {
        const UcdEepromQueueTypes::message * pMessage = reinterpret_cast<const UcdEepromQueueTypes::message *>( pMsg );

        if ( pMessage )
        {
            switch ( pMessage->u32Type )
            {
                case UcdEepromQueueTypes::messageType::CtrlUartCommand:
                {
                    processBaseCtrlUartCommand( pMessage->payload.ctrlUartCommand.m_u8DriverId,
                                                pMessage->payload.ctrlUartCommand.m_u32MsgCounter,
                                                pMessage->payload.ctrlUartCommand.m_u8RepeatCounter,
                                                pMessage->payload.ctrlUartCommand.m_pMessagePayload );
                }
                break;

                case UcdEepromQueueTypes::messageType::AckNackReceived:
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
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

UcdEepromQueue * UcdEepromTask::eepromQueue( void )
{
    return dynamic_cast<UcdEepromQueue *>( taskQueue() );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

const UcdEepromQueue * UcdEepromTask::eepromQueue( void ) const
{
    return dynamic_cast<const UcdEepromQueue *>( taskQueue() );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void UcdEepromTask::processEepromData( const bool          bValid,
                                       const std::string & strProductNo,
                                       const std::string & strSerialNo )
{
    m_bValid = bValid;

    m_strProductNo = strProductNo;
    m_strSerialNo  = strSerialNo;

    vlogInfo( "processEepromData: Valid:%d %s %s", m_bValid, m_strProductNo.c_str(), m_strSerialNo.c_str() );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void UcdEepromTask::reportDriverStatus( const bool bFirst )
{
    //vlogInfo( "reportDriverStatus" );

    EkxProtocol::Payload::EepromStatus * pPayload = new EkxProtocol::Payload::EepromStatus();
    pPayload->valid().setValue( m_bValid );
    pPayload->productNo().setValue( m_strProductNo );
    pPayload->serialNo().setValue( m_strSerialNo );

    sendCtrlUartReply( pPayload, bFirst );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void UcdEepromTask::processCtrlUartCommand( const EkxProtocol::DriverId                        u8DriverId,
                                            const uint32_t                                     u32MsgCounter,
                                            const uint8_t                                      u8RepeatCounter,
                                            const EkxProtocol::Payload::MessagePayload * const pMessagePayload )
{
    const EkxProtocol::Payload::EepromFlash * pPayload = dynamic_cast<const EkxProtocol::Payload::EepromFlash *>( pMessagePayload );

    if ( pPayload )
    {
        if ( pPayload->productNo().value().size()
             && pPayload->serialNo().value().size() )
        {
            ekxApp().pluginTaskAt24c16().pluginTaskAt24c16Queue()->sendWriteProductNo( pPayload->productNo().value() );
            ekxApp().pluginTaskAt24c16().pluginTaskAt24c16Queue()->sendWriteSerialNo( pPayload->serialNo().value() );

            sendCtrlUartReplyAck( u32MsgCounter, u8RepeatCounter );
        }
        else
        {
            vlogError( "processCtrlUartCommand: cant't flash data; product no or serial no not given in payload" );
            sendCtrlUartReplyNack( u32MsgCounter, u8RepeatCounter, EkxProtocol::Payload::Nack::NackReason::WrongParameter );
        }
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

bool UcdEepromTask::queueCtrlUartCommand( const EkxProtocol::DriverId                  u8DriverId,
                                          const uint32_t                               u32MsgCounter,
                                          const uint8_t                                u8RepeatCounter,
                                          EkxProtocol::Payload::MessagePayload * const pMessagePayload )
{
    return eepromQueue()->sendCtrlUartCommand( u8DriverId, u32MsgCounter, u8RepeatCounter, pMessagePayload ) == pdPASS;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

bool UcdEepromTask::queueAckNackReceived( const uint32_t u32MsgCounter,
                                          const bool     bSuccess )
{
    return eepromQueue()->sendAckNackReceived( u32MsgCounter, bSuccess ) == pdPASS;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/
