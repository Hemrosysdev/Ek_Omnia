/*!*************************************************************************************************************************************************************
 *
 * @file Eeprom.h
 * @brief Source file of component unit Eeprom-Task.
 *
 * This file was developed as part of SCD  holder of PluginTaskAt24c16.
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

#ifndef UcdEepromTask_h
#define UcdEepromTask_h

/*!*************************************************************************************************************************************************************
 *
 **************************************************************************************************************************************************************/

#include "UartControlledDevice.h"
#include "UcdEepromQueue.h"
#include "FreeRtosQueueTask.h"
#include "PluginTaskAt24c16.h"
#include "PluginTaskAt24c16Consumer.h"
#include "PluginTaskAt24c16Types.h"
#include "EkxProtocol.h"

/*!*************************************************************************************************************************************************************
 *
 **************************************************************************************************************************************************************/

class UcdEepromTask : public UartControlledDevice
    , public UxEspCppLibrary::PluginTaskAt24c16Consumer
{

public:

    UcdEepromTask( UxEspCppLibrary::EspApplication *    pApplication,
                   UxEspCppLibrary::PluginTaskAt24c16 * pPluginTaskAt24c16,
                   const EkxProtocol::DriverId          u8DriverId,
                   const int                            nTaskStackSize,
                   const UBaseType_t                    uTaskPriority );

    ~UcdEepromTask() override;

    void processTaskMessage( void ) override;

    UcdEepromQueue * eepromQueue( void );

    const UcdEepromQueue * eepromQueue( void ) const;

    void processEepromData( const bool          bValid,
                            const std::string & strProductNo,
                            const std::string & strzSerialNo ) override;

    bool queueCtrlUartCommand( const EkxProtocol::DriverId                  u8DriverId,
                               const uint32_t                               u32MsgCounter,
                               const uint8_t                                u8RepeatCounter,
                               EkxProtocol::Payload::MessagePayload * const pMessagePayload ) override;

    bool queueAckNackReceived( const uint32_t u32MsgCounter,
                               const bool     bSuccess ) override;

private:

    bool startupStep( const FreeRtosQueueTask::startupStep_t nStartupStep ) override;

    void processCtrlUartCommand( const EkxProtocol::DriverId                        u8DriverId,
                                 const uint32_t                                     u32MsgCounter,
                                 const uint8_t                                      u8RepeatCounter,
                                 const EkxProtocol::Payload::MessagePayload * const pMessagePayload ) override;

    void reportDriverStatus( const bool bFirst ) override;

private:

    bool        m_bValid { false };
    std::string m_strProductNo;
    std::string m_strSerialNo;
};

/*!*************************************************************************************************************************************************************
 *
 **************************************************************************************************************************************************************/

#endif /* UcdEepromTask_h */
