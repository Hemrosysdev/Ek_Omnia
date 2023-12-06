///////////////////////////////////////////////////////////////////////////////
///
/// @file FreeRtosQueue.cpp
///
/// This file was developed as part of UX Extended Eco System Testframework (UX-EES-TSFW)
///
/// @brief Implementation file of class FreeRtosQueue.
///
/// @author Ultratronik GmbH
///         Dornierstr. 9
///         D-82205 Gilching
///         http://www.ultratronik.de
///
/// @author written by Gerd Esser, Research & Development, gesser@ultratronik.de
///
/// @date 21.10.2019
///
/// @copyright Copyright 2021 by Ultratronik GmbH.
///
/// This file and/or parts of it are subject to Ultratronik´s software license terms (SoLiT, Version 1.16.2).
/// With the use of this software you accept the SoLiT. Without written approval of Ultratronik GmbH this
/// software may not be copied, redistributed or used in any other way than stated in the conditions of the
/// SoLiT.
///
///////////////////////////////////////////////////////////////////////////////

#include "FreeRtosQueue.h"

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

namespace UxEspCppLibrary
{

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

FreeRtosQueue::FreeRtosQueue( const UBaseType_t uQueueLength,
                              const UBaseType_t uQueueItemSize,
                              const std::string & strName )
: EspLog( strName )
, m_uQueueLength( uQueueLength )
, m_uQueueItemSize( uQueueItemSize )
, m_hQueue( nullptr )
, m_strName( strName )
{
    create();
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

FreeRtosQueue::~FreeRtosQueue()
{
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

BaseType_t FreeRtosQueue::reset( void )
{
    return xQueueReset( m_hQueue );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

BaseType_t FreeRtosQueue::send( const void * const pvItemToQueue,
                                const TickType_t xTicksToWait )
{
    BaseType_t xReturn = xQueueSend( m_hQueue, pvItemToQueue, xTicksToWait );

    if ( xReturn != pdPASS )
    {
        vlogError( "send: xQueueSend failed (%d)", xReturn );
    }

    return xReturn;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

BaseType_t FreeRtosQueue::sendFromIsr( const void * const pvItemToQueue )
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    BaseType_t xReturn = xQueueSendFromISR( m_hQueue, pvItemToQueue, &xHigherPriorityTaskWoken );

    if ( xReturn != pdPASS )
    {
        // don't log result -> will crash
        // vlogError( "sendFromIsr: xQueueSendFromISR failed (%d)", xReturn );
    }

    // Now the buffer is empty we can switch context if necessary.
    if( xHigherPriorityTaskWoken )
    {
        // Actual macro used here is port specific.
        portYIELD_FROM_ISR ();
    }

    return xReturn;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

BaseType_t FreeRtosQueue::receive( void * const pvBuffer,
                                   const TickType_t xTicksToWait )
{
    return xQueueReceive( m_hQueue,
                          pvBuffer,
                          xTicksToWait );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

const std::string & FreeRtosQueue::name( void ) const
{
    return m_strName;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void FreeRtosQueue::create( void )
{
    m_hQueue = xQueueCreate( m_uQueueLength, m_uQueueItemSize );

    if ( m_hQueue == nullptr )
    {
        ESP_ERROR_CHECK( ESP_FAIL );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

UBaseType_t FreeRtosQueue::queueItemSize( void ) const
{
    return m_uQueueItemSize;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

UBaseType_t FreeRtosQueue::queueLength() const
{
    return m_uQueueLength;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

BaseType_t FreeRtosQueue::sendSimpleEvent( const uint32_t u32MessageId )
{
    uint8_t * pu8MsgBuf = new uint8_t[m_uQueueItemSize];

    struct message
    {
        uint32_t u32Type;
    };

    message * pMessage = reinterpret_cast<message*>( pu8MsgBuf );

    pMessage->u32Type = u32MessageId;

    BaseType_t xReturn = send( pMessage );

    delete [] pu8MsgBuf;

    return xReturn;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

BaseType_t FreeRtosQueue::sendSimpleEventFromIsr( const uint32_t u32MessageId )
{
    uint8_t * pu8MsgBuf = new uint8_t[m_uQueueItemSize];

    struct message
    {
        uint32_t u32Type;
    };

    message * pMessage = reinterpret_cast<message*>( pu8MsgBuf );

    pMessage->u32Type = u32MessageId;

    BaseType_t xReturn = sendFromIsr( pMessage );

    delete [] pu8MsgBuf;

    return xReturn;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

} // namespace UxEspCppLibrary

