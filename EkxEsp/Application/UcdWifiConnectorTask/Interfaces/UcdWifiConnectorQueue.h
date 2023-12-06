/*
 * UcdWifiConnectorQueue.h
 *
 *  Created on: 24.10.2019
 *      Author: gesser
 */

#ifndef UcdWifiConnectorQueue_h
#define UcdWifiConnectorQueue_h

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#include "FreeRtosQueue.h"

#include <string>

#include "EkxProtocol.h"

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class UcdWifiConnectorQueue : public UxEspCppLibrary::FreeRtosQueue
{
public:

    UcdWifiConnectorQueue();

    ~UcdWifiConnectorQueue() override;

    BaseType_t sendCtrlUartCommand( const EkxProtocol::DriverId                  u8DriverId,
                                    const uint32_t                               u32MsgCounter,
                                    const uint8_t                                u8RepeatCounter,
                                    EkxProtocol::Payload::MessagePayload * const pMessagePayload );

    BaseType_t sendAckNackReceived( const uint32_t u32MsgCounter,
                                    const bool     bSuccess );

    BaseType_t sendAlive( const bool bAlive );

};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#endif /* UcdWifiConnectorQueue_h */
