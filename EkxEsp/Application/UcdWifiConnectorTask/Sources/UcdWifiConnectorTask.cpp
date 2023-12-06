/*
 * UcdWifiConnectorTask.cpp
 *
 *  Created on: 21.10.2019
 *      Author: gesser
 */
#include "UcdWifiConnectorTask.h"

#include <arpa/inet.h>

#include <sstream>
#include <sys/stat.h>
#include <cstring>

#include <esp_system.h>
#include <esp_wifi.h>
#include <esp_event.h>

#include "ApplicationGlobals.h"
#include "UcdWifiConnectorQueue.h"
#include "UcdWifiConnectorQueueTypes.h"
#include "EkxApplication.h"
#include "PluginTaskWifiConnectorQueue.h"
#include "EkxProtocol.h"

/*!************************************************************************************************************************************************************
 *   Defines
 *************************************************************************************************************************************************************/

//#define CONFIG_TEST_ACCESS_POINT_PERMANENT

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

UcdWifiConnectorTask::UcdWifiConnectorTask( UxEspCppLibrary::EspApplication * pApplication )
    : UartControlledDevice( "Wifi",
                            "Wifi",
                            EkxProtocol::DriverId::WifiDriver,
                            pApplication,
                            ApplicationGlobals::c_nWifiConnectorTaskStackSize,
                            ApplicationGlobals::c_nWifiConnectorTaskPriority,
                            "UcdWifiConnectorTask",
                            new UcdWifiConnectorQueue() )
    , m_connectionTimeout( taskQueue(),
                           static_cast<uint32_t>( UcdWifiConnectorQueueTypes::messageType::ConnectionTimeout ) )
{
    logInfo( "constructor" );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

UcdWifiConnectorTask::~UcdWifiConnectorTask()
{
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

bool UcdWifiConnectorTask::startupStep( const FreeRtosQueueTask::startupStep_t nStartupStep )
{
    bool bContinueStartup = true;

    switch ( nStartupStep )
    {
        case UxEspCppLibrary::FreeRtosQueueTask::startupStep_t::startupStep9:
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

void UcdWifiConnectorTask::processTaskMessage( void )
{
    const UcdWifiConnectorQueueTypes::message * pMessage = reinterpret_cast<const UcdWifiConnectorQueueTypes::message *>( receiveMsg() );

    if ( pMessage )
    {
        switch ( pMessage->u32Type )
        {
            case UcdWifiConnectorQueueTypes::messageType::CtrlUartCommand:
            {
                processBaseCtrlUartCommand( pMessage->payload.ctrlUartCommand.m_u8DriverId,
                                            pMessage->payload.ctrlUartCommand.m_u32MsgCounter,
                                            pMessage->payload.ctrlUartCommand.m_u8RepeatCounter,
                                            pMessage->payload.ctrlUartCommand.m_pMessagePayload );
            }
            break;

            case UcdWifiConnectorQueueTypes::messageType::AckNackReceived:
            {
                processAckNackReceived( pMessage->payload.ackNackReceived.m_u32MsgCounter,
                                        pMessage->payload.ackNackReceived.m_bSuccess );
            }
            break;

            case UcdWifiConnectorQueueTypes::messageType::Alive:
            {
                processBaseAlive( pMessage->payload.aliveStatus.m_bAlive );
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

void UcdWifiConnectorTask::processAlive( const bool bAlive )
{
    if ( !bAlive )
    {
        ekxApp().pluginTaskWifiConnector().pluginTaskWifiConnectorQueue()->sendWifiOff();
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void UcdWifiConnectorTask::setWifiConfig( const bool                                                    bActive,
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
                                          const std::string &                                           strStaStaticDns )
{
//    vlogInfo( "setWifiConfig: Mode %d On %d SSID %s PW %s",
//              static_cast<int>(nWifiMode),
//              static_cast<int>(bActive),
//              strApSsid.c_str(),
//              strApPassword.c_str() );

    ekxApp().pluginTaskWifiConnector().pluginTaskWifiConnectorQueue()->sendWifiConfig( nWifiMode,
                                                                                       strApSsid,
                                                                                       strApPassword,
                                                                                       strApIp,
                                                                                       strStaSsid,
                                                                                       strStaPassword,
                                                                                       bStaDhcp,
                                                                                       strStaStaticIp,
                                                                                       strStaStaticGateway,
                                                                                       strStaStaticNetmask,
                                                                                       strStaStaticDns );

    if ( bActive )
    {
        ekxApp().pluginTaskWifiConnector().pluginTaskWifiConnectorQueue()->sendWifiOn();
    }
    else
    {
        ekxApp().pluginTaskWifiConnector().pluginTaskWifiConnectorQueue()->sendWifiOff();
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void UcdWifiConnectorTask::reportDriverStatus( const bool bFirst )
{
    //vlogInfo( "reportDriverStatus" );

    EkxProtocol::Payload::WifiStatus * pPayload = new EkxProtocol::Payload::WifiStatus();
    pPayload->valid().setValue( true );
    pPayload->apIp().setValue( "" );
    pPayload->apMac().setValue( "" );
    pPayload->staIp().setValue( "" );
    pPayload->staMac().setValue( "" );

    sendCtrlUartReply( pPayload, bFirst );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

UcdWifiConnectorQueue * UcdWifiConnectorTask::wifiConnectorQueue( void )
{
    return dynamic_cast<UcdWifiConnectorQueue *>( taskQueue() );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

const UcdWifiConnectorQueue * UcdWifiConnectorTask::wifiConnectorQueue( void ) const
{
    return dynamic_cast<const UcdWifiConnectorQueue *>( taskQueue() );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

bool UcdWifiConnectorTask::queueCtrlUartCommand( const EkxProtocol::DriverId                  u8DriverId,
                                                 const uint32_t                               u32MsgCounter,
                                                 const uint8_t                                u8RepeatCounter,
                                                 EkxProtocol::Payload::MessagePayload * const pMessagePayload )
{
    return wifiConnectorQueue()->sendCtrlUartCommand( u8DriverId, u32MsgCounter, u8RepeatCounter, pMessagePayload ) == pdPASS;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

bool UcdWifiConnectorTask::queueAlive( const bool bAlive )
{
    return wifiConnectorQueue()->sendAlive( bAlive );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void UcdWifiConnectorTask::processCtrlUartCommand( const EkxProtocol::DriverId                        u8DriverId,
                                                   const uint32_t                                     u32MsgCounter,
                                                   const uint8_t                                      u8RepeatCounter,
                                                   const EkxProtocol::Payload::MessagePayload * const pMessagePayload )
{
    const EkxProtocol::Payload::WifiConfig * const pPayload = dynamic_cast<const EkxProtocol::Payload::WifiConfig *>( pMessagePayload );

    if ( pPayload )
    {
        bool                                                    bActive   = false;
        UxEspCppLibrary::PluginTaskWifiConnectorTypes::WifiMode nWifiMode = UxEspCppLibrary::PluginTaskWifiConnectorTypes::WifiMode::Ap;

        switch ( pPayload->wifiMode().value() )
        {
            case EkxProtocol::Payload::WifiMode::Off:
                bActive   = false;
                nWifiMode = UxEspCppLibrary::PluginTaskWifiConnectorTypes::WifiMode::Ap;
                break;
            case EkxProtocol::Payload::WifiMode::Ap:
                bActive   = true;
                nWifiMode = UxEspCppLibrary::PluginTaskWifiConnectorTypes::WifiMode::Ap;
                break;
            case EkxProtocol::Payload::WifiMode::ApSta:
                bActive   = true;
                nWifiMode = UxEspCppLibrary::PluginTaskWifiConnectorTypes::WifiMode::Both;
                break;
            case EkxProtocol::Payload::WifiMode::Sta:
                bActive   = true;
                nWifiMode = UxEspCppLibrary::PluginTaskWifiConnectorTypes::WifiMode::Sta;
                break;
            case EkxProtocol::Payload::WifiMode::Last:
            default:
                bActive   = false;
                nWifiMode = UxEspCppLibrary::PluginTaskWifiConnectorTypes::WifiMode::Ap;
                break;
        }

        setWifiConfig( bActive,
                       nWifiMode,
                       pPayload->apSsid().value(),
                       pPayload->apPassword().value(),
                       pPayload->apIp().value(),
                       pPayload->staSsid().value(),
                       pPayload->staPassword().value(),
                       pPayload->staDhcp().value(),
                       pPayload->staStaticIp().value(),
                       pPayload->staStaticGateway().value(),
                       pPayload->staStaticNetmask().value(),
                       pPayload->staStaticDns().value() );

        sendCtrlUartReplyAck( u32MsgCounter, u8RepeatCounter );
    }
    else
    {
        vlogError( "processCtrlUartCommand: cant't process data; unknown command" );
        sendCtrlUartReplyNack( u32MsgCounter, u8RepeatCounter, EkxProtocol::Payload::Nack::NackReason::PayloadBroken );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

bool UcdWifiConnectorTask::queueAckNackReceived( const uint32_t u32MsgCounter,
                                                 const bool     bSuccess )
{
    return wifiConnectorQueue()->sendAckNackReceived( u32MsgCounter, bSuccess ) == pdPASS;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/
