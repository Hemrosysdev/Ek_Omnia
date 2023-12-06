/*
 * SystemObserver.cpp
 *
 *  Created on: 12.06.2023
 *      Author: gesser
 */

#include "SystemObserver.h"
#include "CtrlUartTask.h"
#include "MessageQueue.h"

namespace CtrlUart
{

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

SystemObserver::SystemObserver( CtrlUart::CtrlUartTask * pCtrlUartTask )
                : m_pCtrlUartTask( pCtrlUartTask )
{
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

SystemObserver::~SystemObserver()
{
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void SystemObserver::incMessageNumTx()
{
    m_u32MessageNumTx++;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void SystemObserver::incMessageNumRx()
{
    m_u32MessageNumRx++;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void SystemObserver::incMessageNumRxErrors()
{
    m_u32MessageNumRxErrors++;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void SystemObserver::incMessageNumTxTimeouts()
{
    m_u32MessageNumTxTimeouts++;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void SystemObserver::detectStartHeapSize()
{
    m_u32StartFreeHeapSize = esp_get_free_heap_size();
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void SystemObserver::statusUpdate()
{
    uint32_t u32FreeHeapSize = esp_get_free_heap_size();

    double dFreeHeapUsage = 100.0 - ( static_cast<double>( u32FreeHeapSize ) / static_cast<double>( m_u32StartFreeHeapSize ) * 100.0 );

    EkxProtocol::Payload::SystemStatus::Status u8SystemStatus = EkxProtocol::Payload::SystemStatus::Status::Normal;
    if ( dFreeHeapUsage > c_dFreeHeapUsageError )
    {
        u8SystemStatus = EkxProtocol::Payload::SystemStatus::Status::HeapErrorReboot;
    }
    else if ( dFreeHeapUsage > c_dFreeHeapUsageWarning )
    {
        u8SystemStatus = EkxProtocol::Payload::SystemStatus::Status::HeapWarning;
    }

    EkxProtocol::Payload::SystemStatus * pStatusPayload = new EkxProtocol::Payload::SystemStatus();

    pStatusPayload->status().setValue( u8SystemStatus );
    pStatusPayload->freeHeapSize().setValue( u32FreeHeapSize );
    pStatusPayload->messageNumTx().setValue( m_u32MessageNumTx );
    pStatusPayload->messageNumRx().setValue( m_u32MessageNumRx );
    pStatusPayload->messageNumRxErrors().setValue( m_u32MessageNumRxErrors );
    pStatusPayload->messageNumTxTimeouts().setValue( m_u32MessageNumTxTimeouts );

    m_pCtrlUartTask->messageQueue()->enqueuMessage( EkxProtocol::DriverId::SystemDriver, m_pCtrlUartTask->incSystemMessageCounter(), pStatusPayload );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

} /* namespace CtrlUart */
