/*
 * EspGpio.h
 *
 *  Created on: 22.10.2019
 *      Author: gesser
 */

#ifndef APPLICATION_COMPONENTS_COMPCOMMON_INTERFACES_ESPGPIO_H_
#define APPLICATION_COMPONENTS_COMPCOMMON_INTERFACES_ESPGPIO_H_

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#include <stdint.h>
#include <string>
#include <driver/gpio.h>
#include "EspLog.h"

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

namespace UxEspCppLibrary
{

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class EspGpio : public EspLog
{
public:

    enum class DriveCapability
    {
        Weak5Ma = 0,
        Stronger10Ma,
        Medium20Ma,
        Strongest40Ma
    };

public:

    EspGpio( const std::string & strLogName );
    EspGpio( const gpio_num_t      nGpioNum,      /*!< GPIO number */
             const gpio_mode_t     nMode,         /*!< GPIO mode: set input/output mode                     */
             const gpio_pullup_t   nPullUpEn,     /*!< GPIO pull-up                                         */
             const gpio_pulldown_t nPullDownEn,   /*!< GPIO pull-down                                       */
             const gpio_int_type_t nIntrType,     /*!< GPIO interrupt type                                  */
             const std::string &   strLogName );

    virtual ~EspGpio();

    esp_err_t create( const gpio_num_t      nGpioNum,      /*!< GPIO number */
                      const gpio_mode_t     nMode,         /*!< GPIO mode: set input/output mode                     */
                      const gpio_pullup_t   nPullUpEn,     /*!< GPIO pull-up                                         */
                      const gpio_pulldown_t nPullDownEn,   /*!< GPIO pull-down                                       */
                      const gpio_int_type_t nIntrType );   /*!< GPIO interrupt type                                  */

    esp_err_t setHigh( void );

    esp_err_t setLow( void );

    esp_err_t set( const uint32_t u32Value );

    bool isHigh( void );

    bool isLow( void );

    int level( void );

    gpio_num_t gpioNum( void ) const;

    esp_err_t disableInterrupt( void );

    esp_err_t enableInterrupt( void );

    static esp_err_t setDriveCapability( const gpio_num_t      nGpioNum,
                                         const DriveCapability nDriveCapability );

    static DriveCapability driveCapability( const gpio_num_t nGpioNum );

    esp_err_t setDriveCapability( const DriveCapability nDriveCapability );

    DriveCapability driveCapability() const;

private:

    EspGpio();

private:

    gpio_num_t      m_nGpioNum;

    gpio_int_type_t m_nIntrType;

};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

} // namespace UxEspCppLibrary

#endif /* APPLICATION_COMPONENTS_COMPCOMMON_INTERFACES_ESPGPIO_H_ */
