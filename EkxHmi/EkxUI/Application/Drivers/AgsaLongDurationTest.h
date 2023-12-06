///////////////////////////////////////////////////////////////////////////////
///
/// @file AgsaLongDurationTest.h
///
/// @brief Header file of class AgsaLongDurationTest.
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

#ifndef AgsaLongDurationTest_h
#define AgsaLongDurationTest_h

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#include <QObject>
#include <QDateTime>
#include <QTimer>

class AgsaControl;
class DddCouple;

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

class AgsaLongDurationTest : public QObject
{
    Q_OBJECT

    Q_PROPERTY( bool loggingEnabled READ loggingEnabled WRITE setLoggingEnabled NOTIFY loggingEnabledChanged )
    Q_PROPERTY( int manualStartDddValue READ manualStartDddValue WRITE setManualStartDddValue NOTIFY manualStartDddValueChanged )
    Q_PROPERTY( int manualStopDddValue READ manualStopDddValue WRITE setManualStopDddValue NOTIFY manualStopDddValueChanged )
    Q_PROPERTY( int manualCyclesNum READ manualCyclesNum WRITE setManualCyclesNum NOTIFY manualCyclesNumChanged )
    Q_PROPERTY( int testSteps READ testSteps WRITE setTestSteps NOTIFY testStepsChanged )
    Q_PROPERTY( int stepCycle READ stepCycle NOTIFY stepCycleChanged )
    Q_PROPERTY( int totalCycle READ totalCycle NOTIFY totalCycleChanged )
    Q_PROPERTY( int stressStep0to200Cycle READ stressStep0to200Cycle NOTIFY stressStep0to200CycleChanged )
    Q_PROPERTY( int stressStep200to400Cycle READ stressStep200to400Cycle NOTIFY stressStep200to400CycleChanged )
    Q_PROPERTY( int stressStep400to600Cycle READ stressStep400to600Cycle NOTIFY stressStep400to600CycleChanged )
    Q_PROPERTY( int stressStep600to800Cycle READ stressStep600to800Cycle NOTIFY stressStep600to800CycleChanged )
    Q_PROPERTY( bool running READ isRunning NOTIFY runningChanged )

public:

    enum class StepStateAction
    {
        None = 0,
        Started,
        AgsaStarted,
        AgsaStopped,
        AgsaFailed,
        WatchdogTimeout,
        MotorStopped,
    };
    Q_ENUM( StepStateAction );

    enum class MetaStateAction
    {
        StartManualTest = 0,
        StartStressTest,
        StartStepsTest,
        NextTestStep
    };
    Q_ENUM( MetaStateAction );

    enum class StepState
    {
        Idle = 0,
        Init,
        MoveToStart,
        MoveToStop,
        MoveForward,
        MoveBackward
    };
    Q_ENUM( StepState );

    enum class MetaState
    {
        Idle = 0,
        Manual,
        StepsTest,
        StressStep0to200,
        StressStep200to400,
        StressStep400to600,
        StressStep600to800,
        StressStep0to800,
    };
    Q_ENUM( MetaState );

private:

    static constexpr int c_nStressTestStepCycleNum = 200;

public:

    explicit AgsaLongDurationTest( QObject * parent = nullptr );
    ~AgsaLongDurationTest() override;

    void create( const QString &     strDeviceSerialNo,
                 AgsaControl * const pAgsaControl,
                 DddCouple *         pDddCouple );

    Q_INVOKABLE void startStressTest();

    Q_INVOKABLE void startManualTest();

    Q_INVOKABLE void startStepsTest();

    Q_INVOKABLE void userStopTest();

    void stopTest();

    bool isRunning() const;

    bool loggingEnabled() const;

    void setLoggingEnabled( bool bEnabled );

    void setManualStartDddValue( const int nDddValue );

    int manualStartDddValue() const;

    void setManualStopDddValue( const int nDddValue );

    int manualStopDddValue() const;

    void setManualCyclesNum( const int nCyclesNum );

    int manualCyclesNum() const;

    void setTestSteps( const int nTestSteps );

    int testSteps() const;

    void setStepCycle( const int nCycle );

    int stepCycle() const;

    void setTotalCycle( const int nCycle );

    int totalCycle() const;

    void setStressStep0to200Cycle( const int nCycle );

    int stressStep0to200Cycle() const;

    void setStressStep200to400Cycle( const int nCycle );

    int stressStep200to400Cycle() const;

    void setStressStep400to600Cycle( const int nCycle );

    int stressStep400to600Cycle() const;

    void setStressStep600to800Cycle( const int nCycle );

    int stressStep600to800Cycle() const;

    void triggerStepStateMachine( const StepStateAction nAction );

    void triggerMetaStateMachine( const MetaStateAction nAction );

signals:

    void loggingEnabledChanged();

    void manualStartDddValueChanged();

    void manualStopDddValueChanged();

    void manualCyclesNumChanged();

    void runningChanged();

    void stepCycleChanged();

    void totalCycleChanged();

    void stressStep0to200CycleChanged();

    void stressStep200to400CycleChanged();

    void stressStep400to600CycleChanged();

    void stressStep600to800CycleChanged();

    void testStepsChanged();

public slots:

    void processDddValue();

    void processWatchdogTimeout();

    void processMotorRunningChanged( const bool bIsRunning );

private:

    bool startMetaTestStep( const int nStartDddValue,
                            const int nStopDddValue,
                            const int nCycleNum );

    void createLogFileHeaders();

    QString logFilename() const;

    QString zipFilename() const;

    void removeOldLogFiles();

    void logEntry( const QString & strEntry,
                   const bool      bTimestamp = true );

    void logData( const QString & strRemarks );

    void moveToIntendedEnd( const StepState nState );

    void zipLogFile();

private:

    AgsaControl * m_pAgsaControl { nullptr };
    DddCouple *   m_pDddCouple { nullptr };

    QString       m_strDeviceSerialNo;

    bool          m_bLoggingEnabled { true };

    int           m_nManualStartDddValue { 0 };
    int           m_nManualStopDddValue { 0 };
    int           m_nManualCyclesNum { 0 };

    int           m_nTestSteps { 0 };

    int           m_nStepStartDddValue { 0 };
    int           m_nStepStopDddValue { 0 };
    int           m_nStepCyclesNum { 0 };

    int           m_nStepCycle { 0 };
    int           m_nTotalCycle { 0 };

    int           m_nStressStep0to200Cycle { 0 };
    int           m_nStressStep200to400Cycle { 0 };
    int           m_nStressStep400to600Cycle { 0 };
    int           m_nStressStep600to800Cycle { 0 };

    int           m_nMetaCyclesNum { 1 };

    StepState     m_nStepState { StepState::Idle };

    MetaState     m_nMetaState { MetaState::Idle };

    QDateTime     m_dtTestStart;

    int           m_nFails { 0 };

    int           m_nContinuousFails { 0 };

    int           m_nSessionFileId { 0 };

    QTimer        m_timerWatchdog;
};

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#endif // AgsaLongDurationTest_h
