/*!*************************************************************************************************************************************************************
 *
 * @file SocketDispatcherQueue.cpp
 * @brief Source file of component unit Socket-Dispatcher-Task.
 *
 * This file was developed as part of Socket-Dispatcher.
 *
 * @author Ultratronik GmbH
 *         Dornierstr. 9
 *         D-82205 Gilching
 *         http://www.ultratronik.de
 * @author Srdjan Nikolic, Entwicklung - <optional according to the authors needs>
 *
 * @copyright Copyright 2020 by Ultratronik GmbH.
 *            All rights reserved.
 *            None of this file or parts of it may be
 *            copied, redistributed or used in any other way
 *            without written approval of Ultratronik GmbH.
 *
 **************************************************************************************************************************************************************/

/***************************************************************************************************************************************************************
 *      Includes
 **************************************************************************************************************************************************************/

#include "SocketDispatcherQueue.h"

#include <cstring>
#include "../../Common/Interfaces/UartControlledDevice.h"

#include "SocketDispatcherQueueTypes.h"
#include "FreeRtosQueueTask.h"

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

SocketDispatcherQueue::SocketDispatcherQueue()
: UxEspCppLibrary::FreeRtosQueue( 100,
                                  UxEspCppLibrary::FreeRtosQueueTask::minTargetQueueSize( sizeof( SocketDispatcherQueueTypes::message ) ),
                                  "SocketDispatcherQueue" )
{
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

SocketDispatcherQueue::~SocketDispatcherQueue()
{
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

BaseType_t SocketDispatcherQueue::sendCheckLink( const uint32_t NoRetries  )
{
    SocketDispatcherQueueTypes::message socketDispatcherMessage;

    socketDispatcherMessage.u32Type = SocketDispatcherQueueTypes::messageType::CHECK_LINK;
    socketDispatcherMessage.payload.checkLinkPayload.u32RetriesNum = NoRetries;
    return send( &socketDispatcherMessage );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

BaseType_t SocketDispatcherQueue::sendDataToSocket( const uint8_t * const pbuf,
                                                    const uint32_t        length )
{
    BaseType_t result = pdFAIL;
    uint32_t payloadBuffSize = 0;

    SocketDispatcherQueueTypes::message socketDispatcherMessage;

    socketDispatcherMessage.u32Type = SocketDispatcherQueueTypes::messageType::PAYLOAD_FROM_SCD;

    //////////payloadBuffSize = sizeof( socketDispatcherMessage.payload.scdPayload.u8Data );
    if (  length <= payloadBuffSize )
    {
        socketDispatcherMessage.payload.scdPayload.u32DataLen = length;
        //memcpy( socketDispatcherMessage.payload.scdPayload.u8Data, pbuf, length);
        result = send( &socketDispatcherMessage );
    }
    else
    {
        vlogError( "sendCUcdPayload Length %d > buff %d",length, payloadBuffSize );
        //@ TODO place for Fatal
    }

    return result;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

BaseType_t SocketDispatcherQueue::sendSocketPayload( void * const  pBuffer,
                                                     const size_t  u32Length )
{
    BaseType_t result = pdFAIL;

    SocketDispatcherQueueTypes::message socketDispatcherMessage;

    socketDispatcherMessage.u32Type = SocketDispatcherQueueTypes::messageType::SOCKET_PAYLOAD;
    socketDispatcherMessage.payload.socketPayload.pBuffer   = pBuffer;
    socketDispatcherMessage.payload.socketPayload.u32Length = u32Length;

    result = send( &socketDispatcherMessage );

    return result;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/
