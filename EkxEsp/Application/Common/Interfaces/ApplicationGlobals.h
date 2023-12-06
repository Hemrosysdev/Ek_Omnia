/*!*****************************************************************************
*
* @file ApplicationGlobals.h
*
* @brief Global definitions for EkxApp
*
* @author Ultratronik GmbH
*         Dornierstr. 9
*         D-82205 Gilching
*         www.ultratronik.de
*
* @author Gerd Esser, Entwicklung
*
* @created 24.04.2020
*
* @copyright Copyright 2020 by Ultratronik GmbH.
*            All rights reserved.
*            None of this file or parts of it may be
*            copied, redistributed or used in any other way
*            without written approval of Ultratronik GmbH.
*
*******************************************************************************/

#ifndef ApplicationGlobals_h
#define ApplicationGlobals_h

#if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL( 5, 0, 0 )
#include <driver/adc.h>
#else
#include <esp_adc/adc_oneshot.h>
#endif

#include <driver/gpio.h>
#include <driver/uart.h>


/****************************************************************************************************************************************************************
 *
 **************************************************************************************************************************************************************/

namespace ApplicationGlobals
{

static const gpio_num_t c_nGpioBoot                = static_cast<gpio_num_t>( 0 );
static const gpio_num_t c_nGpioPduPower            = static_cast<gpio_num_t>( 2 );
static const gpio_num_t c_nGpioPwmOutStartStopLedN = static_cast<gpio_num_t>( 4 );
static const gpio_num_t c_nGpioI2cSda              = static_cast<gpio_num_t>( 5 );
static const gpio_num_t c_nGpioMtdi                = static_cast<gpio_num_t>( 12 );
static const gpio_num_t c_nGpioMtck                = static_cast<gpio_num_t>( 13 );
static const gpio_num_t c_nGpioPduFaultN_Mtms      = static_cast<gpio_num_t>( 14 );
static const gpio_num_t c_nGpioMtdo                = static_cast<gpio_num_t>( 15 );
static const gpio_num_t c_nGpioPwmOutSpotLightN    = static_cast<gpio_num_t>( 18 );
static const gpio_num_t c_nGpioPduDcHallEncoderA   = static_cast<gpio_num_t>( 19 );
static const gpio_num_t c_nGpioStartBtnN           = static_cast<gpio_num_t>( 21 );
static const gpio_num_t c_nGpioAgsaStepperStep     = static_cast<gpio_num_t>( 22 );
static const gpio_num_t c_nGpioMotCtrlTx           = static_cast<gpio_num_t>( 23 );
static const gpio_num_t c_nGpioPduDcHallMotorPwm   = static_cast<gpio_num_t>( 25 );
static const gpio_num_t c_nGpioPortaFilter2N       = static_cast<gpio_num_t>( 27 );
static const gpio_num_t c_nGpioI2cScl              = static_cast<gpio_num_t>( 33 );
static const gpio_num_t c_nGpioAgsaFaultN          = static_cast<gpio_num_t>( 34 );
static const gpio_num_t c_nGpioMotCtrlRx           = static_cast<gpio_num_t>( 35 );
static const gpio_num_t c_nGpioNtcAin              = static_cast<gpio_num_t>( 39 );

#define SOFTWARE_UART
#ifdef SOFTWARE_UART
static const gpio_num_t c_nGpioAgsaStepperEn  = static_cast<gpio_num_t>( 16 );
static const gpio_num_t c_nGpioAgsaStepperDir = static_cast<gpio_num_t>( 17 );
static const gpio_num_t c_nGpioPortaFilter1N  = static_cast<gpio_num_t>( 26 );
static const gpio_num_t c_nGpioCtrlUartTx     = static_cast<gpio_num_t>( 32 );
static const gpio_num_t c_nGpioCtrlUartRx     = static_cast<gpio_num_t>( 36 );
#else
static const gpio_num_t c_nGpioAgsaStepperEn  = static_cast<gpio_num_t>( 26 );
static const gpio_num_t c_nGpioAgsaStepperDir = static_cast<gpio_num_t>( 32 );
static const gpio_num_t c_nGpioPortaFilter1N  = static_cast<gpio_num_t>( 36 );
static const gpio_num_t c_nGpioCtrlUartTx     = static_cast<gpio_num_t>( 17 );
static const gpio_num_t c_nGpioCtrlUartRx     = static_cast<gpio_num_t>( 16 );
#endif

static const int c_nCtrlUartTaskStackSize         = 8092;
static const int c_nCtrlUartDrvTaskStackSize      = 8092;
static const int c_nStartStopBtnTaskStackSize     = 4096;
static const int c_nStartStopLightTaskStackSize   = 4096;
static const int c_nSpotLightTaskStackSize        = 4096;
static const int c_nEepromTaskStackSize           = 4096;
static const int c_nSwUpdateTaskStackSize         = 4096;
static const int c_nMcuUartTaskStackSize          = 4096;
static const int c_nWifiConnectorTaskStackSize    = 8092;
static const int c_nTempSensorTaskStackSize       = 4096;
static const int c_nAgsaStepperMotorTaskStackSize = 4096;
static const int c_nPduDcHallMotorTaskStackSize   = 4096;
static const int c_nHttpServerTaskStackSize       = 4096;

// task priorities
static const int c_nPluginTaskAt24c16Priority       = 5;
static const int c_nPluginTaskWifiConnectorPriority = 5;
static const int c_nStartStopBtnTaskPriority        = 4;
static const int c_nStartStopLightTaskPriority      = 4;
static const int c_nSpotLightTaskPriority           = 4;
static const int c_nEepromTaskPriority              = 4;
static const int c_nSwUpdateTaskPriority            = 4;
static const int c_nMcuUartTaskPriority             = 7;
static const int c_nWifiConnectorTaskPriority       = 5;
static const int c_nTempSensorTaskPriority          = 3;
static const int c_nAgsaStepperMotorTaskPriority    = 7;
static const int c_nPduDcHallMotorTaskPriority      = 7;
static const int c_nCtrlUartTaskPriority            = 8;
static const int c_nCtrlUartDrvTaskPriority         = 8;
static const int c_nHttpServerTaskPriority          = 5;

static const uart_port_t c_nCtrlUartNum = UART_NUM_2;
static const uart_port_t c_nMcuUartNum  = UART_NUM_1;

static const adc_unit_t    c_nADC_NTC_AIN_UNIT    = ADC_UNIT_1;
static const adc_channel_t c_nADC_NTC_AIN_CHANNEL = ADC_CHANNEL_3;

static const int c_httpServerDefaultPort = 80;

} // namespace ApplicationGlobals

/*##*************************************************************************************************************************************************************
 *
 **************************************************************************************************************************************************************/

#endif /* ApplicationGlobals_h */
