///////////////////////////////////////////////////////////////////////////////
///
/// @file StartStopLightDriver.cpp
///
/// @brief main application entry point of StartStopLightDriver.
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
/// @copyright Copyright 2020 by Hemro International AG
///            Hemro International AG
///            Länggenstrasse 34
///            CH 8184 Bachenbülach
///            Switzerland
///            Homepage: www.hemrogroup.com
///
///////////////////////////////////////////////////////////////////////////////

#include "StartStopLightDriver.h"

#include <QDebug>
#include <QThread>

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

StartStopLightDriver::StartStopLightDriver( const EkxProtocol::DriverId u8DriverId )
    : EspDeviceDriver( u8DriverId )
{
    m_timerStandbyFader.setSingleShot( true );
    connect( &m_timerStandbyFader, &QTimer::timeout, this, &StartStopLightDriver::processStandbyFaderTimeout );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

StartStopLightDriver::~StartStopLightDriver()
{
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void StartStopLightDriver::startGrinding()
{
    setStartStopDutyCycle( StartStopDutyGrinding );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void StartStopLightDriver::stopGrinding()
{
    setStartStopDutyCycle( StartStopDutyIdle );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void StartStopLightDriver::processMessageFrame( const EkxProtocol::MessageFrame & frame )
{
    replyNack( frame.driverId().value(),
               frame.msgCounter().value(),
               frame.repeatCounter().value(),
               EkxProtocol::Payload::Nack::NackReason::UnknownDriverCommand );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void StartStopLightDriver::processAlive( const bool bAlive )
{
    if ( bAlive )
    {
        updateStartStopPwm();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

int StartStopLightDriver::startStopDutyCycle() const
{
    return m_nStartStopPwmDuty;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void StartStopLightDriver::setStartStopDutyCycle( const int nPwmDuty )
{
    if ( m_nStartStopPwmDuty != nPwmDuty )
    {
        m_nStartStopPwmDuty = nPwmDuty;
        emit startStopDutyCycleChanged( nPwmDuty );

        updateStartStopPwm();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void StartStopLightDriver::resetStartStopDutyCycle()
{
    setStartStopDutyCycle( StartStopDutyIdle );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void StartStopLightDriver::processStandbyChanged( const bool bStandbyActive )
{
    if ( m_bStandbyActive != bStandbyActive )
    {
        m_bStandbyActive = bStandbyActive;

        if ( m_bStandbyActive )
        {
            m_timerStandbyFader.start( 1000 );
            m_nStandbyFaderState = StandbyFaderState::FadeIn;
        }
        else
        {
            m_timerStandbyFader.stop();
            m_nStandbyFaderState = StandbyFaderState::Idle;
        }

        updateStartStopPwm();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void StartStopLightDriver::updateStartStopPwm()
{
    if ( m_bStandbyActive )
    {
        // here nothing to do
    }
    else
    {
        EkxProtocol::Payload::PwmSetDuty * pPayload = new EkxProtocol::Payload::PwmSetDuty();
        pPayload->pwmDuty().setValue( m_nStartStopPwmDuty );
        send( pPayload );
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void StartStopLightDriver::processStandbyFaderTimeout()
{
    switch ( m_nStandbyFaderState )
    {
        case StandbyFaderState::Idle:
            // do nothing
            break;

        case StandbyFaderState::FadeIn:
        {
            m_nStandbyFaderState = StandbyFaderState::FadeOut;
            m_timerStandbyFader.start( 1000 );

            send( new EkxProtocol::Payload::PwmFadeIn() );
        }
        break;

        case StandbyFaderState::FadeOut:
        {
            m_nStandbyFaderState = StandbyFaderState::Pause;
            m_timerStandbyFader.start( 3000 );

            send( new EkxProtocol::Payload::PwmFadeOut() );
        }
        break;

        case StandbyFaderState::Pause:
        {
            m_nStandbyFaderState = StandbyFaderState::FadeIn;
            m_timerStandbyFader.start( 1000 );
        }
        break;
    }

    updateStartStopPwm();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////
