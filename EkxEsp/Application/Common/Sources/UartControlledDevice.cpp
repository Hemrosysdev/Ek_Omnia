/*!*************************************************************************************************************************************************************
 *
 * @file SocketControlledDevice.cpp
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

#include "UartControlledDevice.h"

#include <cstring>

#include "FreeRtosQueueTask.h"
#include "FreeRtosQueueTimer.h"
#include "EkxApplication.h"

/***************************************************************************************************************************************************************
 *
 **************************************************************************************************************************************************************/

UartControlledDevice::UartControlledDevice( const std::string &               strUcdName,
                                            const std::string &               strType,
                                            const EkxProtocol::DriverId       u8DriverId,
                                            UxEspCppLibrary::EspApplication * pApplication,
                                            const uint32_t                    u32StackSize,
                                            const UBaseType_t                 uTaskPriority,
                                            const std::string &               strTaskName,
                                            UxEspCppLibrary::FreeRtosQueue *  pQueue,
                                            const TickType_t                  xReceiveTicksToWait )
    : UxEspCppLibrary::FreeRtosQueueTask( pApplication,
                                          u32StackSize,
                                          uTaskPriority,
                                          strTaskName,
                                          pQueue,
                                          xReceiveTicksToWait )
    , m_strName( strUcdName )
    , m_strType( strType )
    , m_u8DriverId( u8DriverId )
{
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

UartControlledDevice::~UartControlledDevice()
{
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

const std::string & UartControlledDevice::name( void ) const
{
    return m_strName;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

const std::string & UartControlledDevice::type( void ) const
{
    return m_strType;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

EkxProtocol::DriverId UartControlledDevice::driverId() const
{
    return m_u8DriverId;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

uint32_t UartControlledDevice::sendCtrlUartReply( EkxProtocol::Payload::MessagePayload * const pMessagePayload,
                                                  const bool                                   bIncMsgCounter )
{
    uint32_t u32MsgCounter = 0;

    if ( pMessagePayload )
    {
        if ( bIncMsgCounter )
        {
            incMsgCounter();
            //resetMsgRepeatCounter();
        }

        u32MsgCounter = msgCounter();

        sendPayloadToCtrlUart( driverId(),
                               msgCounter(),
                               0,
                               pMessagePayload );
    }

    return u32MsgCounter;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void UartControlledDevice::sendPayloadToCtrlUart( const EkxProtocol::DriverId                  u8DriverId,
                                                  const uint32_t                               u32MsgCounter,
                                                  const uint8_t                                u8RepeatCounter,
                                                  EkxProtocol::Payload::MessagePayload * const pMessagePayload )
{
    ekxApp().ctrlUartTask().ctrlUartQueue()->sendMessagePayload( u8DriverId, u32MsgCounter, u8RepeatCounter, pMessagePayload );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

bool UartControlledDevice::queueAlive( const bool bAlive )
{
    // standard implementation, nothing to be done; special stuff done in overloaded classes
    return false;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void UartControlledDevice::reportDriverStatus( const bool bFirst )
{
    vlogWarning( "reportDriverStatus: not implemented" );

    (void) bFirst;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

uint32_t UartControlledDevice::msgCounter() const
{
    return m_u32MsgCounter;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void UartControlledDevice::incMsgCounter()
{
    m_u32MsgCounter++;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void UartControlledDevice::sendCtrlUartReplyAck( const uint32_t u32AckCounter,
                                                 const uint8_t  u8RepeatCounter )
{
    EkxProtocol::Payload::Ack * pAck = new EkxProtocol::Payload::Ack();

    sendPayloadToCtrlUart( driverId(), u32AckCounter, u8RepeatCounter, pAck );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void UartControlledDevice::sendCtrlUartReplyNack( const uint32_t                               u32AckCounter,
                                                  const uint8_t                                u8RepeatCounter,
                                                  const EkxProtocol::Payload::Nack::NackReason u8NackReason )
{
    EkxProtocol::Payload::Nack * pNack = new EkxProtocol::Payload::Nack();
    pNack->nackReason().setValue( u8NackReason );

    sendPayloadToCtrlUart( driverId(), u32AckCounter, u8RepeatCounter, pNack );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void UartControlledDevice::processBaseCtrlUartCommand( const EkxProtocol::DriverId                  u8DriverId,
                                                       const uint32_t                               u32MsgCounter,
                                                       const uint8_t                                u8RepeatCounter,
                                                       EkxProtocol::Payload::MessagePayload * const pMessagePayload )
{
    if ( pMessagePayload )
    {
        switch ( pMessagePayload->payloadType() )
        {
            case EkxProtocol::Payload::PayloadType::Ack:
            case EkxProtocol::Payload::PayloadType::Nack:
                // already handled in CtrlUart::MessageQueue
                break;

            case EkxProtocol::Payload::PayloadType::RequestDriverStatus:
            {
                if ( u8DriverId != EkxProtocol::DriverId::Broadcast )
                {
                    sendCtrlUartReplyAck( u32MsgCounter, u8RepeatCounter );
                }
                reportDriverStatus( true );
            }
            break;

            default:
            {
                processCtrlUartCommand( u8DriverId, u32MsgCounter, u8RepeatCounter, pMessagePayload );
            }
            break;
        }
    }

    delete pMessagePayload;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void UartControlledDevice::queueRequestDriverStatus()
{
    EkxProtocol::Payload::RequestDriverStatus * pPayload = new EkxProtocol::Payload::RequestDriverStatus();
    queueCtrlUartCommand( EkxProtocol::DriverId::Broadcast, 0, 0, pPayload );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void UartControlledDevice::processCtrlUartCommand( const EkxProtocol::DriverId                        u8DriverId,
                                                   const uint32_t                                     u32MsgCounter,
                                                   const uint8_t                                      u8RepeatCounter,
                                                   const EkxProtocol::Payload::MessagePayload * const pMessagePayload )
{
    ( void ) u32MsgCounter;
    ( void ) u8RepeatCounter;
    ( void ) pMessagePayload;

    // do nothing is standard behavior
    vlogError( "processCtrlUartCommand not implemented for driver ID %d/%s",
               driverId(),
               name().c_str() );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void UartControlledDevice::registerDriver()
{
    ekxApp().ctrlUartTask().ctrlUartQueue()->sendRegisterDriver( this );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void UartControlledDevice::processBaseAlive( const bool bAlive )
{
    m_bAlive = bAlive;

    processAlive( bAlive );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void UartControlledDevice::processAlive( const bool bAlive )
{
    // all stuf done in overloaded method
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

bool UartControlledDevice::isAlive() const
{
    return m_bAlive;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void UartControlledDevice::processAckNackReceived( const uint32_t u32MsgCounter,
                                                   const bool     bSuccess )
{
    ( void ) u32MsgCounter;
    ( void ) bSuccess;
    // do nothing resp. all stuff to be done in overloaded method
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/
