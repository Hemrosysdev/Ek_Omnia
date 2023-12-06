/*!*************************************************************************************************************************************************************
 *
 * @file UartControlledDevice.h
 * @brief Source file of component unit <title>.
 *
 * This file was developed as part of <component>.
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

#ifndef UartControlledDevice_h
#define UartControlledDevice_h

/***************************************************************************************************************************************************************
 *
 **************************************************************************************************************************************************************/

#include "FreeRtosQueueTask.h"

#include <string>
#include "FreeRtosQueueTimer.h"
#include "EkxProtocol.h"

/***************************************************************************************************************************************************************
 *
 **************************************************************************************************************************************************************/

class UartControlledDevice : public UxEspCppLibrary::FreeRtosQueueTask
{

public:

    UartControlledDevice( const std::string &               strUcdName,
                          const std::string &               strType,
                          const EkxProtocol::DriverId       u8DriverId,
                          UxEspCppLibrary::EspApplication * pApplication,
                          const uint32_t                    u32StackSize,
                          const UBaseType_t                 uTaskPriority,
                          const std::string &               strTaskName,
                          UxEspCppLibrary::FreeRtosQueue *  pQueue,
                          const TickType_t                  xReceiveTicksToWait = portMAX_DELAY );

    ~UartControlledDevice() override;

    EkxProtocol::DriverId driverId() const;

    const std::string & name( void ) const;

    const std::string & type( void ) const;

    virtual bool queueAlive( const bool bAlive );

    void sendCtrlUartReplyAck( const uint32_t u32AckCounter,
                               const uint8_t  u8RepeatCounter );

    void sendCtrlUartReplyNack( const uint32_t                               u32AckCounter,
                                const uint8_t                                u8RepeatCounter,
                                const EkxProtocol::Payload::Nack::NackReason u8NackReason );

    uint32_t sendCtrlUartReply( EkxProtocol::Payload::MessagePayload * const pMessagePayload,
                                const bool                                   bIncMsgCounter );

    void sendPayloadToCtrlUart( const EkxProtocol::DriverId                  u8DriverId,
                                const uint32_t                               u32MsgCounter,
                                const uint8_t                                u8RepeatCounter,
                                EkxProtocol::Payload::MessagePayload * const pMessagePayload );

    virtual bool queueCtrlUartCommand( const EkxProtocol::DriverId                  u8DriverId,
                                       const uint32_t                               u32MsgCounter,
                                       const uint8_t                                u8RepeatCounter,
                                       EkxProtocol::Payload::MessagePayload * const pMessagePayload ) = 0;

    virtual bool queueAckNackReceived( const uint32_t u32MsgCounter,
                                       const bool     bSuccess ) = 0;

    void queueRequestDriverStatus();

    void processBaseCtrlUartCommand( const EkxProtocol::DriverId                  u8DriverId,
                                     const uint32_t                               u32MsgCounter,
                                     const uint8_t                                u8RepeatCounter,
                                     EkxProtocol::Payload::MessagePayload * const pMessagePayload );

    void processBaseAlive( const bool bAlive );

    virtual void processAckNackReceived( const uint32_t u32MsgCounter,
                                         const bool     bSuccess );

    uint32_t msgCounter() const;

    void incMsgCounter();

    void registerDriver();

    bool isAlive() const;

protected:

    virtual void reportDriverStatus( const bool bFirst );

    virtual void processCtrlUartCommand( const EkxProtocol::DriverId                        u8DriverId,
                                         const uint32_t                                     u32MsgCounter,
                                         const uint8_t                                      u8RepeatCounter,
                                         const EkxProtocol::Payload::MessagePayload * const pMessagePayload );

    virtual void processAlive( const bool bAlive );

private:

    std::string                 m_strName;

    std::string                 m_strType;

    const EkxProtocol::DriverId m_u8DriverId { EkxProtocol::DriverId::Last };

    uint32_t                    m_u32MsgCounter { 0 };

    bool                        m_bAlive { false };

};

/***************************************************************************************************************************************************************
 *
 **************************************************************************************************************************************************************/

#endif /* UartControlledDevice_h */
