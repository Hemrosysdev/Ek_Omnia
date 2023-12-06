/*!*************************************************************************************************************************************************************
 *
 * @file UcdDiscreteInTask.cpp
 * @brief Source file of component unit UcdDiscreteInTask.
 *
 * This file was developed as part of UcdDiscreteInTask
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

/*!*************************************************************************************************************************************************************
 *
 **************************************************************************************************************************************************************/

#include "UcdDiscreteInTask.h"

#include <string>
#include "UartControlledDevice.h"

#include "UcdDiscreteInQueue.h"
#include "UcdDiscreteInQueueTypes.h"
#include "EspLog.h"
#include "FreeRtosQueueTask.h"
#include "EkxProtocol.h"

/*!*************************************************************************************************************************************************************
 *
 **************************************************************************************************************************************************************/

UcdDiscreteInTask::UcdDiscreteInTask( UxEspCppLibrary::EspApplication * pApplication,
                                      const gpio_num_t                  nGpioNum,
                                      const EkxProtocol::DriverId       u8DriverId,
                                      const std::string &               strUcdName,
                                      const std::string &               strTaskName,
                                      const int                         nTaskStackSize,
                                      const UBaseType_t                 uTaskPriority )
    : UartControlledDevice( strUcdName,
                            "Discrete",
                            u8DriverId,
                            pApplication,
                            nTaskStackSize,
                            uTaskPriority,
                            strTaskName,
                            new UcdDiscreteInQueue() )
    , UxEspCppLibrary::EspGpio( nGpioNum,
                                GPIO_MODE_INPUT,
                                GPIO_PULLUP_DISABLE,
                                GPIO_PULLDOWN_DISABLE,
                                GPIO_INTR_DISABLE,
                                strTaskName )
    , m_timerDebounce( taskQueue(),
                       static_cast<uint32_t>( UcdDiscreteInQueueTypes::messageType::DebounceTimeout ) )
{
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

UcdDiscreteInTask::~UcdDiscreteInTask()
{

}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

bool UcdDiscreteInTask::startupStep( const FreeRtosQueueTask::startupStep_t nStartupStep )
{
    bool bContinueStartup = true;

    switch ( nStartupStep )
    {
        case UxEspCppLibrary::FreeRtosQueueTask::startupStep_t::startupStep1:
        {
            registerDriver();

            m_timerDebounce.startPeriodic( samplingPeriodUs() );
        }
        break;

        default:
        {
            // do nothing
        }
        break;
    }

    return bContinueStartup;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void UcdDiscreteInTask::processTaskMessage( void )
{
    const UcdDiscreteInQueueTypes::message * pMessage = reinterpret_cast<const UcdDiscreteInQueueTypes::message *>( receiveMsg() );

    if ( pMessage )
    {
        switch ( pMessage->u32Type )
        {
            case UcdDiscreteInQueueTypes::messageType::CtrlUartCommand:
            {
                processBaseCtrlUartCommand( pMessage->payload.ctrlUartCommand.m_u8DriverId,
                                            pMessage->payload.ctrlUartCommand.m_u32MsgCounter,
                                            pMessage->payload.ctrlUartCommand.m_u8RepeatCounter,
                                            pMessage->payload.ctrlUartCommand.m_pMessagePayload );
            }
            break;

            case UcdDiscreteInQueueTypes::messageType::AckNackReceived:
            {
                processAckNackReceived( pMessage->payload.ackNackReceived.m_u32MsgCounter,
                                        pMessage->payload.ackNackReceived.m_bSuccess );
            }
            break;

            case UcdDiscreteInQueueTypes::messageType::DebounceTimeout:
            {
                processDebounceTimeout();
            }
            break;

            default:
            {
                UartControlledDevice::vlogError( "processTaskMessage: receive illegal message type %d",
                                                 pMessage->u32Type );
            }
            break;
        }
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

UcdDiscreteInQueue * UcdDiscreteInTask::discreteInQueue( void )
{
    return dynamic_cast<UcdDiscreteInQueue *>( taskQueue() );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

const UcdDiscreteInQueue * UcdDiscreteInTask::discreteInQueue( void ) const
{
    return dynamic_cast<const UcdDiscreteInQueue *>( taskQueue() );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void UcdDiscreteInTask::processDebounceTimeout()
{
    bool bValue = isHigh();

    if ( m_bLastValue == bValue )
    {
        m_nDebounceCounter++;
    }
    else
    {
        m_nDebounceCounter = 0;
    }

    m_bLastValue = bValue;

    if ( m_nDebounceCounter >= debounceCounter() )
    {
        m_nDebounceCounter = debounceCounter();

        if ( bValue != m_bDiscreteState )
        {
            m_bDiscreteState = bValue;
            reportDriverStatus( true );
        }
    }
}

/*************************************************************************************************************************************************************
*
*************************************************************************************************************************************************************/

bool UcdDiscreteInTask::queueAckNackReceived( const uint32_t u32MsgCounter,
                                              const bool     bSuccess )
{
    return discreteInQueue()->sendAckNackReceived( u32MsgCounter, bSuccess ) == pdPASS;
}

/*************************************************************************************************************************************************************
*
*************************************************************************************************************************************************************/

void UcdDiscreteInTask::reportDriverStatus( const bool bFirst )
{
    EkxProtocol::Payload::DiscreteStatus * pPayload = new EkxProtocol::Payload::DiscreteStatus();
    pPayload->valid().setValue( true );
    pPayload->discreteHigh().setValue( m_bDiscreteState );

    sendCtrlUartReply( pPayload, bFirst );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

int UcdDiscreteInTask::debounceCounter( void ) const
{
    return 3;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

uint64_t UcdDiscreteInTask::samplingPeriodUs( void ) const
{
    return 20000U;  // microseconds
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

bool UcdDiscreteInTask::queueCtrlUartCommand( const EkxProtocol::DriverId                  u8DriverId,
                                              const uint32_t                               u32MsgCounter,
                                              const uint8_t                                u8RepeatCounter,
                                              EkxProtocol::Payload::MessagePayload * const pMessagePayload )
{
    return discreteInQueue()->sendCtrlUartCommand( u8DriverId, u32MsgCounter, u8RepeatCounter, pMessagePayload );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/
