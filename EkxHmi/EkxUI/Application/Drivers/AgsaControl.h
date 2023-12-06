///////////////////////////////////////////////////////////////////////////////
///
/// @file AgsaControl.h
///
/// @brief Header file of class AgsaControl.
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

#ifndef AgsaControl_h
#define AgsaControl_h

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#include <QTimer>

class AgsaLongDurationTest;
class DddCouple;
class EspStepperMotorDriver;

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

class AgsaControl : public QObject
{
    Q_OBJECT

    Q_PROPERTY( bool testLiveMode READ testLiveMode WRITE setTestLiveMode NOTIFY testLiveModeChanged )
    Q_PROPERTY( bool testBlockageDetect READ testBlockageDetect WRITE setTestBlockageDetect NOTIFY testBlockageDetectChanged )
    Q_PROPERTY( bool testMode READ testMode WRITE setTestMode NOTIFY testModeChanged )
    Q_PROPERTY( bool failed READ isFailed NOTIFY failedChanged )
    Q_PROPERTY( int targetDddValue READ targetDddValue WRITE setTargetDddValue NOTIFY targetDddValueChanged )
    Q_PROPERTY( int steps READ steps WRITE setSteps NOTIFY stepsChanged )
    Q_PROPERTY( int startFrequency READ startFrequency WRITE setStartFrequency NOTIFY startFrequencyChanged )
    Q_PROPERTY( int runFrequency READ runFrequency WRITE setRunFrequency NOTIFY runFrequencyChanged )
    Q_PROPERTY( int approachFrequency READ approachFrequency WRITE setApproachFrequency NOTIFY approachFrequencyChanged )

public:

    enum class MovingMode
    {
        Idle = 0,
        MoveToDdd,
        MoveSteps
    };
    Q_ENUM( MovingMode );

public:

    explicit AgsaControl( QObject * parent = nullptr );
    ~AgsaControl() override;

    void create( const QString &               strDeviceSerialNo,
                 EspStepperMotorDriver * const pAgsaStepperMotorDriver,
                 DddCouple *                   pDddCouple );

    void setTestMode( const bool bEnable );

    bool testMode() const;

    void setTestLiveMode( const bool bEnable );

    bool testLiveMode() const;

    void setTestBlockageDetect( const bool bEnable );

    bool testBlockageDetect() const;

    Q_INVOKABLE void stopAgsaControl();

    Q_INVOKABLE void moveSteps( const int  nSteps,
                                const int  nStartFrequency,
                                const int  nRunFrequency,
                                const bool bIgnoreDddFailed,
                                const bool bNoWatchdog );

    Q_INVOKABLE void moveToDddValue( const int nDddValue );

    Q_INVOKABLE void moveStepsToDddValue( const int nDddValue );

    Q_INVOKABLE void incrementTargetDddValue();

    Q_INVOKABLE void decrementTargetDddValue();

    void setTargetDddValue( const int nTargetDddValue );

    int targetDddValue() const;

    bool isFailed() const;

    void setFailedTimeout( const bool bFailed );

    bool isFailedTimeout() const;

    void setFailedBlockage( const bool bFailed );

    bool isFailedBlockage() const;

    void setFailedNoDdd( const bool bFailed );

    bool isFailedNoDdd() const;

    void setSteps( const int nSteps );

    int steps() const;

    void setStartFrequency( const int nFrequency );

    int startFrequency() const;

    void setRunFrequency( const int nFrequency );

    int runFrequency() const;

    void setApproachFrequency( const int nFrequency );

    int approachFrequency() const;

    AgsaLongDurationTest * longDurationTest();

signals:

    void testModeChanged();

    void testLiveModeChanged();

    void testBlockageDetectChanged();

    void targetDddValueChanged();

    void failedChanged();

    void stepsChanged();

    void startFrequencyChanged();

    void runFrequencyChanged();

    void approachFrequencyChanged();

public slots:

    void processDddValue();

    void processRawDddValue();

    void processDddFailed();

    void processProgressTimeout();

    void processProcessTimeout();

    void processRunningChanged( const bool bIsRunning );

#ifndef TARGETBUILD
    void processSimulationTimeout();

#endif

private:

    void startDddTargetController();

    void startWatchdog();

    void stopWatchdog();

    int calcMoveToDddFrequency( const int nCurrentDddValue ) const;

private:

    EspStepperMotorDriver * m_pAgsaStepperMotorDriver { nullptr };

    DddCouple *             m_pDddCouple { nullptr };

    AgsaLongDurationTest *  m_pLongDurationTest { nullptr };

    bool                    m_bTestMode { false };

    bool                    m_bTestLiveMode { false };

    bool                    m_bTestBlockageDetect { true };

    int                     m_nTargetDddValue { 0 };

    int                     m_nLastProgressDddValue { 0 };

    int                     m_nLastUpdateDddValue { 0 };

    int                     m_nCurrentDddValue { 0 };

    int                     m_nCurrentRawDddValue { 0 };

    int                     m_nBlockageCounter { 0 };

    bool                    m_bDddFailed { false };

    bool                    m_bFailedTimeout { false };

    bool                    m_bFailedBlockage { false };

    bool                    m_bFailedNoDdd { false };

    MovingMode              m_nMovingMode { MovingMode::Idle };

    QTimer                  m_timerProgress;

    QTimer                  m_timerProcess;

#ifndef TARGETBUILD
    QTimer m_timerSimulation;

    double m_dSimulationMotorSpeed { 0.0 };
#endif

    int m_nSteps { 1000 };
    int m_nStartFrequency { 4000 };
    int m_nRunFrequency { 10000 };
    int m_nApproachFrequency { 300 };

};

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#endif // AgsaControl_h
