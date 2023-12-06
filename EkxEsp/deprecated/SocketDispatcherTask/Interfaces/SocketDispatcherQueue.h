/*!*************************************************************************************************************************************************************
 *
 * @file SocketDispatcherQueue.h
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

#ifndef SocketDispatcherQueue_h
#define SocketDispatcherQueue_h

/*!*************************************************************************************************************************************************************
 *
 **************************************************************************************************************************************************************/

#include "../../Common/Interfaces/UartControlledDevice.h"
#include "FreeRtosQueue.h"

/*!*************************************************************************************************************************************************************
 *
 **************************************************************************************************************************************************************/

class SocketDispatcherQueue : public UxEspCppLibrary::FreeRtosQueue
{
public:

    SocketDispatcherQueue();

    virtual ~SocketDispatcherQueue();

    BaseType_t sendCheckLink( const uint32_t NoRetries );
    BaseType_t sendDataToSocket( const uint8_t * const pbuf,
                                 const uint32_t        length );
    BaseType_t sendSocketPayload( void * const  pBuffer,
                                  const size_t  u32Length );
};

/*!*************************************************************************************************************************************************************
 *
 **************************************************************************************************************************************************************/

#endif /* SocketDispatcherQueue_h */
