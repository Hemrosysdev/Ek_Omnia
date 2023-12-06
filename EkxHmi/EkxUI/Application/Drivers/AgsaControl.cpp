///////////////////////////////////////////////////////////////////////////////
///
/// @file AgsaControl.cpp
///
/// @brief main application entry point of AgsaControl.
///
/// @author Ultratronik GmbH
///         Dornierstr. 9
///         D-82205 Gilching
///         Germany
///         http://www.ultratronik.de
///
/// @author written by Florian Sonntag, Forschung & Entwicklung, gesser@ultratronik.de
///
/// @date 24.03.2022
///
/// @copyright Copyright 2022 by Hemro International AG
///            Hemro International AG
///            Länggenstrasse 34
///            CH 8184 Bachenbülach
///            Switzerland
///            Homepage: www.hemrogroup.com
///
///////////////////////////////////////////////////////////////////////////////

#include "AgsaControl.h"

#include <QDebug>
#include <cmath>

#include "AgsaLongDurationTest.h"
#include "DddCouple.h"
#include "EspStepperMotorDriver.h"

#define MAX_FULL_SPEED_BLOCKAGE_COUNTER      3
#define MAX_APPROACH_SPEED_BLOCKAGE_COUNTER  30

#ifdef TARGETBUILD

//#define AGSA_FULL_SPEED       ( 1000 )
//#define AGSA_APPROACH_SPEED   ( 150 )
#define MAX_AGSA_DDD_DIFF       0

//#define LINEAR_DECELERATE_TEST
#define SIMPLE_DIST_TEST

#ifdef LINEAR_DECELERATE_TEST
#define DECELERATE_DDD              7
#define DECELERATE_STEPS            30
#endif

#ifdef SIMPLE_DIST_TEST
#define APPROACH_SPEED_DDD_DIST     20
#endif

#else
#define AGSA_FULL_SPEED                         ( 0.5 )
#define AGSA_APPROACH_SPEED                     ( 0.03 )
#define MAX_AGSA_DDD_DIFF                       0
#define APPROACH_SPEED_DDD_DIFF                 20

#endif

#define DDD_FULL_SPEED_BLOCKAGE_DISTANCE        10
#define DDD_APPROACH_SPEED_BLOCKAGE_DISTANCE    1
#define RAMP_STEPS                              50

#define MAX_HEMRO_SCALE_FLOAT                   ( 800.0 )
#define MAX_DDD_DEGREE_SCALE_FLOAT              ( 355.8 )
#define MAX_PHYS_DEGREE_SCALE_FLOAT             ( 288.0 )
#define HEMRO_GEAR_RATIO                        ( MAX_PHYS_DEGREE_SCALE_FLOAT / MAX_HEMRO_SCALE_FLOAT )
#define AGSA_GEAR_RATIO                         ( 83.0 / 10.0 )
#define STEPPER_MICROSTEPS_PER_STEP             ( 32.0 )
#define STEPPER_STEPS_PER_REVOLUTION            ( 200.0 )
#define STEPPER_DEGREE_PER_STEP                 ( 360.0 / STEPPER_STEPS_PER_REVOLUTION )

#define MICROSTEPS_PER_HEMRO                    ( HEMRO_GEAR_RATIO * AGSA_GEAR_RATIO / STEPPER_DEGREE_PER_STEP * STEPPER_MICROSTEPS_PER_STEP )

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

