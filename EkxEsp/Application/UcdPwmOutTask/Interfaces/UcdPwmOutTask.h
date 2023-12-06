/*!*************************************************************************************************************************************************************
 *
 * @file UcdPwmOutTask.h
 * @brief Source file of component unit UcdPwmOut.
 *
 * This file was developed as part of Spotlight.
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

#ifndef UcdPwmOutTask_h
#define UcdPwmOutTask_h

/*!**************************************************************************************************************************************************************
 *
 **************************************************************************************************************************************************************/

#include "UartControlledDevice.h"
#include "UcdPwmOutQueue.h"
#include "FreeRtosQueueTask.h"
#include "EspLedcTimer.h"
#include "EspLedcFader.h"
#include "EkxProtocol.h"

/*!**************************************************************************************************************************************************************
 *
 **************************************************************************************************************************************************************/

class UcdPwmOutTask : public UartControlledDevice
{

public:

    UcdPwmOutTask( UxEspCppLibrary::EspApplication * pApplication,
                   const gpio_num_t                  nGpioNum,
                   const ledc_timer_t                nLedcTimerNum,
                   const ledc_channel_t              nLedcChannel,
                   const uint8_t                     u8InitPercent,
                   const EkxProtocol::DriverId       u8DriverId,
                   const std::string &               strUcdName,
                   const std::string &               strTaskName,
                   const int                         nTaskStackSize,
                   const UBaseType_t                 uTaskPriority );

    ~UcdPwmOutTask() override;

    bool startupStep( const UxEspCppLibrary::FreeRtosQueueTask::startupStep_t nStartupStep ) override;

    void processTaskMessage( void ) override;

    UcdPwmOutQueue * pwmOutQueue( void );

    const UcdPwmOutQueue * pwmOutQueue( void ) const;

    bool queueCtrlUartCommand( const EkxProtocol::DriverId                  u8DriverId,
                               const uint32_t                               u32MsgCounter,
                               const uint8_t                                u8RepeatCounter,
                               EkxProtocol::Payload::MessagePayload * const pMessagePayload ) override;

    bool queueAckNackReceived( const uint32_t u32MsgCounter,
                               const bool     bSuccess ) override;

protected:

    virtual void channelDefaultFaderSettings( uint8_t *  pu8FadeInDutyPercent,
                                              uint8_t *  pu8FadeOutDutyPercent,
                                              uint32_t * pu32FadeInTimeMs,
                                              uint32_t * pu32FadeOutTimeMs );

    void fadeIn( void );

    void fadeOut( void );

    void setPwmDuty( const uint8_t u8PwmDutyPercent );

    void setFaderSettings( const uint8_t  u8FadeInDutyPercent,
                           const uint32_t u32FadeInTimeMs,
                           const uint8_t  u8FadeOutDutyPercent,
                           const uint32_t u32FadeOutTimeMs );

private:

    UcdPwmOutTask() = delete;

    bool initEspLedcTimer( const ledc_mode_t      nSpeedMode,
                           const ledc_timer_bit_t nBitNum,
                           const ledc_timer_t     nTimerNum,
                           const uint32_t         u32FreqHz );

    bool initEspLedcFader( void );

    bool installEspLedcFader( void );

    // percent 0 - 200; 200 = 100%
    uint32_t percent2Duty( uint8_t percent ) const;

    void processCtrlUartCommand( const EkxProtocol::DriverId                        u8DriverId,
                                 const uint32_t                                     u32MsgCounter,
                                 const uint8_t                                      u8RepeatCounter,
                                 const EkxProtocol::Payload::MessagePayload * const pMessagePayload ) override;

private:

    gpio_num_t                    m_nGpioNum { GPIO_NUM_MAX };

    ledc_timer_t                  m_nLedcTimerNum { LEDC_TIMER_MAX };

    ledc_channel_t                m_nLedcChannel;

    uint8_t                       m_u8InitPercent { 0 };

    UxEspCppLibrary::EspLedcFader m_theLedcFader;

    UxEspCppLibrary::EspLedcTimer m_theLedcTimer;

};

#endif /* UcdPwmOutTask_h */
