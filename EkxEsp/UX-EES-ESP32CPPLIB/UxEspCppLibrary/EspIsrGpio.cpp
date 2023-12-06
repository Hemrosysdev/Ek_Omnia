/*
 * EspIsrGpio.cpp
 *
 *  Created on: 27.10.2019
 *      Author: gesser
 */

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#include "EspIsrGpio.h"

#include "FreeRtosQueue.h"

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

namespace UxEspCppLibrary
{

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

EspIsrGpio::EspIsrGpio( const std::string & strLogName )
    : EspGpio( strLogName )
    , m_pQueue( NULL )
    , m_nMessageId( -1 )
{
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

EspIsrGpio::EspIsrGpio( const gpio_num_t      nGpioNum,     /*!< GPIO number */
                        const gpio_mode_t     nMode,        /*!< GPIO mode: set input/output mode                     */
                        const gpio_pullup_t   nPullUpEn,    /*!< GPIO pull-up                                         */
                        const gpio_pulldown_t nPullDownEn,  /*!< GPIO pull-down                                       */
                        const gpio_int_type_t nIntrType,    /*!< GPIO interrupt type                                  */
                        FreeRtosQueue *       pQueue,
                        const int             nMessageId,
                        const std::string &   strLogName )
    : EspGpio( strLogName )
    , m_pQueue( NULL )
    , m_nMessageId( -1 )
{
    create( nGpioNum, nMode, nPullUpEn, nPullDownEn, nIntrType, pQueue, nMessageId );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

EspIsrGpio::~EspIsrGpio()
{
    m_pQueue = NULL;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

esp_err_t EspIsrGpio::create( const gpio_num_t      nGpioNum,    /*!< GPIO number */
                              const gpio_mode_t     nMode,       /*!< GPIO mode: set input/output mode                     */
                              const gpio_pullup_t   nPullUpEn,   /*!< GPIO pull-up                                         */
                              const gpio_pulldown_t nPullDownEn, /*!< GPIO pull-down                                       */
                              const gpio_int_type_t nIntrType,   /*!< GPIO interrupt type                                  */
                              FreeRtosQueue *       pQueue,
                              const int             nMessageId )
{
    esp_err_t nEspError = ESP_FAIL;

    m_pQueue     = pQueue;
    m_nMessageId = nMessageId;

    nEspError = EspGpio::create( nGpioNum, nMode, nPullUpEn, nPullDownEn, nIntrType );

    if ( nEspError == ESP_OK )
    {
        //change gpio interrupt type for one pin
        nEspError = gpio_set_intr_type( nGpioNum, nIntrType );
    }

    if ( nEspError == ESP_OK )
    {
        //hook isr handler for this gpio pin
        nEspError = gpio_isr_handler_add( nGpioNum, isrCallback, this );
    }

    return nEspError;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void EspIsrGpio::isrCallback( void * pArg )
{
    EspIsrGpio * pThis = reinterpret_cast<EspIsrGpio *>( pArg );

    if ( pThis
         && pThis->m_pQueue )
    {
        pThis->m_pQueue->sendSimpleEventFromIsr( pThis->m_nMessageId );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

// static
void EspIsrGpio::installService( const int nLevel )
{
    //install gpio isr service
    ESP_ERROR_CHECK( gpio_install_isr_service( nLevel ) );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

} // namespace UxEspCppLibrary
