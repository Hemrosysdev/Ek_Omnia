/*!*************************************************************************************************************************************************************
 *
 * @file UcdDiscreteInTask.h
 * @brief Source file of component unit UcdDiscreteInTask.
 *
 * This file was developed as part of UcdDiscreteInTask Type.
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

#ifndef UcdDiscreteInTask_h
#define UcdDiscreteInTask_h

/*!*************************************************************************************************************************************************************
 *
 **************************************************************************************************************************************************************/

#include "CtrlUartDispatcher.h"
#include "UartControlledDevice.h"
#include "UcdDiscreteInQueue.h"
#include "EspGpio.h"
#include "FreeRtosQueueTimer.h"
#include "EkxProtocol.h"

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class UcdDiscreteInTask
    : public UartControlledDevice
    , public UxEspCppLibrary::EspGpio
{
public:

    UcdDiscreteInTask( UxEspCppLibrary::EspApplication * pApplication,
                       const gpio_num_t                  nGpioNum,
                       const EkxProtocol::DriverId       u8DriverId,
                       const std::string &               strUcdName,
                       const std::string &               strTaskName,
                       const int                         nTaskStackSize,
                       const UBaseType_t                 uTaskPriority );

    ~UcdDiscreteInTask() override;

    bool startupStep( const FreeRtosQueueTask::startupStep_t nStartupStep ) override;

    void processTaskMessage( void ) override;

    bool queueCtrlUartCommand( const EkxProtocol::DriverId                  u8DriverId,
                               const uint32_t                               u32MsgCounter,
                               const uint8_t                                u8RepeatCounter,
                               EkxProtocol::Payload::MessagePayload * const pMessagePayload ) override;

    bool queueAckNackReceived( const uint32_t u32MsgCounter,
                               const bool     bSuccess ) override;

    UcdDiscreteInQueue * discreteInQueue( void );

    const UcdDiscreteInQueue * discreteInQueue( void ) const;

protected:

    virtual int debounceCounter( void ) const;

    virtual uint64_t samplingPeriodUs( void ) const;

private:

    UcdDiscreteInTask() = delete;

    void processDebounceTimeout();

    void reportDriverStatus( const bool bFirst ) override;

private:

    UxEspCppLibrary::FreeRtosQueueTimer m_timerDebounce;

    bool                                m_bDiscreteState = false;

    int                                 m_nDebounceCounter = 0;

    bool                                m_bLastValue = false;

};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#endif /* UcdDiscreteInTask_h */
