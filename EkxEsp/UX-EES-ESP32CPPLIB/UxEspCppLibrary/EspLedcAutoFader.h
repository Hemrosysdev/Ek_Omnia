/*
 * EspLedcAutoFader.h
 *
 *  Created on: 05.11.2019
 *      Author: gesser
 */

#ifndef UXESPCPPLIBRARY_ESPLEDCAUTOFADER_H
#define UXESPCPPLIBRARY_ESPLEDCAUTOFADER_H

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#include "EspLedcFader.h"
#include "FreeRtosTimer.h"

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

namespace UxEspCppLibrary
{

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class EspLedcAutoFader : public EspLedcFader, public FreeRtosTimer
{

private:

    typedef enum
    {
        autoFaderState_Stopped = 0,
        autoFaderState_FadeIn,
        autoFaderState_FadeInPost,
        autoFaderState_FadeOut,
        autoFaderState_FadeOutPost,
    } autoFaderState;

public:

    EspLedcAutoFader();
    EspLedcAutoFader( const std::string & strLogName );

    virtual ~EspLedcAutoFader();

    esp_err_t init( const ledc_channel_t nLedcChannel,
                    const gpio_num_t     nGpioNum,
                    EspLedcTimer *       pLedcTimer,
                    const uint32_t       u32FadeInDuty,
                    const uint32_t       u32FadeInTimeMs,
                    const uint32_t       u32FadeInPostTimeMs,
                    const uint32_t       u32FadeOutDuty,
                    const uint32_t       u32FadeOutTimeMs,
                    const uint32_t       u32FadeOutPostTimeMs );

    esp_err_t startFade( void );

    esp_err_t stopFade( void );

    uint32_t fadeInPostTimeMs( void ) const;

    uint32_t fadeOutPostTimeMs( void ) const;

private:

    void processTimeout( void );

private:

    //FreeRtosTimer  m_autoTimer;

    uint32_t       m_u32FadeInPostTimeMs;

    uint32_t       m_u32FadeOutPostTimeMs;

    autoFaderState m_nAutoFaderState;

};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

} // namespace UxEspCppLibrary

#endif /* UXESPCPPLIBRARY_ESPLEDCAUTOFADER_H */