AgsaControl::AgsaControl( QObject * parent )
    : QObject( parent )
    , m_pLongDurationTest( new AgsaLongDurationTest( this ) )
{
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

AgsaControl::~AgsaControl()
{
    delete m_pLongDurationTest;
    m_pLongDurationTest = nullptr;

    m_pAgsaStepperMotorDriver = nullptr;
    m_pDddCouple              = nullptr;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void AgsaControl::create( const QString &               strDeviceSerialNo,
                          EspStepperMotorDriver * const pAgsaStepperMotorDriver,
                          DddCouple *                   pDddCouple )
{
    m_pAgsaStepperMotorDriver = pAgsaStepperMotorDriver;
    m_pDddCouple              = pDddCouple;

    m_pLongDurationTest->create( strDeviceSerialNo, this, m_pDddCouple );

    connect( &m_timerProgress, &QTimer::timeout, this, &AgsaControl::processProgressTimeout );
    connect( &m_timerProcess, &QTimer::timeout, this, &AgsaControl::processProcessTimeout );
#ifndef TARGETBUILD
    connect( &m_timerSimulation, &QTimer::timeout, this, &AgsaControl::processSimulationTimeout );
#endif

    m_timerProgress.setInterval( 400 );
    m_timerProgress.setSingleShot( false );

    m_timerProcess.setInterval( 15000 );
    m_timerProcess.setSingleShot( true );

#ifndef TARGETBUILD
    m_timerSimulation.setInterval( 20 );
    m_timerSimulation.setSingleShot( false );
#endif
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void AgsaControl::setTestMode( const bool bEnable )
{
    if ( m_bTestMode != bEnable )
    {
        m_bTestMode = bEnable;
        emit testModeChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool AgsaControl::testMode() const
{
    return m_bTestMode;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void AgsaControl::setTestLiveMode( const bool bEnable )
{
    if ( m_bTestLiveMode != bEnable )
    {
        m_bTestLiveMode = bEnable;
        emit testLiveModeChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool AgsaControl::testLiveMode() const
{
    return m_bTestLiveMode;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void AgsaControl::setTestBlockageDetect( const bool bEnable )
{
    if ( m_bTestBlockageDetect != bEnable )
    {
        m_bTestBlockageDetect = bEnable;
        emit testBlockageDetectChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool AgsaControl::testBlockageDetect() const
{
    return m_bTestBlockageDetect;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void AgsaControl::moveSteps( const int  nSteps,
                             const int  nStartFrequency,
                             const int  nRunFrequency,
                             const bool bIgnoreDddFailed,
                             const bool bNoWatchdog )
{
    qInfo() << "moveSteps()" << nSteps << " Steps," << nRunFrequency << "Hz (run), ignore DDD" << bIgnoreDddFailed << ", no WDG" << bNoWatchdog;

    setFailedTimeout( false );
    setFailedBlockage( false );
    setFailedNoDdd( false );

    if ( m_bDddFailed
         && !bIgnoreDddFailed )
    {
        setFailedNoDdd( true );
    }
    else
    {
        m_nMovingMode = MovingMode::MoveSteps;

        m_nLastProgressDddValue = m_nCurrentDddValue;

        m_pAgsaStepperMotorDriver->startMotorSteps( nSteps < 0,
                                                    std::abs( nSteps ),
                                                    nStartFrequency,
                                                    nRunFrequency,
                                                    RAMP_STEPS );

        if ( !isFailed()
             && !bNoWatchdog )
        {
            startWatchdog();
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void AgsaControl::moveToDddValue( const int nDddValue )
{
    qDebug() << "moveToDddValue: " << nDddValue;

    setTargetDddValue( nDddValue );

    if ( m_nCurrentDddValue != nDddValue )
    {
        // we do small moves without control, just stepping
        //if ( abs( m_nCurrentDddValue - nDddValue ) <= 10 )
        //{
        //    moveStepsToDddValue( nDddValue );
        //    startWatchdog();
        //}
        //else
        {
            m_nMovingMode = MovingMode::MoveToDdd;

            setFailedTimeout( false );
            setFailedBlockage( false );
            setFailedNoDdd( false );

            startDddTargetController();

            if ( !isFailed() )
            {
                startWatchdog();
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void AgsaControl::moveStepsToDddValue( const int nDddValue )
{
    setTargetDddValue( nDddValue );

    if ( m_nCurrentDddValue != nDddValue )
    {
        qDebug() << "moveStepsToDddValue: " << nDddValue;
        moveSteps( ( nDddValue - m_nCurrentDddValue ) * MICROSTEPS_PER_HEMRO,
                   startFrequency(),
                   runFrequency(),
                   false, false );
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void AgsaControl::incrementTargetDddValue()
{
    if ( m_nTargetDddValue < MAX_HEMRO_SCALE )
    {
        if ( m_pAgsaStepperMotorDriver->isRunning() )
        {
            setTargetDddValue( m_nTargetDddValue + 1 );
        }
        else
        {
            moveToDddValue( m_nTargetDddValue + 1 );
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void AgsaControl::decrementTargetDddValue()
{
    if ( m_nTargetDddValue > 0 )
    {
        if ( m_pAgsaStepperMotorDriver->isRunning() )
        {
            setTargetDddValue( m_nTargetDddValue - 1 );
        }
        else
        {
            moveToDddValue( m_nTargetDddValue - 1 );
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void AgsaControl::setTargetDddValue( const int nTargetDddValue )
{
    if ( m_nTargetDddValue != nTargetDddValue )
    {
        if ( nTargetDddValue >= 0
             && nTargetDddValue <= MAX_HEMRO_SCALE )
        {
            m_nTargetDddValue = nTargetDddValue;
            emit targetDddValueChanged();
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

int AgsaControl::targetDddValue() const
{
    return m_nTargetDddValue;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void AgsaControl::setFailedTimeout( const bool bFailed )
{
    if ( m_bFailedTimeout != bFailed )
    {
        m_bFailedTimeout = bFailed;
        emit failedChanged();

        if ( m_bFailedTimeout )
        {
            m_pLongDurationTest->triggerStepStateMachine( AgsaLongDurationTest::StepStateAction::AgsaFailed );
            stopAgsaControl();
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool AgsaControl::isFailedTimeout() const
{
    return m_bFailedTimeout;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void AgsaControl::setFailedBlockage( const bool bFailed )
{
    if ( m_bFailedBlockage != bFailed )
    {
        m_bFailedBlockage = bFailed;
        emit failedChanged();

        if ( m_bFailedBlockage )
        {
            m_pLongDurationTest->triggerStepStateMachine( AgsaLongDurationTest::StepStateAction::AgsaFailed );
            stopAgsaControl();
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool AgsaControl::isFailedBlockage() const
{
    return m_bFailedBlockage;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void AgsaControl::setFailedNoDdd( const bool bFailed )
{
    if ( m_bFailedNoDdd != bFailed )
    {
        m_bFailedNoDdd = bFailed;
        emit failedChanged();

        if ( m_bFailedNoDdd )
        {
            m_pLongDurationTest->triggerStepStateMachine( AgsaLongDurationTest::StepStateAction::AgsaFailed );
            stopAgsaControl();
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool AgsaControl::isFailedNoDdd() const
{
    return m_bFailedNoDdd;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool AgsaControl::isFailed() const
{
    return m_bFailedTimeout
           || m_bFailedBlockage
           || m_bFailedNoDdd;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void AgsaControl::setSteps( const int nSteps )
{
    if ( m_nSteps != nSteps )
    {
        m_nSteps = nSteps;
        emit stepsChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

int AgsaControl::steps() const
{
    return m_nSteps;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void AgsaControl::setStartFrequency( const int nFrequency )
{
    if ( m_nStartFrequency != nFrequency )
    {
        m_nStartFrequency = nFrequency;
        emit startFrequencyChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

int AgsaControl::startFrequency() const
{
    return m_nStartFrequency;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void AgsaControl::setRunFrequency( const int nFrequency )
{
    if ( m_nRunFrequency != nFrequency )
    {
        m_nRunFrequency = nFrequency;
        emit runFrequencyChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

int AgsaControl::runFrequency() const
{
    return m_nRunFrequency;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void AgsaControl::setApproachFrequency( const int nFrequency )
{
    if ( m_nApproachFrequency != nFrequency )
    {
        m_nApproachFrequency = nFrequency;
        emit approachFrequencyChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

int AgsaControl::approachFrequency() const
{
    return m_nApproachFrequency;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

AgsaLongDurationTest * AgsaControl::longDurationTest()
{
    return m_pLongDurationTest;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void AgsaControl::processDddValue()
{
    m_nCurrentDddValue = m_pDddCouple->dddValue();

    int nCurrentDddValue = m_nCurrentRawDddValue;

    //qInfo() << "processDddValue" << m_nCurrentDddValue;

    if ( m_nMovingMode == MovingMode::MoveToDdd )
    {
        if ( std::abs( m_nCurrentRawDddValue - m_nTargetDddValue ) <= MAX_AGSA_DDD_DIFF )
        {
            qInfo() << "processDddValue stop motor";
#ifdef TARGETBUILD
            m_pAgsaStepperMotorDriver->stopMotor();
#else
            stopAgsaControl();
#endif
        }
        else
        {
#ifdef TARGETBUILD
            m_pAgsaStepperMotorDriver->changeRunFrequency( calcMoveToDddFrequency( m_nCurrentRawDddValue ) );
#endif

            // overshoot left turn
            if ( nCurrentDddValue > m_nTargetDddValue
                 && nCurrentDddValue > m_nLastUpdateDddValue )
            {
                qInfo() << "processDddValue turn direction right";
                startDddTargetController();
            }
            // overshoot right turn
            else if ( nCurrentDddValue < m_nTargetDddValue
                      && nCurrentDddValue < m_nLastUpdateDddValue )
            {
                qInfo() << "processDddValue turn direction left";
                startDddTargetController();
            }
            else
            {
#ifdef TARGETBUILD
                //changeRunFrequency( calcMoveToDddFrequency( m_nCurrentRawDddValue ) );
#else
                if ( std::abs( m_nCurrentDddValue - m_nTargetDddValue ) >= APPROACH_SPEED_DDD_DIFF )
                {
                    m_dSimulationMotorSpeed = AGSA_FULL_SPEED;
                }
                else
                {
                    m_dSimulationMotorSpeed = AGSA_APPROACH_SPEED;
                }
#endif
            }
        }
    }

    m_nLastUpdateDddValue = nCurrentDddValue;
    //    if ( m_nMovingMode == MovingMode::MoveToDdd )
    //    {
    //        if ( std::abs( m_nCurrentDddValue - m_nTargetDddValue ) <= MAX_AGSA_DDD_DIFF )
    //        {
    //            //qInfo() << "processDddValue target reached";
    //            //stopAgsaControl();
    //            stopMotor();

    //            // do nothing wait for "blockage" detection
    //        }
    //        else
    //        {
    //            if ( m_nStartDddDistance > 0 &&  m_nCurrentDddValue < m_nTargetDddValue )
    //            {
    //                qInfo() << "processDddValue turn direction";
    //                startDddTargetController();
    //            }
    //            else if ( m_nStartDddDistance < 0 && m_nCurrentDddValue > m_nTargetDddValue )
    //            {
    //                qInfo() << "processDddValue turn direction";
    //                startDddTargetController();
    //            }

    //#ifdef TARGETBUILD
    //            //changeRunFrequency( calcMoveToDddFrequency() );
    //#else
    //            if ( std::abs( m_nCurrentDddValue - m_nTargetDddValue ) >= LOW_SPEED_DDD_DIFF )
    //            {
    //                m_dSimulationMotorSpeed = AGSA_HIGH_SPEED;
    //            }
    //            else
    //            {
    //                m_dSimulationMotorSpeed = AGSA_LOW_SPEED;
    //            }
    //#endif
    //        }
    //    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void AgsaControl::processRawDddValue()
{
    m_nCurrentRawDddValue = m_pDddCouple->rawDddValue();

    //qInfo() << "processRawDddValue" << m_nCurrentRawDddValue;

    //    if ( m_nMovingMode == MovingMode::MoveToDdd )
    //    {
    //        if ( std::abs( m_nCurrentRawDddValue - m_nTargetDddValue ) <= MAX_AGSA_DDD_DIFF )
    //        {
    //            qInfo() << "processRawDddValue stop motor";
    //            //stopAgsaControl();
    //            stopMotor();
    //        }
    //        else
    //        {
    //            if ( m_nCurrentRawDddValue > m_nTargetDddValue &&  m_nCurrentRawDddValue > m_nLastUpdateDddValue )
    //            {
    //                qInfo() << "processDddValue turn direction right";
    //                startDddTargetController();
    //            }
    //            else if ( m_nCurrentRawDddValue < m_nTargetDddValue && m_nCurrentRawDddValue < m_nLastUpdateDddValue )
    //            {
    //                qInfo() << "processDddValue turn direction left";
    //                startDddTargetController();
    //            }
    //            else
    //            {
    //#ifdef TARGETBUILD
    //                changeRunFrequency( calcMoveToDddFrequency( m_nCurrentRawDddValue ) );
    //#else
    //                if ( std::abs( m_nCurrentDddValue - m_nTargetDddValue ) >= LOW_SPEED_DDD_DIFF )
    //                {
    //                    m_dSimulationMotorSpeed = AGSA_HIGH_SPEED;
    //                }
    //                else
    //                {
    //                    m_dSimulationMotorSpeed = AGSA_LOW_SPEED;
    //                }
    //#endif
    //            }
    //        }
    //    }

    //    m_nLastUpdateDddValue = m_nCurrentRawDddValue;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void AgsaControl::processDddFailed()
{
    bool bDddFailed = m_pDddCouple->isDddFailed();

    qWarning() << "processDddFailed()" << bDddFailed;

    if ( m_bDddFailed != bDddFailed )
    {
        m_bDddFailed = bDddFailed;

        if ( m_pAgsaStepperMotorDriver->isRunning() )
        {
            setFailedNoDdd( true );
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void AgsaControl::processProgressTimeout()
{
    if ( std::abs( m_nLastProgressDddValue - m_nCurrentDddValue ) <= DDD_FULL_SPEED_BLOCKAGE_DISTANCE )
    {
        m_nBlockageCounter++;
    }
    else
    {
        m_nBlockageCounter = 0;
    }
    m_nLastProgressDddValue = m_nCurrentDddValue;

    int nMaxBlockageCounter = MAX_FULL_SPEED_BLOCKAGE_COUNTER;
    if ( ( m_nMovingMode == MovingMode::MoveToDdd )
         && ( calcMoveToDddFrequency( m_nCurrentDddValue ) == approachFrequency() ) )
    {
        nMaxBlockageCounter = MAX_APPROACH_SPEED_BLOCKAGE_COUNTER;
    }

    if ( ( m_nMovingMode == MovingMode::MoveToDdd )
         && !m_pAgsaStepperMotorDriver->isRunning()
         && ( std::abs( m_nCurrentDddValue - m_nTargetDddValue ) <= MAX_AGSA_DDD_DIFF ) )
    {
        qInfo() << "processProgressTimeout() reach target";
        stopAgsaControl();
    }

    else if ( ( m_nMovingMode == MovingMode::MoveToDdd )
              && !m_pAgsaStepperMotorDriver->isRunning()
              && ( std::abs( m_nCurrentDddValue - m_nTargetDddValue ) > MAX_AGSA_DDD_DIFF ) )
    {
        qInfo() << "processProgressTimeout() restart stopped motor to reach target";
        startDddTargetController();
    }

    else if ( m_nBlockageCounter > nMaxBlockageCounter )
    {
        if ( ( m_nMovingMode == MovingMode::MoveToDdd )
             && !m_pAgsaStepperMotorDriver->isRunning()
             && ( std::abs( m_nCurrentDddValue - m_nTargetDddValue ) <= MAX_AGSA_DDD_DIFF ) )
        {
            qInfo() << "processProgressTimeout target reached " << m_nTargetDddValue;
            stopAgsaControl();
        }
        else
        {
            if ( ( m_nBlockageCounter > nMaxBlockageCounter )
                 && ( !m_bTestMode
                      || ( m_bTestMode
                           && m_bTestBlockageDetect ) ) )
            {
                qWarning() << "processProgressTimeout() blockage";
                setFailedBlockage( true );
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void AgsaControl::processProcessTimeout()
{
    qWarning() << "processProcessTimeout()";
    setFailedTimeout( true );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void AgsaControl::processRunningChanged( const bool bIsRunning )
{
    Q_UNUSED( bIsRunning );

    // do nothing for the time
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#ifndef TARGETBUILD
void AgsaControl::processSimulationTimeout()
{
    if ( m_nCurrentDddValue >= m_nTargetDddValue )
    {
        m_pDddCouple->incSimulatedScaleRotation( -m_dSimulationMotorSpeed );
    }
    else
    {
        m_pDddCouple->incSimulatedScaleRotation( +m_dSimulationMotorSpeed );
    }
}

#endif

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void AgsaControl::startDddTargetController()
{
    qInfo() << "startDddTargetController() " << targetDddValue();

    if ( m_bDddFailed )
    {
        setFailedNoDdd( true );
    }
    else
    {
        m_nLastProgressDddValue = m_nCurrentDddValue;
        m_nLastUpdateDddValue   = m_nCurrentDddValue;
        m_nBlockageCounter      = 0;

        int nStartDddDistance = m_nCurrentDddValue - m_nTargetDddValue;// m_nCurrentDddValue - m_nTargetDddValue;
        if ( abs( m_nCurrentDddValue - m_nCurrentRawDddValue ) > 20 )
        {
            nStartDddDistance = m_nCurrentRawDddValue - m_nTargetDddValue;// m_nCurrentDddValue - m_nTargetDddValue;
        }

        //        if ( nStartDddDistance == 0 )
        //        {
        //            qWarning() << "startDddTargetController() m_nDddDirection == 0";
        //            stopMotor();
        //            //stopAgsaControl();
        //        }
        //        else
        {
#ifdef TARGETBUILD
            //MovingMode nMovingMode = m_nMovingMode;

            //int nFrequency = runFrequency();//calcMoveToDddFrequency( m_nCurrentDddValue );
            //moveSteps( -nStartDddDistance * STEPS_PER_800_HEMRO / MAX_HEMRO_SCALE, nFrequency );
            //m_nMovingMode = nMovingMode;

            int nFrequency = calcMoveToDddFrequency( m_nCurrentDddValue );
            m_pAgsaStepperMotorDriver->startMotor( nStartDddDistance > 0,
                                                   startFrequency(),
                                                   nFrequency,
                                                   RAMP_STEPS );
            //qInfo() << "Steps" << std::fabs( nStartDddDistance ) * MICROSTEPS_PER_HEMRO;
            //m_pAgsaStepperMotorDriver->startMotorSteps( nStartDddDistance > 0,
            //                                        std::fabs( nStartDddDistance ) * MICROSTEPS_PER_HEMRO,
            //                                        nFrequency,
            //                                        nFrequency,
            //                                        RAMP_STEPS );
#else
            if ( std::abs( nStartDddDistance ) >= APPROACH_SPEED_DDD_DIFF )
            {
                m_dSimulationMotorSpeed = AGSA_FULL_SPEED;
            }
            else
            {
                m_dSimulationMotorSpeed = AGSA_APPROACH_SPEED;
            }
            m_timerSimulation.start();
#endif
            m_pLongDurationTest->triggerStepStateMachine( AgsaLongDurationTest::StepStateAction::AgsaStarted );
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void AgsaControl::stopAgsaControl()
{
    qInfo() << "stopAgsaControl()";

#ifdef TARGETBUILD
    if ( m_pAgsaStepperMotorDriver->isRunning() )
    {
        m_pAgsaStepperMotorDriver->stopMotor();
    }
#else
    m_timerSimulation.stop();
#endif

    stopWatchdog();
    m_nMovingMode = MovingMode::Idle;

    m_pLongDurationTest->triggerStepStateMachine( AgsaLongDurationTest::StepStateAction::AgsaStopped );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void AgsaControl::startWatchdog()
{
    m_timerProgress.start();
    m_timerProcess.start();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void AgsaControl::stopWatchdog()
{
    m_timerProgress.stop();
    m_timerProcess.stop();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

int AgsaControl::calcMoveToDddFrequency( const int nCurrentDddValue ) const
{
    int nFrequency = 0;

#ifdef TARGETBUILD
    int nDddDistance = std::abs( nCurrentDddValue - m_nTargetDddValue );

#ifdef LINEAR_DECELERATE_TEST
    if ( nDddDistance / DECELERATE_DDD <= ( runFrequency() - approachFrequency() ) / DECELERATE_STEPS )
    {
        int nPhase = nDddDistance / DECELERATE_DDD;

        nFrequency = approachFrequency() + DECELERATE_STEPS * nPhase;
    }
#endif

#ifdef SIMPLE_DIST_TEST
    if ( nDddDistance <= APPROACH_SPEED_DDD_DIST )
    {
        nFrequency = approachFrequency();
    }
    else if ( nDddDistance <= 2 * APPROACH_SPEED_DDD_DIST )
    {
        nFrequency = std::max( approachFrequency(), runFrequency() / 2 );
    }
    else
    {
        nFrequency = runFrequency();
    }
#endif

#else
    Q_UNUSED( nCurrentDddValue );
#endif

    qInfo() << "calcMoveToDddFrequency() " << nFrequency << "Hz";

    return nFrequency;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////
