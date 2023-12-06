/*
 * EspI2c.cpp
 *
 *  Created on: 19.02.2021
 *      Author: gesser
 */

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#include "EspI2c.h"

#include <cstring>

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#define WRITE_BIT                       I2C_MASTER_WRITE                /*!< I2C master write */
#define READ_BIT                        I2C_MASTER_READ                 /*!< I2C master read */
#define ACK_CHECK_EN                    0x1                             /*!< I2C master will check ack from slave*/
#define ACK_CHECK_DIS                   0x0                             /*!< I2C master will not check ack from slave */
#define ACK_VAL                         ( ( i2c_ack_type_t ) 0x0 )      /*!< I2C ack value */
#define NACK_VAL                        ( ( i2c_ack_type_t ) 0x1 )      /*!< I2C nack value */

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

namespace UxEspCppLibrary
{

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

EspI2c::EspI2c( const std::string & strLogTag,
                const i2c_port_t nPort /* = 0 */,
                const uint8_t u8SlaveAddress /* = 0 */ )
: EspLog( strLogTag )
, m_nPort( nPort )
, m_u8SlaveAddress( u8SlaveAddress )
{
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

EspI2c::~EspI2c()
{
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

i2c_port_t EspI2c::port() const
{
    return m_nPort;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

i2c_cmd_handle_t EspI2c::cmdHandle() const
{
    return m_hCmd;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void EspI2c::setSlaveAddress( const uint8_t u8SlaveAddress )
{
    m_u8SlaveAddress = u8SlaveAddress;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

uint8_t EspI2c::slaveAddress() const
{
    return m_u8SlaveAddress;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

// static
esp_err_t EspI2c::initMaster( const i2c_port_t nMasterPort,
                              const gpio_num_t nGpioSda,
                              const gpio_pullup_t nGpioSdaPullup,
                              const gpio_num_t nGpioScl,
                              const gpio_pullup_t nGpioSclPullup,
                              const uint32_t u32ClkSpeed )
{
    i2c_config_t conf;
    std::memset( &conf, 0, sizeof( conf ) );

    conf.mode             = I2C_MODE_MASTER;
    conf.sda_io_num       = nGpioSda;
    conf.sda_pullup_en    = nGpioSdaPullup;
    conf.scl_io_num       = nGpioScl;
    conf.scl_pullup_en    = nGpioSclPullup;
    conf.master.clk_speed = u32ClkSpeed;

    esp_err_t nEspError = i2c_param_config( nMasterPort, &conf );

    if ( nEspError == ESP_OK )
    {
        nEspError = i2c_driver_install( nMasterPort,
                                        conf.mode,
                                        0, //I2C_MASTER_RX_BUF_DISABLE,
                                        0, //I2C_MASTER_TX_BUF_DISABLE,
                                        0 );
    }

    return nEspError;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

esp_err_t EspI2c::openTransaction( const bool bWrite )
{
    esp_err_t nEspError = ESP_FAIL;

    if ( m_hCmd )
    {
        vlogError( "openTransaction failed, previous transaction not closed" );
    }
    else
    {
        m_hCmd = i2c_cmd_link_create();

        if ( m_hCmd )
        {
            nEspError = i2c_master_start( m_hCmd );

            if ( nEspError == ESP_OK )
            {
                if ( bWrite )
                    nEspError = i2c_master_write_byte( m_hCmd, ( m_u8SlaveAddress << 1 ) | WRITE_BIT, ACK_CHECK_EN );
                else
                    nEspError = i2c_master_write_byte( m_hCmd, ( m_u8SlaveAddress << 1 ) | READ_BIT, ACK_CHECK_EN );
            }

            clearTransaction( nEspError != ESP_OK );
        }

        if ( nEspError != ESP_OK )
        {
            vlogError( "openTransaction failed %s", esp_err_to_name( nEspError ) );
        }
    }

    return nEspError;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

esp_err_t EspI2c::closeTransaction( void )
{
    esp_err_t nEspError = ESP_FAIL;

    if ( m_hCmd )
    {
        nEspError = i2c_master_stop( m_hCmd );

        if ( nEspError == ESP_OK )
        {
            nEspError = i2c_master_cmd_begin( m_nPort, m_hCmd, 500 / portTICK_RATE_MS );
        }
    }

    if ( nEspError != ESP_OK )
    {
        vlogError( "closeTransaction failed %s", esp_err_to_name( nEspError ) );
    }

    return nEspError;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void EspI2c::clearTransaction( const bool bDoClear /* = true */ )
{
    if ( m_hCmd && bDoClear )
    {
        i2c_cmd_link_delete( m_hCmd );
        m_hCmd = nullptr;
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

esp_err_t EspI2c::setRegister( const uint8_t u8Register )
{
    esp_err_t nEspError = openTransaction( true );

    if ( nEspError == ESP_OK )
    {
        nEspError = i2c_master_write_byte( m_hCmd, u8Register, ACK_CHECK_EN );
    }

    clearTransaction( nEspError != ESP_OK );

    if ( nEspError != ESP_OK )
    {
        vlogError( "setRegister failed %s", esp_err_to_name( nEspError ) );
    }

    return nEspError;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

esp_err_t EspI2c::readTransaction( const uint8_t   u8Register,
                                   uint8_t *       pu8Data,
                                   const size_t    u32ByteNum )
{
    // set write pointer for writing data
    esp_err_t nEspError = setRegister( u8Register );

    // read data
    if ( nEspError == ESP_OK )
    {
        nEspError = readData( pu8Data, u32ByteNum );
    }

    return nEspError;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

esp_err_t EspI2c::readData( uint8_t * pu8Buffer,
                            const size_t u32ByteNum )
{
    esp_err_t nEspError = ESP_FAIL;

    if ( u32ByteNum && pu8Buffer && m_hCmd )
    {
        nEspError = i2c_master_start( m_hCmd );

        if ( nEspError == ESP_OK )
        {
            nEspError = i2c_master_write_byte( m_hCmd, ( m_u8SlaveAddress << 1 ) | READ_BIT, ACK_CHECK_EN );
        }

        // read data bytes and send ACK
        uint32_t i = 0;
        for ( i = 0; ( i < u32ByteNum - 1 ) && nEspError == ESP_OK; i++ )
        {
            nEspError = i2c_master_read_byte( m_hCmd, &pu8Buffer[i], ACK_VAL );
        }

        // read last byte and send NACK
        if ( nEspError == ESP_OK )
        {
            nEspError = i2c_master_read_byte( m_hCmd, &pu8Buffer[i], NACK_VAL );
        }

        if ( nEspError == ESP_OK )
        {
            nEspError = closeTransaction();
        }
    }

    clearTransaction();

    if ( nEspError != ESP_OK )
    {
        vlogError( "readData failed %s (slave=0x%02x, rxBuffer=%p, rxLen=0x%02x=%d)",
                   esp_err_to_name( nEspError ),
                   m_u8SlaveAddress,
                   pu8Buffer,
                   u32ByteNum,
                   u32ByteNum );
    }

    return nEspError;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

esp_err_t EspI2c::writeTransaction( const uint8_t u8Register,
                                    const uint8_t u8Data )
{
    // set write pointer for writing data
    esp_err_t nEspError = setRegister( u8Register );

    // write data
    if ( nEspError == ESP_OK )
    {
        nEspError = writeData( &u8Data, 1 );
    }

    return nEspError;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

esp_err_t EspI2c::writeTransaction( const uint8_t   u8Register,
                                    const uint8_t * pu8Data,
                                    const size_t    u32ByteNum )
{
    // set write pointer for writing data
    esp_err_t nEspError = setRegister( u8Register );

    // write data
    if ( nEspError == ESP_OK )
    {
        nEspError = writeData( pu8Data, u32ByteNum );
    }

    return nEspError;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

esp_err_t EspI2c::writeData( const uint8_t* pu8Buffer,
                             const size_t u32ByteNum )
{
    esp_err_t nEspError = ESP_FAIL;

    if ( u32ByteNum && pu8Buffer && m_hCmd )
    {
        nEspError = ESP_OK;

        // write data bytes and send ACK
        uint32_t i = 0;
        for ( i = 0; ( i < u32ByteNum ) && nEspError == ESP_OK; i++ )
        {
            nEspError = i2c_master_write_byte( m_hCmd, pu8Buffer[i], ACK_CHECK_EN );
        }

        if ( nEspError == ESP_OK )
        {
            nEspError = closeTransaction();
        }
    }

    clearTransaction();

    if ( nEspError != ESP_OK )
    {
        vlogError( "writeData failed %s (len %d)",
                   esp_err_to_name( nEspError ),
                   u32ByteNum );
    }

    return nEspError;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

esp_err_t EspI2c::transaction( const uint8_t * pu8TxData,
                               const uint8_t   u8TxLen,
                               uint8_t       * pu8RxData,
                               const uint8_t   u8RxLen )
{
    esp_err_t nEspError = openTransaction( true );

    if ( nEspError == ESP_OK && u8TxLen )
    {
        if ( pu8TxData )
        {
            // write data bytes and send ACK
            uint32_t i = 0;
            for ( i = 0; ( i < u8TxLen ) && nEspError == ESP_OK; i++ )
            {
                nEspError = i2c_master_write_byte( m_hCmd, pu8TxData[i], ACK_CHECK_EN );
            }
        }
        else
        {
            nEspError = ESP_FAIL;
        }
    }

    if ( nEspError == ESP_OK )
    {
        if ( u8RxLen )
        {
            nEspError = readData( pu8RxData, u8RxLen );
        }
        else
        {
            nEspError = closeTransaction();
        }
    }

    clearTransaction();

    if ( nEspError != ESP_OK )
    {
        vlogError( "writeReadTransaction failed %s", esp_err_to_name( nEspError ) );
    }

    return nEspError;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void EspI2c::printTransaction( const char    * pszMsg,
                               const uint8_t * pu8TxData,
                               const uint8_t   u8TxLen,
                               uint8_t       * pu8RxData,
                               const uint8_t   u8RxLen )
{
    char szTxText[3*u8TxLen+1] = {0};
    char szRxText[3*u8RxLen+1] = {0};

    char *p = szTxText;
    for (int i=0; i<u8TxLen; i++)
    {
        sprintf(p, " %02x", pu8TxData[i]);
        p+=3;
    }

    p = szRxText;
    for (int i=0; i<u8RxLen; i++)
    {
        sprintf(p, " %02x", pu8RxData[i]);
        p+=3;
    }

    vlogInfo("%s tx:%s rx:%s", pszMsg, szTxText, szRxText);
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

} // namespace UxEspCppLibrary

