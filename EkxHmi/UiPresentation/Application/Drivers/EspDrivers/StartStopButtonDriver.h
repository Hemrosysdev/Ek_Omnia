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

#include <QObject>

class EspDataInterface;
class EspDiscreteIn;

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

class StartStopButtonDriver : public QObject
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

    explicit StartStopButtonDriver( QObject *parent = nullptr );
    virtual ~StartStopButtonDriver( ) override;

    void connectInterface( EspDiscreteIn* pEspStartStopButton );
    void disconnectInterface();

    ButtonState buttonState() const;
    bool isValid() const;

    void setPressed( const bool bPressed );
    void setValid( const bool bValid );
    bool isPressed( void ) const;

signals:

    void buttonStateChanged( const ButtonState nButtonState  );

public slots:

    void processDataChanged( EspDataInterface * pData );
    void processValidChanged( EspDataInterface * pData );

private:

    EspDiscreteIn*  m_pEspStartStopButton { nullptr };
    ButtonState     m_nButtonState { Invalid };
    bool            m_bPressed { true };
    bool            m_bValid { false };

};

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#endif // StartStopButtonDriver_h
