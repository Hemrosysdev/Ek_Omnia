/*!*****************************************************************************
*
* @file EkxApplication.c
*
* @brief Main entry point of HEM-EKX-ESP-S01 application.
*
* @author Ultratronik GmbH
*         Dornierstr. 9
*         D-82205 Gilching
*         www.ultratronik.de
*
* @author Gerd Esser, Forschung & Entwicklung
*
* @created 04.03.2020
*
* @copyright Copyright 2020 by Ultratronik GmbH.
*            All rights reserved.
*            None of this file or parts of it may be
*            copied, redistributed or used in any other way
*            without written approval of Ultratronik GmbH.
*
*******************************************************************************/

/*##*************************************************************************************************************************************************************
 *      Includes
 **************************************************************************************************************************************************************/

#include "EkxApplication.h"

#include <stdio.h>
#include <string.h>
#if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL( 5, 0, 0 )
#include <esp_eth.h>
#include <driver/adc.h>
#else
#include <esp_eth_com.h>
#include <driver/rmt_tx.h>
#include <esp_adc/adc_oneshot.h>
#endif
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/i2c.h>
#include <lwip/inet.h>
#include <lwip/ip4.h>

#include "ApplicationGlobals.h"
#include "../../build/version.h"
#include "EspLog.h"
#include "EspI2c.h"
#include "EspIsrGpio.h"
#include "EspLedcFader.h"

/*!************************************************************************************************************************************************************
 *      Local function declarations
 *************************************************************************************************************************************************************/

#define I2C_MASTER_FREQ_HZ              100000
#define I2C_MASTER_PORT                 ( ( i2c_port_t ) 1 )
#define I2C_MASTER_TX_BUF_DISABLE       0                               /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE       0                               /*!< I2C master doesn't need buffer */

EkxApplication * EkxApplication::m_pSingleton = nullptr;

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

