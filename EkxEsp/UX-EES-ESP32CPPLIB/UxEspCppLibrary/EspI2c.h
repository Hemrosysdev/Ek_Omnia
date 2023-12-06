/*
 * EspI2c.h
 *
 *  Created on: 19.02.2021
 *      Author: gesser
 */

#ifndef EspI2c_h
#define EspI2c_h

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#include "EspLog.h"

#include <driver/i2c.h>

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

namespace UxEspCppLibrary
{

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class EspI2c : public EspLog
{
public:

    EspI2c( const std::string & strLogTag,
            const i2c_port_t    nPort = I2C_NUM_MAX,
            const uint8_t       u8SlaveAddress = 0 );

    ~EspI2c() override;

    void setPort( const i2c_port_t nPort );

    i2c_port_t port() const;

    i2c_cmd_handle_t cmdHandle() const;

    void setSlaveAddress( const uint8_t u8SlaveAddress );

    uint8_t slaveAddress() const;

    static esp_err_t initMaster( const i2c_port_t    nMasterPort,
                                 const gpio_num_t    nGpioSda,
                                 const gpio_pullup_t nGpioSdaPullup,
                                 const gpio_num_t    nGpioScl,
                                 const gpio_pullup_t nGpioSclPullup,
                                 const uint32_t      u32ClkSpeed );

    esp_err_t openTransaction( const bool bWrite );

    esp_err_t closeTransaction( void );

    void clearTransaction( const bool bDoClear = true );

    esp_err_t setRegister( const uint8_t u8Register );

    esp_err_t readTransaction( const uint8_t u8Register,
                               uint8_t *     pu8Buffer,
                               const size_t  u32ByteNum );

    esp_err_t readData( uint8_t *    pu8Buffer,
                        const size_t u32ByteNum );

    esp_err_t writeTransaction( const uint8_t u8Register,
                                const uint8_t u8Data );

    esp_err_t writeTransaction( const uint8_t   u8Register,
                                const uint8_t * pu8Data,
                                const size_t    u32ByteNum );

    esp_err_t writeData( const uint8_t * pu8Buffer,
                         const size_t    u32ByteNum );

    esp_err_t transaction( const uint8_t * pu8TxData,
                           const uint8_t   u8TxLen,
                           uint8_t *       pu8RxData,
                           const uint8_t   u8RxLen );

    void printTransaction( const char *    pszMsg,
                           const uint8_t * pu8TxData,
                           const uint8_t   u8TxLen,
                           uint8_t *       pu8RxData,
                           const uint8_t   u8RxLen );

private:

    EspI2c() = delete;

private:

    i2c_port_t       m_nPort { I2C_NUM_MAX };

    i2c_cmd_handle_t m_hCmd { nullptr };

    uint8_t          m_u8SlaveAddress { 0 };

};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

} // namespace UxEspCppLibrary

#endif /* EspI2c_h */
