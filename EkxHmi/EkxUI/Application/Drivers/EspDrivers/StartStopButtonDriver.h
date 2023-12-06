///////////////////////////////////////////////////////////////////////////////
///
/// @file StartStopButtonDriver.h
///
/// @brief Header file of class StartStopButtonDriver.
///
/// @author Ultratronik GmbH
///         Dornierstr. 9
///         D-82205 Gilching
///         Germany
///         http://www.ultratronik.de
///
/// @author written by Gerd Esser, Forschung & Entwicklung, gesser@ultratronik.de
///
/// @date 10.02.2021
///
/// @copyright Copyright 2021 by Hemro International AG
///            Hemro International AG
///            Länggenstrasse 34
///            CH 8184 Bachenbülach
///            Switzerland
///            Homepage: www.hemrogroup.com
///
///////////////////////////////////////////////////////////////////////////////

#ifndef StartStopButtonDriver_h
#define StartStopButtonDriver_h

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#include "EspDeviceDriver.h"
#include "EkxProtocol.h"

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

class StartStopButtonDriver : public EspDeviceDriver
{
    Q_OBJECT

public:

    enum ButtonState
    {
        Invalid = 0,
        Released,
        Pressed
    };

public:

    explicit StartStopButtonDriver( const EkxProtocol::DriverId u8DriverId );
    virtual ~StartStopButtonDriver() override;

    void setButtonState( const ButtonState nButtonState );

    ButtonState buttonState() const;

    void setValid( const bool bValid );

    bool isValid() const;

    void setPressed( const bool bPressed );

    bool isPressed( void ) const;

    void processMessageFrame( const EkxProtocol::MessageFrame & frame ) override;

    void processAlive( const bool bAlive ) override;

signals:

    void buttonStateChanged( const ButtonState nButtonState );

private:

    ButtonState m_nButtonState { Invalid };
    bool        m_bPressed { true };
    bool        m_bValid { false };

};

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#endif // StartStopButtonDriver_h
