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

#ifndef EspMcuDriver_h
#define EspMcuDriver_h

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#include "EspDeviceDriver.h"
#include "EkxProtocol.h"

class McuDriver;
class MainStatemachine;

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

class EspMcuDriver : public EspDeviceDriver
{
    Q_OBJECT

public:

    EspMcuDriver() = delete;
    EspMcuDriver( const EkxProtocol::DriverId u8DriverId );
    ~EspMcuDriver() override;

    void create( MainStatemachine * pMainStatemachine );

    McuDriver * mcuDriver();

    void processMcuConfigChanged();

    void processMessageFrame( const EkxProtocol::MessageFrame & frame ) override;

    void processAlive( const bool bAlive ) override;

    void processMcuStreamData( const QByteArray & array );

private:

    void setMcuUartInValid( const bool bMcuUartInValid );

private:

    MainStatemachine * m_pMainStatemachine { nullptr };

    McuDriver *        m_pMcuDriver { nullptr };

};

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#endif // EspMcuDriver_h
