///////////////////////////////////////////////////////////////////////////////
///
/// @file EspStepperMotorDriver.cpp
///
/// @brief main application entry point of EspStepperMotorDriver.
///
/// @author Ultratronik GmbH
///         Dornierstr. 9
///         D-82205 Gilching
///         Germany
///         http://www.ultratronik.de
///
/// @author written by Florian Sonntag, Forschung & Entwicklung, gesser@ultratronik.de
///
/// @date 29.10.2021
///
/// @copyright Copyright 2021 by Hemro International AG
///            Hemro International AG
///            Länggenstrasse 34
///            CH 8184 Bachenbülach
///            Switzerland
///            Homepage: www.hemrogroup.com
///
///////////////////////////////////////////////////////////////////////////////

#include "EspStepperMotorDriver.h"

#include <QDebug>

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

EspStepperMotorDriver::EspStepperMotorDriver( const EkxProtocol::DriverId u8DriverId,
                                              const QString &             strName )
    : EspDeviceDriver( u8DriverId )
    , m_strName( strName )
{
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

EspStepperMotorDriver::~EspStepperMotorDriver()
{
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void EspStepperMotorDriver::startMotor()
{
    startMotor( m_bDirection,
                m_nStartFrequency,
                m_nRunFrequency,
                m_nRampFreqPerStep );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void EspStepperMotorDriver::startMotor( const bool bDirection,
                                        const int  nStartFrequency,
                                        const int  nRunFrequency,
                                        const int  nRampFreqPerStep )
{
    qInfo() << "startMotor " << m_strName
            << " / Dir " << bDirection
            << " / StartFreq" << nStartFrequency
            << " / RunFreq" << nRunFrequency
            << " / RampFreqPerStep" << nRampFreqPerStep;

    setDirection( bDirection );
    setStartFrequency( nStartFrequency );
    setRunFrequency( nRunFrequency );
    setRampFreqPerStep( nRampFreqPerStep );

    EkxProtocol::Payload::StepperMotorStart * pPayload = new EkxProtocol::Payload::StepperMotorStart();
    pPayload->direction().setValue( bDirection );
    pPayload->startFrequency().setValue( nStartFrequency );
    pPayload->runFrequency().setValue( nRunFrequency );
    pPayload->rampFreqPerStep().setValue( nRampFreqPerStep );

    send( pPayload );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void EspStepperMotorDriver::startMotorSteps()
{
    startMotorSteps( m_bDirection,
                     m_nSteps,
                     m_nStartFrequency,
                     m_nRunFrequency,
                     m_nRampFreqPerStep );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void EspStepperMotorDriver::startMotorSteps( const bool bDirection,
                                             const int  nSteps,
                                             const int  nStartFrequency,
                                             const int  nRunFrequency,
                                             const int  nRampFreqPerStep )
{
    qInfo() << "startMotorSteps " << m_strName
            << " / Dir " << bDirection
            << " / Steps " << nSteps
            << " / StartFreq" << nStartFrequency
            << " / RunFreq" << nRunFrequency
            << " / RampFreqPerStep" << nRampFreqPerStep;

    setDirection( bDirection );
    setSteps( nSteps );
    setStartFrequency( nStartFrequency );
    setRunFrequency( nRunFrequency );
    setRampFreqPerStep( nRampFreqPerStep );

    EkxProtocol::Payload::StepperMotorStartSteps * pPayload = new EkxProtocol::Payload::StepperMotorStartSteps();
    pPayload->direction().setValue( bDirection );
    pPayload->steps().setValue( nSteps );
    pPayload->startFrequency().setValue( nStartFrequency );
    pPayload->runFrequency().setValue( nRunFrequency );
    pPayload->rampFreqPerStep().setValue( nRampFreqPerStep );

    send( pPayload );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void EspStepperMotorDriver::stopMotor()
{
    qInfo() << "stopMotor " << m_strName;

    send( new EkxProtocol::Payload::StepperMotorStop() );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void EspStepperMotorDriver::changeRunFrequency( const int nFrequency )
{
    if ( m_nRunFrequency != nFrequency )
    {
        setRunFrequency( nFrequency );

        EkxProtocol::Payload::StepperMotorChangeRunFrequency * pPayload = new EkxProtocol::Payload::StepperMotorChangeRunFrequency();
        pPayload->runFrequency().setValue( nFrequency );

        send( pPayload );
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool EspStepperMotorDriver::isRunning() const
{
    return m_bRunning;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool EspStepperMotorDriver::isValid() const
{
    return m_bValid;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool EspStepperMotorDriver::isFaultPinActive() const
{
    return m_bFaultPinActive;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void EspStepperMotorDriver::setDirection( const bool bDirection )
{
    if ( m_bDirection != bDirection )
    {
        m_bDirection = bDirection;
        emit directionChanged( m_bDirection );
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool EspStepperMotorDriver::direction() const
{
    return m_bDirection;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void EspStepperMotorDriver::setValid( const bool bValid )
{
    if ( bValid != m_bValid )
    {
        m_bValid = bValid;
        emit validChanged( m_bValid );
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void EspStepperMotorDriver::setSteps( const int nSteps )
{
    if ( m_nSteps != nSteps )
    {
        m_nSteps = nSteps;
        emit stepsChanged( m_nSteps );
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

int EspStepperMotorDriver::steps() const
{
    return m_nSteps;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void EspStepperMotorDriver::setStartFrequency( const int nFrequency )
{
    if ( m_nStartFrequency != nFrequency )
    {
        m_nStartFrequency = nFrequency;
        emit startFrequencyChanged( m_nStartFrequency );
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

int EspStepperMotorDriver::startFrequency() const
{
    return m_nStartFrequency;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void EspStepperMotorDriver::setRunFrequency( const int nFrequency )
{
    if ( m_nRunFrequency != nFrequency )
    {
        m_nRunFrequency = nFrequency;
        emit runFrequencyChanged( m_nRunFrequency );
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

int EspStepperMotorDriver::runFrequency() const
{
    return m_nRunFrequency;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void EspStepperMotorDriver::setRampFreqPerStep( const int nRampFreqPerStep )
{
    if ( m_nRampFreqPerStep != nRampFreqPerStep )
    {
        m_nRampFreqPerStep = nRampFreqPerStep;
        emit rampFreqPerStepChanged( m_nRampFreqPerStep );
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

int EspStepperMotorDriver::rampFreqPerStep() const
{
    return m_nRampFreqPerStep;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void EspStepperMotorDriver::processMessageFrame( const EkxProtocol::MessageFrame & frame )
{
    const EkxProtocol::Payload::StepperMotorStatus * pPayload = dynamic_cast<const EkxProtocol::Payload::StepperMotorStatus *>( frame.payloadRef().value() );

    if ( pPayload )
    {
        setValid( pPayload->valid().value() );
        setFaultPinActive( pPayload->faultPinActive().value() );
        setRunning( pPayload->motorRunning().value() );

        emit statusChanged();

        replyAck( frame.driverId().value(),
                  frame.msgCounter().value(),
                  frame.repeatCounter().value() );
    }
    else
    {
        replyNack( frame.driverId().value(),
                   frame.msgCounter().value(),
                   frame.repeatCounter().value(),
                   EkxProtocol::Payload::Nack::NackReason::PayloadBroken );
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void EspStepperMotorDriver::processAlive( const bool bAlive )
{
    if ( !bAlive )
    {
        setValid( false );
        setRunning( false );
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void EspStepperMotorDriver::setRunning( const bool bRunning )
{
    if ( m_bRunning != bRunning )
    {
        m_bRunning = bRunning;
        emit runningChanged( m_bRunning );
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void EspStepperMotorDriver::setFaultPinActive( const bool bActive )
{
    if ( m_bFaultPinActive != bActive )
    {
        m_bFaultPinActive = bActive;
        emit faultPinActiveChanged( m_bFaultPinActive );
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

