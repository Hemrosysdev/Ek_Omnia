/*
 * EspAdc.h
 *
 *  Created on: 06.11.2019
 *      Author: gesser
 */

#ifndef UXESPCPPLIBRARY_ESPADC_H
#define UXESPCPPLIBRARY_ESPADC_H

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#include "EspLog.h"

#include <esp_idf_version.h>
#if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL( 5, 0, 0 )
#include <driver/adc.h>
#include <esp_adc_cal.h>
#else
#include <esp_adc/adc_oneshot.h>
#include <esp_adc/adc_cali_scheme.h>
#endif

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

namespace UxEspCppLibrary
{

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class EspAdc : public EspLog
{
public:

    EspAdc();
    EspAdc( const std::string & strLogName );

    virtual ~EspAdc();

    esp_err_t init( const adc_unit_t       nUnit,
                    const adc_channel_t    nChannel,
                    const adc_atten_t      nAtten,
#if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL( 5, 0, 0 )
                    const adc_bits_width_t   nBitsWidth,
#else
                    const adc_bitwidth_t   nBitsWidth,
#endif
                    const uint32_t         u32Vref = 0 );      // if voltage reading is used

    uint32_t readRaw( void );

    uint32_t readVoltage( void );

private:

    adc_unit_t                    m_nUnit;

    adc_channel_t                 m_nChannel;

    adc_atten_t                   m_nAtten;

#if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL( 5, 0, 0 )
    adc_bits_width_t              m_nBitsWidth;
    esp_adc_cal_characteristics_t m_adcCalCharacteristics;
#else
    adc_bitwidth_t                m_nBitsWidth;
    adc_oneshot_unit_handle_t     m_nAdcHandle;
    adc_cali_handle_t             m_nAdcCaliHandle;
#endif

    uint32_t                	  m_u32Vref;

};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

} // namespace UxEspCppLibrary

#endif /* UXESPCPPLIBRARY_ESPADC_H */
