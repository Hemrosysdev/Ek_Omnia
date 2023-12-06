///////////////////////////////////////////////////////////////////////////////
///
/// @file EspStepperMotorDriver.h
///
/// @brief Header file of class EspStepperMotorDriver.
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

#ifndef EspStepperMotorDriver_h
#define EspStepperMotorDriver_h

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

class EspStepperMotorDriver : public EspDeviceDriver
{
    Q_OBJECT

    Q_PROPERTY( bool running READ isRunning NOTIFY runningChanged )
    Q_PROPERTY( bool valid READ isValid NOTIFY validChanged )
    Q_PROPERTY( bool direction READ direction WRITE setDirection NOTIFY directionChanged )
    Q_PROPERTY( int steps READ steps WRITE setSteps NOTIFY stepsChanged )
    Q_PROPERTY( int startFrequency READ startFrequency WRITE setStartFrequency NOTIFY startFrequencyChanged )
    Q_PROPERTY( int runFrequency READ runFrequency WRITE setRunFrequency NOTIFY runFrequencyChanged )
    Q_PROPERTY( int rampFreqPerStep READ rampFreqPerStep WRITE setRampFreqPerStep NOTIFY rampFreqPerStepChanged )

public:

    explicit EspStepperMotorDriver( const EkxProtocol::DriverId u8DriverId,
                                    const QString &             strName );
    ~EspStepperMotorDriver() override;

    Q_INVOKABLE void startMotor();

    Q_INVOKABLE void startMotor( const bool bDirection,
                                 const int  nStartFrequency,
                                 const int  nRunFrequency,
                                 const int  nRampFreqPerStep );

    Q_INVOKABLE void startMotorSteps();

    Q_INVOKABLE void startMotorSteps( const bool bDirection,
                                      const int  nSteps,
                                      const int  nStartFrequency,
                                      const int  nRunFrequency,
                                      const int  nRampFreqPerStep );

    Q_INVOKABLE void stopMotor( void );

    bool isRunning( void ) const;

    bool isValid() const;

    bool isFaultPinActive( void ) const;

    void setDirection( const bool bDirection );

    bool direction() const;

    void setSteps( const int nSteps );

    int steps() const;

    void setStartFrequency( const int nFrequency );

    int startFrequency() const;

    void setRunFrequency( const int nFrequency );

    int runFrequency() const;

    void changeRunFrequency( const int nFrequency );

    void setRampFreqPerStep( const int nRampFreqPerStep );

    int rampFreqPerStep() const;

    void processMessageFrame( const EkxProtocol::MessageFrame & frame ) override;

    void processAlive( const bool bAlive ) override;

signals:

    void statusChanged();

    void runningChanged( const bool bIsRunning );

    void faultPinActiveChanged( const bool bIsActive );

    void validChanged( const bool bValid );

    void directionChanged( const bool bDirection );

    void stepsChanged( const int nSteps );

    void startFrequencyChanged( const int nFrequency );

    void runFrequencyChanged( const int nFrequency );

    void rampFreqPerStepChanged( const int nRampFreqPerStep );

private:

    void setRunning( const bool bRunning );

    void setFaultPinActive( const bool bActive );

    void setValid( const bool bValid );

private:

    QString m_strName;

    bool    m_bRunning { false };
    bool    m_bFaultPinActive { false };
    bool    m_bValid { false };

    bool    m_bDirection { false };
    int     m_nSteps { 1000 };
    int     m_nStartFrequency { 500 };
    int     m_nRunFrequency { 1000 };
    int     m_nRampFreqPerStep { 5 };

};

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#endif // EspStepperMotorDriver_h
