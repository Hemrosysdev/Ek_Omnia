/*
 * UartWorkerTask.h
 *
 *  Created on: 19.11.2020
 *      Author: gesser
 */

#ifndef UartMcuWorkerTask_h
#define UartMcuWorkerTask_h

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#include "FreeRtosTask.h"

#include <freertos/queue.h>
#include <driver/uart.h>
#include <string>

class UcdMcuUartTask;

#define UART_RX_TX_BUFFER_SIZE  1024

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class UartMcuWorkerTask : public UxEspCppLibrary::FreeRtosTask
{

public:

    UartMcuWorkerTask( UcdMcuUartTask *    pUcdUartTask,
                       const uint32_t      u32StackSize,
                       const UBaseType_t   uTaskPriority,
                       const std::string & strTaskName );

    ~UartMcuWorkerTask() override;

    bool create( const uart_port_t        nUartNum,
                 const int                nTxPin,
                 const int                nRxPin,
                 const int                nBaudRate,
                 const uart_word_length_t nDataBits,
                 const uart_parity_t      nParity,
                 const uart_stop_bits_t   nStopBits );

private:

    void execute( void ) override;

private:

    UcdMcuUartTask * m_pUcdUartTask { nullptr };

    QueueHandle_t    m_hUartReceiveQueue { nullptr };

    uart_port_t      m_nUartNum { UART_NUM_MAX };

    int              m_nTxPin { 0 };

    int              m_nRxPin { 0 };

};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#endif /* UartMcuWorkerTask_h */
