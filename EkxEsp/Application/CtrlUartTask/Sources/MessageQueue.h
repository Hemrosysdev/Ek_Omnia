/*
 * MessageQueue.h
 *
 *  Created on: 12.05.2023
 *      Author: gesser
 */

#ifndef MessageQueue_h
#define MessageQueue_h

#include "EspLog.h"

#include <vector>
#include <utility>
#include <chrono>
#include <mutex>

#include "EkxProtocol.h"

namespace CtrlUart
{
class CtrlUartTask;

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class MessageQueue : public UxEspCppLibrary::EspLog
{

private:

    struct MsgQueueInfo
    {
        std::chrono::steady_clock::time_point m_tLastSend;
        EkxProtocol::MessageFrame *           m_pFrame;
    };

    using MsgQueue = std::vector<MsgQueueInfo>;

    static const uint8_t  c_u8MaxRepeatCounter = 10;
    static const uint32_t c_u32MaxRepeatTimeUs = 50000;

public:

    explicit MessageQueue( CtrlUartTask * const pCtrlUartTask );
    ~MessageQueue() override;

    void enqueuMessage( const EkxProtocol::DriverId                  u8DriverId,
                        const uint32_t                               u32MsgCounter,
                        EkxProtocol::Payload::MessagePayload * const pMessagePayload );

    void processAck( const uint32_t u32MsgCounter );

    void processNack( const uint32_t                               u32MsgCounter,
                      const EkxProtocol::Payload::Nack::NackReason u8NackReason );

    void timeoutRun();

private:

    MessageQueue() = delete;

    void sendFrame( EkxProtocol::MessageFrame * const pFrame );

private:

    CtrlUartTask * m_pCtrlUartTask { nullptr };

    MsgQueue       m_theMsgQueue;

    std::mutex     m_mutexMsgQueue;
};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

} // namespace CtrlUart

#endif /* MessageQueue_h */
