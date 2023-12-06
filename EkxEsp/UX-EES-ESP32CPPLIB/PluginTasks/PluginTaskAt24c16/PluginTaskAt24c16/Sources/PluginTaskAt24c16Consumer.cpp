/*
 * PluginTaskAt24c16Consumer.cpp
 *
 *  Created on: 27.04.2020
 *      Author: gesser
 */

#include "PluginTaskAt24c16Consumer.h"
#include "PluginTaskAt24c16QueueTypes.h"

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

namespace UxEspCppLibrary
{

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

PluginTaskAt24c16Consumer::PluginTaskAt24c16Consumer( PluginTask * const    pPluginTask,
                                                      FreeRtosQueue * const pConsumerQueue )
    : PluginTaskConsumer( pPluginTask,
                          pConsumerQueue )
{
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

PluginTaskAt24c16Consumer::~PluginTaskAt24c16Consumer()
{
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

bool PluginTaskAt24c16Consumer::processTaskMessage( const uint8_t * const pReceiveMsg )
{
    bool bProcessed = false;

    const PluginTaskAt24c16ConsumerQueueTypes::message * pMessage = reinterpret_cast<const PluginTaskAt24c16ConsumerQueueTypes::message *>( pReceiveMsg );

    if ( pMessage )
    {
        switch ( pMessage->u32Type )
        {
            case PluginTaskAt24c16ConsumerQueueTypes::messageType::EEPROM_DATA:
            {
                processEepromData( pMessage->payload.eepromData.m_bValid,
                                   pMessage->payload.eepromData.m_szProductNo,
                                   pMessage->payload.eepromData.m_szSerialNo );
                bProcessed = true;
            }
            break;

            default:
            {
                // no message for this plugin
            }
            break;
        }
    }

    return bProcessed;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void PluginTaskAt24c16Consumer::processEepromData( const bool          bValid,
                                                   const std::string & strProductNo,
                                                   const std::string & strSerialNo )
{
    // do nothing; work will be done in overloaded function
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

} // namespace UxEspCppLibrary
