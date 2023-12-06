///////////////////////////////////////////////////////////////////////////////
///
/// @file EspMcuDriver.h
///
/// @brief Header file of class EspMcuDriver.
///
/// @author Ultratronik GmbH
///         Dornierstr. 9
///         D-82205 Gilching
///         Germany
///         http://www.ultratronik.de
///
/// @author written by Florian Sonntag, Forschung & Entwicklung, gesser@ultratronik.de
///
/// @date 31.03.2021
///
/// @copyright Copyright 2021 by Hemro International AG
///            Hemro International AG
///            Länggenstrasse 34
///            CH 8184 Bachenbülach
///            Switzerland
///            Homepage: www.hemrogroup.com
///
///////////////////////////////////////////////////////////////////////////////

#ifndef EspSystemDriver_h
#define EspSystemDriver_h

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#include "EspDeviceDriver.h"
#include "EkxProtocol.h"

class MainStatemachine;

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

class EspSystemDriver : public EspDeviceDriver
{
    Q_OBJECT

public:

    EspSystemDriver() = delete;
    explicit EspSystemDriver( const EkxProtocol::DriverId u8DriverId );

    ~EspSystemDriver() override;

    void create(MainStatemachine * const pMainStatemachine );

    EkxProtocol::Payload::SystemStatus::Status status() const;

    quint32 u32MessageNumTx() const;

    quint32 u32MessageNumRx() const;

    quint32 u32MessageNumRxErrors() const;

    quint32 u32MessageNumTxTimeouts() const;

    quint32 freeHeapSize() const;

private:

    void processMessageFrame( const EkxProtocol::MessageFrame & frame ) override;

    void setStatus(EkxProtocol::Payload::SystemStatus::Status u8Status);
    void setMessageNumTx( const quint32 u32MessageNumTx);
    void setMessageNumRx( const quint32 u32MessageNumRx);
    void setMessageNumRxErrors( const quint32 u32MessageNumRxErrors);
    void setMessageNumTxTimeouts( const quint32 u32MessageNumTxTimeouts );
    void setFreeHeapSize( const quint32 u32FreeHeapSize );

private:

    MainStatemachine * m_pMainStatemachine { nullptr };

    EkxProtocol::Payload::SystemStatus::Status  m_u8Status { EkxProtocol::Payload::SystemStatus::Status::Normal };
    quint32  m_u32MessageNumTx { 0 };
    quint32  m_u32MessageNumRx { 0 };
    quint32  m_u32MessageNumRxErrors { 0 };
    quint32  m_u32MessageNumTxTimeouts { 0 };
    quint32  m_u32FreeHeapSize { 0 };

};

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#endif // EspSystemDriver_h
