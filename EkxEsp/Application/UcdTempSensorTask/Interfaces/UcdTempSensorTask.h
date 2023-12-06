/*!*************************************************************************************************************************************************************
 *
 * @file UcdTempSensorTask.h
 * @brief Source file of component unit Temperature-Sensor Task.
 *
 * This file was developed as part of Temperature-Sensor Task.
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

#ifndef UcdTempSensorTask_h
#define UcdTempSensorTask_h

/***************************************************************************************************************************************************************
 *      Includes
 **************************************************************************************************************************************************************/

#include <esp_idf_version.h>
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL( 5, 0, 0 )
#include <esp_adc/adc_oneshot.h>
#endif

#include <EspAdc.h>
#include <FreeRtosQueueTimer.h>
#include "UartControlledDevice.h"

class CtrlUartDispatcher;
class UcdTempSensorQueue;

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class UcdTempSensorTask : public UartControlledDevice
{

public:

    UcdTempSensorTask( UxEspCppLibrary::EspApplication * pApplication,
                       const adc_unit_t                  nNtcAdcUnit,
                       const adc_channel_t               nNtcAdcChannel,
                       const adc_atten_t                 nNtcAdcAtten,
#if ESP_IDF_VERSION <                                    ESP_IDF_VERSION_VAL( 5, \
                                                                              0, \
                                                                              0 )
                       const adc_bits_width_t nNtcAdcBitsWidth,
#else
                       const adc_bitwidth_t   nNtcAdcBitsWidth,
#endif
                       const uint32_t         u32Vref );

    virtual ~UcdTempSensorTask() override;

    bool startupStep( const FreeRtosQueueTask::startupStep_t nStartupStep ) override;

    void processTaskMessage( void ) override;

    UcdTempSensorQueue * tempSensorQueue( void );

    const UcdTempSensorQueue * tempSensorQueue( void ) const;

    bool queueCtrlUartCommand( const EkxProtocol::DriverId                  u8DriverId,
                               const uint32_t                               u32MsgCounter,
                               const uint8_t                                u8RepeatCounter,
                               EkxProtocol::Payload::MessagePayload * const pMessagePayload ) override;

    bool queueAckNackReceived( const uint32_t u32MsgCounter,
                               const bool     bSuccess ) override;

private:

    void processReadTemperature( void );

    void reportDriverStatus( const bool bFirst ) override;

private:

    uint32_t                            m_u32AdcVoltage { 0 };

    UxEspCppLibrary::EspAdc             m_adcNtc;

    UxEspCppLibrary::FreeRtosQueueTimer m_timerNtcRead;

};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#endif /* UcdTempSensorTask_h */
