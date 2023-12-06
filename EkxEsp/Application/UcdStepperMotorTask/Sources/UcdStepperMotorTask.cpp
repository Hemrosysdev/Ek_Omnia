/*
 * UcdStepperMotorTask.cpp
 *
 *  Created on: 31.10.2021
 *      Author: gesser
 */

#include "UcdStepperMotorTask.h"

#include <cstring>

#include "UcdStepperMotorQueue.h"
#include "UcdStepperMotorQueueTypes.h"
#include "ApplicationGlobals.h"
#include "EkxApplication.h"
#include "EkxProtocol.h"

/*!************************************************************************************************************************************************************
 *   Defines
 *************************************************************************************************************************************************************/

#define RAMP_PERIOD_US      500

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

UcdStepperMotorTask::UcdStepperMotorTask( UxEspCppLibrary::EspApplication * pApplication,
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
                                          const UBaseType_t                 uTaskPriority )
    : UartControlledDevice( strUcdName,
                            "StepperMotor",
                            u8DriverId,
                            pApplication,
                            nTaskStackSize,
                            uTaskPriority,
                            strTaskName,
                            new UcdStepperMotorQueue() )
    , m_nGpioNumFaultN( nGpioNumFaultN )
    , m_gpioDirection( nGpioNumDirection,
                       GPIO_MODE_OUTPUT,
                       GPIO_PULLUP_DISABLE,
                       GPIO_PULLDOWN_DISABLE,
                       GPIO_INTR_DISABLE,
                       strUcdName + "GpioDirection" )
#ifndef STEPPER_USE_LEDC_PWM
    , m_gpioStep( nGpioNumStep,
                  GPIO_MODE_OUTPUT,
                  GPIO_PULLUP_DISABLE,
                  GPIO_PULLDOWN_DISABLE,
                  GPIO_INTR_DISABLE,
                  strUcdName + "GpioStep" )
#endif
    , m_gpioEnable( nGpioNumEnable,
                    GPIO_MODE_OUTPUT,
                    GPIO_PULLUP_DISABLE,
                    GPIO_PULLDOWN_DISABLE,
                    GPIO_INTR_DISABLE,
                    strUcdName + "GpioEnable" )
    , m_gpioFaultN( strUcdName + "GpioFaultN" )
#ifndef STEPPER_USE_LEDC_PWM
    , m_timerStep( this )
#endif
    , m_timerRampStep( taskQueue(),
                       static_cast<uint32_t>( UcdStepperMotorQueueTypes::messageType::TimeoutRampStep ) )
//, m_timerTest( taskQueue(),
//               static_cast<uint32_t>( UcdStepperMotorQueueTypes::messageType::ISR_FAULT_N ) )
#ifdef STEPPER_USE_LEDC_PWM
    , m_motorStepTimer( strUcdName + "StepLedcTimer" )
    , m_motorStep( strUcdName + "StepLedcChannel" )
    , m_nGpioNumStep( nGpioNumStep )
    , m_nStepLedcTimerNum( nStepLedcTimerNum )
    , m_nStepLedcChannel( nStepLedcChannel )
