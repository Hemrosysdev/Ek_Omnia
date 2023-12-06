/*!*************************************************************************************************************************************************************
 *
 * @file UcdDiscreteInQueue.h
 * @brief Source file of component unit DiscreteInTask.
 *
 * This file was developed as part of generic DiscreteIn interface
 *
 * @author Ultratronik GmbH
 *         Dornierstr. 9
 *         D-82205 Gilching
 *         http://www.ultratronik.de
 * @author Srdjan Nikolic, Entwicklung
 *
 * @copyright Copyright 2020 by Ultratronik GmbH.
 *            All rights reserved.
 *            None of this file or parts of it may be
 *            copied, redistributed or used in any other way
 *            without written approval of Ultratronik GmbH.
 *
 **************************************************************************************************************************************************************/

#ifndef UcdDiscreteInQueue_h
#define UcdDiscreteInQueue_h

/***************************************************************************************************************************************************************
 *      Includes
 **************************************************************************************************************************************************************/

#include "FreeRtosQueue.h"
#include "EkxProtocol.h"

/***************************************************************************************************************************************************************
 *      Declarations
 **************************************************************************************************************************************************************/

class UcdDiscreteInQueue : public UxEspCppLibrary::FreeRtosQueue
{
public:

    UcdDiscreteInQueue();

    ~UcdDiscreteInQueue() override;

    BaseType_t sendCtrlUartCommand( const EkxProtocol::DriverId                  u8DriverId,
                                    const uint32_t                               u32MsgCounter,
                                    const uint8_t                                u8RepeatCounter,
                                    EkxProtocol::Payload::MessagePayload * const pMessagePayload );

    BaseType_t sendAckNackReceived( const uint32_t u32MsgCounter,
                                    const bool     bSuccess );

};


#endif /* UcdDiscreteInQueue_h */
