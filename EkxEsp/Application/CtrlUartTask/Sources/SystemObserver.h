/*
 * SystemObserver.h
 *
 *  Created on: 12.06.2023
 *      Author: gesser
 */

#ifndef SystemObserver_h
#define SystemObserver_h

#include "EspLog.h"
#include "EkxProtocol.h"

namespace CtrlUart
{
class CtrlUartTask;

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class SystemObserver : public UxEspCppLibrary::EspLog
{

private:

    static constexpr double c_dFreeHeapUsageError = 98.0;
    static constexpr double c_dFreeHeapUsageWarning = 95.0;

public:

    SystemObserver() = delete;
    SystemObserver( CtrlUart::CtrlUartTask * pCtrlUartTask );
    ~SystemObserver() override;

    void detectStartHeapSize();

    void incMessageNumTx();
    void incMessageNumRx();
    void incMessageNumRxErrors();
    void incMessageNumTxTimeouts();

    void statusUpdate();

private:

    CtrlUart::CtrlUartTask * m_pCtrlUartTask { nullptr };

    EkxProtocol::Payload::SystemStatus::Status  m_status { EkxProtocol::Payload::SystemStatus::Status::Normal };
    uint32_t  m_u32MessageNumTx { 0 };
    uint32_t  m_u32MessageNumRx { 0 };
    uint32_t  m_u32MessageNumRxErrors { 0 };
    uint32_t  m_u32MessageNumTxTimeouts { 0 };
    uint32_t  m_u32StartFreeHeapSize { 0 };

};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

} // namespace CtrlUart

#endif /* SystemObserver_h */
