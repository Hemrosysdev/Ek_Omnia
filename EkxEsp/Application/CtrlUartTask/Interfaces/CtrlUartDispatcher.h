/*!*************************************************************************************************************************************************************
 *
 * @file SocketDispatcherTask.h
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

#ifndef SocketDispatcherTask_h
#define SocketDispatcherTask_h

/*!*************************************************************************************************************************************************************
 *
 **************************************************************************************************************************************************************/

#include "EspLog.h"

#include <string>
#include <map>

#include "EkxProtocol.h"
#include "FreeRtosQueueTimer.h"

class UartControlledDevice;

namespace CtrlUart
{
class CtrlUartTask;

/*!*************************************************************************************************************************************************************
 *
 **************************************************************************************************************************************************************/

class CtrlUartDispatcher : public UxEspCppLibrary::EspLog
{

private:

    using deviceMap =std::map<EkxProtocol::DriverId, UartControlledDevice *>;
    using devicePair=std::pair<EkxProtocol::DriverId, UartControlledDevice *>;

    static const int c_nAliveTimeoutUs = 1600000;

public:

    CtrlUartDispatcher( CtrlUartTask * pCtrlUartTask );

    virtual ~CtrlUartDispatcher();

    void registerDriver( UartControlledDevice * const pDevice );

    void unregisterDriver( const UartControlledDevice * const pDevice );

    UartControlledDevice * device( const EkxProtocol::DriverId u8DriverId );

    void dispatchCtrlUartCommand( const EkxProtocol::Deserializer::ParseResult nResult,
                                  EkxProtocol::MessageFrame &                  frame );

    void dispatchMessageProcessed( const EkxProtocol::DriverId u8DriverId,
                                   const uint32_t              u32MsgCounter,
                                   const bool                  bSuccess );

    void setAlive( const bool bAlive );

    bool isAlive() const;

private:

    void processBroadcastCommand( const uint32_t                                     u32MsgCounter,
                                  const uint8_t                                      u8RepeatCounter,
                                  const EkxProtocol::Payload::MessagePayload * const pMessagePayload );

    void processSystemCommand( const EkxProtocol::DriverId                        u8DriverId,
                               const uint32_t                                     u32MsgCounter,
                               const uint8_t                                      u8RepeatCounter,
                               const EkxProtocol::Payload::MessagePayload * const pMessagePayload );

    void dispatchAlive( const bool bAlive );

    void dispatchRequestDriverStatus( void );

    void replyPong( const uint32_t u32MsgCounter,
                    const uint8_t  u8RepeatCounter );

    void replyAck( const EkxProtocol::DriverId u8DriverId,
                   const uint32_t              u32MsgCounter,
                   const uint8_t               u8RepeatCounter );

    void replyNack( const EkxProtocol::DriverId                  u8DriverId,
                    const uint32_t                               u32MsgCounter,
                    const uint8_t                                u8RepeatCounter,
                    const EkxProtocol::Payload::Nack::NackReason u8NackReason );

private:

    CtrlUartTask *                      m_pCtrlUartTask = nullptr;

    deviceMap                           m_theDevices;

    bool                                m_bAlive = false;

    UxEspCppLibrary::FreeRtosQueueTimer m_timerAlive;

};

/*!*************************************************************************************************************************************************************
 *
 **************************************************************************************************************************************************************/

} // namespace CtrlUart

#endif /* SocketDispatcherTask_h */
