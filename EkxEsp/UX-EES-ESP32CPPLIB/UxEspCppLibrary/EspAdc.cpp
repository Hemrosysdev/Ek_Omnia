/*
 * EspAdc.cpp
 *
 *  Created on: 06.11.2019
 *      Author: gesser
 */

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#include "EspAdc.h"

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

namespace UxEspCppLibrary
{

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

EspAdc::EspAdc()
                                                    : EspLog( "EspAdc" )
                                                      , m_nUnit( ADC_UNIT_1 )
                                                      , m_nChannel( ADC_CHANNEL_0 )
                                                      , m_nAtten( ADC_ATTEN_DB_0 )
#if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL( 5, 0, 0 )
                                                      , m_nBitsWidth( ADC_WIDTH_MAX )
#else
                                                      , m_nBitsWidth( ADC_BITWIDTH_DEFAULT )
#endif
                                                      , m_u32Vref( 0 )
                                                      {
                                                      }

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

EspAdc::EspAdc( const std::string & strLogName )
: EspLog( "EspAdc" )
, m_nUnit( ADC_UNIT_1 )
, m_nChannel( ADC_CHANNEL_0 )
, m_nAtten( ADC_ATTEN_DB_0 )
#if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL( 5, 0, 0 )
, m_nBitsWidth( ADC_WIDTH_MAX )
#else
, m_nBitsWidth( ADC_BITWIDTH_DEFAULT )
#endif
, m_u32Vref( 0 )
{
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

EspAdc::~EspAdc()
{
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

esp_err_t EspAdc::init( const adc_unit_t       nUnit,
                        const adc_channel_t    nChannel,
                        const adc_atten_t      nAtten,
#if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL( 5, 0, 0 )
                        const adc_bits_width_t   nBitsWidth,
#else
                        const adc_bitwidth_t   nBitsWidth,
#endif
                        const uint32_t         u32Vref /* = 0 */ )
{
    esp_err_t nEspError = ESP_OK;

    m_nUnit      = nUnit;
    m_nChannel   = nChannel;
    m_nAtten     = nAtten;
    m_nBitsWidth = nBitsWidth;
    m_u32Vref    = u32Vref;

    /*
     * Init Adc
     */

#if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL( 5, 0, 0 )
    if ( m_nUnit == ADC_UNIT_1 )
    {
        nEspError = adc1_config_width( m_nBitsWidth );
        adc1_config_channel_atten( static_cast<adc1_channel_t>( m_nChannel ), m_nAtten );
    }
    else
    {
        adc2_config_channel_atten( static_cast<adc2_channel_t>( m_nChannel ), m_nAtten );
    }

    esp_adc_cal_value_t val_type = esp_adc_cal_characterize( m_nUnit,
                                                             m_nAtten,
                                                             m_nBitsWidth,
                                                             m_u32Vref,
                                                             &m_adcCalCharacteristics );
    (void) val_type;
#else
    adc_oneshot_unit_init_cfg_t init_config = {
                    .unit_id = m_nUnit,
                    .ulp_mode = ADC_ULP_MODE_DISABLE,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config, &m_nAdcHandle));

    adc_oneshot_chan_cfg_t config = {
                    .atten = m_nAtten,
                    .bitwidth = m_nBitsWidth,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(m_nAdcHandle, m_nChannel, &config));


    /*
     * Init Adc Calibration
     */
    adc_cali_line_fitting_config_t cali_config = {
                    .unit_id = m_nUnit,
                    .atten = m_nAtten,
                    .bitwidth = m_nBitsWidth,
                    .default_vref = 0,
    };
    ESP_ERROR_CHECK(adc_cali_create_scheme_line_fitting(&cali_config, &m_nAdcCaliHandle));
#endif

    return nEspError;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

uint32_t EspAdc::readRaw( void )
{
    int nReading = 0;

#if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL( 5, 0, 0 )
    if ( m_nUnit == ADC_UNIT_1 )
    {
        nReading += adc1_get_raw( static_cast<adc1_channel_t>( m_nChannel ) );
    }
    else
    {
        adc2_get_raw( static_cast<adc2_channel_t>( m_nChannel ),
                      ADC_WIDTH_BIT_12,
                      &nReading );
    }
#else
    ESP_ERROR_CHECK( adc_oneshot_read( m_nAdcHandle, m_nChannel, &nReading ) );
#endif

    return static_cast<uint32_t>( nReading );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

uint32_t EspAdc::readVoltage( void )
{
    uint32_t u32Raw = readRaw();

#if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL( 5, 0, 0 )
    u32Raw = esp_adc_cal_raw_to_voltage( u32Raw, &m_adcCalCharacteristics );
    return u32Raw;
#else
    int nVoltage;

    u32Raw = adc_cali_raw_to_voltage(m_nAdcCaliHandle, u32Raw, &nVoltage );

    return u32Raw;
#endif

}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

} // namespace UxEspCppLibrary
