/*!*************************************************************************************************************************************************************
 *
 * @file UcdDcHallMotorTask.h
 * @brief Source file of component unit SW Update.
 *
 * @author Ultratronik GmbH
 *         Dornierstr. 9
 *         D-82205 Gilching
 *         http://www.ultratronik.de
 *
 * @author Gerd Esser, Research & Development
 *
 * @date 06.11.2021
 *
 * @copyright Copyright 2021 by Ultratronik GmbH.
 *            All rights reserved.
 *            None of this file or parts of it may be
 *            copied, redistributed or used in any other way
 *            without written approval of Ultratronik GmbH.
 *
 **************************************************************************************************************************************************************/

#ifndef UcdDcHallMotorTask_H
#define UcdDcHallMotorTask_H

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#include "UartControlledDevice.h"

#include <driver/gpio.h>
#include <chrono>

#include "EspGpio.h"
#include "EspIsrGpio.h"
#include "EspLedcTimer.h"
#include "EspLedcChannel.h"
#include "FreeRtosQueueTimer.h"
#include "EkxProtocol.h"

class UcdDcHallMotorQueue;

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class UcdDcHallMotorTask : public UartControlledDevice
{

private:

    static const int c_nMaxPduMotorTestCycles = 10;

public:

    UcdDcHallMotorTask( UxEspCppLibrary::EspApplication * pApplication,
                        const gpio_num_t                  nGpioNumPowerEnable,
                        const gpio_num_t                  nGpioNumMotorPwm,
                        const gpio_num_t                  nGpioNumEncoderA,
                        const gpio_num_t                  nGpioNumFaultN,
                        const ledc_timer_t                nLedcTimerNum,
                        const ledc_channel_t              nLedcChannel,
                        const EkxProtocol::DriverId       u8DriverId,
                        const std::string &               strUcdName,
                        const std::string &               strTaskName,
                        const int                         nTaskStackSize,
                        const UBaseType_t                 uTaskPriority );

    ~UcdDcHallMotorTask() override;

    bool startupStep( const FreeRtosQueueTask::startupStep_t nStartupStep ) override;

    void processTaskMessage( void ) override;

    UcdDcHallMotorQueue * dcHallMotorQueue( void );

    const UcdDcHallMotorQueue * dcHallMotorQueue( void ) const;

    bool queueCtrlUartCommand( const EkxProtocol::DriverId                  u8DriverId,
                               const uint32_t                               u32MsgCounter,
                               const uint8_t                                u8RepeatCounter,
                               EkxProtocol::Payload::MessagePayload * const pMessagePayload ) override;

    bool queueAckNackReceived( const uint32_t u32MsgCounter,
                               const bool     bSuccess ) override;

    bool queueAlive( const bool bAlive ) override;

private:

    UcdDcHallMotorTask() = delete;

    esp_err_t initLedc( const bool bTryRun );

    uint32_t percent2Duty( const int nPwmDuty10th );

    void reportDriverStatus( const bool bFirst ) override;

    void processCtrlUartCommand( const EkxProtocol::DriverId                        u8DriverId,
                                 const uint32_t                                     u32MsgCounter,
                                 const uint8_t                                      u8RepeatCounter,
                                 const EkxProtocol::Payload::MessagePayload * const pMessagePayload ) override;

    void processAlive( const bool bAlive ) override;

    void processStartMotor( const uint32_t                                       u32MsgCounter,
                            const uint8_t                                        u8RepeatCounter,
                            const EkxProtocol::Payload::DcHallStartMotor * const pMessagePayload );

    void processStartMotorSteps( const uint32_t                                            u32MsgCounter,
                                 const uint8_t                                             u8RepeatCounter,
                                 const EkxProtocol::Payload::DcHallStartMotorSteps * const pMessagePayload );

    void processStopMotor( const uint32_t                                      u32MsgCounter,
                           const uint8_t                                       u8RepeatCounter,
                           const EkxProtocol::Payload::DcHallStopMotor * const pMessagePayload );

    void processTestMotor( const uint32_t                                      u32MsgCounter,
                           const uint8_t                                       u8RepeatCounter,
                           const EkxProtocol::Payload::DcHallTestMotor * const pMessagePayload );

    void processTimeoutMotorTest( void );

    void processIsrEncoderA( void );

    void processIsrFaultN( void );

    bool isMotorTestOk() const;

    void startMotor( const int  nPwmDuty10th,
                     const int  nPwmFrequency,
                     const bool bQuiet );

    void startMotorSteps( const int  nSteps,
                          const int  nPwmDuty10th,
                          const int  nPwmFrequency,
                          const bool bQuiet );

    void stopMotor( const bool bQuiet );

    void testMotor( void );

private:

    int                                                m_nSteps { 0 };

    int                                                m_nPwmDuty10th { 0 };

    int                                                m_nPwmFrequency { 1000 };

    bool                                               m_bRunning { false };

    std::chrono::time_point<std::chrono::system_clock> m_clockMotorStart;

    std::chrono::time_point<std::chrono::system_clock> m_clockMotorStop;

    UxEspCppLibrary::EspIsrGpio                        m_gpioEncoderA;

    UxEspCppLibrary::EspGpio                           m_gpioPowerEnable;

    UxEspCppLibrary::EspIsrGpio                        m_gpioFaultN;

    uint32_t                                           m_u32StepCounter { 0 };

    uint32_t                                           m_u32MsgCounter { 0 };

    bool                                               m_bUseStepCount { false };

    UxEspCppLibrary::EspLedcTimer                      m_ledcTimer;

    UxEspCppLibrary::EspLedcChannel                    m_ledcChannel;

    ledc_timer_t                                       m_nLedcTimerNum { LEDC_TIMER_MAX };

    ledc_channel_t                                     m_nLedcChannel { LEDC_CHANNEL_MAX };

    gpio_num_t                                         m_nGpioNumPowerEnable { GPIO_NUM_MAX };

    gpio_num_t                                         m_nGpioNumMotorPwm { GPIO_NUM_MAX };

    gpio_num_t                                         m_nGpioNumEncoderA { GPIO_NUM_MAX };

    gpio_num_t                                         m_nGpioNumFaultN { GPIO_NUM_MAX };

    UxEspCppLibrary::FreeRtosQueueTimer                m_timerReportOvershootSteps;

    bool                                               m_bMotorTestRunning { false };

    UxEspCppLibrary::FreeRtosQueueTimer                m_timerMotorTest;

    int                                                m_nPduMotorTestCycles { c_nMaxPduMotorTestCycles + 1 };

    bool                                               m_bFaultPinActive { false };

};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#endif /* UcdDcHallMotorTask_H */
