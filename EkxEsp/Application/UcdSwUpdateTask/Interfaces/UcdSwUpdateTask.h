/*!*************************************************************************************************************************************************************
 *
 * @file UcdSwUpdateTask.h
 * @brief Source file of component unit SW Update.
 *
 * @author Ultratronik GmbH
 *         Dornierstr. 9
 *         D-82205 Gilching
 *         http://www.ultratronik.de
 *
 * @author Gerd Esser, Research & Development
 *
 * @date 27.08.2020
 *
 * @copyright Copyright 2020 by Ultratronik GmbH.
 *            All rights reserved.
 *            None of this file or parts of it may be
 *            copied, redistributed or used in any other way
 *            without written approval of Ultratronik GmbH.
 *
 **************************************************************************************************************************************************************/

#ifndef UcdSwUpdateTask_h
#define UcdSwUpdateTask_h

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#include "UartControlledDevice.h"

#include <esp_ota_ops.h>
#include <map>
#include <string>

#include "FreeRtosQueueTimer.h"
#include "EkxProtocol.h"

class UcdSwUpdateQueue;

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#define MAX_SW_VERSION_NO_LEN   10
#define MAX_DATA_SIZE  500

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class UcdSwUpdateTask : public UartControlledDevice
{

public:

    UcdSwUpdateTask( UxEspCppLibrary::EspApplication * pApplication );

    virtual ~UcdSwUpdateTask();

    void processTaskMessage( void );

    UcdSwUpdateQueue * swUpdateQueue( void );

    const UcdSwUpdateQueue * swUpdateQueue( void ) const;

    bool queueCtrlUartCommand( const EkxProtocol::DriverId                  u8DriverId,
                               const uint32_t                               u32MsgCounter,
                               const uint8_t                                u8RepeatCounter,
                               EkxProtocol::Payload::MessagePayload * const pMessagePayload ) override;

    bool queueAckNackReceived( const uint32_t u32MsgCounter,
                               const bool     bSuccess ) override;

private:

    UcdSwUpdateTask() = delete;

    bool startupStep( const FreeRtosQueueTask::startupStep_t nStartupStep ) override;

    void reportDriverStatus( const bool bFirst ) override;

    void processCtrlUartCommand( const EkxProtocol::DriverId                        u8DriverId,
                                 const uint32_t                                     u32MsgCounter,
                                 const uint8_t                                      u8RepeatCounter,
                                 const EkxProtocol::Payload::MessagePayload * const pMessagePayload ) override;

    void processTransaction( const uint32_t                                      u32MsgCounter,
                             const uint8_t                                       u8RepeatCounter,
                             const EkxProtocol::Payload::SwUpdateCommand * const pCommand );

    void processTimeoutAsyncTransaction();

    void systemRestart( void );

private:

    UxEspCppLibrary::FreeRtosQueueTimer m_timerAsyncTransaction;

    EkxProtocol::Payload::SwUpdateCmd   m_u8Command { EkxProtocol::Payload::SwUpdateCmd::Idle };

    uint32_t                            m_u32TransactionId { 0 };

    uint32_t                            m_u32LastChunkNo { 0 };

    uint32_t                            m_u32ChunkNo { 0 };

    bool                                m_bSuccess { false };

    esp_ota_handle_t                    m_pOtaHandle { 0 };

    uint32_t                            m_u32DataSize { 0 };

};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#endif /* UcdSwUpdateTask_h */
