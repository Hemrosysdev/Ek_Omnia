/*!************************************************************************************************************************************************************
 *
 * @file PluginTaskWifiConnectorQueue.h
 *
 * This file was developed as part of UX Extended Eco System
 *
 * @brief Header file of class PluginTaskWifiConnectorQueue.
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

#ifndef PluginTaskWifiConnectorQueue_h
#define PluginTaskWifiConnectorQueue_h

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#include <stdint.h>
#include <string>

#include "FreeRtosQueue.h"
#include "PluginTaskWifiConnectorTypes.h"

#include <esp_interface.h>

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

namespace UxEspCppLibrary
{

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class PluginTaskWifiConnectorQueue : public FreeRtosQueue
{
public:

    PluginTaskWifiConnectorQueue();

    virtual ~PluginTaskWifiConnectorQueue() override;

    BaseType_t sendWifiConfig( const UxEspCppLibrary::PluginTaskWifiConnectorTypes::WifiMode nWifiMode,
                               const std::string &                                           strApSsid,
                               const std::string &                                           strApPassword,
                               const std::string &                                           strApIpv4,
                               const std::string &                                           strStaSsid,
                               const std::string &                                           strStaPassword,
                               const bool                                                    bStaDhcp,
                               const std::string &                                           strStaStaticIpv4,
                               const std::string &                                           strStaStaticGateway,
                               const std::string &                                           strStaStaticNetmask,
                               const std::string &                                           strStaStaticDns );

    BaseType_t sendHostname( const std::string & strHostname );

    BaseType_t sendWifiOn( void );

    BaseType_t sendWifiOff( void );

    BaseType_t sendSleepOn( void );

    BaseType_t sendSleepOff( void );

    BaseType_t sendScanAccessPoints( void );

}; /* PluginTaskWifiConnectorQueue_h */

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

} // namespace UxEspCppLibrary

#endif /* PluginTaskWifiConnectorQueue_H */
