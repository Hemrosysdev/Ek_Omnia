///////////////////////////////////////////////////////////////////////////////
///
/// @file FreeRtosQueueTimer.cpp
///
/// This file was developed as part of UX Extended Eco System Testframework (UX-EES-TSFW)
///
/// @brief Implementation file of class FreeRtosQueueTimer.
///
/// @author Ultratronik GmbH
///         Dornierstr. 9
///         D-82205 Gilching
///         http://www.ultratronik.de
///
/// @author written by Gerd Esser, Research & Development, gesser@ultratronik.de
///
/// @date 22.10.2019
///
/// @copyright Copyright 2021 by Ultratronik GmbH.
///
/// This file and/or parts of it are subject to Ultratronik´s software license terms (SoLiT, Version 1.16.2).
/// With the use of this software you accept the SoLiT. Without written approval of Ultratronik GmbH this
/// software may not be copied, redistributed or used in any other way than stated in the conditions of the
/// SoLiT.
///
///////////////////////////////////////////////////////////////////////////////

#include "FreeRtosQueueTimer.h"
#include "FreeRtosQueue.h"

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

namespace UxEspCppLibrary
{

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

FreeRtosQueueTimer::FreeRtosQueueTimer( FreeRtosQueue * const pQueue,
                                        const uint32_t        u32MessageType )
    : FreeRtosTimer( "FreeRtosQueueTimer" )
    , m_pQueue( pQueue )
    , m_u32MessageType( u32MessageType )
{
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

FreeRtosQueueTimer::~FreeRtosQueueTimer()
{
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void FreeRtosQueueTimer::processTimeout( void )
{
    if ( m_pQueue != nullptr )
    {
        const BaseType_t xReturn = m_pQueue->sendSimpleEvent( m_u32MessageType );

        if ( xReturn != pdPASS )
        {
            vlogError( "processTimeout: can't send message %d into queue '%s' (Ret %d)",
                       m_u32MessageType,
                       m_pQueue->name().c_str(),
                       xReturn );
        }
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

uint32_t FreeRtosQueueTimer::messageType() const
{
    return m_u32MessageType;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

} // namespace UxEspCppLibrary

