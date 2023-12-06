/*
 * EspLedcFader.h
 *
 *  Created on: 05.11.2019
 *      Author: gesser
 */

#ifndef UXESPCPPLIBRARY_ESPLEDCFADER_H
#define UXESPCPPLIBRARY_ESPLEDCFADER_H

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#include "EspLedcChannel.h"

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

namespace UxEspCppLibrary
{

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class EspLedcFader : public EspLedcChannel
{

public:

    EspLedcFader();
    EspLedcFader( const std::string & strLogName );

    virtual ~EspLedcFader();

    esp_err_t init( const ledc_channel_t nLedcChannel,
                    const gpio_num_t     nGpioNum,
                    EspLedcTimer *       pLedcTimer,
                    const uint32_t       u32FadeInDuty,
                    const uint32_t       u32FadeInTimeMs,
                    const uint32_t       u32FadeOutDuty,
                    const uint32_t       u32FadeOutTimeMs );

    uint32_t fadeInTimeMs( void ) const;

    uint32_t fadeInDuty( void ) const;

    uint32_t fadeOutTimeMs( void ) const;

    uint32_t fadeOutDuty( void ) const;

    void setFadeSettings( const uint32_t u32FadeInDuty,
                          const uint32_t u32FadeInTimeMs,
                          const uint32_t u32FadeOutDuty,
                          const uint32_t u32FadeOutTimeMs );

    esp_err_t fadeIn( void );

    esp_err_t fadeOut( void );

    static esp_err_t install( const int nIntrAllocFlags );

private:

    uint32_t m_u32FadeInDuty { 0 };

    uint32_t m_u32FadeInTimeMs { 0 };

    uint32_t m_u32FadeOutDuty { 0 };

    uint32_t m_u32FadeOutTimeMs { 0 };

};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

} // namespace UxEspCppLibrary

#endif /* UXESPCPPLIBRARY_ESPLEDCFADER_H */
