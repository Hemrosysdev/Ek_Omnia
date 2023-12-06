///////////////////////////////////////////////////////////////////////////////
///
/// @file PluginTaskConsumer.h
///
/// This file was developed as part of UX Extended Eco System Testframework (UX-EES-TSFW)
///
/// @brief Header file of class PluginTaskConsumer.
///
/// @author Ultratronik GmbH
///         Dornierstr. 9
///         D-82205 Gilching
///         http://www.ultratronik.de
///
/// @author written by Gerd Esser, Research & Development, gesser@ultratronik.de
///
/// @date 29.04.2020
///
/// @copyright Copyright 2021 by Ultratronik GmbH.
///
/// This file and/or parts of it are subject to Ultratronik´s software license terms (SoLiT, Version 1.16.2).
/// With the use of this software you accept the SoLiT. Without written approval of Ultratronik GmbH this
/// software may not be copied, redistributed or used in any other way than stated in the conditions of the
/// SoLiT.
///
///////////////////////////////////////////////////////////////////////////////

#ifndef PluginTaskConsumer_h
#define PluginTaskConsumer_h

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#include <cstdint>

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

namespace UxEspCppLibrary
{

class PluginTask;
class FreeRtosQueue;

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class PluginTaskConsumer
{

public:

    PluginTaskConsumer( PluginTask * const    pPluginTask,
                        FreeRtosQueue * const pConsumerQueue );

    virtual ~PluginTaskConsumer();

    virtual bool processTaskMessage( const uint8_t * pReceiveMsg ) = 0;

private:

    PluginTask *    m_pPluginTask { nullptr };

    FreeRtosQueue * m_pConsumerQueue { nullptr };

};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

} // namespace UxEspCppLibrary

#endif /* PluginTaskConsumer_h */
