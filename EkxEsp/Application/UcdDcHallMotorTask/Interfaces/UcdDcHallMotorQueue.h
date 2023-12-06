/*!*************************************************************************************************************************************************************
 *
 * @file UcdDcHallMotorQueue.h
 * @brief Source file of component unit SW Update.
 *
 * @author Ultratronik GmbH
 *         Dornierstr. 9
 *         D-82205 Gilching
 *         http://www.ultratronik.de
 *
 * @author Gerd Esser, Research & Development
 *
 * @date 06.11.2021
 *
 * @copyright Copyright 2021 by Ultratronik GmbH.
 *            All rights reserved.
 *            None of this file or parts of it may be
 *            copied, redistributed or used in any other way
 *            without written approval of Ultratronik GmbH.
 *
 **************************************************************************************************************************************************************/

#ifndef UcdDcHallMotorQueue_H
#define UcdDcHallMotorQueue_H

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#include "FreeRtosQueue.h"

#include <cstdint>
#include "EkxProtocol.h"

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class UcdDcHallMotorQueue : public UxEspCppLibrary::FreeRtosQueue
{

public:

    UcdDcHallMotorQueue();

    virtual ~UcdDcHallMotorQueue();

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

#endif /* UcdDcHallMotorQueue_H */
