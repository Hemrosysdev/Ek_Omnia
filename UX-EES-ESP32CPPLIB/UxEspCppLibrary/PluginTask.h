///////////////////////////////////////////////////////////////////////////////
///
/// @file PluginTask.h
///
/// This file was developed as part of UX Extended Eco System Testframework (UX-EES-TSFW)
///
/// @brief Header file of class PluginTask.
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

#ifndef PluginTask_h
#define PluginTask_h

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#include "FreeRtosQueueTask.h"

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

namespace UxEspCppLibrary
{

class PluginTaskDistributor;

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class PluginTask : public FreeRtosQueueTask
{   

public:

    PluginTask( EspApplication * const pApplication,
                const uint32_t u32StackSize,
                const UBaseType_t  uTaskPriority,
                const std::string & strName,
                const UBaseType_t uQueueLength,
                const UBaseType_t uQueueItemSize,
                PluginTaskDistributor * const pDistributor,
                const UBaseType_t uRequiredConsumerQueueItemSize,
                const TickType_t xReceiveTicksToWait = portMAX_DELAY );

    PluginTask( EspApplication * const pApplication,
                const uint32_t u32StackSize,
                const UBaseType_t  uTaskPriority,
                const std::string & strName,
                FreeRtosQueue * const pQueue,
                PluginTaskDistributor * const pDistributor,
                const UBaseType_t uRequiredConsumerQueueItemSize,
                const TickType_t xReceiveTicksToWait = portMAX_DELAY );

    ~PluginTask() override;

    PluginTaskDistributor * distributor( void );
    const PluginTaskDistributor * distributor( void ) const;

    UBaseType_t requiredConsumerQueueItemSize() const;

private:

    PluginTaskDistributor * m_pDistributor { nullptr };

    UBaseType_t m_uRequiredConsumerQueueItemSize { 0 };

};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

} // namespace UxEspCppLibrary

#endif /* PluginTask_h */
