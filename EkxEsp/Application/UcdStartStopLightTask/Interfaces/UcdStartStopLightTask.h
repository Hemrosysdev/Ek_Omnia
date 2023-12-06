/*!*************************************************************************************************************************************************************
 *
 * @file UcdStartStopLightTask.h
 *
 * @brief Source file of component unit StartStopLight.
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

#ifndef UcdStartStopLightTask_h
#define UcdStartStopLightTask_h

/***************************************************************************************************************************************************************
 *      Includes
 **************************************************************************************************************************************************************/

#include <stdint.h>

#include "UcdPwmOutTask.h"

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class UcdStartStopLightTask : public UcdPwmOutTask
{

public:

    UcdStartStopLightTask( UxEspCppLibrary::EspApplication * pApplication,
                           const ledc_timer_t                nLedcTimerNum,
                           const ledc_channel_t              nLedcChannel );
    ~UcdStartStopLightTask() override;

    bool startupStep( const FreeRtosQueueTask::startupStep_t nStartupStep ) override;

protected:

    void channelDefaultFaderSettings( uint8_t *  pu8FadeInDutyPercent,
                                      uint8_t *  pu8FadeOutDutyPercent,
                                      uint32_t * pu32FadeInTimeMs,
                                      uint32_t * pu32FadeOutTimeMs ) override;

};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#endif /* UcdStartStopLightTask_h */
