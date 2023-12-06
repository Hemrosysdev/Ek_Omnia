/*!*************************************************************************************************************************************************************
 *
 * @file UcdStepperMotorTask.h
 * @brief Source file of component unit SW Update.
 *
 * @author Ultratronik GmbH
 *         Dornierstr. 9
 *         D-82205 Gilching
 *         http://www.ultratronik.de
 *
 * @author Gerd Esser, Research & Development
 *
 * @date 27.08.2020
 *
 * @copyright Copyright 2020 by Ultratronik GmbH.
 *            All rights reserved.
 *            None of this file or parts of it may be
 *            copied, redistributed or used in any other way
 *            without written approval of Ultratronik GmbH.
 *
 **************************************************************************************************************************************************************/

#ifndef UcdStepperMotorTask_h
#define UcdStepperMotorTask_h

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#include "UartControlledDevice.h"

#include <string>
#include <driver/gpio.h>

#include "StepperTimer.h"
#include "EspGpio.h"
#include "EspIsrGpio.h"
#ifdef STEPPER_USE_LEDC_PWM
#include "EspLedcChannel.h"
#include "EspLedcTimer.h"
#endif
#include "FreeRtosQueueTimer.h"
#include "EkxProtocol.h"

class UcdStepperMotorQueue;

//#define STEPPER_USE_LEDC_PWM
//#define STEPS_STRESS_TEST

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class UcdStepperMotorTask : public UartControlledDevice
{

private:

    enum StepMode
    {
        FullStep = 0,
        HalfStep
    };

#ifdef STEPS_STRESS_TEST
    static constexpr int c_nTestStartFrequency  = 10000;
    static constexpr int c_nTestRunFrequency    = 10000;
    static constexpr int c_nTestRampFreqPerStep = 5;
    static constexpr int c_nTestSteps           = 40000;
#endif

public:

    UcdStepperMotorTask( UxEspCppLibrary::EspApplication * pApplication,
                         const gpio_num_t                  nGpioNumDirection,
                         const gpio_num_t                  nGpioNumStep,
                         const gpio_num_t                  nGpioNumEnable,
                         const gpio_num_t                  nGpioNumFaultN,
#ifdef STEPPER_USE_LEDC_PWM
                         const ledc_timer_t                nStepLedcTimerNum,
                         const ledc_channel_t              nStepLedcChannel,
#endif
                         const EkxProtocol::DriverId       u8DriverId,
                         const std::string &               strUcdName,
                         const std::string &               strTaskName,
                         const int                         nTaskStackSize,
                         const UBaseType_t                 uTaskPriority );

    ~UcdStepperMotorTask() override;

    bool startupStep( const FreeRtosQueueTask::startupStep_t nStartupStep ) override;

    void processTaskMessage( void ) override;

    UcdStepperMotorQueue * stepperMotorQueue( void );

    const UcdStepperMotorQueue * stepperMotorQueue( void ) const;

    bool queueCtrlUartCommand( const EkxProtocol::DriverId                  u8DriverId,
                               const uint32_t                               u32MsgCounter,
                               const uint8_t                                u8RepeatCounter,
                               EkxProtocol::Payload::MessagePayload * const pMessagePayload ) override;

    bool queueAckNackReceived( const uint32_t u32MsgCounter,
                               const bool     bSuccess ) override;

    bool queueAlive( const bool bAlive ) override;

    void processMotorStep( void );

private:

    UcdStepperMotorTask() = delete;

#ifdef STEPPER_USE_LEDC_PWM
    esp_err_t initLedc( const bool bTryRun );

#endif

    void reportDriverStatus( const bool bFirst ) override;

    void processRampStep( void );

    void processIsrFaultN( void );

    void processCtrlUartCommand( const EkxProtocol::DriverId                        u8DriverId,
                                 const uint32_t                                     u32MsgCounter,
                                 const uint8_t                                      u8RepeatCounter,
                                 const EkxProtocol::Payload::MessagePayload * const pMessagePayload );

    void processAlive( const bool bAlive ) override;

    void processStartMotor( const uint32_t                                        u32MsgCounter,
                            const uint8_t                                         u8RepeatCounter,
                            const EkxProtocol::Payload::StepperMotorStart * const pMessagePayload );

    void processStartMotorSteps( const uint32_t                                             u32MsgCounter,
                                 const uint8_t                                              u8RepeatCounter,
                                 const EkxProtocol::Payload::StepperMotorStartSteps * const pMessagePayload );

    void processStopMotor( const uint32_t                                       u32MsgCounter,
                           const uint8_t                                        u8RepeatCounter,
                           const EkxProtocol::Payload::StepperMotorStop * const pMessagePayload );

    void processChangeRunFrequency( const uint32_t                                                     u32MsgCounter,
                                    const uint8_t                                                      u8RepeatCounter,
                                    const EkxProtocol::Payload::StepperMotorChangeRunFrequency * const pMessagePayload );

    void startMotor( const bool bDirection,
                     const int  nStartFrequency,
                     const int  nRunFrequency,
                     const int  nRampFreqPerStep );

    void startMotorSteps( const bool bDirection,
                          const int  nSteps,
                          const int  nStartFrequency,
                          const int  nRunFrequency,
                          const int  nRampFreqPerStep );

    void stopMotor( void );

    void setFrequency( const int nFrequency );

private:

    bool                     m_bDirection { false };

    int                      m_nSteps { 0 };

    int                      m_nCurrentFrequency { 0 };

    int                      m_nStartFrequency { 0 };

    int                      m_nRunFrequency { 0 };

    int                      m_nRampFreqPerStep { 0 };

    bool                     m_bRunning { false };

    gpio_num_t               m_nGpioNumFaultN { GPIO_NUM_MAX };

    UxEspCppLibrary::EspGpio m_gpioDirection;

#ifndef STEPPER_USE_LEDC_PWM
    UxEspCppLibrary::EspGpio m_gpioStep;
#endif

    UxEspCppLibrary::EspGpio    m_gpioEnable;

    UxEspCppLibrary::EspIsrGpio m_gpioFaultN;

#ifndef STEPPER_USE_LEDC_PWM
    StepperTimer m_timerStep;
#endif

    UxEspCppLibrary::FreeRtosQueueTimer m_timerRampStep;
    //UxEspCppLibrary::FreeRtosQueueTimer  m_timerTest;

    uint32_t m_u32StepCounter { 0 };

    bool     m_bUseStepCount { false };

    bool     m_bStepHigh { false };

    bool     m_bRampUp { false };

    bool     m_bRampDown { false };

#ifdef STEPPER_USE_LEDC_PWM
    UxEspCppLibrary::EspLedcTimer   m_motorStepTimer;

    UxEspCppLibrary::EspLedcChannel m_motorStep;

    gpio_num_t                      m_nGpioNumStep { GPIO_NUM_MAX };

    ledc_timer_t                    m_nStepLedcTimerNum { LEDC_TIMER_MAX };

    ledc_channel_t                  m_nStepLedcChannel { LEDC_CHANNEL_MAX };
#endif

#ifdef STEPS_STRESS_TEST
    bool m_bTestDirection   { true };
#endif

    bool m_bFaultPinActive { false };

};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#endif /* UcdStepperMotorTask_h */
