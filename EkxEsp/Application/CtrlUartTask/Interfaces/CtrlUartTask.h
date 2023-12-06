/*!*****************************************************************************
*
* @file CtrlUartTask.h
*
* @brief Header file for class CtrlUartTask.
*
* @author Ultratronik GmbH
*         Dornierstr. 9
*         D-82205 Gilching
*         www.ultratronik.de
*
* @author Gerd Esser, Forschung & Entwicklung
*
* @created 27.05.2021
*
* @copyright Copyright (C) Bircher Smart Access, CH-8222 Beringen, 2021
*            All rights reserved.
*            None of this file or parts of it may be
*            copied, redistributed or used in any other way
*            without written approval of Bircher BSA.
*
* Information from SVN:
* revision of last commit: $Rev:  $
* date of last commit: $Date: $
* author of last commit: $Author: $
*
*******************************************************************************/

#ifndef CtrlUartTask_h
#define CtrlUartTask_h

/*##***********************************************************************************************************************************************************
 *  Includes
 *************************************************************************************************************************************************************/

#include "CtrlUartQueue.h"
#include "FreeRtosQueueTask.h"
#include "FreeRtosQueueTimer.h"

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

namespace EkxProtocol
{
class MessageFrame;
}

namespace UxEspCppLibrary
{
class EspApplication;
};

namespace CtrlUart
{
class CtrlUartDispatcher;
class MessageQueue;
class SystemObserver;

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class CtrlUartDrvTask;

/*!************************************************************************************************************************************************************
 * @brief This class controls the incoming and outgoing UART frames on the CPS-uart bus.
 *        It controls how to handle incoming CPS UART frames and organizes converting and redirection of them.
 *        For outgoing CPS-UART-frames it triggers conversion from native CPS objects.
 *        Internally it depends on an underlying CtrlUartDrvTask as well as HighSpeedToUart and UartToHighSpeed converter classes.
 *************************************************************************************************************************************************************/

class CtrlUartTask : public UxEspCppLibrary::FreeRtosQueueTask
{

private:

    static const uint64_t c_u64MaxUint16Size             { 0xffffUL };
    static const uint64_t c_u64TimeoutMessageResponseUs  { 500000UL };           //< 500ms

public:

    explicit CtrlUartTask( UxEspCppLibrary::EspApplication * const pApplication );

    ~CtrlUartTask() override;

    bool startupStep( const UxEspCppLibrary::FreeRtosQueueTask::startupStep_t nStartupStep ) override;

    void processTaskMessage( void ) override;

    CtrlUartQueue * ctrlUartQueue( void );

    const CtrlUartQueue * ctrlUartQueue( void ) const;

    void processReceivedUartFrame( const EkxProtocol::Deserializer::ParseResult nresult,
                                   EkxProtocol::MessageFrame &                  frame );

    void processReplyMessage( const EkxProtocol::DriverId                  u8DriverId,
                              const uint32_t                               u32MsgCounter,
                              EkxProtocol::Payload::MessagePayload * const pMessagePayload );

    CtrlUartDrvTask * uartDrvTask();

    CtrlUartDispatcher * dispatcher();

    MessageQueue * messageQueue();

    SystemObserver * systemObserver();

    uint32_t incSystemMessageCounter();

private:

    CtrlUartTask() = default;

private:

    CtrlUartDrvTask *                   m_pUartDrvTask { nullptr };

    CtrlUartDispatcher *                m_pDispatcher { nullptr };

    MessageQueue *                      m_pMessageQueue { nullptr };

    UxEspCppLibrary::FreeRtosQueueTimer m_timerMessageQueue;

    UxEspCppLibrary::FreeRtosQueueTimer m_timerStatusUpdate;

    SystemObserver *                    m_pSystemObserver { nullptr };

    uint32_t                            m_u32SystemMessageCounter { 0 };

};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

} /* namespace CtrlUart */

#endif /* CtrlUartTask_h */
