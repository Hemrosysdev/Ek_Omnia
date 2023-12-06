/*
 * EspLedcTimer.h
 *
 *  Created on: 05.11.2019
 *      Author: gesser
 */

#ifndef UXESPCPPLIBRARY_ESPLEDCTIMER_H
#define UXESPCPPLIBRARY_ESPLEDCTIMER_H

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#include "EspLog.h"
#include <driver/ledc.h>

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

namespace UxEspCppLibrary
{

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class EspLedcTimer : public EspLog
{
public:

    EspLedcTimer();
    EspLedcTimer( const std::string & strLogName );

    virtual ~EspLedcTimer();

    esp_err_t init( const ledc_mode_t      nSpeedMode,
                    const ledc_timer_bit_t nBitNum,
                    const ledc_timer_t     nTimerNum,
                    const uint32_t         u32FreqHz );

    ledc_mode_t speedMode( void ) const;

    ledc_timer_t timerNum( void ) const;

    ledc_timer_bit_t bitNum( void ) const;

    uint32_t freqHz( void ) const;

    esp_err_t setFreqHz( const uint32_t u32FreqHz );

private:

    ledc_mode_t      m_nSpeedMode;

    ledc_timer_t     m_nTimerNum;

    ledc_timer_bit_t m_nBitNum;

    uint32_t         m_u32FreqHz;

};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

} // namespace UxEspCppLibrary

#endif /* UXESPCPPLIBRARY_ESPLEDCTIMER_H */
