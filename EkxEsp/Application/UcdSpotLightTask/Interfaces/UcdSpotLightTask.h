/*!*************************************************************************************************************************************************************
 *
 * @file UcdSpotLightTask.h
 *
 * @brief Source file of component unit SpotLight.
 *
 * This file was developed as part of pwmOut Specialization.
 *
 * @author Ultratronik GmbH
 *         Dornierstr. 9
 *         D-82205 Gilching
 *         http://www.ultratronik.de
 * @author Srdjan Nikolic, Entwicklung
 *
 * @copyright Copyright 2020 by Ultratronik GmbH.
 *            All rights reserved.
 *            None of this file or parts of it may be
 *            copied, redistributed or used in any other way
 *            without written approval of Ultratronik GmbH.
 *
 **************************************************************************************************************************************************************/

#ifndef UcdSpotLightTask_h
#define UcdSpotLightTask_h

/*!*************************************************************************************************************************************************************
 *
 **************************************************************************************************************************************************************/

#include <stdint.h>

#include "UcdPwmOutTask.h"

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class UcdSpotLightTask : public UcdPwmOutTask
{

public:

    UcdSpotLightTask( UxEspCppLibrary::EspApplication * pApplication,
                      const ledc_timer_t                nLedcTimerNum,
                      const ledc_channel_t              nLedcChannel );
    ~UcdSpotLightTask() override;

    void channelDefaultFaderSettings( uint8_t *  pu8FadeInDutyPercent,
                                      uint8_t *  pu8FadeOutDutyPercent,
                                      uint32_t * pu32FadeInTimeMs,
                                      uint32_t * pu32FadeOutTimeMs ) override;

    bool startupStep( const FreeRtosQueueTask::startupStep_t nStartupStep ) override;

    void processFirstFadeOut( void );

};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#endif /* UcdSpotLightTask_h */
