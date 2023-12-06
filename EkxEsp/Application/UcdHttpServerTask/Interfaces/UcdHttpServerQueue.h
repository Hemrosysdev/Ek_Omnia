/*
 * UcdHttpServerQueue.h
 *
 *  Created on: 24.10.2019
 *      Author: gesser
 */

#ifndef UcdHttpServerQueue_h
#define UcdHttpServerQueue_h

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#include <cstdint>
#include <esp_err.h>

#include "FreeRtosQueue.h"
#include "EkxProtocol.h"

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class UcdHttpServerQueue : public UxEspCppLibrary::FreeRtosQueue
{

public:

    UcdHttpServerQueue();

    ~UcdHttpServerQueue() override;

    BaseType_t sendServerStart( void );

    BaseType_t sendServerStop( void );

    BaseType_t sendCtrlUartCommand( const EkxProtocol::DriverId                  u8DriverId,
                                    const uint32_t                               u32MsgCounter,
                                    const uint8_t                                u8RepeatCounter,
                                    EkxProtocol::Payload::MessagePayload * const pMessagePayload );

    BaseType_t sendAckNackReceived( const uint32_t u32MsgCounter,
                                    const bool     bSuccess );

};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#endif /* UcdHttpServerQueue_h */
