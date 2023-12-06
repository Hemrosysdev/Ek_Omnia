///////////////////////////////////////////////////////////////////////////////
///
/// @file FreeRtosTimer.h
///
/// This file was developed as part of UX Extended Eco System Testframework (UX-EES-TSFW)
///
/// @brief Header file of class FreeRtosTimer.
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

#ifndef FreeRtosTimer_h
#define FreeRtosTimer_h

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#include <esp_timer.h>
#include <cstdlib>
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

class FreeRtosTimer : public EspLog
{   
public:

    explicit FreeRtosTimer( const std::string & strDebugName = "" );
    ~FreeRtosTimer() override;

    const std::string & debugName() const;

    esp_err_t startPeriodic();
    esp_err_t startPeriodic( const uint64_t u64TimeoutUs );

    esp_err_t startOnce();
    esp_err_t startOnce( const uint64_t u64TimeoutUs );

    void stop();

private:

    ////////////////////////////////////////
    // private methods
    ////////////////////////////////////////

    virtual void processTimeout( void ) = 0;

    static void timerCallback( void * const pArg );

    ////////////////////////////////////////
    // private member variables
    ////////////////////////////////////////

    std::string  m_strDebugName;

    esp_timer_handle_t  m_hTimer { nullptr };

    esp_timer_create_args_t m_timerArgs;

    uint64_t m_u64TimeoutUs { 0U };

    static bool  m_bTimerInit;

};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

} // namespace UxEspCppLibrary

#endif /* FreeRtosTimer_h */
