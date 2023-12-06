/*
 * EspLedcAutoFader.cpp
 *
 *  Created on: 05.11.2019
 *      Author: gesser
 */

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#include "EspLedcAutoFader.h"

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

namespace UxEspCppLibrary
{

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

EspLedcAutoFader::EspLedcAutoFader()
    : EspLedcFader( "EspLedcAutoFader" )
//, m_autoTimer( "EspLedcAutoFaderTimer" )
    , m_u32FadeInPostTimeMs( 0 )
    , m_u32FadeOutPostTimeMs( 0 )
    , m_nAutoFaderState( autoFaderState_Stopped )
{
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

EspLedcAutoFader::EspLedcAutoFader( const std::string & strLogName )
    : EspLedcFader( strLogName )
//, m_autoTimer( "EspLedcAutoFaderTimer" )
    , m_u32FadeInPostTimeMs( 0 )
    , m_u32FadeOutPostTimeMs( 0 )
    , m_nAutoFaderState( autoFaderState_Stopped )
{
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

EspLedcAutoFader::~EspLedcAutoFader()
{
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

esp_err_t EspLedcAutoFader::init( const ledc_channel_t nLedcChannel,
                                  const gpio_num_t     nGpioNum,
                                  EspLedcTimer *       pLedcTimer,
                                  const uint32_t       u32FadeInDuty,
                                  const uint32_t       u32FadeInTimeMs,
                                  const uint32_t       u32FadeInPostTimeMs,
                                  const uint32_t       u32FadeOutDuty,
                                  const uint32_t       u32FadeOutTimeMs,
                                  const uint32_t       u32FadeOutPostTimeMs )
{
    m_u32FadeInPostTimeMs  = u32FadeInPostTimeMs;
    m_u32FadeOutPostTimeMs = u32FadeOutPostTimeMs;

    return EspLedcFader::init( nLedcChannel,
                               nGpioNum,
                               pLedcTimer,
                               u32FadeInDuty,
                               u32FadeInTimeMs,
                               u32FadeOutDuty,
                               u32FadeOutTimeMs );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

esp_err_t EspLedcAutoFader::startFade( void )
{
    esp_err_t nEspError = ESP_FAIL;

    nEspError = setDuty( fadeOutDuty() );

    if ( nEspError == ESP_OK )
    {
        m_nAutoFaderState = autoFaderState_FadeOutPost;
        processTimeout();
    }

    return nEspError;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

esp_err_t EspLedcAutoFader::stopFade( void )
{
    esp_err_t nEspError = ESP_FAIL;

    FreeRtosTimer::stop();

    m_nAutoFaderState = autoFaderState_Stopped;

    nEspError = setDuty( fadeOutDuty() );

    return nEspError;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

uint32_t EspLedcAutoFader::fadeInPostTimeMs( void ) const
{
    return m_u32FadeInPostTimeMs;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

uint32_t EspLedcAutoFader::fadeOutPostTimeMs( void ) const
{
    return m_u32FadeOutPostTimeMs;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void EspLedcAutoFader::processTimeout( void )
{
    switch ( m_nAutoFaderState )
    {
        case autoFaderState_FadeIn:
        {
            FreeRtosTimer::startOnce( fadeInPostTimeMs() * 1000 );
            m_nAutoFaderState = autoFaderState_FadeInPost;
        }
        break;

        case autoFaderState_FadeInPost:
        {
            FreeRtosTimer::startOnce( fadeOutTimeMs() * 1000 );
            fadeOut();
            m_nAutoFaderState = autoFaderState_FadeOut;
        }
        break;

        case autoFaderState_FadeOut:
        {
            FreeRtosTimer::startOnce( fadeOutPostTimeMs() * 1000 );
            m_nAutoFaderState = autoFaderState_FadeOutPost;
        }
        break;

        case autoFaderState_FadeOutPost:
        {
            FreeRtosTimer::startOnce( fadeInTimeMs() * 1000 );
            fadeIn();
            m_nAutoFaderState = autoFaderState_FadeIn;
        }
        break;

        case autoFaderState_Stopped:
        default:
        {
        }
        break;
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

} // namespace UxEspCppLibrary

