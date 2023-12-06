/*
 * I2cAt24c16.cpp
 *
 *  Created on: 27.04.2020
 *      Author: gesser
 */

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#include "I2cAt24c16.h"

#include "PluginTaskAt24c16Types.h"
#include "PluginTaskAt24c16Distributor.h"
#include "PluginTaskAt24c16QueueTypes.h"

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

//
// AT26C16 specific constants see datasheet and/or HW concept
//

/// start address of product string data within EEPROM chip
#define EEPROM_PROD_NO_ADDR                    ( 0 * 16 )

/// max. length of product string data within EEPROM chip
#define EEPROM_PROD_NO_LEN                     ( MAX_PROD_NO_LEN + 1 )

/// start address of serial number string within EEPROM chip
#define EEPROM_SERIAL_NO_ADDR                  ( 2 * 16 )

/// max. length of serial number string within EEPROM chip
#define EEPROM_SERIAL_NO_LEN                   ( MAX_SERIAL_NO_LEN + 1 )

/// I2C device address of EEPROM chip
#define EEPROM_DEV_ADDR                        ( ( uint16_t ) 0x50U )

/// timeout to read any data from EEPROM chip
#define EEPROM_READ_TIMEOUT                    ( ( uint32_t ) 100UL )

/// Serial-Number string that will be used in case of EEPROM read error
#define EEPROM_SZ_DEFAULT_SERIAL_NO            "000000"

/// product string that will be used in case of EEPROM read error
#define EEPROM_SZ_DEFAULT_PROD_NO              "Ultratronik"

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

namespace UxEspCppLibrary
{

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

I2cAt24c16::I2cAt24c16( const i2c_port_t               nPort,
                        PluginTaskAt24c16Distributor * pDistributor )
    : EspI2c( "I2cAt24c16",
              nPort,
              EEPROM_DEV_ADDR )
    , m_pDistributor( pDistributor )
{
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

I2cAt24c16::~I2cAt24c16()
{
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void I2cAt24c16::readProductData( void )
{
    uint8_t   u8ProductNo[EEPROM_PROD_NO_LEN];
    uint8_t   u8SerialNo[EEPROM_SERIAL_NO_LEN];
    esp_err_t nEspError = ESP_OK;

    // set read pointer for reading production no data
    if ( nEspError == ESP_OK )
    {
        nEspError = readTransaction( EEPROM_PROD_NO_ADDR, u8ProductNo, EEPROM_PROD_NO_LEN );
    }

    // set read pointer for reading serial number data
    if ( nEspError == ESP_OK )
    {
        nEspError = readTransaction( EEPROM_SERIAL_NO_ADDR, u8SerialNo, EEPROM_SERIAL_NO_LEN );
    }

    if ( nEspError == ESP_OK )
    {
        bool bValid = true;

        // just to be sure ...
        u8ProductNo[EEPROM_PROD_NO_LEN - 1]  = '\0';
        u8SerialNo[EEPROM_SERIAL_NO_LEN - 1] = '\0';

        // validity check of data
        int i;
        for ( i = 0; u8SerialNo[i] != '\0'; i++ )
        {
            if ( !isxdigit( u8SerialNo[i] ) )
            {
                u8SerialNo[i] = '?';
                bValid        = false;
            }
        }
        if ( i != MAX_SERIAL_NO_LEN )
        {
            bValid = false;
        }

        for ( i = 0; u8ProductNo[i] != '\0'; i++ )
        {
            if ( !isascii( u8ProductNo[i] ) )
            {
                u8ProductNo[i] = '?';
                bValid         = false;
            }
        }

        if ( m_pDistributor->sendEepromData( bValid,
                                             reinterpret_cast<char *>( u8ProductNo ),
                                             reinterpret_cast<char *>( u8SerialNo ) ) != pdPASS )
        {
            nEspError = ESP_FAIL;
        }
    }
    else
    {
        if ( m_pDistributor->sendEepromData( false,
                                             EEPROM_SZ_DEFAULT_PROD_NO,
                                             EEPROM_SZ_DEFAULT_SERIAL_NO ) != pdPASS )
        {
            nEspError = ESP_FAIL;
        }
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void I2cAt24c16::writeProductNo( const std::string & strProductNo )
{
    esp_err_t nEspError = ESP_FAIL;

    if ( strProductNo.size() < EEPROM_PROD_NO_LEN )
    {
        nEspError = ESP_OK;

        for ( int i = 0; i < strProductNo.size() + 1
              && nEspError == ESP_OK; i++ )
        {
            vlogInfo( "writeProductNo byte %c", strProductNo[i] );

            nEspError = writeTransaction( EEPROM_PROD_NO_ADDR + i, strProductNo[i] );

            vTaskDelay( 100 / portTICK_PERIOD_MS );
        }

        if ( nEspError != ESP_OK )
        {
            vlogError( "writeProductNo: %s - error %s", strProductNo.c_str(), esp_err_to_name( nEspError ) );
        }
        else
        {
            vlogInfo( "writeProductNo: %s - ok!", strProductNo.c_str() );
        }
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void I2cAt24c16::writeSerialNo( const std::string & strSerialNo )
{
    esp_err_t nEspError = ESP_FAIL;

    if ( strSerialNo.size() < EEPROM_SERIAL_NO_LEN )
    {
        nEspError = ESP_OK;

        for ( int i = 0; i < strSerialNo.size() + 1
              && nEspError == ESP_OK; i++ )
        {
            vlogInfo( "writeSerialNo byte %c", strSerialNo[i] );

            nEspError = writeTransaction( EEPROM_SERIAL_NO_ADDR + i, strSerialNo[i] );

            vTaskDelay( 100 / portTICK_PERIOD_MS );
        }

        if ( nEspError != ESP_OK )
        {
            vlogError( "writeSerialNo: %s - error %s", strSerialNo.c_str(), esp_err_to_name( nEspError ) );
        }
        else
        {
            vlogInfo( "writeSerialNo: %s - ok!", strSerialNo.c_str() );
        }
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

} // namespace UxEspCppLibrary
