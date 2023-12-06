///////////////////////////////////////////////////////////////////////////////
///
/// @file NtcTempDriver.h
///
/// @brief Header file of class NtcTempDriver.
///
/// @author Ultratronik GmbH
///         Dornierstr. 9
///         D-82205 Gilching
///         Germany
///         http://www.ultratronik.de
///
/// @author written by Florian Sonntag, Forschung & Entwicklung, gesser@ultratronik.de
///
/// @date 01.02.2021
///
/// @copyright Copyright 2021 by Hemro International AG
///            Hemro International AG
///            Länggenstrasse 34
///            CH 8184 Bachenbülach
///            Switzerland
///            Homepage: www.hemrogroup.com
///
///////////////////////////////////////////////////////////////////////////////

#ifndef NtcTempDriver_h
#define NtcTempDriver_h

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#include "EspDeviceDriver.h"

class MainStatemachine;

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

class NtcTempDriver : public EspDeviceDriver
{
    Q_OBJECT

    Q_PROPERTY( int overtempWarningActive READ overtempWarningActive NOTIFY overtempWarningActiveChanged )
    Q_PROPERTY( int currentTemp READ currentTemp NOTIFY currentTempChanged )
    Q_PROPERTY( bool currentTempValid READ currentTempValid NOTIFY currentTempValidChanged )

public:

    explicit NtcTempDriver( const EkxProtocol::DriverId u8DriverId );

    virtual ~NtcTempDriver() override;

    void create( MainStatemachine * pMainStatemachine );

    int currentTemp() const;

    bool currentTempValid() const;

    int adcToTemperature( const quint32 u32NtcValue ) const;

    bool overtempWarningActive() const;

    void setOvertempWarningActive( const bool bActive );

    void processMessageFrame( const EkxProtocol::MessageFrame & frame ) override;

    void processAlive( const bool bAlive ) override;

    void updateOvertempWarning();

signals:

    void currentTempChanged();

    void currentTempValidChanged();

    void overtempWarningActiveChanged();

private:

    void setCurrentTemp( const int nTemp );

    void setCurrentTempValid( const bool bTempValid );

private:

    MainStatemachine * m_pMainStatemachine { nullptr };
    int                m_nCurrentTemp { 0 };
    bool               m_bCurrentTempValid { true };
    bool               m_bOvertempWarningActive { false };

};

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#endif // NtcTempDriver_h
