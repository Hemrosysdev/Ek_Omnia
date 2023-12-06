/*!*************************************************************************************************************************************************************
 *
 * @file UcdEepromQueue.h
 * @brief Source file of component unit Eeprom-Task.
 *
 * This file was developed as part of SCD  holder of PluginTaskAt24c16.
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

#ifndef UcdEepromQueue_h
#define UcdEepromQueue_h

/***************************************************************************************************************************************************************
 *
 **************************************************************************************************************************************************************/

#include "FreeRtosQueue.h"
#include "EkxProtocol.h"

/***************************************************************************************************************************************************************
 *
 **************************************************************************************************************************************************************/

class UcdEepromQueue : public UxEspCppLibrary::FreeRtosQueue
{
public:

    UcdEepromQueue( const UBaseType_t uRequiredConsumerQueueItemSize );

    ~UcdEepromQueue() override;

    BaseType_t sendCtrlUartCommand( const EkxProtocol::DriverId                  u8DriverId,
                                    const uint32_t                               u32MsgCounter,
                                    const uint8_t                                u8RepeatCounter,
                                    EkxProtocol::Payload::MessagePayload * const pMessagePayload );

    BaseType_t sendAckNackReceived( const uint32_t u32MsgCounter,
                                    const bool     bSuccess );

};

/***************************************************************************************************************************************************************
 *
 **************************************************************************************************************************************************************/

#endif /* UcdEepromQueue_h */
