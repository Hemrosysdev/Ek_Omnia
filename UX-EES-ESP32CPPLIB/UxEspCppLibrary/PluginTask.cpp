///////////////////////////////////////////////////////////////////////////////
///
/// @file PluginTask.cpp
///
/// This file was developed as part of UX Extended Eco System Testframework (UX-EES-TSFW)
///
/// @brief Implementation file of class PluginTask.
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

#include "PluginTask.h"
#include "PluginTaskDistributor.h"

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

namespace UxEspCppLibrary
{

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

PluginTask::PluginTask( EspApplication* const pApplication,                 // NOSONAR
                        const uint32_t u32StackSize,
                        const UBaseType_t uTaskPriority,
                        const std::string& strName,
                        const UBaseType_t uQueueLength,
                        const UBaseType_t uQueueItemSize,
                        PluginTaskDistributor* const pDistributor,
                        const UBaseType_t uRequiredConsumerQueueItemSize,    // NOSONAR
                        const TickType_t xReceiveTicksToWait )               // NOSONAR
: FreeRtosQueueTask( pApplication,
                     u32StackSize,
                     uTaskPriority,
                     strName,
                     uQueueLength,
                     uQueueItemSize,
                     xReceiveTicksToWait )
, m_pDistributor( pDistributor )
, m_uRequiredConsumerQueueItemSize( uRequiredConsumerQueueItemSize )
{
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

PluginTask::PluginTask( EspApplication* const pApplication,                 // NOSONAR
                        const uint32_t u32StackSize,
                        const UBaseType_t uTaskPriority,
                        const std::string& strName,
                        FreeRtosQueue* const pQueue,
                        PluginTaskDistributor* const pDistributor,
                        const UBaseType_t uRequiredConsumerQueueItemSize,
                        const TickType_t xReceiveTicksToWait )               // NOSONAR
: FreeRtosQueueTask( pApplication,
                     u32StackSize,
                     uTaskPriority,
                     strName,
                     pQueue,
                     xReceiveTicksToWait )
, m_pDistributor( pDistributor )
, m_uRequiredConsumerQueueItemSize( uRequiredConsumerQueueItemSize )
{
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

PluginTask::~PluginTask()
{
    delete m_pDistributor;
    m_pDistributor = nullptr;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

PluginTaskDistributor * PluginTask::distributor( void )
{
    return m_pDistributor;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

const PluginTaskDistributor * PluginTask::distributor( void ) const
{
    return m_pDistributor;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

UBaseType_t PluginTask::requiredConsumerQueueItemSize() const
{
    return m_uRequiredConsumerQueueItemSize;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

} // namespace UxEspCppLibrary

