///////////////////////////////////////////////////////////////////////////////
///
/// @file FreeRtosTimer.cpp
///
/// This file was developed as part of UX Extended Eco System Testframework (UX-EES-TSFW)
///
/// @brief Implementation file of class FreeRtosTimer.
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

#include "FreeRtosTimer.h"

#include <esp_timer.h>
#include <esp_idf_version.h>

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

namespace UxEspCppLibrary
{

bool FreeRtosTimer::m_bTimerInit = false;

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

FreeRtosTimer::FreeRtosTimer( const std::string & strDebugName )
    : EspLog( strDebugName )
    , m_strDebugName( strDebugName )
{
// Das eigentlich auskommentiert, weil der timer in der idf (esp_timer.c) bereits initialisiert wird
//#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(4, 3, 1)
//    if ( !m_bTimerInit )
//    {
//        ESP_ERROR_CHECK( esp_timer_init() );
//        m_bTimerInit = true;
//    }
//#endif

    m_timerArgs.callback        = reinterpret_cast<esp_timer_cb_t>( &FreeRtosTimer::timerCallback );
    m_timerArgs.arg             = this;
    m_timerArgs.name            = m_strDebugName.c_str();
    m_timerArgs.dispatch_method = ESP_TIMER_TASK;

    esp_err_t nEspError = esp_timer_create( &m_timerArgs,
                                            &m_hTimer );
    if ( nEspError != ESP_OK )
    {
        vlogError( "constructor() esp_timer_create failed (%s)", esp_err_to_name( nEspError ) );
        abort();
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

FreeRtosTimer::~FreeRtosTimer()
{
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

esp_err_t FreeRtosTimer::startPeriodic()
{
    stop();
    return esp_timer_start_periodic( m_hTimer, m_u64TimeoutUs );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

esp_err_t FreeRtosTimer::startPeriodic( const uint64_t u64TimeoutUs )
{
    m_u64TimeoutUs = u64TimeoutUs;

    stop();
    return esp_timer_start_periodic( m_hTimer, m_u64TimeoutUs );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

esp_err_t FreeRtosTimer::startOnce()
{
    stop();
    return esp_timer_start_once( m_hTimer, m_u64TimeoutUs );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

esp_err_t FreeRtosTimer::startOnce( const uint64_t u64TimeoutUs )
{
    m_u64TimeoutUs = u64TimeoutUs;

    stop();
    return esp_timer_start_once( m_hTimer, m_u64TimeoutUs );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void FreeRtosTimer::stop()
{
    esp_timer_stop( m_hTimer );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void FreeRtosTimer::setIntervall( const uint64_t u64TimeoutUs )
{
    m_u64TimeoutUs = u64TimeoutUs;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void FreeRtosTimer::timerCallback( void * const pArg )          // NOSONAR void * required by IDF API
{
    FreeRtosTimer * const pThis = reinterpret_cast<FreeRtosTimer *>( pArg );

    if ( pThis != nullptr )
    {
        pThis->processTimeout();
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

const std::string & FreeRtosTimer::debugName() const
{
    return m_strDebugName;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

} // namespace UxEspCppLibrary

