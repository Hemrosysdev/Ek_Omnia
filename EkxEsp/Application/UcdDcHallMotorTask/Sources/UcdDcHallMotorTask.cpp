/*
 * UcdDcHallMotorTask.cpp
 *
 *  Created on: 06.11.2021
 *      Author: gesser
 */

#include "UcdDcHallMotorTask.h"

#include <string.h>
#include "UcdDcHallMotorQueue.h"
#include "UcdDcHallMotorQueueTypes.h"

#include "../../build/version.h"
#include "ApplicationGlobals.h"
#include "EkxApplication.h"
#include "EkxProtocol.h"

/*!************************************************************************************************************************************************************
 *   Defines
 *************************************************************************************************************************************************************/

#define TIMEOUT_REPORT_OVERSHOOT_STEPS_US 300000            // 300ms

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

UcdDcHallMotorTask::UcdDcHallMotorTask( UxEspCppLibrary::EspApplication * pApplication,
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
                                        const UBaseType_t                 uTaskPriority )
    : UartControlledDevice( strUcdName,
                            "DcHallMotor",
                            u8DriverId,
                            pApplication,
                            nTaskStackSize,
                            uTaskPriority,
                            strTaskName,
                            new UcdDcHallMotorQueue() )
    , m_gpioEncoderA( strUcdName + "GpioEncoderA" )
    , m_gpioPowerEnable( strUcdName + "GpioPowerEnable" )
    , m_gpioFaultN( strUcdName + "GpioFaultN" )
    , m_ledcTimer( strUcdName + "LedcTimer" )
    , m_ledcChannel( strUcdName + "LedcChannel" )
    , m_nLedcTimerNum( nLedcTimerNum )
    , m_nLedcChannel( nLedcChannel )
    , m_nGpioNumPowerEnable( nGpioNumPowerEnable )
    , m_nGpioNumMotorPwm( nGpioNumMotorPwm )
    , m_nGpioNumEncoderA( nGpioNumEncoderA )
    , m_nGpioNumFaultN( nGpioNumFaultN )
    , m_timerReportOvershootSteps( taskQueue(),
                                   static_cast<uint32_t>( UcdDcHallMotorQueueTypes::messageType::TimeoutReportOvershootSteps ) )
    , m_timerMotorTest( taskQueue(),
                        static_cast<uint32_t>( UcdDcHallMotorQueueTypes::messageType::TimeoutMotorTest ) )
{
    logInfo( "constructor" );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

UcdDcHallMotorTask::~UcdDcHallMotorTask()
{
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

bool UcdDcHallMotorTask::startupStep( const FreeRtosQueueTask::startupStep_t nStartupStep )
{
    bool bContinueStartup = true;

    switch ( nStartupStep )
    {
        case UxEspCppLibrary::FreeRtosQueueTask::startupStep_t::startupStep1:
        {
            registerDriver();

            m_gpioPowerEnable.create( m_nGpioNumPowerEnable,
                                      GPIO_MODE_OUTPUT,
                                      GPIO_PULLUP_DISABLE,
                                      GPIO_PULLDOWN_DISABLE,
                                      GPIO_INTR_DISABLE );
            m_gpioPowerEnable.setLow();
            m_gpioEncoderA.create( m_nGpioNumEncoderA,
                                   GPIO_MODE_INPUT,
                                   GPIO_PULLUP_DISABLE,
                                   GPIO_PULLDOWN_DISABLE,
                                   GPIO_INTR_POSEDGE,
                                   taskQueue(),
                                   static_cast<int>( UcdDcHallMotorQueueTypes::messageType::IsrEncoderA ) );
            initLedc( false );
            m_gpioFaultN.create( m_nGpioNumFaultN,
                                 GPIO_MODE_INPUT,
                                 GPIO_PULLUP_DISABLE,
                                 GPIO_PULLDOWN_DISABLE,
                                 GPIO_INTR_ANYEDGE,
                                 taskQueue(),
                                 static_cast<int>( UcdDcHallMotorQueueTypes::messageType::IsrFaultN ) );
            m_bFaultPinActive = m_gpioFaultN.isLow();

            testMotor();
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

esp_err_t UcdDcHallMotorTask::initLedc( const bool bTryRun )
{
    esp_err_t nEspErr = ESP_FAIL;

    if ( bTryRun )
    {
        nEspErr = m_ledcTimer.init( LEDC_LOW_SPEED_MODE,
                                    LEDC_TIMER_10_BIT,
                                    m_nLedcTimerNum,
                                    m_nPwmFrequency );

        if ( nEspErr == ESP_OK )
        {
            nEspErr = m_ledcChannel.init( m_nLedcChannel,
                                          m_nGpioNumMotorPwm,
                                          &m_ledcTimer );

            if ( nEspErr == ESP_OK )
            {
                if ( bTryRun )
                {
                    nEspErr = m_ledcChannel.setDuty( percent2Duty( m_nPwmDuty10th ) );
                }
                else
                {
                    nEspErr = m_ledcChannel.setDuty( 0 );
                }

                if ( nEspErr != ESP_OK )
                {
                    vlogError( "initLedc(): m_ledcChannel.setDuty failed, %s", esp_err_to_name( nEspErr ) );
                }
            }
            else
            {
                vlogError( "initLedc(): m_ledcChannel.init failed, %s", esp_err_to_name( nEspErr ) );
            }
        }
        else
        {
            vlogError( "initLedc(): m_ledcTimer.init failed, %s", esp_err_to_name( nEspErr ) );
        }
    }
    else
    {
        nEspErr = ledc_stop( LEDC_LOW_SPEED_MODE, m_nLedcChannel, 0 );

        if ( nEspErr != ESP_OK )
        {
            vlogError( "initLedc(): ledc_stop failed, %s", esp_err_to_name( nEspErr ) );
        }
    }

    return nEspErr;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

uint32_t UcdDcHallMotorTask::percent2Duty( const int nPwmDuty10th )
{
    const uint32_t u32MaxDuty = ( 1 << m_ledcTimer.bitNum() ) - 1;
    const uint32_t u32Duty    = ( u32MaxDuty * std::min( nPwmDuty10th, 1000 ) ) / 1000U;

    return u32Duty;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void UcdDcHallMotorTask::processTaskMessage( void )
{
    const UcdDcHallMotorQueueTypes::message * pMessage = reinterpret_cast<const UcdDcHallMotorQueueTypes::message *>( receiveMsg() );

    if ( pMessage )
    {
        switch ( pMessage->u32Type )
        {
            case UcdDcHallMotorQueueTypes::messageType::CtrlUartCommand:
            {
                processBaseCtrlUartCommand( pMessage->payload.ctrlUartCommand.m_u8DriverId,
                                            pMessage->payload.ctrlUartCommand.m_u32MsgCounter,
                                            pMessage->payload.ctrlUartCommand.m_u8RepeatCounter,
                                            pMessage->payload.ctrlUartCommand.m_pMessagePayload );
            }
            break;

            case UcdDcHallMotorQueueTypes::messageType::Alive:
            {
                processBaseAlive( pMessage->payload.aliveStatus.m_bAlive );
            }
            break;

            case UcdDcHallMotorQueueTypes::messageType::AckNackReceived:
            {
                processAckNackReceived( pMessage->payload.ackNackReceived.m_u32MsgCounter,
                                        pMessage->payload.ackNackReceived.m_bSuccess );
            }
            break;

            case UcdDcHallMotorQueueTypes::messageType::IsrEncoderA:
            {
                processIsrEncoderA();
            }
            break;

            case UcdDcHallMotorQueueTypes::messageType::IsrFaultN:
            {
                processIsrFaultN();
            }
            break;

            case UcdDcHallMotorQueueTypes::messageType::TimeoutReportOvershootSteps:
            {
                reportDriverStatus( true );
            }
            break;

            case UcdDcHallMotorQueueTypes::messageType::TimeoutMotorTest:
            {
                processTimeoutMotorTest();
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

void UcdDcHallMotorTask::reportDriverStatus( const bool bFirst )
{
    vlogInfo( "reportDriverStatus" );

    int64_t n64RunTime = std::chrono::duration_cast<std::chrono::milliseconds>( m_clockMotorStop - m_clockMotorStart ).count();

    EkxProtocol::Payload::DcHallStatus * pPayload = new EkxProtocol::Payload::DcHallStatus();
    pPayload->valid().setValue( true );
    pPayload->faultPinActive().setValue( m_bFaultPinActive );
    pPayload->motorRunning().setValue( m_bRunning );
    pPayload->motorRunTime().setValue( n64RunTime );
    pPayload->motorTestOk().setValue( ( m_nPduMotorTestCycles < c_nMaxPduMotorTestCycles ) );
    pPayload->stepCounter().setValue( m_u32StepCounter );

    sendCtrlUartReply( pPayload, bFirst );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

UcdDcHallMotorQueue * UcdDcHallMotorTask::dcHallMotorQueue( void )
{
    return dynamic_cast<UcdDcHallMotorQueue *>( taskQueue() );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

const UcdDcHallMotorQueue * UcdDcHallMotorTask::dcHallMotorQueue( void ) const
{
    return dynamic_cast<const UcdDcHallMotorQueue *>( taskQueue() );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void UcdDcHallMotorTask::startMotor( const int  nPwmDuty10th,
                                     const int  nPwmFrequency,
                                     const bool bQuiet )
{
    vlogInfo( "startMotor, %d%% 1/10 PWM duty, %d Hz", nPwmDuty10th, nPwmFrequency );
    vlogInfo( "startMotor, last step counter %d", m_u32StepCounter );

    m_nPwmDuty10th   = nPwmDuty10th;
    m_nPwmFrequency  = nPwmFrequency;
    m_bUseStepCount  = false;
    m_u32StepCounter = 0;

    if ( initLedc( true ) == ESP_OK )
    {
        m_bRunning = true;
        m_gpioPowerEnable.setHigh();
        m_clockMotorStart = std::chrono::system_clock::now();
    }
    else
    {
        m_bRunning = false;
    }

    if ( !bQuiet )
    {
        reportDriverStatus( true );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void UcdDcHallMotorTask::startMotorSteps( const int  nSteps,
                                          const int  nPwmDuty10th,
                                          const int  nPwmFrequency,
                                          const bool bQuiet )
{
    vlogInfo( "startMotorSteps, %d steps, %d%% 1/10 PWM duty, %d Hz", nSteps, nPwmDuty10th, nPwmFrequency );
    vlogInfo( "startMotorSteps, last step counter %d", m_u32StepCounter );

    m_nSteps         = nSteps;
    m_nPwmDuty10th   = nPwmDuty10th;
    m_nPwmFrequency  = nPwmFrequency;
    m_bUseStepCount  = true;
    m_u32StepCounter = 0;

    if ( initLedc( true ) == ESP_OK )
    {
        m_bRunning = true;
        m_gpioPowerEnable.setHigh();
        m_clockMotorStart = std::chrono::system_clock::now();
    }
    else
    {
        m_bRunning = false;
    }

    if ( !bQuiet )
    {
        reportDriverStatus( true );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void UcdDcHallMotorTask::stopMotor( const bool bQuiet )
{
    vlogInfo( "stopMotor" );

    if ( m_bRunning )
    {
        m_clockMotorStop = std::chrono::system_clock::now();
    }

    m_bRunning = false;

    initLedc( false );
    m_gpioPowerEnable.setLow();

    if ( !bQuiet )
    {
        reportDriverStatus( true );
    }

    m_timerReportOvershootSteps.startOnce( TIMEOUT_REPORT_OVERSHOOT_STEPS_US );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void UcdDcHallMotorTask::processIsrEncoderA()
{
    m_u32StepCounter++;

    if ( m_bRunning )
    {
        if ( m_bUseStepCount
             && ( m_u32StepCounter >= m_nSteps ) )
        {
            stopMotor( false );
        }
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void UcdDcHallMotorTask::processIsrFaultN()
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

void UcdDcHallMotorTask::processAlive( const bool bAlive )
{
    if ( !bAlive )
    {
        stopMotor( true );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void UcdDcHallMotorTask::testMotor( void )
{
    if ( !m_bMotorTestRunning )
    {
        m_u32StepCounter      = 0;
        m_nPduMotorTestCycles = 0;
        m_timerMotorTest.startOnce( 50000 );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void UcdDcHallMotorTask::processTimeoutMotorTest( void )
{
    if ( m_bMotorTestRunning )
    {
        m_bMotorTestRunning = false;
        stopMotor( true );

        if ( !m_u32StepCounter )
        {
            m_timerMotorTest.startOnce( 100000 );
            vlogInfo( "processTaskMessage() no steps - wait cycle %d", m_nPduMotorTestCycles );
        }
        else
        {
            vlogInfo( "processTaskMessage() steps %d, trials %d - PDU detected", m_u32StepCounter, m_nPduMotorTestCycles );
            reportDriverStatus( true );
        }
    }
    else
    {
        if ( m_u32StepCounter )
        {
            vlogInfo( "processTaskMessage() steps %d, trials %d (post stop) - PDU detected", m_u32StepCounter, m_nPduMotorTestCycles );
            reportDriverStatus( true );
        }
        else if ( m_nPduMotorTestCycles < c_nMaxPduMotorTestCycles )
        {
            m_timerMotorTest.startOnce( 50000 );
            m_bMotorTestRunning = true;
            startMotor( 100, 100, true );
            m_nPduMotorTestCycles++;
        }
        else
        {
            m_nPduMotorTestCycles++;
            vlogWarning( "processTaskMessage() abort PDU motor test - no PDU detected" );
            reportDriverStatus( true );
        }
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

bool UcdDcHallMotorTask::queueCtrlUartCommand( const EkxProtocol::DriverId                  u8DriverId,
                                               const uint32_t                               u32MsgCounter,
                                               const uint8_t                                u8RepeatCounter,
                                               EkxProtocol::Payload::MessagePayload * const pMessagePayload )
{
    return dcHallMotorQueue()->sendCtrlUartCommand( u8DriverId, u32MsgCounter, u8RepeatCounter, pMessagePayload ) == pdPASS;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

bool UcdDcHallMotorTask::queueAlive( const bool bAlive )
{
    return dcHallMotorQueue()->sendAlive( bAlive );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void UcdDcHallMotorTask::processCtrlUartCommand( const EkxProtocol::DriverId                        u8DriverId,
                                                 const uint32_t                                     u32MsgCounter,
                                                 const uint8_t                                      u8RepeatCounter,
                                                 const EkxProtocol::Payload::MessagePayload * const pMessagePayload )
{
    switch ( pMessagePayload->payloadType() )
    {
        case EkxProtocol::Payload::PayloadType::DcHallStartMotor:
        {
            processStartMotor( u32MsgCounter, u8RepeatCounter, dynamic_cast<const EkxProtocol::Payload::DcHallStartMotor *>( pMessagePayload ) );
        }
        break;
        case EkxProtocol::Payload::PayloadType::DcHallStartMotorSteps:
        {
            processStartMotorSteps( u32MsgCounter, u8RepeatCounter, dynamic_cast<const EkxProtocol::Payload::DcHallStartMotorSteps *>( pMessagePayload ) );
        }
        break;
        case EkxProtocol::Payload::PayloadType::DcHallStopMotor:
        {
            processStopMotor( u32MsgCounter, u8RepeatCounter, dynamic_cast<const EkxProtocol::Payload::DcHallStopMotor *>( pMessagePayload ) );
        }
        break;
        case EkxProtocol::Payload::PayloadType::DcHallTestMotor:
        {
            processTestMotor( u32MsgCounter, u8RepeatCounter, dynamic_cast<const EkxProtocol::Payload::DcHallTestMotor *>( pMessagePayload ) );
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

void UcdDcHallMotorTask::processStartMotor( const uint32_t                                       u32MsgCounter,
                                            const uint8_t                                        u8RepeatCounter,
                                            const EkxProtocol::Payload::DcHallStartMotor * const pMessagePayload )
{
    if ( pMessagePayload )
    {
        uint32_t u32PwmDuty10th  = pMessagePayload->pwmDuty10th().value();
        uint32_t u32PwmFrequency = pMessagePayload->pwmFrequency().value();

        startMotor( u32PwmDuty10th, u32PwmFrequency, false );

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

void UcdDcHallMotorTask::processStartMotorSteps( const uint32_t                                            u32MsgCounter,
                                                 const uint8_t                                             u8RepeatCounter,
                                                 const EkxProtocol::Payload::DcHallStartMotorSteps * const pMessagePayload )
{
    if ( pMessagePayload )
    {
        uint32_t u32Steps        = pMessagePayload->steps().value();
        uint32_t u32PwmDuty10th  = pMessagePayload->pwmDuty10th().value();
        uint32_t u32PwmFrequency = pMessagePayload->pwmFrequency().value();

        startMotorSteps( u32Steps, u32PwmDuty10th, u32PwmFrequency, false );

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

void UcdDcHallMotorTask::processStopMotor( const uint32_t                                      u32MsgCounter,
                                           const uint8_t                                       u8RepeatCounter,
                                           const EkxProtocol::Payload::DcHallStopMotor * const pMessagePayload )
{
    if ( pMessagePayload )
    {
        stopMotor( false );

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

void UcdDcHallMotorTask::processTestMotor( const uint32_t                                      u32MsgCounter,
                                           const uint8_t                                       u8RepeatCounter,
                                           const EkxProtocol::Payload::DcHallTestMotor * const pMessagePayload )
{
    if ( pMessagePayload )
    {
        testMotor();

        sendCtrlUartReplyAck( u32MsgCounter, u8RepeatCounter );
    }
    else
    {
        vlogError( "processTestMotor: message payload broken" );
        sendCtrlUartReplyNack( u32MsgCounter, u8RepeatCounter, EkxProtocol::Payload::Nack::NackReason::PayloadBroken );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

bool UcdDcHallMotorTask::isMotorTestOk() const
{
    return m_nPduMotorTestCycles < c_nMaxPduMotorTestCycles;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

bool UcdDcHallMotorTask::queueAckNackReceived( const uint32_t u32MsgCounter,
                                               const bool     bSuccess )
{
    return dcHallMotorQueue()->sendAckNackReceived( u32MsgCounter, bSuccess ) == pdPASS;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/
