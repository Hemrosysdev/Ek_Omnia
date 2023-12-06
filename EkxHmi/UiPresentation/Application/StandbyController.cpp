///////////////////////////////////////////////////////////////////////////////
///
/// @file StandbyController.cpp
///
/// @brief main application entry point of StandbyController.
///
/// @author Ultratronik GmbH
///         Dornierstr. 9
///         D-82205 Gilching
///         Germany
///         http://www.ultratronik.de
///
/// @author written by Gerd Esser, Forschung & Entwicklung, gesser@ultratronik.de
///
/// @date 25.01.2021
///
/// @copyright Copyright 2021 by Hemro International AG
///            Hemro International AG
///            Länggenstrasse 34
///            CH 8184 Bachenbülach
///            Switzerland
///            Homepage: www.hemrogroup.com
///
///////////////////////////////////////////////////////////////////////////////

#include "StandbyController.h"

#include <QQmlEngine>
#include <QQmlContext>
#include <QDebug>

#include "StartStopButtonDriver.h"
#include "EspDriver.h"
#include "DddDriver.h"
#include "MainStatemachine.h"

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

StandbyController::StandbyController( QQmlEngine * pEngine,
                                      MainStatemachine * pMainStatemachine )
    : QObject()
    , m_pMainStatemachine( pMainStatemachine )
{
    pEngine->rootContext()->setContextProperty( "standbyController", this );

    m_timerPreStandbyDimTrigger.setSingleShot( true );
    m_timerStandbyTrigger.setSingleShot( true );

    setStandbyTimeout( 5 );   // 5 minutes by default

    connect( &m_timerStandbyTrigger, &QTimer::timeout, this, &StandbyController::processTimeoutStandbyTrigger );
    connect( &m_timerPreStandbyDimTrigger, &QTimer::timeout, this, &StandbyController::processTimeoutPreStandbyDimTrigger );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

StandbyController::~StandbyController()
{
    m_pMainStatemachine = nullptr;

    m_timerStandbyTrigger.stop();
    m_timerPreStandbyDimTrigger.stop();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void StandbyController::create()
{
    connect( m_pMainStatemachine->dddDriver(), &DddDriver::dddValueChanged, this, &StandbyController::wakeUp );
    connect( m_pMainStatemachine->espDriver()->startStopButtonDriver(), &StartStopButtonDriver::buttonStateChanged, this, &StandbyController::wakeUp );
    connect( m_pMainStatemachine->settingsStatemachine()->settingsSerializer(), &SettingsSerializer::stdbyTimeIndexChanged, this, &StandbyController::processStandbyTimeChanged );
    connect( m_pMainStatemachine, &MainStatemachine::ekxMainstateChanged, this, &StandbyController::prelongStandbyTrigger );

    // force inital sync with settings
    prelongStandbyTrigger();
    processStandbyTimeChanged();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool StandbyController::isStandbyActive() const
{
    return m_bStandbyActive;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool StandbyController::isPreStandbyDimActive() const
{
    return m_bPreStandbyDimActive;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void StandbyController::setStandbyTimeout( const int nMinutes )
{
    m_timerStandbyTrigger.setInterval( nMinutes * 60 * 1000 );

    // dimming change for display 70% of standby time
    m_timerPreStandbyDimTrigger.setInterval( nMinutes * 60 * 1000 * 7 / 10 );

    prelongStandbyTrigger();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void StandbyController::processTimeoutStandbyTrigger()
{
    if ( !m_bStandbyActive )
    {
        qInfo() << "processTimeoutStandbyTrigger(): going into standby";

        m_bStandbyActive = true;
        emit standbyActivated();
        emit standbyChanged( m_bStandbyActive );

        m_timerStandbyTrigger.stop();  // just to be sure
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void StandbyController::processTimeoutPreStandbyDimTrigger()
{
    if ( !m_bPreStandbyDimActive )
    {
        qInfo() << "processTimeoutPreStandbyDimTrigger(): going into dimmed phase";

        m_bPreStandbyDimActive = true;
        emit preStandbyDimChanged( m_bPreStandbyDimActive );

        m_timerPreStandbyDimTrigger.stop();  // just to be sure
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void StandbyController::processUserInput()
{
    prelongStandbyTrigger();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void StandbyController::wakeUp()
{
    if ( m_bPreStandbyDimActive )
    {
        qInfo() << "wakeUp(): return from dimmed phase";

        m_bPreStandbyDimActive = false;
        emit preStandbyDimChanged( m_bPreStandbyDimActive );
    }

    if ( m_bStandbyActive )
    {
        qInfo() << "wakeUp(): return from standby";

        m_bStandbyActive = false;
        emit standbyDeactivated();
        emit standbyChanged( m_bStandbyActive );
    }

    prelongStandbyTrigger();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void StandbyController::processStandbyTimeChanged()
{
    int stdByTimeIndex   = m_pMainStatemachine->settingsStatemachine()->settingsSerializer()->getStdbyTimeIndex();
    int stdByTimeMinutes = m_pMainStatemachine->settingsStatemachine()->settingsSerializer()->stdbyTimeIndexToStdbyTimeMinutes(stdByTimeIndex);
    setStandbyTimeout( stdByTimeMinutes );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void StandbyController::prelongStandbyTrigger()
{
    if ( m_pMainStatemachine->ekxMainstate() != MainStatemachine::EKX_SWUPDATE )
    {
        m_timerStandbyTrigger.start();
        m_timerPreStandbyDimTrigger.start();
    }
    else
    {
        m_timerStandbyTrigger.stop();
        m_timerPreStandbyDimTrigger.stop();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////
