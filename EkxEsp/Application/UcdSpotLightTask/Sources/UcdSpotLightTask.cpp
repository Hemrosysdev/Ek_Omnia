/*!*************************************************************************************************************************************************************
 *
 * @file UcdSpotLightTask.cpp
 * @brief Source file of component unit SpotLight.
 *
 * This file was developed as part of pwmOut Specialization.
 *
 * @author Ultratronik GmbH
 *         Dornierstr. 9
 *         D-82205 Gilching
 *         http://www.ultratronik.de
 * @author Srdjan Nikolic, Entwicklung - <optional according to the authors needs>
 *
 * @copyright Copyright 2020 by Ultratronik GmbH.
 *            All rights reserved.
 *            None of this file or parts of it may be
 *            copied, redistributed or used in any other way
 *            without written approval of Ultratronik GmbH.
 *
 **************************************************************************************************************************************************************/

/***************************************************************************************************************************************************************
 *
 **************************************************************************************************************************************************************/

#include "UcdSpotLightTask.h"

#include "ApplicationGlobals.h"

/***************************************************************************************************************************************************************
 *
 **************************************************************************************************************************************************************/

UcdSpotLightTask::UcdSpotLightTask( UxEspCppLibrary::EspApplication * pApplication,
                                    const ledc_timer_t                nLedcTimerNum,
                                    const ledc_channel_t              nLedcChannel )
    : UcdPwmOutTask( pApplication,
                     ApplicationGlobals::c_nGpioPwmOutSpotLightN,
                     nLedcTimerNum,
                     nLedcChannel,
                     0,
                     EkxProtocol::DriverId::SpotLightDriver,
                     "SpotLight",
                     "UcdSpotLightTask",
                     ApplicationGlobals::c_nSpotLightTaskStackSize,
                     ApplicationGlobals::c_nSpotLightTaskPriority )
{
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

UcdSpotLightTask::~UcdSpotLightTask()
{

}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void UcdSpotLightTask::channelDefaultFaderSettings( uint8_t *  pu8FadeInDutyPercent,
                                                    uint8_t *  pu8FadeOutDutyPercent,
                                                    uint32_t * pu32FadeInTimeMs,
                                                    uint32_t * pu32FadeOutTimeMs )
{
    *pu8FadeInDutyPercent  = 200U;     // 100%
    *pu8FadeOutDutyPercent = 0U;       // 0%
    *pu32FadeInTimeMs      = 1000UL;
    *pu32FadeOutTimeMs     = 1000UL;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

bool UcdSpotLightTask::startupStep( const FreeRtosQueueTask::startupStep_t nStartupStep )
{
    bool bContinueStartup = UcdPwmOutTask::startupStep( nStartupStep );

    switch ( nStartupStep )
    {
        case UxEspCppLibrary::FreeRtosQueueTask::startupStep_t::startupStep2:
        {
            fadeIn();
        }
        break;

        default:
        {
        }
        break;
    }

    return bContinueStartup;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void UcdSpotLightTask::processFirstFadeOut( void )
{
    vlogInfo( "processFirstFadeOut()" );

    fadeOut();
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

