///////////////////////////////////////////////////////////////////////////////
///
/// @file AgsaLongDurationTest.cpp
///
/// @brief main application entry point of AgsaLongDurationTest.
///
/// @author Ultratronik GmbH
///         Dornierstr. 9
///         D-82205 Gilching
///         Germany
///         http://www.ultratronik.de
///
/// @author written by Florian Sonntag, Forschung & Entwicklung, gesser@ultratronik.de
///
/// @date 06.05.2022
///
/// @copyright Copyright 2022 by Hemro International AG
///            Hemro International AG
///            Länggenstrasse 34
///            CH 8184 Bachenbülach
///            Switzerland
///            Homepage: www.hemrogroup.com
///
///////////////////////////////////////////////////////////////////////////////

#include "AgsaLongDurationTest.h"

#include <QDebug>
#include <QFile>
#include <QDir>
#include <QDateTime>
#include <QProcess>

#include "EkxGlobals.h"
#include "AgsaLongDurationTest.h"
#include "AgsaControl.h"
#include "MainStatemachine.h"
#include "DddCouple.h"
#include "DddDriver.h"

#define MAX_ZIP_FILE_NUM       ( 100 )

#define LOG_DIRECTORY          "/log/HemroEkxAgsaLdtLogs"
#define ZIP_DIRECTORY          "/storage/HemroEkxAgsaLdtLogs"
#define MAX_CONTINUOUS_FAILS   ( 10 )

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

