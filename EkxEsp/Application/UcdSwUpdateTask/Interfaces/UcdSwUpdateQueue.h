/*!*************************************************************************************************************************************************************
 *
 * @file UcdSwUpdateQueue.h
 * @brief Source file of component unit SW Update.
 *
 * @author Ultratronik GmbH
 *         Dornierstr. 9
 *         D-82205 Gilching
 *         http://www.ultratronik.de
 *
 * @author Gerd Esser, Research & Development
 *
 * @date 27.08.2020
 *
 * @copyright Copyright 2020 by Ultratronik GmbH.
 *            All rights reserved.
 *            None of this file or parts of it may be
 *            copied, redistributed or used in any other way
 *            without written approval of Ultratronik GmbH.
 *
 **************************************************************************************************************************************************************/

#ifndef UcdSwUpdateQueue_h
#define UcdSwUpdateQueue_h

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#include "FreeRtosQueue.h"
#include "EkxProtocol.h"

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class UcdSwUpdateQueue : public UxEspCppLibrary::FreeRtosQueue
{

public:

    UcdSwUpdateQueue();

    ~UcdSwUpdateQueue() override;

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

#endif /* UcdSwUpdateQueue_h */
