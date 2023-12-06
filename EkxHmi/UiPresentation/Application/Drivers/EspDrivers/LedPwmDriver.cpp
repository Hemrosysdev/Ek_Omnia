///////////////////////////////////////////////////////////////////////////////
///
/// @file PwmDriver.cpp
///
/// @brief main application entry point of PwmDriver.
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

#include "LedPwmDriver.h"

#include <QDebug>

#include "EspPwmOut.h"

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

LedPwmDriver::LedPwmDriver( QObject *parent )
    : QObject( parent )
{
    m_timerFader.setSingleShot( true );
    connect( &m_timerFader, &QTimer::timeout, this, &LedPwmDriver::processFaderTimeout );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

LedPwmDriver::~LedPwmDriver()
{
    m_pEspPwmOutSpotlight = nullptr;
    m_pEspPwmOutStartStop = nullptr;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void LedPwmDriver::connectStartStopInterface( EspPwmOut * pEspPwmOut )
{
    m_pEspPwmOutStartStop = pEspPwmOut;

    updateStartStopPwm();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void LedPwmDriver::disconnectStartStopInterface()
{
    m_pEspPwmOutStartStop = nullptr;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void LedPwmDriver::connectSpotlightInterface( EspPwmOut *pEspPwmOut )
{
    m_pEspPwmOutSpotlight = pEspPwmOut;

    updateSpotlightPwm();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void LedPwmDriver::disconnectSpotlightInterface()
{
    m_pEspPwmOutSpotlight = nullptr;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void LedPwmDriver::setSpotlightPwm( const int nPwmDuty )
{
    if ( m_nSpotlightPwmDuty != nPwmDuty )
    {
        m_nSpotlightPwmDuty = nPwmDuty;
        updateSpotlightPwm();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void LedPwmDriver::setStartStopPwm( const int nPwmDuty )
{
    if ( m_nStartStopPwmDuty != nPwmDuty )
    {
        m_nStartStopPwmDuty = nPwmDuty;
        updateStartStopPwm();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void LedPwmDriver::processStandbyChanged(const bool bStandbyActive)
{
    if ( m_bStandbyActive != bStandbyActive )
    {
        m_bStandbyActive = bStandbyActive;

        if ( m_bStandbyActive )
        {
            m_timerFader.start( 1000 );
            m_nFaderState = FadeIn;
        }
        else
        {
            m_timerFader.stop();
            m_nFaderState = Idle;
        }

        updateSpotlightPwm();
        updateStartStopPwm();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void LedPwmDriver::updateSpotlightPwm()
{
    if ( m_pEspPwmOutSpotlight )
    {
        if ( m_bStandbyActive )
        {
            m_pEspPwmOutSpotlight->setDuty( SpotlightIdle );
        }
        else
        {
            m_pEspPwmOutSpotlight->setDuty( m_nSpotlightPwmDuty );
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void LedPwmDriver::updateStartStopPwm()
{
    if ( m_pEspPwmOutStartStop )
    {
        if ( m_bStandbyActive )
        {
            // here nothing to do
        }
        else
        {
            m_pEspPwmOutStartStop->setDuty( m_nStartStopPwmDuty );
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void LedPwmDriver::processFaderTimeout()
{
    switch ( m_nFaderState )
    {
        case Idle:
            // do nothing
        break;

        case FadeIn:
        {
            m_nFaderState = FadeOut;
            m_timerFader.start( 1000 );

            if ( m_pEspPwmOutStartStop )
            {
                m_pEspPwmOutStartStop->fadeIn();
            }
        }
        break;

        case FadeOut:
        {
            m_nFaderState = Pause;
            m_timerFader.start( 3000 );

            if ( m_pEspPwmOutStartStop )
            {
                m_pEspPwmOutStartStop->fadeOut();
            }
        }
        break;

        case Pause:
        {
            m_nFaderState = FadeIn;
            m_timerFader.start( 1000 );
        }
        break;
    }

    updateStartStopPwm();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////
