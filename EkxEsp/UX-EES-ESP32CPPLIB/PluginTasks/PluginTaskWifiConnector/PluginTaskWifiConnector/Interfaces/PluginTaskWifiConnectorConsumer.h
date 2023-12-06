/*!************************************************************************************************************************************************************
 *
 * @file PluginTaskWifiConnectorConsumer.h
 *
 * This file was developed as part of UX Extended Eco System
 *
 * @brief Header file of class PluginTaskWifiConnectorConsumer.
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

#ifndef PluginTaskWifiConnectorConsumer_h
#define PluginTaskWifiConnectorConsumer_h

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#include "PluginTaskConsumer.h"

#include <stdint.h>
#include <string>
#include <esp_wifi_types.h>

#include "PluginTaskWifiConnectorTypes.h"

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

namespace UxEspCppLibrary
{

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class PluginTaskWifiConnectorConsumer : public PluginTaskConsumer
{

public:

    PluginTaskWifiConnectorConsumer( PluginTask * const    pPluginTask,
                                     FreeRtosQueue * const pConsumerQueue );

    ~PluginTaskWifiConnectorConsumer() override;

    bool processTaskMessage( const uint8_t * pReceiveMsg ) override;

    virtual void processWifiStatus( const PluginTaskWifiConnectorTypes::WifiStatus nStatus );

    virtual void processWifiEventApStart( void );

    virtual void processWifiEventApStop( void );

    virtual void processWifiEventApStaConnected( const std::string & strMac );

    virtual void processWifiEventApStaDisconnected( const std::string & strMac );

    virtual void processWifiEventStaStart( void );

    virtual void processWifiEventStaStop( void );

    virtual void processWifiEventStaConnected( void );

    virtual void processWifiEventStaDisconnected( void );

    virtual void processWifiEventScanDone( void );

    virtual void processWifiEventStaGotIp( const std::string & strIp );

    virtual void processWifiInfo( const std::string & strApIpv4,
                                  const std::string & strApMac,
                                  const std::string & strStaIpv4,
                                  const std::string & strStaMac );

    virtual void processWifiScanResults( const bool               bSuccess,
                                         const uint16_t           u16Number,
                                         wifi_ap_record_t * const pApRecordList );

};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

} // namespace UxEspCppLibrary

#endif /* PluginTaskWifiConnectorConsumer_h */