#endif
{
    FreeRtosQueueTask::logInfo( "constructor" );

    m_gpioEnable.setHigh();
#ifndef STEPPER_USE_LEDC_PWM
    m_gpioStep.setLow();
#endif

    //m_timerTest.startPeriodic(500000);
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

UcdStepperMotorTask::~UcdStepperMotorTask()
{
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

bool UcdStepperMotorTask::startupStep( const FreeRtosQueueTask::startupStep_t nStartupStep )
{
    bool bContinueStartup = true;

    switch ( nStartupStep )
    {
        case UxEspCppLibrary::FreeRtosQueueTask::startupStep_t::startupStep1:
        {
            registerDriver();

            m_gpioFaultN.create( m_nGpioNumFaultN,
                                 GPIO_MODE_INPUT,
                                 GPIO_PULLUP_DISABLE,
                                 GPIO_PULLDOWN_DISABLE,
                                 GPIO_INTR_ANYEDGE,
                                 taskQueue(),
                                 static_cast<int>( UcdStepperMotorQueueTypes::messageType::IsrFaultN ) );
            m_bFaultPinActive = m_gpioFaultN.isLow();

#ifdef STEPPER_USE_LEDC_PWM
            initLedc( false );
#endif

#ifdef STEPS_STRESS_TEST
            startMotorSteps( m_bTestDirection,
                             c_nTestSteps,
                             c_nTestStartFrequency,
                             c_nTestRunFrequency,
                             c_nTestRampFreqPerStep );
#endif
        }
        break;

        default:
        {
            // do nothing
        }
        break;
    }

    return bContinueStartup;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#ifdef STEPPER_USE_LEDC_PWM
esp_err_t UcdStepperMotorTask::initLedc( const bool bTryRun )
{
    esp_err_t nEspErr = ESP_FAIL;

    if ( bTryRun )
    {
        nEspErr = m_motorStepTimer.init( LEDC_LOW_SPEED_MODE,
                                         LEDC_TIMER_10_BIT,
                                         m_nStepLedcTimerNum,
                                         100 );

        if ( nEspErr == ESP_OK )
        {
            nEspErr = m_motorStep.init( m_nStepLedcChannel,
                                        m_nGpioNumStep,
                                        &m_motorStepTimer );

            if ( nEspErr == ESP_OK )
            {
                if ( bTryRun )
                {
                    const int      nPwmDuty   = 50;
                    const uint32_t u32MaxDuty = ( 1 << m_motorStepTimer.bitNum() ) - 1;
                    const uint32_t u32Duty    = ( u32MaxDuty * std::min( nPwmDuty, 50 ) ) / 100U;

                    nEspErr = m_motorStep.setDuty( u32Duty );
                }
                else
                {
                    nEspErr = m_motorStep.setDuty( 0 );
                }

                if ( nEspErr != ESP_OK )
                {
                    vlogError( "initLedc(): m_motorStep.setDuty failed, %s", esp_err_to_name( nEspErr ) );
                }
            }
            else
            {
                vlogError( "initLedc(): m_motorStep.init failed, %s", esp_err_to_name( nEspErr ) );
            }
        }
        else
        {
            vlogError( "initLedc(): m_motorStepTimer.init failed, %s", esp_err_to_name( nEspErr ) );
        }
    }
    else
    {
        nEspErr = ledc_stop( LEDC_LOW_SPEED_MODE, m_nStepLedcChannel, 0 );

        if ( nEspErr != ESP_OK )
        {
            vlogError( "initLedc(): ledc_stop failed, %s", esp_err_to_name( nEspErr ) );
        }
    }

    return nEspErr;
}

#endif

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void UcdStepperMotorTask::processTaskMessage( void )
{
    const UcdStepperMotorQueueTypes::message * pMessage = reinterpret_cast<const UcdStepperMotorQueueTypes::message *>( receiveMsg() );

    if ( pMessage )
    {
        switch ( pMessage->u32Type )
        {
            case UcdStepperMotorQueueTypes::messageType::CtrlUartCommand:
            {
                processBaseCtrlUartCommand( pMessage->payload.ctrlUartCommand.m_u8DriverId,
                                            pMessage->payload.ctrlUartCommand.m_u32MsgCounter,
                                            pMessage->payload.ctrlUartCommand.m_u8RepeatCounter,
                                            pMessage->payload.ctrlUartCommand.m_pMessagePayload );
            }
            break;

            case UcdStepperMotorQueueTypes::messageType::AckNackReceived:
            {
                processAckNackReceived( pMessage->payload.ackNackReceived.m_u32MsgCounter,
                                        pMessage->payload.ackNackReceived.m_bSuccess );
            }
            break;

            case UcdStepperMotorQueueTypes::messageType::Alive:
            {
                processBaseAlive( pMessage->payload.aliveStatus.m_bAlive );
            }
            break;

            case UcdStepperMotorQueueTypes::messageType::TimeoutRampStep:
            {
                processRampStep();
            }
            break;

            case UcdStepperMotorQueueTypes::messageType::IsrFaultN:
            {
                processIsrFaultN();
            }
            break;

            default:
            {
                vlogError( "processTaskMessage: receive illegal message type %d",
                           pMessage->u32Type );
            }
            break;
        }
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

bool UcdStepperMotorTask::queueCtrlUartCommand( const EkxProtocol::DriverId                  u8DriverId,
                                                const uint32_t                               u32MsgCounter,
                                                const uint8_t                                u8RepeatCounter,
                                                EkxProtocol::Payload::MessagePayload * const pMessagePayload )
{
    return stepperMotorQueue()->sendCtrlUartCommand( u8DriverId, u32MsgCounter, u8RepeatCounter, pMessagePayload ) == pdPASS;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

bool UcdStepperMotorTask::queueAlive( const bool bAlive )
{
    return stepperMotorQueue()->sendAlive( bAlive );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void UcdStepperMotorTask::reportDriverStatus( const bool bFirst )
{
    vlogInfo( "reportDriverStatus: first %d, running %d", bFirst, m_bRunning );

    EkxProtocol::Payload::StepperMotorStatus * pPayload = new EkxProtocol::Payload::StepperMotorStatus();
    pPayload->valid().setValue( true );
    pPayload->faultPinActive().setValue( m_bFaultPinActive );
    pPayload->motorRunning().setValue( m_bRunning );

    sendCtrlUartReply( pPayload, bFirst );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

UcdStepperMotorQueue * UcdStepperMotorTask::stepperMotorQueue( void )
{
    return dynamic_cast<UcdStepperMotorQueue *>( taskQueue() );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

const UcdStepperMotorQueue * UcdStepperMotorTask::stepperMotorQueue( void ) const
{
    return dynamic_cast<const UcdStepperMotorQueue *>( taskQueue() );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void UcdStepperMotorTask::startMotor( const bool bDirection,
                                      const int  nStartFrequency,
                                      const int  nRunFrequency,
                                      const int  nRampFreqPerStep )
{
    vlogInfo( "startMotor, dir %d, start %d Hz, run %d Hz, ramp %d Hz/500us",
              bDirection,
              nStartFrequency,
              nRunFrequency,
              nRampFreqPerStep );

    m_bDirection        = bDirection;
    m_bRampUp           = true;
    m_nStartFrequency   = std::min( nStartFrequency, nRunFrequency );
    m_nRunFrequency     = nRunFrequency;
    m_nRampFreqPerStep  = nRampFreqPerStep;
    m_bUseStepCount     = false;
    m_u32StepCounter    = 0;
    m_bStepHigh         = false;
    m_nCurrentFrequency = 0;

    m_bRunning = true;

    m_gpioDirection.set( !m_bDirection );
    m_gpioEnable.setLow();
#ifdef STEPPER_USE_LEDC_PWM
    initLedc( true );
#else
    m_gpioStep.setLow();
#endif

    m_timerRampStep.startPeriodic( RAMP_PERIOD_US );

    setFrequency( m_nStartFrequency );

    reportDriverStatus( true );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void UcdStepperMotorTask::startMotorSteps( const bool bDirection,
                                           const int  nSteps,
                                           const int  nStartFrequency,
                                           const int  nRunFrequency,
                                           const int  nRampFreqPerStep )
{
    vlogInfo( "startMotorSteps, dir %d, %d steps, start %d Hz, run %d Hz, ramp %d Hz/500us",
              bDirection,
              nSteps,
              nStartFrequency,
              nRunFrequency,
              nRampFreqPerStep );

    m_bDirection        = bDirection;
    m_nSteps            = nSteps;
    m_bRampUp           = true;
    m_nStartFrequency   = std::min( nStartFrequency, nRunFrequency );
    m_nRunFrequency     = nRunFrequency;
    m_nRampFreqPerStep  = nRampFreqPerStep;
    m_bUseStepCount     = true;
    m_u32StepCounter    = 0;
    m_bStepHigh         = false;
    m_nCurrentFrequency = 0;

    m_bRunning = true;

    m_gpioDirection.set( !m_bDirection );
    m_gpioEnable.setLow();
#ifdef STEPPER_USE_LEDC_PWM
    initLedc( true );
#else
    m_gpioStep.setLow();
#endif

    m_timerRampStep.startPeriodic( RAMP_PERIOD_US );

    setFrequency( m_nStartFrequency );

    reportDriverStatus( true );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void UcdStepperMotorTask::stopMotor()
{
    vlogInfo( "stopMotor" );

    if ( m_bRunning )
    {
        m_bRunning          = false;
        m_nCurrentFrequency = 0;

        m_timerRampStep.stop();

#ifndef STEPPER_USE_LEDC_PWM
        m_timerStep.stop();
#endif

        m_gpioEnable.setHigh();
#ifdef STEPPER_USE_LEDC_PWM
        initLedc( false );
#else
        m_gpioStep.setLow();
#endif

        reportDriverStatus( true );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void UcdStepperMotorTask::processAlive( const bool bAlive )
{
    if ( !bAlive )
    {
        stopMotor();
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void UcdStepperMotorTask::setFrequency( const int nFrequency )
{
    if ( m_nCurrentFrequency != nFrequency )
    {
        //vlogInfo( "setFrequency %d", nFrequency );

        m_nCurrentFrequency = nFrequency;

#ifdef STEPPER_USE_LEDC_PWM
        m_motorStepTimer.setFreqHz( m_nCurrentFrequency );
#else
        const uint64_t u64StepSizeUs = 500000 / nFrequency;
        m_timerStep.startPeriodic( u64StepSizeUs );
#endif
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void UcdStepperMotorTask::processRampStep()
{
    //vlogInfo( "processRampStep" );

    if ( m_bRunning
         && ( m_bRampUp
              || m_bRampDown ) )
    {
        if ( m_nCurrentFrequency == m_nRunFrequency )
        {
            vlogInfo( "processRampStep: ramp stopped" );
            m_bRampUp   = false;
            m_bRampDown = false;
            m_timerRampStep.stop();
        }
        else if ( m_bRampUp
                  && m_nCurrentFrequency < m_nRunFrequency )
        {
            setFrequency( std::min( m_nCurrentFrequency + m_nRampFreqPerStep, m_nRunFrequency ) );
        }
        else if ( m_bRampDown
                  && m_nCurrentFrequency > m_nRunFrequency )
        {
            setFrequency( std::max( m_nCurrentFrequency - m_nRampFreqPerStep, m_nStartFrequency ) );
        }
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void UcdStepperMotorTask::processIsrFaultN()
{
    vlogError( "processIsrFaultN %d", m_gpioFaultN.isLow() );

    if ( m_gpioFaultN.isLow() != m_bFaultPinActive )
    {
        m_bFaultPinActive = m_gpioFaultN.isLow();

        reportDriverStatus( true );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#ifndef STEPPER_USE_LEDC_PWM
void UcdStepperMotorTask::processMotorStep( void )
{
    if ( m_bRunning )
    {
        m_bStepHigh = !m_bStepHigh;
        m_gpioStep.set( m_bStepHigh );

        if ( m_bStepHigh )
        {
            m_u32StepCounter++;
        }
        else
        {
            if ( m_bUseStepCount
                 && ( m_u32StepCounter >= m_nSteps ) )
            {
                stopMotor();

#ifdef STEPS_STRESS_TEST
                m_bTestDirection = !m_bTestDirection;
                startMotorSteps( m_bTestDirection,
                                 c_nTestSteps,
                                 c_nTestStartFrequency,
                                 c_nTestRunFrequency,
                                 c_nTestRampFreqPerStep );
#endif
            }

            // do ramp up
            //            if ( ( m_nRunFrequency > m_nStartFrequency )
            //                    && ( m_nRampFreqPerStep > 0 )
            //                    && ( m_nCurrentFrequency != m_nRunFrequency ) )
            //            {
            //                setFrequency( std::min( m_nCurrentFrequency + m_nRampFreqPerStep, m_nRunFrequency ) );
            //            }
        }
    }
}

#endif

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void UcdStepperMotorTask::processCtrlUartCommand( const EkxProtocol::DriverId                        u8DriverId,
                                                  const uint32_t                                     u32MsgCounter,
                                                  const uint8_t                                      u8RepeatCounter,
                                                  const EkxProtocol::Payload::MessagePayload * const pMessagePayload )
{
    vlogInfo( "processCtrlUartCommand cmd %d msgCnt %d repeatCnt %d",
              static_cast<int>( pMessagePayload->payloadType() ),
              u32MsgCounter,
              u8RepeatCounter );

    switch ( pMessagePayload->payloadType() )
    {
        case EkxProtocol::Payload::PayloadType::StepperMotorStart:
        {
            processStartMotor( u32MsgCounter, u8RepeatCounter, dynamic_cast<const EkxProtocol::Payload::StepperMotorStart *>( pMessagePayload ) );
        }
        break;
        case EkxProtocol::Payload::PayloadType::StepperMotorStartSteps:
        {
            processStartMotorSteps( u32MsgCounter, u8RepeatCounter, dynamic_cast<const EkxProtocol::Payload::StepperMotorStartSteps *>( pMessagePayload ) );
        }
        break;
        case EkxProtocol::Payload::PayloadType::StepperMotorStop:
        {
            processStopMotor( u32MsgCounter, u8RepeatCounter, dynamic_cast<const EkxProtocol::Payload::StepperMotorStop *>( pMessagePayload ) );
        }
        break;
        case EkxProtocol::Payload::PayloadType::StepperMotorChangeRunFrequency:
        {
            processChangeRunFrequency( u32MsgCounter, u8RepeatCounter, dynamic_cast<const EkxProtocol::Payload::StepperMotorChangeRunFrequency *>( pMessagePayload ) );
        }
        break;
        default:
        {
            vlogError( "processCtrlUartCommand: cant't process data; unknown command" );
            sendCtrlUartReplyNack( u32MsgCounter, u8RepeatCounter, EkxProtocol::Payload::Nack::NackReason::UnknownDriverCommand );
        }
        break;
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void UcdStepperMotorTask::processStartMotor( const uint32_t                                        u32MsgCounter,
                                             const uint8_t                                         u8RepeatCounter,
                                             const EkxProtocol::Payload::StepperMotorStart * const pMessagePayload )
{
    if ( pMessagePayload )
    {
        bool     bDirection         = pMessagePayload->direction().value();
        uint32_t u32StartFrequency  = pMessagePayload->startFrequency().value();
        uint32_t u32RunFrequency    = pMessagePayload->runFrequency().value();
        uint32_t u32RampFreqPerStep = pMessagePayload->rampFreqPerStep().value();

        startMotor( bDirection, u32StartFrequency, u32RunFrequency, u32RampFreqPerStep );

        sendCtrlUartReplyAck( u32MsgCounter, u8RepeatCounter );
    }
    else
    {
        vlogError( "processStopMotor: message payload broken" );
        sendCtrlUartReplyNack( u32MsgCounter, u8RepeatCounter, EkxProtocol::Payload::Nack::NackReason::PayloadBroken );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void UcdStepperMotorTask::processStartMotorSteps( const uint32_t                                             u32MsgCounter,
                                                  const uint8_t                                              u8RepeatCounter,
                                                  const EkxProtocol::Payload::StepperMotorStartSteps * const pMessagePayload )
{
    if ( pMessagePayload )
    {
        bool     bDirection         = pMessagePayload->direction().value();
        uint32_t u32Steps           = pMessagePayload->steps().value();
        uint32_t u32StartFrequency  = pMessagePayload->startFrequency().value();
        uint32_t u32RunFrequency    = pMessagePayload->runFrequency().value();
        uint32_t u32RampFreqPerStep = pMessagePayload->rampFreqPerStep().value();

        startMotorSteps( bDirection, u32Steps, u32StartFrequency, u32RunFrequency, u32RampFreqPerStep );

        sendCtrlUartReplyAck( u32MsgCounter, u8RepeatCounter );
    }
    else
    {
        vlogError( "processStartMotorSteps: message payload broken" );
        sendCtrlUartReplyNack( u32MsgCounter, u8RepeatCounter, EkxProtocol::Payload::Nack::NackReason::PayloadBroken );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void UcdStepperMotorTask::processStopMotor( const uint32_t                                       u32MsgCounter,
                                            const uint8_t                                        u8RepeatCounter,
                                            const EkxProtocol::Payload::StepperMotorStop * const pMessagePayload )
{
    if ( pMessagePayload )
    {
        stopMotor();

        sendCtrlUartReplyAck( u32MsgCounter, u8RepeatCounter );
    }
    else
    {
        vlogError( "processStopMotor: message payload broken" );
        sendCtrlUartReplyNack( u32MsgCounter, u8RepeatCounter, EkxProtocol::Payload::Nack::NackReason::PayloadBroken );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void UcdStepperMotorTask::processChangeRunFrequency( const uint32_t                                                     u32MsgCounter,
                                                     const uint8_t                                                      u8RepeatCounter,
                                                     const EkxProtocol::Payload::StepperMotorChangeRunFrequency * const pMessagePayload )
{
    if ( pMessagePayload )
    {
        uint32_t u32RunFrequency = pMessagePayload->runFrequency().value();

        setFrequency( u32RunFrequency );

        sendCtrlUartReplyAck( u32MsgCounter, u8RepeatCounter );
    }
    else
    {
        vlogError( "processChangeRunFrequency: message payload broken" );
        sendCtrlUartReplyNack( u32MsgCounter, u8RepeatCounter, EkxProtocol::Payload::Nack::NackReason::PayloadBroken );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

bool UcdStepperMotorTask::queueAckNackReceived( const uint32_t u32MsgCounter,
                                                const bool     bSuccess )
{
    return stepperMotorQueue()->sendAckNackReceived( u32MsgCounter, bSuccess ) == pdPASS;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/
