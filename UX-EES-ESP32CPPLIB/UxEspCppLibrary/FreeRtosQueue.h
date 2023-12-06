///////////////////////////////////////////////////////////////////////////////
///
/// @file FreeRtosQueue.h
///
/// This file was developed as part of UX Extended Eco System Testframework (UX-EES-TSFW)
///
/// @brief Header file of class FreeRtosQueue.
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

#ifndef FreeRtosQueue_h
#define FreeRtosQueue_h

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <string>

#include "EspLog.h"

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

namespace UxEspCppLibrary
{

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class FreeRtosQueue : public EspLog
{

public:

    FreeRtosQueue( const UBaseType_t uQueueLength,
                   const UBaseType_t uQueueItemSize,
                   const std::string & strName );

    ~FreeRtosQueue( void ) override;

    UBaseType_t queueItemSize( void ) const;

    UBaseType_t queueLength() const;

    BaseType_t reset( void );

    BaseType_t sendSimpleEvent( const uint32_t u32MessageId );

    BaseType_t sendSimpleEventFromIsr( const uint32_t u32MessageId );

    BaseType_t send( const void * const pvItemToQueue,
                     const TickType_t xTicksToWait = 100 );

    BaseType_t sendFromIsr( const void * const pvItemToQueue );

    BaseType_t receive( void * const pvBuffer,
                        const TickType_t xTicksToWait = portMAX_DELAY );

    const std::string & name( void ) const;

private:

    ////////////////////////////////////////
    // private methods
    ////////////////////////////////////////

    // standard constructor hidden
    FreeRtosQueue() = delete;

    void create( void );

    ////////////////////////////////////////
    // private member variables
    ////////////////////////////////////////

    const UBaseType_t m_uQueueLength;

    const UBaseType_t m_uQueueItemSize;

    QueueHandle_t  m_hQueue;

    std::string  m_strName;

};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

} // namespace UxEspCppLibrary

#endif /* FreeRtosQueue_h */
