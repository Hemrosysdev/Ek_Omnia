///////////////////////////////////////////////////////////////////////////////
///
/// @file SpotLightDriver.cpp
///
/// @brief main application entry point of SpotLightDriver.
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

#include "SpotLightDriver.h"

#include <QDebug>
#include <QThread>

#define SPOT_LIGHT_FADE_IN_TIME_MS          500         // 0.5sec
#define SPOT_LIGHT_FADE_OUT_TIME_MS        1000         // 1sec
#define SPOT_LIGHT_POST_GRIND_TIME_MS      3000         // 3secs

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

SpotLightDriver::SpotLightDriver( const EkxProtocol::DriverId u8DriverId )
    : EspDeviceDriver( u8DriverId )
{
    m_timerStandbyFader.setSingleShot( true );
    connect( &m_timerStandbyFader, &QTimer::timeout, this, &SpotLightDriver::processStandbyFaderTimeout );

    m_timerSpotLightPostGrind.setSingleShot( true );
    m_timerSpotLightPostGrind.setInterval( SPOT_LIGHT_POST_GRIND_TIME_MS );
    connect( &m_timerSpotLightPostGrind, &QTimer::timeout, this, &SpotLightDriver::processSpotLightPostGrindTimeout );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

SpotLightDriver::~SpotLightDriver()
{
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

int SpotLightDriver::spotLightDutyCycle() const
{
    return m_nSpotLightPwmDuty;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SpotLightDriver::setSpotLightDutyCycle( const int nPwmDuty )
{
    if ( m_nSpotLightPwmDuty != nPwmDuty )
    {
        m_nSpotLightPwmDuty = nPwmDuty;
        emit spotLightDutyCycleChanged( nPwmDuty );

        EkxProtocol::Payload::PwmSetDuty * pPayload = new EkxProtocol::Payload::PwmSetDuty();
        pPayload->pwmDuty().setValue( m_nSpotLightPwmDuty );

        send( pPayload );
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SpotLightDriver::resetSpotLightDutyCycle()
{
    setSpotLightDutyCycle( SpotLightDutyIdle );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SpotLightDriver::setSpotLightState( const SpotLightState nState )
{
    if ( m_nSpotLightState != nState )
    {
        //qInfo() << "setSpotLightState" << nState;

        switch ( m_nSpotLightState )
        {
            case SpotLightState::Idle:
                switch ( nState )
                {
                    case SpotLightState::On:
                        send( new EkxProtocol::Payload::PwmFadeIn() );
                        m_nSpotLightState = nState;
                        break;

                    default:
                        // do nothing
                        break;
                }
                break;

            case SpotLightState::On:
                switch ( nState )
                {
                    case SpotLightState::PostGrind:
                        m_timerSpotLightPostGrind.start();
                        m_nSpotLightState = nState;
                        break;

                    case SpotLightState::Standby:
                    case SpotLightState::Idle:
                        send( new EkxProtocol::Payload::PwmFadeOut() );
                        m_nSpotLightState = nState;
                        break;

                    default:
                        // do nothing
                        break;
                }
                break;

            case SpotLightState::PostGrind:
                switch ( nState )
                {
                    case SpotLightState::Standby:
                    case SpotLightState::Idle:
                        send( new EkxProtocol::Payload::PwmFadeOut() );
                        m_nSpotLightState = nState;
                        break;

                    case SpotLightState::On:
                        m_timerSpotLightPostGrind.stop();
                        m_nSpotLightState = nState;
                        break;

                    default:
                        // do nothing
                        break;
                }
                break;

            case SpotLightState::Standby:
                switch ( nState )
                {
                    case SpotLightState::On:
                        send( new EkxProtocol::Payload::PwmFadeIn() );
                        m_nSpotLightState = nState;
                        break;

                    default:
                        // do nothing
                        break;
                }
                break;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SpotLightDriver::startGrinding()
{
    if ( m_bShowSpotLight )
    {
        setSpotLightState( SpotLightState::On );
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SpotLightDriver::stopGrinding()
{
    setSpotLightState( SpotLightState::PostGrind );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SpotLightDriver::processMessageFrame( const EkxProtocol::MessageFrame & frame )
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

void SpotLightDriver::processAlive( const bool bAlive )
{
    if ( bAlive )
    {
        EkxProtocol::Payload::PwmFaderSettings * pPayload = new EkxProtocol::Payload::PwmFaderSettings();
        pPayload->fadeInDutyPercent().setValue( SpotLightDutyGrinding );
        pPayload->fadeInTimeMs().setValue( SPOT_LIGHT_FADE_IN_TIME_MS );
        pPayload->fadeOutDutyPercent().setValue( SpotLightDutyIdle );
        pPayload->fadeOutTimeMs().setValue( SPOT_LIGHT_FADE_OUT_TIME_MS );

        send( pPayload );

        // ESP has faded in upon startup (much earlier than Linux does)
        // now we have to fade out accordingly
        setSpotLightState( SpotLightState::On );
        QThread::msleep( 100 );
        setSpotLightState( SpotLightState::PostGrind );
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SpotLightDriver::processShowSpotLightChanged( const bool bShow )
{
    m_bShowSpotLight = bShow;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SpotLightDriver::processStandbyChanged( const bool bStandbyActive )
{
    if ( m_bStandbyActive != bStandbyActive )
    {
        m_bStandbyActive = bStandbyActive;

        if ( m_bStandbyActive )
        {
            m_timerStandbyFader.start( 1000 );
            m_nStandbyFaderState = StandbyFaderState::FadeIn;
            setSpotLightState( SpotLightState::Standby );
        }
        else
        {
            m_timerStandbyFader.stop();
            m_nStandbyFaderState = StandbyFaderState::Idle;
            setSpotLightState( SpotLightState::Idle );
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SpotLightDriver::processStandbyFaderTimeout()
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
        }
        break;

        case StandbyFaderState::FadeOut:
        {
            m_nStandbyFaderState = StandbyFaderState::Pause;
            m_timerStandbyFader.start( 3000 );
        }
        break;

        case StandbyFaderState::Pause:
        {
            m_nStandbyFaderState = StandbyFaderState::FadeIn;
            m_timerStandbyFader.start( 1000 );
        }
        break;
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SpotLightDriver::processSpotLightPostGrindTimeout()
{
    setSpotLightState( SpotLightState::Idle );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////
