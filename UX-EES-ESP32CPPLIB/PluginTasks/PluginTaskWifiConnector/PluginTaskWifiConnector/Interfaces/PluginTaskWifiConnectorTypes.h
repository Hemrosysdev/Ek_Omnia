/*!************************************************************************************************************************************************************
 *
 * @file PluginTaskWifiConnectorTypes.h
 * 
 * This file was developed as part of UX Extended Eco System
 *
 * @brief Header file of class PluginTaskWifiConnectorTypes.
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

#ifndef PluginTaskWifiConnectorTypes_h
#define PluginTaskWifiConnectorTypes_h

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#include <stdint.h>

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

namespace UxEspCppLibrary
{

namespace PluginTaskWifiConnectorTypes
{

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

const int HOSTNAME_LENGTH     = 31;
const int HOSTNAME_BUF_LENGTH = ( HOSTNAME_LENGTH + 1 );
const int SSID_LENGTH         = 31;
const int SSID_BUF_LENGTH     = ( SSID_LENGTH + 1 );
const int PW_LENGTH           = 63;
const int PW_BUF_LENGTH       = ( PW_LENGTH + 1 );
const int IP4_LENGTH          = 15;
const int IP4_BUF_LENGTH      = ( IP4_LENGTH + 1 );
const int MAC_LENGTH          = 17;
const int MAC_BUF_LENGTH      = ( MAC_LENGTH + 1 );

enum class WifiStatus
{
    Unknown = 0,
    Off,
    Failed,
    Standby,
    Connecting,
    Connected
};

enum class WifiMode
{
    Ap = 0,
    Sta,
    Both
};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

} // namespace PluginTaskWifiConnectorTypes

} // namespace UxEspCppLibrary

#endif /* PluginTaskWifiConnectorTypes_h */
