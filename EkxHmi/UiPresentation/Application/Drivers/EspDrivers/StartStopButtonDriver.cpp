#include "StartStopButtonDriver.h"

#include <QDebug>

#include "EkxGlobals.h"
#include "EspDiscreteIn.h"

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

StartStopButtonDriver::StartStopButtonDriver( QObject *parent )
    : QObject( parent )
{
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

StartStopButtonDriver::~StartStopButtonDriver()
{
    if ( m_pEspStartStopButton )
    {
        setPressed( false );
        setValid( false );

        disconnect( m_pEspStartStopButton, &EspDataInterface::dataChanged, this, &StartStopButtonDriver::processDataChanged );
        disconnect( m_pEspStartStopButton, &EspDataInterface::validChanged, this, &StartStopButtonDriver::processValidChanged );

        m_pEspStartStopButton = nullptr;
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void StartStopButtonDriver::connectInterface( EspDiscreteIn * pEspStartStopButton )
{
    if ( pEspStartStopButton != m_pEspStartStopButton )
    {
        disconnectInterface();
        m_pEspStartStopButton = pEspStartStopButton;

        if ( pEspStartStopButton )
        {
            connect( m_pEspStartStopButton, &EspDataInterface::dataChanged, this, &StartStopButtonDriver::processDataChanged );
            connect( m_pEspStartStopButton, &EspDataInterface::validChanged, this, &StartStopButtonDriver::processValidChanged );

            setPressed( !m_pEspStartStopButton->level() );
            setValid( m_pEspStartStopButton->isValid() );
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void StartStopButtonDriver::disconnectInterface()
{
    setPressed( false );
    setValid( false );

    // no disconnect here, because the stored pointer is already deleted and would lead into crash
    m_pEspStartStopButton = nullptr;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

StartStopButtonDriver::ButtonState StartStopButtonDriver::buttonState() const
{
    ButtonState nState = Invalid;

    if ( isValid() )
    {
        if ( isPressed() )
        {
            nState = Pressed;
        }
        else
        {
            nState = Released;
        }
    }

    return nState;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void StartStopButtonDriver::processDataChanged( EspDataInterface * pData )
{
    EspDiscreteIn * pDiscreteIn = dynamic_cast<EspDiscreteIn*>( pData );

    if ( pDiscreteIn )
    {
        setPressed( !pDiscreteIn->level() );
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void StartStopButtonDriver::processValidChanged( EspDataInterface * pData )
{
    EspDiscreteIn * pDiscreteIn = dynamic_cast<EspDiscreteIn*>( pData );

    if ( pDiscreteIn )
    {
        setValid( pDiscreteIn->isValid() );
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool StartStopButtonDriver::isPressed() const
{
    return m_bPressed;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool StartStopButtonDriver::isValid( ) const
{
    return m_bValid;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void StartStopButtonDriver::setPressed( const bool bPressed )
{
    if ( bPressed != m_bPressed )
    {
        m_bPressed = bPressed;
        emit buttonStateChanged( buttonState() );
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void StartStopButtonDriver::setValid( const bool bValid )
{
    if ( bValid != m_bValid )
    {
        m_bValid = bValid;
        emit buttonStateChanged( buttonState() );
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////


