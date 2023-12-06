/*!*************************************************************************************************************************************************************
 *
 * @file SocketDispatcherQueueTypes.h
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

#ifndef SocketDispatcherQueueTypes_h
#define SocketDispatcherQueueTypes_h

/*!*************************************************************************************************************************************************************
 *
 **************************************************************************************************************************************************************/

#include "UartControlledDevice.h"

/*!*************************************************************************************************************************************************************
 *
 **************************************************************************************************************************************************************/

namespace SocketDispatcherQueueTypes
{
    enum class messageType : uint32_t
    {
        PAYLOAD_FROM_SCD,
        SOCKET_PAYLOAD
    };

    struct UcdPayload
    {
        //uint8_t   u8Data[ SOCKET_CONTROLLED_DEVICE_MAX_PAYLOAD_SIZE ];
        uint32_t  u32DataLen;
    };

    struct SocketPayload
    {
        size_t  u32Length;
        void*   pBuffer;
    };

    union messagePayload
    {
        UcdPayload         scdPayload;
        SocketPayload      socketPayload;
    };

    struct message
    {
        messageType    u32Type;
        messagePayload payload;
    };

    const uint32_t u32MsgSize = sizeof( message );
}

/*!*************************************************************************************************************************************************************
 *
 **************************************************************************************************************************************************************/

#endif /* SocketDispatcherQueueTypes_h */
