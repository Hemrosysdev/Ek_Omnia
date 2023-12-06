/*!************************************************************************************************************************************************************
 *
 * @file PluginTaskWifiConnectorQueue.cpp
 *
 * This file was developed as part of UX Extended Eco System
 *
 * @brief Implementation file of class PluginTaskWifiConnectorQueue.
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

#include "PluginTaskWifiConnectorQueue.h"

#include <string.h>

#include "FreeRtosQueueTask.h"
#include "PluginTaskWifiConnectorQueueTypes.h"
#include "UxLibCommon.h"

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

namespace UxEspCppLibrary
{

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

PluginTaskWifiConnectorQueue::PluginTaskWifiConnectorQueue()
    : FreeRtosQueue( 10,
                     FreeRtosQueueTask::minTargetQueueSize( sizeof( PluginTaskWifiConnectorQueueTypes::message ) ),
                     "PluginTaskWifiConnectorQueue" )
{

}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

PluginTaskWifiConnectorQueue::~PluginTaskWifiConnectorQueue()
{
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

BaseType_t PluginTaskWifiConnectorQueue::sendWifiConfig( const UxEspCppLibrary::PluginTaskWifiConnectorTypes::WifiMode nWifiMode,
                                                         const std::string &                                           strApSsid,
                                                         const std::string &                                           strApPassword,
                                                         const std::string &                                           strApIpv4,
                                                         const std::string &                                           strStaSsid,
                                                         const std::string &                                           strStaPassword,
                                                         const bool                                                    bStaDhcp,
                                                         const std::string &                                           strStaStaticIpv4,
                                                         const std::string &                                           strStaStaticGateway,
                                                         const std::string &                                           strStaStaticNetmask,
                                                         const std::string &                                           strStaStaticDns )
{
    PluginTaskWifiConnectorQueueTypes::message wifiConfigMessage;

    wifiConfigMessage.u32Type = PluginTaskWifiConnectorQueueTypes::messageType::WifiConfig;

    UxEspCppLibrary::UxLibCommon::saveStringCopy( wifiConfigMessage.payload.wifiConfigPayload.pszApSsid,
                                                  strApSsid.c_str(),
                                                  sizeof( wifiConfigMessage.payload.wifiConfigPayload.pszApSsid ) );

    UxEspCppLibrary::UxLibCommon::saveStringCopy( wifiConfigMessage.payload.wifiConfigPayload.pszApPassword,
                                                  strApPassword.c_str(),
                                                  sizeof( wifiConfigMessage.payload.wifiConfigPayload.pszApPassword ) );

    UxEspCppLibrary::UxLibCommon::saveStringCopy( wifiConfigMessage.payload.wifiConfigPayload.pszApIpv4,
                                                  strApIpv4.c_str(),
                                                  sizeof( wifiConfigMessage.payload.wifiConfigPayload.pszApIpv4 ) );

    wifiConfigMessage.payload.wifiConfigPayload.nWifiMode = nWifiMode;
    wifiConfigMessage.payload.wifiConfigPayload.bStaDhcp  = bStaDhcp;

    UxEspCppLibrary::UxLibCommon::saveStringCopy( wifiConfigMessage.payload.wifiConfigPayload.pszStaSsid,
                                                  strStaSsid.c_str(),
                                                  sizeof( wifiConfigMessage.payload.wifiConfigPayload.pszStaSsid ) );

    UxEspCppLibrary::UxLibCommon::saveStringCopy( wifiConfigMessage.payload.wifiConfigPayload.pszStaPassword,
                                                  strStaPassword.c_str(),
                                                  sizeof( wifiConfigMessage.payload.wifiConfigPayload.pszStaPassword ) );

    UxEspCppLibrary::UxLibCommon::saveStringCopy( wifiConfigMessage.payload.wifiConfigPayload.pszStaStaticIpv4,
                                                  strStaStaticIpv4.c_str(),
                                                  sizeof( wifiConfigMessage.payload.wifiConfigPayload.pszStaStaticIpv4 ) );

    UxEspCppLibrary::UxLibCommon::saveStringCopy( wifiConfigMessage.payload.wifiConfigPayload.pszStaStaticGateway,
                                                  strStaStaticGateway.c_str(),
                                                  sizeof( wifiConfigMessage.payload.wifiConfigPayload.pszStaStaticGateway ) );

    UxEspCppLibrary::UxLibCommon::saveStringCopy( wifiConfigMessage.payload.wifiConfigPayload.pszStaStaticNetmask,
                                                  strStaStaticNetmask.c_str(),
                                                  sizeof( wifiConfigMessage.payload.wifiConfigPayload.pszStaStaticNetmask ) );

    UxEspCppLibrary::UxLibCommon::saveStringCopy( wifiConfigMessage.payload.wifiConfigPayload.pszStaStaticDns,
                                                  strStaStaticDns.c_str(),
                                                  sizeof( wifiConfigMessage.payload.wifiConfigPayload.pszStaStaticDns ) );

    return send( &wifiConfigMessage );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

BaseType_t PluginTaskWifiConnectorQueue::sendWifiOn( void )
{
    return sendSimpleEvent( static_cast<uint32_t>( PluginTaskWifiConnectorQueueTypes::messageType::WifiOn ) );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

BaseType_t PluginTaskWifiConnectorQueue::sendWifiOff( void )
{
    return sendSimpleEvent( static_cast<uint32_t>( PluginTaskWifiConnectorQueueTypes::messageType::WifiOff ) );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

BaseType_t PluginTaskWifiConnectorQueue::sendSleepOn( void )
{
    return sendSimpleEvent( static_cast<uint32_t>( PluginTaskWifiConnectorQueueTypes::messageType::SleepOn ) );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

BaseType_t PluginTaskWifiConnectorQueue::sendSleepOff( void )
{
    return sendSimpleEvent( static_cast<uint32_t>( PluginTaskWifiConnectorQueueTypes::messageType::SleepOff ) );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

BaseType_t PluginTaskWifiConnectorQueue::sendScanAccessPoints( void )
{
    return sendSimpleEvent( static_cast<uint32_t>( PluginTaskWifiConnectorQueueTypes::messageType::ScanAccessPoints ) );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

BaseType_t PluginTaskWifiConnectorQueue::sendHostname( const std::string & strHostname )
{
    PluginTaskWifiConnectorQueueTypes::message message;

    message.u32Type = PluginTaskWifiConnectorQueueTypes::messageType::Hostname;

    UxEspCppLibrary::UxLibCommon::saveStringCopy( message.payload.hostnamePayload.pszHostname,
                                                  strHostname.c_str(),
                                                  sizeof( message.payload.hostnamePayload.pszHostname ) );

    return send( &message );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

} // namespace UxEspCppLibrary
