///////////////////////////////////////////////////////////////////////////////
///
/// @file PluginTaskDistributor.h
///
/// This file was developed as part of UX Extended Eco System Testframework (UX-EES-TSFW)
///
/// @brief Header file of class PluginTaskDistributor.
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

#ifndef PluginTaskDistributor_h
#define PluginTaskDistributor_h

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#include <vector>
#include <cstdint>

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

namespace UxEspCppLibrary
{

class FreeRtosQueue;
class PluginTask;

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class PluginTaskDistributor
{   

public:

    PluginTaskDistributor();

    virtual ~PluginTaskDistributor();

    PluginTask * pluginTask();

    void registerConsumerQueue( const PluginTask * const pPluginTask,
                                FreeRtosQueue * const pConsumerQueue );

    std::vector<FreeRtosQueue *>  consumerQueues( void );

    uint8_t * queueItemBuffer( void );

private:

    std::vector<FreeRtosQueue *>  m_theConsumerQueues;

    uint8_t * m_pQueueItemBuffer { nullptr };

    uint32_t m_u32QueueItemSize { 0 };

};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

} // namespace UxEspCppLibrary

#endif /* PluginTaskDistributor_h */
