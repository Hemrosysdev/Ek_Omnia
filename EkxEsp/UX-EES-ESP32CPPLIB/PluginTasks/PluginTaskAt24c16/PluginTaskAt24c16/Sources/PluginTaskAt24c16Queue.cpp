/*
 * PluginTaskAt24c16Queue.cpp
 *
 *  Created on: 27.04.2020
 *      Author: gesser
 */

#include "PluginTaskAt24c16Queue.h"

#include <string.h>

#include "FreeRtosQueueTask.h"
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

PluginTaskAt24c16Queue::PluginTaskAt24c16Queue()
    : FreeRtosQueue( 10,
                     FreeRtosQueueTask::minTargetQueueSize( sizeof( PluginTaskAt24c16QueueTypes::message ) ),
                     "PluginTaskAt24c16Queue" )
{

}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

PluginTaskAt24c16Queue::~PluginTaskAt24c16Queue()
{
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

BaseType_t PluginTaskAt24c16Queue::sendWriteProductNo( const std::string & strProductNo )
{
    PluginTaskAt24c16QueueTypes::message message;

    message.u32Type = PluginTaskAt24c16QueueTypes::messageType::WRITE_PRODUCT_NO;

    UxEspCppLibrary::UxLibCommon::saveStringCopy( message.payload.productNoView.m_pszProductNo,
                                                  strProductNo.c_str(),
                                                  sizeof( message.payload.productNoView.m_pszProductNo ) );

    return send( &message );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

BaseType_t PluginTaskAt24c16Queue::sendWriteSerialNo( const std::string & strSerialNo )
{
    PluginTaskAt24c16QueueTypes::message message;

    message.u32Type = PluginTaskAt24c16QueueTypes::messageType::WRITE_SERIAL_NO;

    UxEspCppLibrary::UxLibCommon::saveStringCopy( message.payload.serialNoView.m_pszSerialNo,
                                                  strSerialNo.c_str(),
                                                  sizeof( message.payload.serialNoView.m_pszSerialNo ) );

    return send( &message );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

BaseType_t PluginTaskAt24c16Queue::sendRequestData( void )
{
    PluginTaskAt24c16QueueTypes::message message;

    message.u32Type = PluginTaskAt24c16QueueTypes::messageType::REQUEST_DATA;

    return send( &message );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

} // namespace UxEspCppLibrary
