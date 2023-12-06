/*!*************************************************************************************************************************************************************
 *
 * @file UcdPwmOutTask.cpp
 * @brief Source file of component unit UcdPwmOut.
 *
 * This file was developed as part of Spotlight.
 *
 * @author Ultratronik GmbH
 *         Dornierstr. 9
 *         D-82205 Gilching
 *         http://www.ultratronik.de
 *
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
 *      Includes
 **************************************************************************************************************************************************************/

#include "UcdPwmOutTask.h"

#include <string>
#include "UartControlledDevice.h"
#include "CtrlUartDispatcher.h"

#include "UcdPwmOutQueue.h"
#include "UcdPwmOutQueueTypes.h"
#include "EspLog.h"
#include "FreeRtosQueueTask.h"

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

UcdPwmOutTask::UcdPwmOutTask( UxEspCppLibrary::EspApplication * pApplication,
                              const gpio_num_t                  nGpioNum,
                              const ledc_timer_t                nLedcTimerNum,
                              const ledc_channel_t              nLedcChannel,
                              const uint8_t                     u8InitPercent,
                              const EkxProtocol::DriverId       u8DriverId,
                              const std::string &               strUcdName,
                              const std::string &               strTaskName,
                              const int                         nTaskStackSize,
                              const UBaseType_t                 uTaskPriority )
    : UartControlledDevice( strUcdName,
                            "Pwm",
                            u8DriverId,
                            pApplication,
                            nTaskStackSize,
                            uTaskPriority,
                            strTaskName,
                            new UcdPwmOutQueue() )
    , m_nGpioNum( nGpioNum )
    , m_nLedcTimerNum( nLedcTimerNum )
    , m_nLedcChannel( nLedcChannel )
    , m_u8InitPercent( u8InitPercent )
    , m_theLedcFader( strTaskName + "LedcFader" )
    , m_theLedcTimer( strTaskName + "LedcTimer" )
{
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

UcdPwmOutTask::~UcdPwmOutTask()
{

}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

bool UcdPwmOutTask::startupStep( const FreeRtosQueueTask::startupStep_t nStartupStep )
{
    bool bContinueStartup = true;

    switch ( nStartupStep )
    {
        case UxEspCppLibrary::FreeRtosQueueTask::startupStep_t::startupStep1:
        {
            registerDriver();

            initEspLedcTimer( LEDC_HIGH_SPEED_MODE,
                              LEDC_TIMER_13_BIT,
                              m_nLedcTimerNum,
                              5000UL );
            initEspLedcFader();
            installEspLedcFader();
            setPwmDuty( m_u8InitPercent );
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

void UcdPwmOutTask::processTaskMessage( void )
{
    const UcdPwmOutQueueTypes::message * pMessage = reinterpret_cast<const UcdPwmOutQueueTypes::message *>( receiveMsg() );

    if ( pMessage )
    {
        switch ( pMessage->u32Type )
        {
            case UcdPwmOutQueueTypes::messageType::CtrlUartCommand:
            {
                processBaseCtrlUartCommand( pMessage->payload.ctrlUartCommand.m_u8DriverId,
                                            pMessage->payload.ctrlUartCommand.m_u32MsgCounter,
                                            pMessage->payload.ctrlUartCommand.m_u8RepeatCounter,
                                            pMessage->payload.ctrlUartCommand.m_pMessagePayload );
            }
            break;

            case UcdPwmOutQueueTypes::messageType::AckNackReceived:
            {
                processAckNackReceived( pMessage->payload.ackNackReceived.m_u32MsgCounter,
                                        pMessage->payload.ackNackReceived.m_bSuccess );
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

UcdPwmOutQueue * UcdPwmOutTask::pwmOutQueue( void )
{
    return dynamic_cast<UcdPwmOutQueue *>( taskQueue() );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

const UcdPwmOutQueue * UcdPwmOutTask::pwmOutQueue( void ) const
{
    return dynamic_cast<const UcdPwmOutQueue *>( taskQueue() );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

bool UcdPwmOutTask::initEspLedcTimer( const ledc_mode_t      nSpeedMode,
                                      const ledc_timer_bit_t nBitNum,
                                      const ledc_timer_t     nTimerNum,
                                      const uint32_t         u32FreqHz )
{
    bool bSuccess = false;

    // if this the first instance of PWMOUT class, init the timer.
    const esp_err_t nEspErr = m_theLedcTimer.init( nSpeedMode,
                                                   nBitNum,
                                                   nTimerNum,
                                                   u32FreqHz );
    if ( ESP_OK == nEspErr )
    {
        bSuccess = true;
    }
    else
    {
        vlogError( "EspLedcTimer.init Failed with %s", esp_err_to_name( nEspErr ) );
    }

    return bSuccess;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

bool UcdPwmOutTask::initEspLedcFader( void )
{
    bool bSuccess = false;

    uint8_t  u8FadeInDutyPercent;
    uint8_t  u8FadeOutDutyPercent;
    uint32_t u32FadeInTimeMs;
    uint32_t u32FadeOutTimeMs;

    channelDefaultFaderSettings( &u8FadeInDutyPercent,
                                 &u8FadeOutDutyPercent,
                                 &u32FadeInTimeMs,
                                 &u32FadeOutTimeMs );

    const esp_err_t res = m_theLedcFader.init( m_nLedcChannel,
                                               m_nGpioNum,
                                               &m_theLedcTimer,
                                               percent2Duty( u8FadeInDutyPercent ),  //u32FadeInDuty
                                               u32FadeInTimeMs,                      //u32FadeInTimeMs
                                               percent2Duty( u8FadeOutDutyPercent ), // u32FadeOutDuty
                                               u32FadeOutTimeMs );                   // u32FadeOutTimeMs

    if ( ESP_OK == res )
    {
        bSuccess = true;
    }
    else
    {
        vlogError( "initEspLedcFader.init Failed with %s", esp_err_to_name( res ) );
    }

    return bSuccess;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

bool UcdPwmOutTask::installEspLedcFader( void )
{
    bool bSuccess = false;

    const esp_err_t res = UxEspCppLibrary::EspLedcFader::install( 0 );

    if ( ESP_OK == res )
    {
        bSuccess = true;
    }
    else
    {
        vlogError( "installEspLedcFader.install Failed with %s", esp_err_to_name( res ) );
    }

    return bSuccess;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

uint32_t UcdPwmOutTask::percent2Duty( uint8_t percent ) const
{
    uint32_t duty;

    if ( percent > 200U )
    {   //clamp down
        percent = 200U;
    }

    const ledc_timer_bit_t bit     = m_theLedcTimer.bitNum();
    uint32_t               maxDuty = 2 << ( ( ( uint8_t ) bit ) - 1U );

    switch ( percent )
    {
        case 0:
            duty = 0UL;
            break;
        case 200:
            duty = maxDuty;
            break;
        default:
            duty = ( maxDuty * percent ) / 200U;
            break;
    }

    return duty;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void UcdPwmOutTask::fadeIn( void )
{
    const esp_err_t res = m_theLedcFader.fadeIn();

    if ( res != ESP_OK )
    {
        vlogError( "fadeIn: Failed with %s", esp_err_to_name( res ) );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void UcdPwmOutTask::fadeOut( void )
{
    const esp_err_t res = m_theLedcFader.fadeOut();

    if ( res != ESP_OK )
    {
        vlogError( "fadeOut: Failed with %s", esp_err_to_name( res ) );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void UcdPwmOutTask::setPwmDuty( const uint8_t u8PwmDutyPercent )
{
    const uint32_t u32Duty = percent2Duty( u8PwmDutyPercent );

    //vlogInfo( "setPwmDuty: %d", ( int ) u8PwmDutyPercent );

    const esp_err_t res = m_theLedcFader.setDuty( u32Duty );

    if ( res != ESP_OK )
    {
        vlogError( "setPwmDuty: Failed with %s", esp_err_to_name( res ) );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void UcdPwmOutTask::setFaderSettings( const uint8_t  u8FadeInDutyPercent,
                                      const uint32_t u32FadeInTimeMs,
                                      const uint8_t  u8FadeOutDutyPercent,
                                      const uint32_t u32FadeOutTimeMs )
{
    m_theLedcFader.setFadeSettings( percent2Duty( u8FadeInDutyPercent ),
                                    u32FadeInTimeMs,
                                    percent2Duty( u8FadeOutDutyPercent ),
                                    u32FadeOutTimeMs );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

bool UcdPwmOutTask::queueAckNackReceived( const uint32_t u32MsgCounter,
                                          const bool     bSuccess )
{
    return pwmOutQueue()->sendAckNackReceived( u32MsgCounter, bSuccess ) == pdPASS;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void UcdPwmOutTask::processCtrlUartCommand( const EkxProtocol::DriverId                        u8DriverId,
                                            const uint32_t                                     u32MsgCounter,
                                            const uint8_t                                      u8RepeatCounter,
                                            const EkxProtocol::Payload::MessagePayload * const pMessagePayload )
{
    switch ( pMessagePayload->payloadType() )
    {
        case EkxProtocol::Payload::PayloadType::PwmFadeIn:
        {
            fadeIn();
            sendCtrlUartReplyAck( u32MsgCounter, u8RepeatCounter );
        }
        break;

        case EkxProtocol::Payload::PayloadType::PwmFadeOut:
        {
            fadeOut();
            sendCtrlUartReplyAck( u32MsgCounter, u8RepeatCounter );
        }
        break;

        case EkxProtocol::Payload::PayloadType::PwmSetDuty:
        {
            const EkxProtocol::Payload::PwmSetDuty * const pPayload = dynamic_cast<const EkxProtocol::Payload::PwmSetDuty *>( pMessagePayload );

            if ( pPayload )
            {
                setPwmDuty( pPayload->pwmDuty().value() );
                sendCtrlUartReplyAck( u32MsgCounter, u8RepeatCounter );
            }
            else
            {
                vlogError( "processCtrlUartCommand: cant't set duty, object not set duty object" );
                sendCtrlUartReplyNack( u32MsgCounter, u8RepeatCounter, EkxProtocol::Payload::Nack::NackReason::PayloadBroken );
            }
        }
        break;

        case EkxProtocol::Payload::PayloadType::PwmFaderSettings:
        {
            const EkxProtocol::Payload::PwmFaderSettings * pPayload = dynamic_cast<const EkxProtocol::Payload::PwmFaderSettings *>( pMessagePayload );

            if ( pPayload )
            {
                setFaderSettings( pPayload->fadeInDutyPercent().value(),
                                  pPayload->fadeInTimeMs().value(),
                                  pPayload->fadeOutDutyPercent().value(),
                                  pPayload->fadeOutTimeMs().value() );
                sendCtrlUartReplyAck( u32MsgCounter, u8RepeatCounter );
            }
            else
            {
                vlogError( "processCtrlUartCommand: cant't set fader settings, object not fader settings" );
                sendCtrlUartReplyNack( u32MsgCounter, u8RepeatCounter, EkxProtocol::Payload::Nack::NackReason::PayloadBroken );
            }
        }
        break;

        default:
        {
            vlogError( "processCtrlUartCommand: cant't process data; unknown command %d", static_cast<int>( pMessagePayload->payloadType() ) );
            sendCtrlUartReplyNack( u32MsgCounter, u8RepeatCounter, EkxProtocol::Payload::Nack::NackReason::UnknownDriverCommand );
        }
        break;
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

bool UcdPwmOutTask::queueCtrlUartCommand( const EkxProtocol::DriverId                  u8DriverId,
                                          const uint32_t                               u32MsgCounter,
                                          const uint8_t                                u8RepeatCounter,
                                          EkxProtocol::Payload::MessagePayload * const pMessagePayload )
{
    return pwmOutQueue()->sendCtrlUartCommand( u8DriverId, u32MsgCounter, u8RepeatCounter, pMessagePayload ) == pdPASS;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void UcdPwmOutTask::channelDefaultFaderSettings( uint8_t *  pu8FadeInDutyPercent,
                                                 uint8_t *  pu8FadeOutDutyPercent,
                                                 uint32_t * pu32FadeInTimeMs,
                                                 uint32_t * pu32FadeOutTimeMs )
{
    *pu8FadeInDutyPercent  = 100U;    // 50%
    *pu8FadeOutDutyPercent = 0U;      // 0%
    *pu32FadeInTimeMs      = 800UL;
    *pu32FadeOutTimeMs     = 800UL;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/
