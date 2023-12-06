///////////////////////////////////////////////////////////////////////////////
///
/// @file EspDcHallMotorDriver.h
///
/// @brief Header file of class EspDcHallMotorDriver.
///
/// @author Ultratronik GmbH
///         Dornierstr. 9
///         D-82205 Gilching
///         Germany
///         http://www.ultratronik.de
///
/// @author written by Florian Sonntag, Forschung & Entwicklung, gesser@ultratronik.de
///
/// @date 30.10.2021
///
/// @copyright Copyright 2021 by Hemro International AG
///            Hemro International AG
///            Länggenstrasse 34
///            CH 8184 Bachenbülach
///            Switzerland
///            Homepage: www.hemrogroup.com
///
///////////////////////////////////////////////////////////////////////////////

#ifndef EspDcHallMotorDriver_h
#define EspDcHallMotorDriver_h

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#include "EspDeviceDriver.h"
#include "EkxProtocol.h"

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

class EspDcHallMotorDriver : public EspDeviceDriver
{
    Q_OBJECT

    Q_PROPERTY( bool running READ isRunning NOTIFY runningChanged )
    Q_PROPERTY( int runTime READ runTimeMs NOTIFY runTimeMsChanged )
    Q_PROPERTY( quint32 stepCounter READ stepCounter NOTIFY stepCounterChanged )
    Q_PROPERTY( quint32 overshootSteps READ overshootSteps NOTIFY overshootStepsChanged )
    Q_PROPERTY( bool valid READ isValid NOTIFY validChanged )
    Q_PROPERTY( bool motorTestOk READ isMotorTestOk NOTIFY motorTestChanged )
    Q_PROPERTY( int steps READ steps WRITE setSteps NOTIFY stepsChanged )
    Q_PROPERTY( int pwmDuty10th READ pwmDuty10th WRITE setPwmDuty10th NOTIFY pwmDuty10thChanged )
    Q_PROPERTY( int pwmFrequency READ pwmFrequency WRITE setPwmFrequency NOTIFY pwmFrequencyChanged )

public:

    explicit EspDcHallMotorDriver( const EkxProtocol::DriverId u8DriverId,
                                   const QString &             strName );
    virtual ~EspDcHallMotorDriver() override;

    Q_INVOKABLE void testMotor();

    Q_INVOKABLE void startMotor();

    Q_INVOKABLE void startMotor( const int nPwmDuty10th,
                                 const int nPwmFrequency );

    Q_INVOKABLE void startMotorSteps();

    Q_INVOKABLE void startMotorSteps( const int nSteps,
                                      const int nPwmDuty10th,
                                      const int nPwmFrequency );

    Q_INVOKABLE void stopMotor( void );

    bool isMotorTestOk( void ) const;

    bool isRunning( void ) const;

    bool isFaultPinActive( void ) const;

    int runTimeMs( void ) const;

    bool isValid() const;

    quint32 stepCounter( void ) const;

    quint32 overshootSteps( void ) const;

    void setSteps( const int nSteps );

    int steps() const;

    void setPwmDuty10th( const int nPwmDuty10th );

    int pwmDuty10th() const;

    void setPwmFrequency( const int nPwmFrequency );

    int pwmFrequency() const;

    void processMessageFrame( const EkxProtocol::MessageFrame & frame ) override;

    void processAlive( const bool bAlive ) override;

signals:

    void statusChanged();

    void motorTestChanged( const bool bOk );

    void runningChanged( const bool bRunning );

    void faultPinActiveChanged( const bool bActive );

    void runTimeMsChanged( const int nRunTimeMs );

    void stepCounterChanged( const quint32 u32StepCounter );

    void overshootStepsChanged( const quint32 u32OvershootSteps );

    void validChanged( const bool bValid );

    void stepsChanged( const int nSteps );

    void pwmDuty10thChanged( const int nPwmDuty10th );

    void pwmFrequencyChanged( const int nPwmFrequency );

#ifdef SIMULATION_BUILD

public:

#else

private:

#endif

    void setMotorTestOk( const bool bOk );

    void setValid( const bool bValid );

private:

    void setRunning( const bool bRunning );

    void setFaultPinActive( const bool bActive );

    void setRunTimeMs( const int nRunTimeMs );

    void setStepCounter( const quint32 u32StepCounter );

    void setOvershootSteps( const quint32 u32OvershootSteps );

private:

    QString m_strName;

    bool    m_bMotorTestOk { false };
    bool    m_bRunning { false };
    bool    m_bFaultPinActive { false };
    int     m_nRunTimeMs { 0 };
    bool    m_bValid { false };
    quint32 m_u32StepCounter { 0 };
    quint32 m_u32OvershootSteps { 0 };

    int     m_nSteps { 1000 };
    int     m_nPwmDuty10th { 500 };
    int     m_nPwmFrequency { 5000 };

};

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#endif // EspDcHallMotorDriver_h
