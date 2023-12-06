/*
 * EspIsrGpio.h
 *
 *  Created on: 27.10.2019
 *      Author: gesser
 */

#ifndef ESPISRGPIO_H
#define ESPISRGPIO_H

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#include "EspGpio.h"

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

namespace UxEspCppLibrary
{

class FreeRtosQueue;

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class EspIsrGpio : public EspGpio
{
public:

    EspIsrGpio( const std::string & strLogName );
    EspIsrGpio( const gpio_num_t      nGpioNum,      /*!< GPIO number */
                const gpio_mode_t     nMode,         /*!< GPIO mode: set input/output mode                     */
                const gpio_pullup_t   nPullUpEn,     /*!< GPIO pull-up                                         */
                const gpio_pulldown_t nPullDownEn,   /*!< GPIO pull-down                                       */
                const gpio_int_type_t nIntrType,     /*!< GPIO interrupt type                                  */
                FreeRtosQueue *       pQueue,
                const int             nMessageId,
                const std::string &   strLogName );

    virtual ~EspIsrGpio();

    esp_err_t create( const gpio_num_t      nGpioNum,      /*!< GPIO number */
                      const gpio_mode_t     nMode,         /*!< GPIO mode: set input/output mode                     */
                      const gpio_pullup_t   nPullUpEn,     /*!< GPIO pull-up                                         */
                      const gpio_pulldown_t nPullDownEn,   /*!< GPIO pull-down                                       */
                      const gpio_int_type_t nIntrType,     /*!< GPIO interrupt type                                  */
                      FreeRtosQueue *       pQueue,
                      const int             nMessageId );

    static void installService( const int nLevel );

protected:

    static void isrCallback( void * pArg );

private:

    EspIsrGpio();

private:

    FreeRtosQueue * m_pQueue;

    int             m_nMessageId;

};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

} // namespace UxEspCppLibrary

#endif /* ESPISRGPIO_H */
