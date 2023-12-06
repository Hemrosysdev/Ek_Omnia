/*!*************************************************************************************************************************************************************
 *
 * @file UcdUartTask.h
 * @brief Source file of component unit UcdUartTask
 *
 * This file was developed as part of svcUcdUart.
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

#ifndef UcdUartTask_h
#define UcdUartTask_h

/*!*************************************************************************************************************************************************************
 *
 **************************************************************************************************************************************************************/

#include "UartControlledDevice.h"

#include <string>
#include <driver/uart.h>

#include "EkxProtocol.h"
#include "UcdMcuUartQueue.h"

class UartMcuWorkerTask;

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class UcdMcuUartTask : public UartControlledDevice
{
public:

    UcdMcuUartTask( UxEspCppLibrary::EspApplication * pApplication,
                    const uart_port_t                 nUcdUartNum,
                    const int                         nTxPin,
                    const int                         nRxPin,
                    const EkxProtocol::DriverId       u8DriverId,
                    const std::string &               strUcdName,
                    const std::string &               strTaskName,
                    const int                         nTaskStackSize,
                    const UBaseType_t                 uTaskPriority );

    virtual ~UcdMcuUartTask() override;

    bool startupStep( const FreeRtosQueueTask::startupStep_t nStartupStep ) override;

    void processTaskMessage( void );

    UcdMcuUartQueue * mcuUartQueue( void );

    const UcdMcuUartQueue * uartQueue( void ) const;

    bool queueCtrlUartCommand( const EkxProtocol::DriverId                  u8DriverId,
                               const uint32_t                               u32MsgCounter,
                               const uint8_t                                u8RepeatCounter,
                               EkxProtocol::Payload::MessagePayload * const pMessagePayload ) override;

    bool queueAckNackReceived( const uint32_t u32MsgCounter,
                               const bool     bSuccess ) override;

    void sendReplyToCtrlUart( const uint8_t * const pData,
                              const uint32_t        u32Size );

private:

    UcdMcuUartTask() = delete;

    void initUcdUart( const int                baud_rate,
                      const uart_word_length_t data_bits,
                      const uart_parity_t      parity,
                      const uart_stop_bits_t   stop_bits );

    void writeMcuData( const std::string & strData );

    void processCtrlUartCommand( const EkxProtocol::DriverId                        u8DriverId,
                                 const uint32_t                                     u32MsgCounter,
                                 const uint8_t                                      u8RepeatCounter,
                                 const EkxProtocol::Payload::MessagePayload * const pMessagePayload ) override;

private:

    uart_port_t         m_nUcdUartNum { -1 };

    int                 m_nTxPin { -1 };

    int                 m_nRxPin { -1 };

    uint8_t             m_u8RxBuffer[256];

    UartMcuWorkerTask * m_pUartWorker { nullptr };

};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#endif /* UcdUartTask_h */