extern "C" void app_main()
{
    EkxApplication::createSingleton();
    ekxApp().appMain();
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

EkxApplication::EkxApplication()
    : EspApplication( "EkxApplication" )
    , m_wifiConnectorTask( this )
    , m_pluginTaskAt24c16( this,
                           ApplicationGlobals::c_nPluginTaskAt24c16Priority,
                           I2C_MASTER_PORT )
    , m_pluginTaskWifiConnector( this,
                                 ApplicationGlobals::c_nPluginTaskWifiConnectorPriority )
    , m_ucdStartStopBtnTask( this )
    , m_scdStartStopLightTask( this,
                               LEDC_TIMER_0,
                               LEDC_CHANNEL_0 )
    , m_mcuUartTask( this,
                     ApplicationGlobals::c_nMcuUartNum,
                     ApplicationGlobals::c_nGpioMotCtrlTx,
                     ApplicationGlobals::c_nGpioMotCtrlRx,
                     EkxProtocol::DriverId::McuDriver,
                     "MotorControl",
                     "UcdMotorControlTask",
                     ApplicationGlobals::c_nMcuUartTaskStackSize,
                     ApplicationGlobals::c_nMcuUartTaskPriority )
    , m_ucdSpotLightTask( this,
                          LEDC_TIMER_1,
                          LEDC_CHANNEL_1 )
    , m_ucdEepromTask( this,
                       &m_pluginTaskAt24c16,
                       EkxProtocol::DriverId::EepromDriver,
                       ApplicationGlobals::c_nEepromTaskStackSize,
                       ApplicationGlobals::c_nEepromTaskPriority )
    , m_ucdTempSensorTask( this,
                           ApplicationGlobals::c_nADC_NTC_AIN_UNIT,
                           ApplicationGlobals::c_nADC_NTC_AIN_CHANNEL,
                           ADC_ATTEN_DB_11,
#if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL( 5, 0, 0 )
                           ADC_WIDTH_BIT_12,
#else
                           ADC_BITWIDTH_12,
#endif
                           1100 )
    , m_swUpdateTask( this )
    , m_agsaStepperMotorTask( this,
                              ApplicationGlobals::c_nGpioAgsaStepperDir,
                              ApplicationGlobals::c_nGpioAgsaStepperStep,
                              ApplicationGlobals::c_nGpioAgsaStepperEn,
                              ApplicationGlobals::c_nGpioAgsaFaultN,
#ifdef STEPPER_USE_LEDC_PWM
                              LEDC_TIMER_3,
                              LEDC_CHANNEL_3,
#endif
                              EkxProtocol::DriverId::AgsaStepperMotorDriver,
                              "AgsaStepperMotor",
                              "UcdAgsaStepperMotorTask",
                              ApplicationGlobals::c_nAgsaStepperMotorTaskStackSize,
                              ApplicationGlobals::c_nAgsaStepperMotorTaskPriority )
    , m_pduDcHallMotorTask( this,
                            ApplicationGlobals::c_nGpioPduPower,
                            ApplicationGlobals::c_nGpioPduDcHallMotorPwm,
                            ApplicationGlobals::c_nGpioPduDcHallEncoderA,
                            ApplicationGlobals::c_nGpioPduFaultN_Mtms,
                            LEDC_TIMER_2,
                            LEDC_CHANNEL_2,
                            EkxProtocol::DriverId::PduDcHallMotorDriver,
                            "PduDcHallMotor",
                            "UcdPduDcHallMotorTask",
                            ApplicationGlobals::c_nPduDcHallMotorTaskStackSize,
                            ApplicationGlobals::c_nPduDcHallMotorTaskPriority )
    , m_ctrlUartTask( this )

    , m_ucdHttpServerTask( this )
{
    registerTask( &m_wifiConnectorTask );
    registerTask( &m_pluginTaskAt24c16 );
    registerTask( &m_pluginTaskWifiConnector );
    registerTask( &m_ucdStartStopBtnTask );
    registerTask( &m_mcuUartTask );
    registerTask( &m_ucdSpotLightTask );
    registerTask( &m_scdStartStopLightTask );
    registerTask( &m_ucdEepromTask );
    registerTask( &m_ucdTempSensorTask );
    registerTask( &m_swUpdateTask );
    registerTask( &m_agsaStepperMotorTask );
    registerTask( &m_pduDcHallMotorTask );
    registerTask( &m_ctrlUartTask );
    registerTask( &m_ucdHttpServerTask );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

EkxApplication::~EkxApplication()
{
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void EkxApplication::createApp()
{
    printf( "\n"
            "=====================================================\n"
            "EkxApplication Version V%s from %s\n"
            "Build %s (%s) from %s on branch %s\n"
            "Copyright 2020-2022 by Ultratronik GmbH\n"
            "=====================================================\n\n",
            VERSION_NO,
            VERSION_DATE,
            BUILD_NO,
            DEV_STATE,
            BUILD_DATE,
            BRANCH );

    UxEspCppLibrary::EspIsrGpio::installService( 0 );

#if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL( 5, 0, 0 )
    ESP_ERROR_CHECK( UxEspCppLibrary::EspLedcFader::install( 0 ) );
#endif

    ESP_ERROR_CHECK( UxEspCppLibrary::EspI2c::initMaster( I2C_MASTER_PORT,
                                                          ApplicationGlobals::c_nGpioI2cSda,
                                                          GPIO_PULLUP_ENABLE,
                                                          ApplicationGlobals::c_nGpioI2cScl,
                                                          GPIO_PULLUP_ENABLE,
                                                          I2C_MASTER_FREQ_HZ ) );

}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

UcdWifiConnectorTask & EkxApplication::wifiConnectorTask()
{
    return m_wifiConnectorTask;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

UxEspCppLibrary::PluginTaskAt24c16 & EkxApplication::pluginTaskAt24c16()
{
    return m_pluginTaskAt24c16;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

UxEspCppLibrary::PluginTaskWifiConnector & EkxApplication::pluginTaskWifiConnector()
{
    return m_pluginTaskWifiConnector;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

UcdEepromTask & EkxApplication::eepromTask()
{
    return m_ucdEepromTask;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

UcdTempSensorTask & EkxApplication::tempSensorTask()
{
    return m_ucdTempSensorTask;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

//UcdStepperMotorTask & EkxApplication::pduStepperMotorTask()
//{
//    return m_agsaStepperMotorTask;
//}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

UcdDcHallMotorTask & EkxApplication::pduDcHallMotorTask()
{
    return m_pduDcHallMotorTask;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

UcdStartStopBtnTask & EkxApplication::startStopBtnTask()
{
    return m_ucdStartStopBtnTask;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

UcdStartStopLightTask & EkxApplication::startStopLightTask()
{
    return m_scdStartStopLightTask;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

UcdMcuUartTask & EkxApplication::mcuUartTask()
{
    return m_mcuUartTask;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

UcdSpotLightTask & EkxApplication::spotLightTask()
{
    return m_ucdSpotLightTask;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

EkxApplication & ekxApp()
{
    if ( !EkxApplication::singleton() )
    {
        printf( "EkxApplication::ekxApp() singleton not instantiated" );
        ESP_ERROR_CHECK( false );
    }

    return *EkxApplication::singleton();
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

EkxApplication * EkxApplication::createSingleton()
{
    if ( !EkxApplication::m_pSingleton )
    {
        EkxApplication::m_pSingleton = new EkxApplication();
    }

    return EkxApplication::m_pSingleton;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

EkxApplication * EkxApplication::singleton()
{
    return EkxApplication::m_pSingleton;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void EkxApplication::deleteSingleton()
{
    delete EkxApplication::m_pSingleton;
    EkxApplication::m_pSingleton = nullptr;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

CtrlUart::CtrlUartTask & EkxApplication::ctrlUartTask()
{
    return m_ctrlUartTask;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

UcdHttpServerTask & EkxApplication::httpServerTask()
{
    return m_ucdHttpServerTask;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/
