/*
 * PluginTaskAt24c16.cpp
 *
 *  Created on: 27.04.2020
 *      Author: gesser
 */

#include "PluginTaskAt24c16.h"
#include "PluginTaskAt24c16Queue.h"
#include "PluginTaskAt24c16QueueTypes.h"
#include "I2cAt24c16.h"
#include "PluginTaskAt24c16Distributor.h"

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

namespace UxEspCppLibrary
{

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

PluginTaskAt24c16::PluginTaskAt24c16( EspApplication * const pApplication,
                                      const int              nTaskPriority,
                                      const i2c_port_t       nI2cMasterPort )
    : PluginTask( pApplication,
                  4096,
                  nTaskPriority,
                  "PluginTaskAt24c16",
                  new PluginTaskAt24c16Queue(),
                  new PluginTaskAt24c16Distributor(),
                  sizeof( PluginTaskAt24c16ConsumerQueueTypes::message ) )
    , m_pI2cAt24c16( new I2cAt24c16( nI2cMasterPort,
                                     pluginTaskAt24c16Distributor() ) )
{
    logInfo( "PluginTaskAt24c16 constructor" );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

PluginTaskAt24c16::~PluginTaskAt24c16()
{
    delete m_pI2cAt24c16;
    m_pI2cAt24c16 = nullptr;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void PluginTaskAt24c16::processTaskMessage( void )
{
    const PluginTaskAt24c16QueueTypes::message * pMessage = reinterpret_cast<const PluginTaskAt24c16QueueTypes::message *>( receiveMsg() );

    if ( pMessage )
    {
        switch ( pMessage->u32Type )
        {
            case PluginTaskAt24c16QueueTypes::messageType::WRITE_PRODUCT_NO:
            {
                m_pI2cAt24c16->writeProductNo( pMessage->payload.productNoView.m_pszProductNo );
            }
            break;

            case PluginTaskAt24c16QueueTypes::messageType::WRITE_SERIAL_NO:
            {
                m_pI2cAt24c16->writeSerialNo( pMessage->payload.serialNoView.m_pszSerialNo );
            }
            break;

            case PluginTaskAt24c16QueueTypes::messageType::REQUEST_DATA:
            {
                m_pI2cAt24c16->readProductData();
            }
            break;

            default:
            {
                vlogError( "processTaskMessage: receive illegal message type %d",
                           pMessage->u32Type );
            }
            break;
        }
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

bool PluginTaskAt24c16::startupStep( const FreeRtosQueueTask::startupStep_t nStartupStep )
{
    bool bContinueStartup = true;

    switch ( nStartupStep )
    {
        case FreeRtosQueueTask::startupStep_t::startupStep2:
        {
            logInfo( "startupStep2" );
            m_pI2cAt24c16->readProductData();
        }
        break;

        default:
        {
            // do nothing
        }
        break;
    }

    return bContinueStartup;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

PluginTaskAt24c16Queue * PluginTaskAt24c16::pluginTaskAt24c16Queue( void )
{
    return dynamic_cast<PluginTaskAt24c16Queue *>( taskQueue() );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

const PluginTaskAt24c16Queue * PluginTaskAt24c16::pluginTaskAt24c16Queue( void ) const
{
    return dynamic_cast<const PluginTaskAt24c16Queue *>( taskQueue() );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

PluginTaskAt24c16Distributor * PluginTaskAt24c16::pluginTaskAt24c16Distributor( void )
{
    return dynamic_cast<PluginTaskAt24c16Distributor *>( distributor() );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

const PluginTaskAt24c16Distributor * PluginTaskAt24c16::pluginTaskAt24c16Distributor( void ) const
{
    return dynamic_cast<const PluginTaskAt24c16Distributor *>( distributor() );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

} // namespace UxEspCppLibrary
