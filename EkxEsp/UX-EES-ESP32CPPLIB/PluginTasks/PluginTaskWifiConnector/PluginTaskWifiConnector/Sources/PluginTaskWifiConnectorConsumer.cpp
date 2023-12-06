/*!************************************************************************************************************************************************************
 *
 * @file PluginTaskWifiConnectorConsumer.cpp
 *
 * This file was developed as part of UX Extended Eco System
 *
 * @brief Implementation file of class PluginTaskWifiConnectorConsumer.
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

#include "PluginTaskWifiConnectorConsumer.h"

#include "PluginTaskWifiConnectorQueueTypes.h"
#include <esp_wifi_types.h>

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

namespace UxEspCppLibrary
{

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

PluginTaskWifiConnectorConsumer::PluginTaskWifiConnectorConsumer( PluginTask * const    pPluginTask,
                                                                  FreeRtosQueue * const pConsumerQueue )
    : PluginTaskConsumer( pPluginTask,
                          pConsumerQueue )
{
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

PluginTaskWifiConnectorConsumer::~PluginTaskWifiConnectorConsumer()
{
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

bool PluginTaskWifiConnectorConsumer::processTaskMessage( const uint8_t * pReceiveMsg )
{
    bool bProcessed = false;

    const PluginTaskWifiConnectorDistributorQueueTypes::message_t * pMessage = reinterpret_cast<const PluginTaskWifiConnectorDistributorQueueTypes::message_t *>( pReceiveMsg );

    if ( pMessage )
    {
        switch ( pMessage->u32Type )
        {
            case PluginTaskWifiConnectorDistributorQueueTypes::messageType_t::Status:
            {
                processWifiStatus( pMessage->payload.statusPayload.m_nStatus );
                bProcessed = true;
            }
            break;

            case PluginTaskWifiConnectorDistributorQueueTypes::messageType_t::WifiEventApStart:
            {
                processWifiEventApStart();
                bProcessed = true;
            }
            break;

            case PluginTaskWifiConnectorDistributorQueueTypes::messageType_t::WifiEventApStop:
            {
                processWifiEventApStop();
                bProcessed = true;
            }
            break;

            case PluginTaskWifiConnectorDistributorQueueTypes::messageType_t::WifiEventApStaConnected:
            {
                processWifiEventApStaConnected( pMessage->payload.apStaConnectedPayload.m_szMac );
                bProcessed = true;
            }
            break;

            case PluginTaskWifiConnectorDistributorQueueTypes::messageType_t::WifiEventApStaDisconnected:
            {
                processWifiEventApStaDisconnected( pMessage->payload.apStaDisconnectedPayload.m_szMac );
                bProcessed = true;
            }
            break;

            case PluginTaskWifiConnectorDistributorQueueTypes::messageType_t::WifiEventStaStart:
            {
                processWifiEventStaStart();
                bProcessed = true;
            }
            break;

            case PluginTaskWifiConnectorDistributorQueueTypes::messageType_t::WifiEventStaStop:
            {
                processWifiEventStaStop();
                bProcessed = true;
            }
            break;

            case PluginTaskWifiConnectorDistributorQueueTypes::messageType_t::WifiEventStaConnected:
            {
                processWifiEventStaConnected();
                bProcessed = true;
            }
            break;

            case PluginTaskWifiConnectorDistributorQueueTypes::messageType_t::WifiEventStaDisconnected:
            {
                processWifiEventStaDisconnected();
                bProcessed = true;
            }
            break;

            case PluginTaskWifiConnectorDistributorQueueTypes::messageType_t::WifiEventScanDone:
            {
                processWifiEventScanDone();
                bProcessed = true;
            }
            break;

            case PluginTaskWifiConnectorDistributorQueueTypes::messageType_t::WifiEventStaGotIp:
            {
                processWifiEventStaGotIp( pMessage->payload.staGotIpPayload.m_szIp );
                bProcessed = true;
            }
            break;

            case PluginTaskWifiConnectorDistributorQueueTypes::messageType_t::WifiInfo:
            {
                processWifiInfo( pMessage->payload.infoPayload.m_szApIpv4,
                                 pMessage->payload.infoPayload.m_szApMac,
                                 pMessage->payload.infoPayload.m_szStaIpv4,
                                 pMessage->payload.infoPayload.m_szStaMac );
                bProcessed = true;
            }
            break;

            case PluginTaskWifiConnectorDistributorQueueTypes::messageType_t::WifiScanResults:
            {
                processWifiScanResults( pMessage->payload.scanResultsPayload.m_bSuccess,
                                        pMessage->payload.scanResultsPayload.m_u16Number,
                                        pMessage->payload.scanResultsPayload.m_pApRecordList );
                bProcessed = true;
            }
            break;

            default:
            {
                // no message for this plugin
            }
            break;
        }
    }

    return bProcessed;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void PluginTaskWifiConnectorConsumer::processWifiStatus( const PluginTaskWifiConnectorTypes::WifiStatus nStatus )
{
    // do nothing; work will be done in overloaded function
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void PluginTaskWifiConnectorConsumer::processWifiEventApStart( void )
{
    // do nothing; work will be done in overloaded function
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void PluginTaskWifiConnectorConsumer::processWifiEventApStop( void )
{
    // do nothing; work will be done in overloaded function
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void PluginTaskWifiConnectorConsumer::processWifiEventApStaConnected( const std::string & strMac )
{
    // do nothing; work will be done in overloaded function
    ( void ) strMac;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void PluginTaskWifiConnectorConsumer::processWifiEventApStaDisconnected( const std::string & strMac )
{
    // do nothing; work will be done in overloaded function
    ( void ) strMac;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void PluginTaskWifiConnectorConsumer::processWifiEventStaStart( void )
{
    // do nothing; work will be done in overloaded function
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void PluginTaskWifiConnectorConsumer::processWifiEventStaStop( void )
{
    // do nothing; work will be done in overloaded function
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void PluginTaskWifiConnectorConsumer::processWifiEventStaConnected( void )
{
    // do nothing; work will be done in overloaded function
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void PluginTaskWifiConnectorConsumer::processWifiEventStaDisconnected( void )
{
    // do nothing; work will be done in overloaded function
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void PluginTaskWifiConnectorConsumer::processWifiEventScanDone( void )
{
    // do nothing; work will be done in overloaded function
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void PluginTaskWifiConnectorConsumer::processWifiEventStaGotIp( const std::string & strIp )
{
    // do nothing; work will be done in overloaded function
    ( void ) strIp;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void PluginTaskWifiConnectorConsumer::processWifiInfo( const std::string & strApIpv4,
                                                       const std::string & strApMac,
                                                       const std::string & strStaIpv4,
                                                       const std::string & strStaMac )
{
    // do nothing; work will be done in overloaded function
    ( void ) strApIpv4;
    ( void ) strApMac;
    ( void ) strStaIpv4;
    ( void ) strStaMac;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void PluginTaskWifiConnectorConsumer::processWifiScanResults( const bool               bSuccess,
                                                              const uint16_t           u16Number,
                                                              wifi_ap_record_t * const pApRecordList )
{
    // do nothing; work will be done in overloaded function
    ( void ) bSuccess;
    ( void ) u16Number;
    ( void ) pApRecordList;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

} // namespace UxEspCppLibrary
