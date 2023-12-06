///////////////////////////////////////////////////////////////////////////////
///
/// @file PluginTaskDistributor.cpp
///
/// This file was developed as part of UX Extended Eco System Testframework (UX-EES-TSFW)
///
/// @brief Implementation file of class PluginTaskDistributor.
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

#include "PluginTaskDistributor.h"
#include "FreeRtosQueue.h"

#include <esp_err.h>
#include <esp_log.h>

#include "PluginTask.h"
#include "FreeRtosQueue.h"

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

namespace UxEspCppLibrary
{

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

PluginTaskDistributor::PluginTaskDistributor()
    : m_pQueueItemBuffer( nullptr )
    , m_u32QueueItemSize( 0 )
{
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

PluginTaskDistributor::~PluginTaskDistributor()
{
    delete [] m_pQueueItemBuffer;
    m_pQueueItemBuffer = nullptr;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void PluginTaskDistributor::registerConsumerQueue( const PluginTask * const pPluginTask,
                                                   FreeRtosQueue * const    pConsumerQueue )
{
    if ( pConsumerQueue )
    {
        m_theConsumerQueues.push_back( pConsumerQueue );

        if ( pConsumerQueue->queueItemSize() > m_u32QueueItemSize )
        {
            m_u32QueueItemSize = pConsumerQueue->queueItemSize();
            delete [] m_pQueueItemBuffer;
            m_pQueueItemBuffer = new uint8_t[m_u32QueueItemSize];
        }

        if ( pConsumerQueue->queueItemSize() < pPluginTask->requiredConsumerQueueItemSize() )
        {
            ESP_LOGE( "PluginTaskDistributor", "registerConsumerQueue - error, consumer queue item size insufficient, is %d, required %d",
                      pConsumerQueue->queueItemSize(),
                      pPluginTask->requiredConsumerQueueItemSize() );
            ESP_ERROR_CHECK( ESP_FAIL );
        }
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

std::vector<FreeRtosQueue *>  PluginTaskDistributor::consumerQueues( void )
{
    return m_theConsumerQueues;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

uint8_t * PluginTaskDistributor::queueItemBuffer( void )
{
    return m_pQueueItemBuffer;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

} // namespace UxEspCppLibrary

