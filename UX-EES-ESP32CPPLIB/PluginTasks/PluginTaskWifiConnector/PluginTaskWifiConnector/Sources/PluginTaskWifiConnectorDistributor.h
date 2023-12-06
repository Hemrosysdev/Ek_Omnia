/*!************************************************************************************************************************************************************
 *
 * @file PluginTaskWifiConnectorDistributor.h
 * 
 * This file was developed as part of UX Extended Eco System
 *
 * @brief Header file of class PluginTaskWifiConnectorDistributor.
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

#ifndef PluginTaskWifiConnectorDistributor_h
#define PluginTaskWifiConnectorDistributor_h

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#include "PluginTaskDistributor.h"

#include "PluginTaskWifiConnectorTypes.h"
#include "FreeRtosQueue.h"

#include <esp_wifi_types.h>

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

namespace UxEspCppLibrary
{

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class PluginTaskWifiConnectorDistributor : public PluginTaskDistributor
{   

public:

    PluginTaskWifiConnectorDistributor();

    virtual ~PluginTaskWifiConnectorDistributor();

    BaseType_t sendWifiStatus( const PluginTaskWifiConnectorTypes::WifiStatus nStatus );

    BaseType_t sendWifiEventApStart( void );
    BaseType_t sendWifiEventApStop( void );
    BaseType_t sendWifiEventApStaConnected( const std::string & strMac );
    BaseType_t sendWifiEventApStaDisconnected( const std::string & strMac );
    BaseType_t sendWifiEventStaStart( void );
    BaseType_t sendWifiEventStaStop( void );
    BaseType_t sendWifiEventStaConnected( void );
    BaseType_t sendWifiEventStaDisconnected( void );
    BaseType_t sendWifiEventScanDone( void );
    BaseType_t sendWifiEventStaGotIp( const std::string & strIp );
    BaseType_t sendWifiInfo( const std::string & strApIpv4,
                             const std::string & strApMac,
                             const std::string & strStaIpv4,
                             const std::string & strStaMac );
    BaseType_t sendWifiScanResults( const bool bSuccess,
                                    const uint16_t u16Number,
                                    wifi_ap_record_t * const pApRecordList );

};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

} // namespace UxEspCppLibrary

#endif /* PluginTaskWifiConnectorDistributor_h */
