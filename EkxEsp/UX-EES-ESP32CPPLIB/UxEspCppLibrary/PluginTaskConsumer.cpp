///////////////////////////////////////////////////////////////////////////////
///
/// @file PluginTaskConsumer.cpp
///
/// This file was developed as part of UX Extended Eco System Testframework (UX-EES-TSFW)
///
/// @brief Implementation file of class PluginTaskConsumer.
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

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#include "PluginTaskConsumer.h"
#include "PluginTaskDistributor.h"
#include "PluginTask.h"

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

namespace UxEspCppLibrary
{

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

PluginTaskConsumer::PluginTaskConsumer( PluginTask * const    pPluginTask,
                                        FreeRtosQueue * const pConsumerQueue )
    : m_pPluginTask( pPluginTask )
    , m_pConsumerQueue( pConsumerQueue )
{
    m_pPluginTask->distributor()->registerConsumerQueue( pPluginTask, pConsumerQueue );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

PluginTaskConsumer::~PluginTaskConsumer()
{
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

} // namespace UxEspCppLibrary

