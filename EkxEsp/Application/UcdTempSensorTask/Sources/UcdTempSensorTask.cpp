/*!*************************************************************************************************************************************************************
 *
 * @file UcdTempSensorTask.cpp
 * @brief Source file of component unit Temperature-Sensor Task.
 *
 * This file was developed as part of Temperature-Sensor Task.
 *
 * @author Ultratronik GmbH
 *         Dornierstr. 9
 *         D-82205 Gilching
 *         http://www.ultratronik.de
 *
 * @author Srdjan Nikolic, Entwicklung
 *
 * @copyright Copyright 2020 by Ultratronik GmbH.
 *            All rights reserved.
 *            None of this file or parts of it may be
 *            copied, redistributed or used in any other way
 *            without written approval of Ultratronik GmbH.
 *
 **************************************************************************************************************************************************************/

/*!*************************************************************************************************************************************************************
 *
 **************************************************************************************************************************************************************/

#include "UcdTempSensorTask.h"

#include "UcdTempSensorQueue.h"
#include "UcdTempSensorQueueTypes.h"
#include "ApplicationGlobals.h"
#include "EkxProtocol.h"

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

UcdTempSensorTask::UcdTempSensorTask( UxEspCppLibrary::EspApplication * pApplication,
                                      const adc_unit_t                  nNtcAdcUnit,
                                      const adc_channel_t               nNtcAdcChannel,
                                      const adc_atten_t                 nNtcAdcAtten,
#if ESP_IDF_VERSION <                                                   ESP_IDF_VERSION_VAL( 5, \
                                                                                             0, \
                                                                                             0 )
                                      const adc_bits_width_t nNtcAdcBitsWidth,
#else
                                      const adc_bitwidth_t   nNtcAdcBitsWidth,
#endif
                                      const uint32_t         u32Vref )
    : UartControlledDevice( "NtcSensor",
                            "TempSensor",
                            EkxProtocol::DriverId::TempSensorDriver,
                            //static_cast<uint32_t>( UcdTempSensorQueueTypes::messageType::TimeoutAcknowledgeTimeout ),
                            pApplication,
                            ApplicationGlobals::c_nTempSensorTaskStackSize,
                            ApplicationGlobals::c_nTempSensorTaskPriority,
                            "UcdTempSensorTask",
                            new UcdTempSensorQueue() )
    , m_adcNtc( "NtcSensor" )
    , m_timerNtcRead( taskQueue(),
                      static_cast<uint32_t>( UcdTempSensorQueueTypes::messageType::TimeoutReadTempSensor ) )
{
    ESP_ERROR_CHECK( m_adcNtc.init( nNtcAdcUnit,
                                    nNtcAdcChannel,
                                    nNtcAdcAtten,
                                    nNtcAdcBitsWidth,
                                    u32Vref ) );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

UcdTempSensorTask::~UcdTempSensorTask()
{
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

bool UcdTempSensorTask::startupStep( const FreeRtosQueueTask::startupStep_t nStartupStep )
{
    bool bContinueStartup = true;

    switch ( nStartupStep )
    {
        case UxEspCppLibrary::FreeRtosQueueTask::startupStep_t::startupStep1:
        {
            registerDriver();

            // start 1min poll timer
            m_timerNtcRead.startPeriodic( 60000000 );
            processReadTemperature();
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

void UcdTempSensorTask::processTaskMessage( void )
{
    const uint8_t * pMsg = receiveMsg();

    const UcdTempSensorQueueTypes::message * pMessage = reinterpret_cast<const UcdTempSensorQueueTypes::message *>( pMsg );

    if ( pMessage )
    {
        switch ( pMessage->u32Type )
        {
            case UcdTempSensorQueueTypes::messageType::CtrlUartCommand:
            {
                processBaseCtrlUartCommand( pMessage->payload.ctrlUartCommand.m_u8DriverId,
                                            pMessage->payload.ctrlUartCommand.m_u32MsgCounter,
                                            pMessage->payload.ctrlUartCommand.m_u8RepeatCounter,
                                            pMessage->payload.ctrlUartCommand.m_pMessagePayload );
            }
            break;

            case UcdTempSensorQueueTypes::messageType::AckNackReceived:
            {
                processAckNackReceived( pMessage->payload.ackNackReceived.m_u32MsgCounter,
                                        pMessage->payload.ackNackReceived.m_bSuccess );
            }
            break;

            case UcdTempSensorQueueTypes::messageType::TimeoutReadTempSensor:
            {
                processReadTemperature();
            }
            break;

            default:
            {
                FreeRtosQueueTask::vlogError( "processTaskMessage: receive illegal message type %d",
                                              pMessage->u32Type );
            }
            break;
        }
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

UcdTempSensorQueue * UcdTempSensorTask::tempSensorQueue( void )
{
    return dynamic_cast<UcdTempSensorQueue *>( taskQueue() );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

const UcdTempSensorQueue * UcdTempSensorTask::tempSensorQueue( void ) const
{
    return dynamic_cast<const UcdTempSensorQueue *>( taskQueue() );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void UcdTempSensorTask::processReadTemperature( void )
{
    uint32_t u32AdcVoltage = m_adcNtc.readVoltage();

    if ( m_u32AdcVoltage != u32AdcVoltage )
    {
        m_u32AdcVoltage = u32AdcVoltage;
        reportDriverStatus( true );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void UcdTempSensorTask::reportDriverStatus( const bool bFirst )
{
    vlogInfo( "reportDriverStatus, first %d", bFirst );

    EkxProtocol::Payload::AdcStatus * pPayload = new EkxProtocol::Payload::AdcStatus();
    pPayload->valid().setValue( true );
    pPayload->adcVoltage().setValue( m_u32AdcVoltage );

    sendCtrlUartReply( pPayload, bFirst );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

bool UcdTempSensorTask::queueCtrlUartCommand( const EkxProtocol::DriverId                  u8DriverId,
                                              const uint32_t                               u32MsgCounter,
                                              const uint8_t                                u8RepeatCounter,
                                              EkxProtocol::Payload::MessagePayload * const pMessagePayload )
{
    return tempSensorQueue()->sendCtrlUartCommand( u8DriverId, u32MsgCounter, u8RepeatCounter, pMessagePayload );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

bool UcdTempSensorTask::queueAckNackReceived( const uint32_t u32MsgCounter,
                                              const bool     bSuccess )
{
    return tempSensorQueue()->sendAckNackReceived( u32MsgCounter, bSuccess ) == pdPASS;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/
