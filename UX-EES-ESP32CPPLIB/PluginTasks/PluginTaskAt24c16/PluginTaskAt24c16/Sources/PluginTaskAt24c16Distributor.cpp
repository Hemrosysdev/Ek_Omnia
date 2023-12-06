/*
 * PluginTaskAt24c16ReceiverQueue.cpp
 *
 *  Created on: 27.04.2020
 *      Author: gesser
 */

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#include "PluginTaskAt24c16Distributor.h"

#include <string.h>

#include "FreeRtosQueue.h"
#include "PluginTaskAt24c16QueueTypes.h"
#include "UxLibCommon.h"

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

namespace UxEspCppLibrary
{

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

PluginTaskAt24c16Distributor::PluginTaskAt24c16Distributor()
{
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

PluginTaskAt24c16Distributor::~PluginTaskAt24c16Distributor()
{
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

BaseType_t PluginTaskAt24c16Distributor::sendEepromData( const bool          bValid,
                                                         const std::string & strProductNo,
                                                         const std::string & strSerialNo )
{
    BaseType_t nReturn = pdFALSE;

    if ( queueItemBuffer() )
    {
        nReturn = pdTRUE;

        PluginTaskAt24c16ConsumerQueueTypes::message * pMessage = reinterpret_cast<PluginTaskAt24c16ConsumerQueueTypes::message *>( queueItemBuffer() );

        pMessage->u32Type = PluginTaskAt24c16ConsumerQueueTypes::messageType::EEPROM_DATA;
        pMessage->payload.eepromData.m_bValid = bValid;
        UxEspCppLibrary::UxLibCommon::saveStringCopy( pMessage->payload.eepromData.m_szProductNo,
                                                      strProductNo.c_str(),
                                                      sizeof( pMessage->payload.eepromData.m_szProductNo ) );
        UxEspCppLibrary::UxLibCommon::saveStringCopy( pMessage->payload.eepromData.m_szSerialNo,
                                                      strSerialNo.c_str(),
                                                      sizeof( pMessage->payload.eepromData.m_szSerialNo ) );

        for ( int i = 0; i < consumerQueues().size(); i++ )
        {
            if ( consumerQueues()[i]->send( pMessage ) != pdTRUE )
            {
                nReturn = pdFALSE;
            }
        }
    }

    return nReturn;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

} // namespace UxEspCppLibrary
