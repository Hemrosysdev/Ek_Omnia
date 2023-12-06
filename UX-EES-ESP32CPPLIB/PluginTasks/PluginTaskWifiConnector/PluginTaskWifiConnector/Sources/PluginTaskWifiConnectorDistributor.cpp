/*!************************************************************************************************************************************************************
 *
 * @file PluginTaskWifiConnectorDistributor.cpp
 * 
 * This file was developed as part of UX Extended Eco System
 *
 * @brief Implementation file of class PluginTaskWifiConnectorDistributor.
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

#include "PluginTaskWifiConnectorDistributor.h"
#include "FreeRtosQueue.h"
#include "PluginTaskWifiConnectorTypes.h"
#include "PluginTaskWifiConnector.h"
#include "PluginTaskWifiConnectorQueueTypes.h"
#include "UxLibCommon.h"

#include <cstring>

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

namespace UxEspCppLibrary
{

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

PluginTaskWifiConnectorDistributor::PluginTaskWifiConnectorDistributor()
                                                                : PluginTaskDistributor()
                                                                  {
                                                                  }

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

PluginTaskWifiConnectorDistributor::~PluginTaskWifiConnectorDistributor()
{
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

BaseType_t PluginTaskWifiConnectorDistributor::sendWifiStatus( const PluginTaskWifiConnectorTypes::WifiStatus nStatus )
{
    BaseType_t nReturn = pdFALSE;

    if ( queueItemBuffer() )
    {
        nReturn = pdTRUE;

        PluginTaskWifiConnectorDistributorQueueTypes::message_t * pMessage = reinterpret_cast<PluginTaskWifiConnectorDistributorQueueTypes::message_t *>( queueItemBuffer() );

        pMessage->u32Type = PluginTaskWifiConnectorDistributorQueueTypes::messageType_t::Status;
        pMessage->payload.statusPayload.m_nStatus  = nStatus;

        for ( int i = 0; i < consumerQueues().size(); i++ )
        {
            if ( consumerQueues()[i]->send( pMessage ) != pdTRUE )
            {
                nReturn = pdFALSE;
            }
        }
    }

    return nReturn;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

BaseType_t PluginTaskWifiConnectorDistributor::sendWifiEventApStart( void )
{
    BaseType_t nReturn = pdFALSE;

    if ( queueItemBuffer() )
    {
        nReturn = pdTRUE;

        for ( int i = 0; i < consumerQueues().size(); i++ )
        {
            if ( consumerQueues()[i]->sendSimpleEvent( static_cast<uint32_t>( PluginTaskWifiConnectorDistributorQueueTypes::messageType_t::WifiEventApStart ) ) != pdTRUE )
            {
                nReturn = pdFALSE;
            }
        }
    }

    return nReturn;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

BaseType_t PluginTaskWifiConnectorDistributor::sendWifiEventApStop( void )
{
    BaseType_t nReturn = pdFALSE;

    if ( queueItemBuffer() )
    {
        nReturn = pdTRUE;

        for ( int i = 0; i < consumerQueues().size(); i++ )
        {
            if ( consumerQueues()[i]->sendSimpleEvent( static_cast<uint32_t>( PluginTaskWifiConnectorDistributorQueueTypes::messageType_t::WifiEventApStop ) ) != pdTRUE )
            {
                nReturn = pdFALSE;
            }
        }
    }

    return nReturn;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

BaseType_t PluginTaskWifiConnectorDistributor::sendWifiEventStaStart( void )
{
    BaseType_t nReturn = pdFALSE;

    if ( queueItemBuffer() )
    {
        nReturn = pdTRUE;

        for ( int i = 0; i < consumerQueues().size(); i++ )
        {
            if ( consumerQueues()[i]->sendSimpleEvent( static_cast<uint32_t>( PluginTaskWifiConnectorDistributorQueueTypes::messageType_t::WifiEventStaStart ) ) != pdTRUE )
            {
                nReturn = pdFALSE;
            }
        }
    }

    return nReturn;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

BaseType_t PluginTaskWifiConnectorDistributor::sendWifiEventStaStop( void )
{
    BaseType_t nReturn = pdFALSE;

    if ( queueItemBuffer() )
    {
        nReturn = pdTRUE;

        for ( int i = 0; i < consumerQueues().size(); i++ )
        {
            if ( consumerQueues()[i]->sendSimpleEvent( static_cast<uint32_t>( PluginTaskWifiConnectorDistributorQueueTypes::messageType_t::WifiEventStaStop ) ) != pdTRUE )
            {
                nReturn = pdFALSE;
            }
        }
    }

    return nReturn;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

BaseType_t PluginTaskWifiConnectorDistributor::sendWifiEventStaConnected( void )
{
    BaseType_t nReturn = pdFALSE;

    if ( queueItemBuffer() )
    {
        nReturn = pdTRUE;

        for ( int i = 0; i < consumerQueues().size(); i++ )
        {
            if ( consumerQueues()[i]->sendSimpleEvent( static_cast<uint32_t>( PluginTaskWifiConnectorDistributorQueueTypes::messageType_t::WifiEventStaConnected ) ) != pdTRUE )
            {
                nReturn = pdFALSE;
            }
        }
    }

    return nReturn;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

BaseType_t PluginTaskWifiConnectorDistributor::sendWifiEventStaDisconnected( void )
{
    BaseType_t nReturn = pdFALSE;

    if ( queueItemBuffer() )
    {
        nReturn = pdTRUE;

        for ( int i = 0; i < consumerQueues().size(); i++ )
        {
            if ( consumerQueues()[i]->sendSimpleEvent( static_cast<uint32_t>( PluginTaskWifiConnectorDistributorQueueTypes::messageType_t::WifiEventStaDisconnected ) ) != pdTRUE )
            {
                nReturn = pdFALSE;
            }
        }
    }

    return nReturn;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

BaseType_t PluginTaskWifiConnectorDistributor::sendWifiEventScanDone( void )
{
    BaseType_t nReturn = pdFALSE;

    if ( queueItemBuffer() )
    {
        nReturn = pdTRUE;

        for ( int i = 0; i < consumerQueues().size(); i++ )
        {
            if ( consumerQueues()[i]->sendSimpleEvent( static_cast<uint32_t>( PluginTaskWifiConnectorDistributorQueueTypes::messageType_t::WifiEventScanDone ) ) != pdTRUE )
            {
                nReturn = pdFALSE;
            }
        }
    }

    return nReturn;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

BaseType_t PluginTaskWifiConnectorDistributor::sendWifiScanResults( const bool bSuccess,
                                                                    const uint16_t u16Number,
                                                                    wifi_ap_record_t * const pApRecordList )
{
    BaseType_t nReturn = pdFALSE;

    if ( queueItemBuffer() )
    {
        nReturn = pdTRUE;

        PluginTaskWifiConnectorDistributorQueueTypes::message_t * pMessage = reinterpret_cast<PluginTaskWifiConnectorDistributorQueueTypes::message_t *>( queueItemBuffer() );

        pMessage->u32Type = PluginTaskWifiConnectorDistributorQueueTypes::messageType_t::WifiScanResults;
        pMessage->payload.scanResultsPayload.m_bSuccess = bSuccess;
        pMessage->payload.scanResultsPayload.m_u16Number = u16Number;
        pMessage->payload.scanResultsPayload.m_pApRecordList = pApRecordList;

        for ( int i = 0; i < consumerQueues().size(); i++ )
        {
            if ( consumerQueues()[i]->send( pMessage ) != pdTRUE )
            {
                nReturn = pdFALSE;
            }
        }
    }

    return nReturn;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

BaseType_t PluginTaskWifiConnectorDistributor::sendWifiEventApStaConnected( const std::string &strMac )
{
    BaseType_t nReturn = pdFALSE;

    if ( queueItemBuffer() )
    {
        nReturn = pdTRUE;

        PluginTaskWifiConnectorDistributorQueueTypes::message_t * pMessage = reinterpret_cast<PluginTaskWifiConnectorDistributorQueueTypes::message_t *>( queueItemBuffer() );

        pMessage->u32Type = PluginTaskWifiConnectorDistributorQueueTypes::messageType_t::WifiEventApStaConnected;
        UxEspCppLibrary::UxLibCommon::saveStringCopy( pMessage->payload.apStaConnectedPayload.m_szMac,
                                                      strMac.c_str(),
                                                      sizeof( pMessage->payload.apStaConnectedPayload.m_szMac ) );

        for ( int i = 0; i < consumerQueues().size(); i++ )
        {
            if ( consumerQueues()[i]->send( pMessage ) != pdTRUE )
            {
                nReturn = pdFALSE;
            }
        }
    }

    return nReturn;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

BaseType_t PluginTaskWifiConnectorDistributor::sendWifiEventApStaDisconnected( const std::string &strMac )
{
    BaseType_t nReturn = pdFALSE;

    if ( queueItemBuffer() )
    {
        nReturn = pdTRUE;

        PluginTaskWifiConnectorDistributorQueueTypes::message_t * pMessage = reinterpret_cast<PluginTaskWifiConnectorDistributorQueueTypes::message_t *>( queueItemBuffer() );

        pMessage->u32Type = PluginTaskWifiConnectorDistributorQueueTypes::messageType_t::WifiEventApStaDisconnected;
        UxEspCppLibrary::UxLibCommon::saveStringCopy( pMessage->payload.apStaDisconnectedPayload.m_szMac,
                                                      strMac.c_str(),
                                                      sizeof( pMessage->payload.apStaDisconnectedPayload.m_szMac ) );

        for ( int i = 0; i < consumerQueues().size(); i++ )
        {
            if ( consumerQueues()[i]->send( pMessage ) != pdTRUE )
            {
                nReturn = pdFALSE;
            }
        }
    }

    return nReturn;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

BaseType_t PluginTaskWifiConnectorDistributor::sendWifiEventStaGotIp( const std::string &strIp )
{
    BaseType_t nReturn = pdFALSE;

    if ( queueItemBuffer() )
    {
        nReturn = pdTRUE;

        PluginTaskWifiConnectorDistributorQueueTypes::message_t * pMessage = reinterpret_cast<PluginTaskWifiConnectorDistributorQueueTypes::message_t *>( queueItemBuffer() );

        pMessage->u32Type = PluginTaskWifiConnectorDistributorQueueTypes::messageType_t::WifiEventStaGotIp;
        UxEspCppLibrary::UxLibCommon::saveStringCopy( pMessage->payload.staGotIpPayload.m_szIp,
                                                      strIp.c_str(),
                                                      sizeof( pMessage->payload.staGotIpPayload.m_szIp ) );

        for ( int i = 0; i < consumerQueues().size(); i++ )
        {
            if ( consumerQueues()[i]->send( pMessage ) != pdTRUE )
            {
                nReturn = pdFALSE;
            }
        }
    }

    return nReturn;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

BaseType_t PluginTaskWifiConnectorDistributor::sendWifiInfo( const std::string &strApIpv4,
                                                             const std::string &strApMac,
                                                             const std::string &strStaIpv4,
                                                             const std::string &strStaMac )
{
    BaseType_t nReturn = pdFALSE;

    if ( queueItemBuffer() )
    {
        nReturn = pdTRUE;

        PluginTaskWifiConnectorDistributorQueueTypes::message_t * pMessage = reinterpret_cast<PluginTaskWifiConnectorDistributorQueueTypes::message_t *>( queueItemBuffer() );

        pMessage->u32Type = PluginTaskWifiConnectorDistributorQueueTypes::messageType_t::WifiInfo;
        UxEspCppLibrary::UxLibCommon::saveStringCopy( pMessage->payload.infoPayload.m_szApIpv4,
                                                      strApIpv4.c_str(),
                                                      sizeof( pMessage->payload.infoPayload.m_szApIpv4 ) );
        UxEspCppLibrary::UxLibCommon::saveStringCopy( pMessage->payload.infoPayload.m_szApMac,
                                                      strApMac.c_str(),
                                                      sizeof( pMessage->payload.infoPayload.m_szApMac ) );
        UxEspCppLibrary::UxLibCommon::saveStringCopy( pMessage->payload.infoPayload.m_szStaIpv4,
                                                      strStaIpv4.c_str(),
                                                      sizeof( pMessage->payload.infoPayload.m_szStaIpv4 ) );
        UxEspCppLibrary::UxLibCommon::saveStringCopy( pMessage->payload.infoPayload.m_szStaMac,
                                                      strStaMac.c_str(),
                                                      sizeof( pMessage->payload.infoPayload.m_szStaMac ) );

        for ( int i = 0; i < consumerQueues().size(); i++ )
        {
            if ( consumerQueues()[i]->send( pMessage ) != pdTRUE )
            {
                nReturn = pdFALSE;
            }
        }
    }

    return nReturn;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

} // namespace UxEspCppLibrary