AgsaLongDurationTest::AgsaLongDurationTest( QObject * pParent )
    : QObject( pParent )
{
    MainStatemachine::ensureExistingDirectory( MainStatemachine::rootPath() + LOG_DIRECTORY + "/dummy" );
    MainStatemachine::ensureExistingDirectory( MainStatemachine::rootPath() + ZIP_DIRECTORY + "/dummy" );

    m_timerWatchdog.setInterval( 30000 );
    m_timerWatchdog.setSingleShot( false );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

AgsaLongDurationTest::~AgsaLongDurationTest()
{
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void AgsaLongDurationTest::create( const QString &     strDeviceSerialNo,
                                   AgsaControl * const pAgsaControl,
                                   DddCouple *         pDddCouple )
{
    m_strDeviceSerialNo = strDeviceSerialNo;
    m_pAgsaControl      = pAgsaControl;
    m_pDddCouple        = pDddCouple;

    connect( &m_timerWatchdog, &QTimer::timeout, this, &AgsaLongDurationTest::processWatchdogTimeout );

    // clear old fragments within log directory
    QProcess proc;
    proc.start( "rm", QStringList() << "-f" << QString( "%1/*" ).arg( LOG_DIRECTORY ) );
    proc.waitForFinished( 2000 );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

QString AgsaLongDurationTest::logFilename() const
{
    return QString( "HemroEkxAgsaLdtLog_%1_%2_%3.csv" )
           .arg( m_strDeviceSerialNo, m_dtTestStart.toString( "yyyyMMdd_hhmmss" ) )
           .arg( m_nSessionFileId, 2, 10, QLatin1Char( '0' ) );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

QString AgsaLongDurationTest::zipFilename() const
{
    return QString( "HemroEkxAgsaLdtLog_%1_%2_%3.zip" )
           .arg( m_strDeviceSerialNo, m_dtTestStart.toString( "yyyyMMdd_hhmmss" ) )
           .arg( m_nSessionFileId, 2, 10, QLatin1Char( '0' ) );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void AgsaLongDurationTest::removeOldLogFiles()
{
    QDir          dir( MainStatemachine::rootPath() + ZIP_DIRECTORY );
    QFileInfoList files = dir.entryInfoList( QDir::Files | QDir::NoDotAndDotDot, QDir::Time );

    for ( int index = files.size(); index > MAX_ZIP_FILE_NUM; --index )
    {
        const QFileInfo & info = files[index - 1];
        QFile::remove( info.absoluteFilePath() );
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void AgsaLongDurationTest::logEntry( const QString & strEntry,
                                     const bool      bTimestamp )
{
    if ( !m_bLoggingEnabled )
    {
        return;
    }

    QFile logFile( QString( MainStatemachine::rootPath() + LOG_DIRECTORY "/" + logFilename() ) );

    // compress log file if it gets to big (10M->zip to approx. 1M)
    if ( logFile.exists()
         && logFile.size() > 5000000 )
    {
        zipLogFile();

        m_nSessionFileId++;
        createLogFileHeaders();

        logFile.setFileName( QString( MainStatemachine::rootPath() + LOG_DIRECTORY "/" + logFilename() ) );
    }

    if ( logFile.open( QIODevice::WriteOnly | QIODevice::Append ) )
    {
        QTextStream stream( &logFile );

        if ( bTimestamp )
        {
            stream << QDateTime::currentDateTime().toString( "yyyy-MM-dd hh:mm:ss.zzz" ) << ";";
        }

        stream << strEntry << QT_ENDL;

        logFile.flush();
        logFile.close();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void AgsaLongDurationTest::logData( const QString & strRemarks )
{
    if ( isRunning() )
    {
        QString strEntry;
        strEntry = QString( "%1;%2;%3;%4;%5;%6;%7" )
                   .arg( totalCycle() )
                   .arg( stepCycle() )
                   .arg( m_pDddCouple->dddValue() )
                   .arg( m_pDddCouple->rawDddValue() )
                   .arg( m_nFails )
                   .arg( static_cast<int>( m_nStepState ) )
                   .arg( strRemarks );
        logEntry( strEntry );
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void AgsaLongDurationTest::moveToIntendedEnd( const StepState nState )
{
    switch ( nState )
    {
        case StepState::Init:
        case StepState::MoveToStart:
            if ( m_pDddCouple->dddValue() > ( m_nStepStopDddValue + m_nStepStartDddValue ) / 2 )
            {
                logData( "Move to start" );
                m_nStepState = nState;
                m_pAgsaControl->moveToDddValue( m_nStepStartDddValue );
            }
            else
            {
                logData( "Change intention-move to stop" );
                m_nStepState = StepState::MoveToStop;
                m_pAgsaControl->moveToDddValue( m_nStepStopDddValue );
            }
            break;

        case StepState::MoveToStop:
            if ( m_pDddCouple->dddValue() < ( m_nStepStopDddValue + m_nStepStartDddValue ) / 2 )
            {
                logData( "Move to stop" );
                m_nStepState = nState;
                m_pAgsaControl->moveToDddValue( m_nStepStopDddValue );
            }
            else
            {
                logData( "Change intention-move to start" );
                m_nStepState = StepState::MoveToStart;
                m_pAgsaControl->moveToDddValue( m_nStepStartDddValue );
            }
            break;

        case StepState::MoveForward:
            logData( "Move forward" );
            m_nStepState = nState;
            m_pAgsaControl->moveSteps( m_nTestSteps, m_pAgsaControl->startFrequency(), m_pAgsaControl->runFrequency(), true, true );
            break;

        case StepState::MoveBackward:
            logData( "Move backward" );
            m_nStepState = nState;
            m_pAgsaControl->moveSteps( -m_nTestSteps, m_pAgsaControl->startFrequency(), m_pAgsaControl->runFrequency(), true, true );
            break;

        default:
            // do nothing
            break;
    }

}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void AgsaLongDurationTest::startStressTest()
{
    if ( !isRunning() )
    {
        m_timerWatchdog.setInterval( 30000 );

        m_nSessionFileId = 1;
        m_dtTestStart    = QDateTime::currentDateTime();

        createLogFileHeaders();

        triggerMetaStateMachine( MetaStateAction::StartStressTest );
        emit runningChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void AgsaLongDurationTest::startManualTest()
{
    if ( !isRunning() )
    {
        m_timerWatchdog.setInterval( 30000 );

        m_nSessionFileId = 1;
        m_dtTestStart    = QDateTime::currentDateTime();

        createLogFileHeaders();

        triggerMetaStateMachine( MetaStateAction::StartManualTest );
        emit runningChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void AgsaLongDurationTest::startStepsTest()
{
    if ( !isRunning() )
    {
        m_timerWatchdog.setInterval( 1000 * m_nTestSteps * 2 / m_pAgsaControl->runFrequency() );

        m_nSessionFileId = 1;
        m_dtTestStart    = QDateTime::currentDateTime();

        m_pAgsaControl->setTestBlockageDetect( false );

        createLogFileHeaders();

        triggerMetaStateMachine( MetaStateAction::StartStepsTest );
        emit runningChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool AgsaLongDurationTest::startMetaTestStep( const int nStartDddValue,
                                              const int nStopDddValue,
                                              const int nCycleNum )
{
    qInfo() << "startMetaTestStep" << nStartDddValue << nStopDddValue << nCycleNum << m_nTestSteps;
    m_timerWatchdog.start();

    m_nStepStartDddValue = nStartDddValue;
    m_nStepStopDddValue  = nStopDddValue;
    m_nStepCyclesNum     = nCycleNum;

    m_nStepState = StepState::Idle;
    triggerStepStateMachine( StepStateAction::Started );

    return true;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void AgsaLongDurationTest::stopTest()
{
    if ( isRunning() )
    {
        m_nMetaState = MetaState::Idle;
        m_nStepState = StepState::Idle;
        emit runningChanged();

        logData( "Test stopped" );

        zipLogFile();

        m_pAgsaControl->stopAgsaControl();
        m_timerWatchdog.stop();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void AgsaLongDurationTest::userStopTest()
{
    logData( "Test stopped by user" );
    stopTest();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void AgsaLongDurationTest::createLogFileHeaders()
{
    QString     strEntry;
    QTextStream streamEntry( &strEntry );

    streamEntry << "Time;Cycles;Start DDD;Stop DDD;Speed [Hz];Approach [Hz];Serial No.;Session File ID";
    logEntry( strEntry, false );

    strEntry.clear();
    streamEntry << m_nStepCyclesNum << ";" << m_nStepStartDddValue << ";" << m_nStepStopDddValue << ";" << m_pAgsaControl->runFrequency() << ";" << m_pAgsaControl->approachFrequency() << ";" << m_strDeviceSerialNo << ";" << m_nSessionFileId << QT_ENDL;
    logEntry( strEntry );

    strEntry.clear();
    streamEntry << "Time;TotalCycle;StepCycle;DDD;Raw DDD;Fails;State;Remarks";
    logEntry( strEntry, false );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool AgsaLongDurationTest::isRunning() const
{
    return m_nMetaState != MetaState::Idle;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void AgsaLongDurationTest::setLoggingEnabled( bool bEnabled )
{
    if ( m_bLoggingEnabled != bEnabled )
    {
        m_bLoggingEnabled = bEnabled;
        Q_EMIT loggingEnabledChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool AgsaLongDurationTest::loggingEnabled() const
{
    return m_bLoggingEnabled;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void AgsaLongDurationTest::setManualStartDddValue( const int nDddValue )
{
    if ( m_nManualStartDddValue != nDddValue )
    {
        if ( nDddValue >= 0
             && nDddValue <= MAX_HEMRO_SCALE )
        {
            m_nManualStartDddValue = nDddValue;
            emit manualStartDddValueChanged();

            if ( m_nManualStartDddValue > m_nManualStopDddValue )
            {
                setManualStopDddValue( m_nManualStartDddValue );
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

int AgsaLongDurationTest::manualStartDddValue() const
{
    return m_nManualStartDddValue;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void AgsaLongDurationTest::setManualStopDddValue( const int nDddValue )
{
    if ( m_nManualStopDddValue != nDddValue )
    {
        if ( nDddValue >= 0
             && nDddValue <= MAX_HEMRO_SCALE )
        {
            m_nManualStopDddValue = nDddValue;
            emit manualStopDddValueChanged();

            if ( m_nManualStartDddValue > m_nManualStopDddValue )
            {
                setManualStartDddValue( m_nManualStopDddValue );
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

int AgsaLongDurationTest::manualStopDddValue() const
{
    return m_nManualStopDddValue;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void AgsaLongDurationTest::setManualCyclesNum( const int nCyclesNum )
{
    if ( m_nManualCyclesNum != nCyclesNum )
    {
        m_nManualCyclesNum = nCyclesNum;
        emit manualCyclesNumChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

int AgsaLongDurationTest::manualCyclesNum() const
{
    return m_nManualCyclesNum;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void AgsaLongDurationTest::setTestSteps( const int nTestSteps )
{
    if ( m_nTestSteps != nTestSteps )
    {
        m_nTestSteps = nTestSteps;
        emit testStepsChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

int AgsaLongDurationTest::testSteps() const
{
    return m_nTestSteps;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void AgsaLongDurationTest::setStepCycle( const int nCycle )
{
    if ( m_nStepCycle != nCycle )
    {
        m_nStepCycle = nCycle;
        emit stepCycleChanged();

        switch ( m_nMetaState )
        {
            case MetaState::StressStep0to200:
                setStressStep0to200Cycle( nCycle );
                break;

            case MetaState::StressStep200to400:
                setStressStep200to400Cycle( nCycle );
                break;

            case MetaState::StressStep400to600:
                setStressStep400to600Cycle( nCycle );
                break;

            case MetaState::StressStep600to800:
                setStressStep600to800Cycle( nCycle );
                break;

            default:
                // nothing to do
                break;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

int AgsaLongDurationTest::stepCycle() const
{
    return m_nStepCycle;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void AgsaLongDurationTest::setTotalCycle( const int nCycle )
{
    if ( m_nTotalCycle != nCycle )
    {
        m_nTotalCycle = nCycle;
        emit totalCycleChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

int AgsaLongDurationTest::totalCycle() const
{
    return m_nTotalCycle;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void AgsaLongDurationTest::setStressStep0to200Cycle( const int nCycle )
{
    if ( m_nStressStep0to200Cycle != nCycle )
    {
        m_nStressStep0to200Cycle = nCycle;
        emit stressStep0to200CycleChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

int AgsaLongDurationTest::stressStep0to200Cycle() const
{
    return m_nStressStep0to200Cycle;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void AgsaLongDurationTest::setStressStep200to400Cycle( const int nCycle )
{
    if ( m_nStressStep200to400Cycle != nCycle )
    {
        m_nStressStep200to400Cycle = nCycle;
        emit stressStep200to400CycleChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

int AgsaLongDurationTest::stressStep200to400Cycle() const
{
    return m_nStressStep200to400Cycle;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void AgsaLongDurationTest::setStressStep400to600Cycle( const int nCycle )
{
    if ( m_nStressStep400to600Cycle != nCycle )
    {
        m_nStressStep400to600Cycle = nCycle;
        emit stressStep400to600CycleChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

int AgsaLongDurationTest::stressStep400to600Cycle() const
{
    return m_nStressStep400to600Cycle;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void AgsaLongDurationTest::setStressStep600to800Cycle( const int nCycle )
{
    if ( m_nStressStep600to800Cycle != nCycle )
    {
        m_nStressStep600to800Cycle = nCycle;
        emit stressStep600to800CycleChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

int AgsaLongDurationTest::stressStep600to800Cycle() const
{
    return m_nStressStep600to800Cycle;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void AgsaLongDurationTest::triggerStepStateMachine( const StepStateAction nAction )
{
    m_timerWatchdog.start();

    //qCritical() << "triggerStepStateMachine state" << m_nStepState << "action" << nAction;

    switch ( m_nStepState )
    {
        case StepState::Idle:
            switch ( nAction )
            {
                case StepStateAction::Started:
                    m_nFails           = 0;
                    m_nContinuousFails = 0;
                    setStepCycle( 0 );

                    if ( m_nMetaState == MetaState::StepsTest )
                    {
                        if ( m_nTestSteps == 0 )
                        {
                            logData( "Illegal start parameters (steps num)" );
                            stopTest();
                        }
                        else
                        {
                            moveToIntendedEnd( StepState::MoveForward );
                        }
                    }
                    else if ( m_nStepStartDddValue == m_nStepStopDddValue
                              && m_nStepCyclesNum <= 0 )
                    {
                        logData( "Illegal start parameters" );
                        stopTest();
                    }
                    else
                    {
                        moveToIntendedEnd( StepState::Init );
                    }
                    break;

                default:
                    // do nothing
                    //qDebug() << "triggerStepStateMachine: stalled in Idle, action" << nAction;
                    break;
            }
            break;

        case StepState::Init:
            // moves to stop position without counting this cyle
            switch ( nAction )
            {
                case StepStateAction::AgsaStopped:
                    if ( !m_pAgsaControl->isFailed() )
                    {
                        m_nContinuousFails = 0;
                    }

                    if ( stepCycle() < m_nStepCyclesNum )
                    {
                        moveToIntendedEnd( StepState::MoveToStop );
                    }
                    else
                    {
                        triggerMetaStateMachine( MetaStateAction::NextTestStep );
                    }
                    break;

                case StepStateAction::WatchdogTimeout:
                    m_nFails++;
                    m_nContinuousFails++;

                    if ( m_nContinuousFails < MAX_CONTINUOUS_FAILS )
                    {
                        moveToIntendedEnd( StepState::MoveToStop );
                    }
                    break;

                case StepStateAction::AgsaFailed:
                    m_nFails++;
                    m_nContinuousFails++;

                    if ( stepCycle() < m_nStepCyclesNum )
                    {
                        if ( m_nContinuousFails < MAX_CONTINUOUS_FAILS )
                        {
                            logData( "Failed before reaching start" );
                            //m_nState = State::MoveToStop;
                            //m_pAgsaControl->moveToDddValue( m_nStopDddValue );
                        }
                    }
                    else
                    {
                        logData( "AGSA failed" );
                        stopTest();
                    }
                    break;

                case StepStateAction::Started:
                case StepStateAction::AgsaStarted:
                case StepStateAction::MotorStopped:
                default:
                    // do nothing
                    //qDebug() << "triggerStepStateMachine: stalled in Init, action" << nAction;
                    break;
            }
            break;

        case StepState::MoveToStart:
            switch ( nAction )
            {
                case StepStateAction::AgsaStopped:
                    if ( !m_pAgsaControl->isFailed() )
                    {
                        m_nContinuousFails = 0;
                    }
                    setStepCycle( stepCycle() + 1 );

                    if ( stepCycle() < m_nStepCyclesNum )
                    {
                        logData( "Reaching start" );
                        moveToIntendedEnd( StepState::MoveToStop );
                    }
                    else
                    {
                        m_nStepState = StepState::Idle;
                        triggerMetaStateMachine( MetaStateAction::NextTestStep );
                    }
                    break;

                case StepStateAction::WatchdogTimeout:
                    m_nFails++;
                    m_nContinuousFails++;

                    if ( m_nContinuousFails < MAX_CONTINUOUS_FAILS )
                    {
                        moveToIntendedEnd( StepState::MoveToStop );
                    }
                    break;

                case StepStateAction::AgsaFailed:
                    m_nFails++;
                    m_nContinuousFails++;

                    if ( stepCycle() < m_nStepCyclesNum )
                    {
                        logData( "Failed before reaching start" );
                        //m_nState = State::MoveToStop;
                        //m_pAgsaControl->moveToDddValue( m_nStopDddValue );
                    }
                    else
                    {
                        logData( "AGSA Failed" );
                        stopTest();
                    }
                    break;

                case StepStateAction::Started:
                case StepStateAction::AgsaStarted:
                case StepStateAction::MotorStopped:
                default:
                    // do nothing
                    //qDebug() << "triggerStepStateMachine: stalled in MoveToStart, action" << nAction;
                    break;
            }
            break;

        case StepState::MoveToStop:
            switch ( nAction )
            {
                case StepStateAction::AgsaStopped:
                    if ( !m_pAgsaControl->isFailed() )
                    {
                        m_nContinuousFails = 0;
                    }

                    logData( "Reaching stop" );
                    moveToIntendedEnd( StepState::MoveToStart );
                    break;

                case StepStateAction::WatchdogTimeout:
                    m_nFails++;
                    m_nContinuousFails++;

                    if ( m_nContinuousFails < MAX_CONTINUOUS_FAILS )
                    {
                        moveToIntendedEnd( StepState::MoveToStart );
                    }
                    break;

                case StepStateAction::AgsaFailed:
                    m_nFails++;
                    m_nContinuousFails++;

                    if ( m_nContinuousFails < MAX_CONTINUOUS_FAILS )
                    {
                        logData( "Failed before reaching stop" );
                        //m_nState = State::MoveToStart;
                        //m_pAgsaControl->moveToDddValue( m_nStartDddValue );
                    }
                    break;

                case StepStateAction::Started:
                case StepStateAction::AgsaStarted:
                case StepStateAction::MotorStopped:
                default:
                    // do nothing
                    //qDebug() << "triggerStepStateMachine: stalled in MoveToStop, action" << nAction;
                    break;
            }
            break;

        case StepState::MoveForward:
            switch ( nAction )
            {
                case StepStateAction::WatchdogTimeout:
                    moveToIntendedEnd( StepState::MoveBackward );
                    break;

                case StepStateAction::MotorStopped:
                case StepStateAction::AgsaStopped:
                case StepStateAction::AgsaFailed:
                case StepStateAction::Started:
                case StepStateAction::AgsaStarted:
                default:
                    // do nothing
                    break;
            }
            break;

        case StepState::MoveBackward:
            switch ( nAction )
            {
                case StepStateAction::WatchdogTimeout:
                    setTotalCycle( totalCycle() + 1 );
                    moveToIntendedEnd( StepState::MoveForward );
                    break;

                case StepStateAction::MotorStopped:
                case StepStateAction::AgsaStopped:
                case StepStateAction::AgsaFailed:
                case StepStateAction::Started:
                case StepStateAction::AgsaStarted:
                default:
                    // do nothing
                    break;
            }
            break;
    }

    if ( m_nContinuousFails >= MAX_CONTINUOUS_FAILS )
    {
        m_nContinuousFails = 0;
        logData( "Continuous failures/Test aborted" );
        stopTest();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void AgsaLongDurationTest::triggerMetaStateMachine( const MetaStateAction nAction )
{
    //qCritical() << "triggerMetaStateMachine state" << m_nMetaState << "action" << nAction;

    const bool bOldIsRunning = isRunning();

    switch ( m_nMetaState )
    {
        case MetaState::Idle:
            switch ( nAction )
            {
                case MetaStateAction::StartManualTest:
                    m_nMetaState = MetaState::Manual;

                    setTotalCycle( 0 );
                    setStressStep0to200Cycle( 0 );
                    setStressStep200to400Cycle( 0 );
                    setStressStep400to600Cycle( 0 );
                    setStressStep600to800Cycle( 0 );

                    startMetaTestStep( m_nManualStartDddValue,
                                       m_nManualStopDddValue,
                                       m_nManualCyclesNum );
                    break;

                case MetaStateAction::StartStepsTest:
                    m_nMetaState = MetaState::StepsTest;

                    setTotalCycle( 0 );
                    setStressStep0to200Cycle( 0 );
                    setStressStep200to400Cycle( 0 );
                    setStressStep400to600Cycle( 0 );
                    setStressStep600to800Cycle( 0 );

                    startMetaTestStep( m_nManualStartDddValue,
                                       m_nManualStopDddValue,
                                       m_nManualCyclesNum );
                    break;

                case MetaStateAction::StartStressTest:
                    m_nMetaState = MetaState::StressStep0to200;

                    setTotalCycle( 0 );
                    setStressStep0to200Cycle( 0 );
                    setStressStep200to400Cycle( 0 );
                    setStressStep400to600Cycle( 0 );
                    setStressStep600to800Cycle( 0 );

                    startMetaTestStep( 0, 210, c_nStressTestStepCycleNum );
                    break;

                default:
                    // do nothing
                    break;
            }
            break;

        case MetaState::Manual:
            switch ( nAction )
            {
                case MetaStateAction::NextTestStep:
                    m_nMetaState = MetaState::Idle;
                    stopTest();
                    break;

                default:
                    m_nMetaState = MetaState::Idle;
                    stopTest();
                    break;
            }
            break;

        case MetaState::StepsTest:
            switch ( nAction )
            {
                case MetaStateAction::NextTestStep:
                    m_nMetaState = MetaState::Idle;
                    stopTest();
                    break;

                default:
                    m_nMetaState = MetaState::Idle;
                    stopTest();
                    break;
            }
            break;

        case MetaState::StressStep0to200:
            switch ( nAction )
            {
                case MetaStateAction::NextTestStep:
                    m_nMetaState = MetaState::StressStep200to400;
                    startMetaTestStep( 190, 410, c_nStressTestStepCycleNum );
                    break;

                default:
                    m_nMetaState = MetaState::Idle;
                    stopTest();
                    break;
            }
            break;

        case MetaState::StressStep200to400:
            switch ( nAction )
            {
                case MetaStateAction::NextTestStep:
                    m_nMetaState = MetaState::StressStep400to600;
                    startMetaTestStep( 390, 610, c_nStressTestStepCycleNum );
                    break;

                default:
                    m_nMetaState = MetaState::Idle;
                    stopTest();
                    break;
            }
            break;

        case MetaState::StressStep400to600:
            switch ( nAction )
            {
                case MetaStateAction::NextTestStep:
                    m_nMetaState = MetaState::StressStep600to800;
                    startMetaTestStep( 590, 800, c_nStressTestStepCycleNum );
                    break;

                default:
                    m_nMetaState = MetaState::Idle;
                    stopTest();
                    break;
            }
            break;

        case MetaState::StressStep600to800:
            switch ( nAction )
            {
                case MetaStateAction::NextTestStep:
                    m_nMetaState = MetaState::StressStep0to800;
                    startMetaTestStep( 0, 800, 1 );
                    break;

                default:
                    m_nMetaState = MetaState::Idle;
                    stopTest();
                    break;
            }
            break;

        case MetaState::StressStep0to800:
            switch ( nAction )
            {
                case MetaStateAction::NextTestStep:
                    m_nMetaState = MetaState::StressStep0to200;
                    setTotalCycle( totalCycle() + 1 );
                    setStressStep0to200Cycle( 0 );
                    setStressStep200to400Cycle( 0 );
                    setStressStep400to600Cycle( 0 );
                    setStressStep600to800Cycle( 0 );

                    startMetaTestStep( 0, 210, c_nStressTestStepCycleNum );
                    break;

                default:
                    m_nMetaState = MetaState::Idle;
                    stopTest();
                    break;
            }
            break;

        default:
            break;
    }

    if ( bOldIsRunning != isRunning() )
    {
        Q_EMIT runningChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void AgsaLongDurationTest::processDddValue()
{
    logData( "" );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void AgsaLongDurationTest::processWatchdogTimeout()
{
    logData( "Watchdog timeout" );
    triggerStepStateMachine( StepStateAction::WatchdogTimeout );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void AgsaLongDurationTest::processMotorRunningChanged( const bool bIsRunning )
{
    qInfo() << "AgsaLongDurationTest::processMotorRunningChanged()";
    if ( !bIsRunning )
    {
        triggerStepStateMachine( StepStateAction::MotorStopped );
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void AgsaLongDurationTest::zipLogFile()
{
    if ( !m_bLoggingEnabled )
    {
        return;
    }

    removeOldLogFiles();

    QString strLogFile = QString( MainStatemachine::rootPath() + LOG_DIRECTORY "/" + logFilename() );
    QString strZipFile = QString( MainStatemachine::rootPath() + ZIP_DIRECTORY "/" + zipFilename() );

    QProcess proc;
    proc.startDetached( "/usr/bin/zip", QStringList() << "-Dm" << strZipFile << strLogFile );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////
