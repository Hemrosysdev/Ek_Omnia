///////////////////////////////////////////////////////////////////////////////
///
/// @file StartStopLightDriver.h
///
/// @brief Header file of class StartStopLightDriver.
///
/// @author Ultratronik GmbH
///         Dornierstr. 9
///         D-82205 Gilching
///         Germany
///         http://www.ultratronik.de
///
/// @author written by Gerd Esser, Forschung & Entwicklung, gesser@ultratronik.de
///
/// @date 01.02.2021
///
/// @copyright Copyright 2021 by Hemro International AG
///            Hemro International AG
///            Länggenstrasse 34
///            CH 8184 Bachenbülach
///            Switzerland
///            Homepage: www.hemrogroup.com
///
///////////////////////////////////////////////////////////////////////////////

#ifndef StartStopLightDriver_h
#define StartStopLightDriver_h

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#include <QObject>
#include <QTimer>

#include "EspDeviceDriver.h"
#include "EkxProtocol.h"

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

class StartStopLightDriver : public EspDeviceDriver
{
    Q_OBJECT

    Q_PROPERTY( int startStopDutyCycle READ startStopDutyCycle WRITE setStartStopDutyCycle RESET resetStartStopDutyCycle NOTIFY startStopDutyCycleChanged )

public:

    enum StartStopDutyCycle
    {
        StartStopDutyIdle = 100,
        StartStopDutyGrinding = 200
    };
    Q_ENUM( StartStopDutyCycle );

    enum class StandbyFaderState
    {
        Idle = 0,
        FadeIn,
        FadeOut,
        Pause,
    };
    Q_ENUM( StandbyFaderState );

public:

    explicit StartStopLightDriver( const EkxProtocol::DriverId u8DriverId );
    ~StartStopLightDriver() override;

    int startStopDutyCycle() const;

    void setStartStopDutyCycle( const int nPwmDuty );

    void resetStartStopDutyCycle();

    void startGrinding();

    void stopGrinding();

    void processMessageFrame( const EkxProtocol::MessageFrame & frame ) override;

    void processAlive( const bool bAlive ) override;

Q_SIGNALS:

    void startStopDutyCycleChanged( const int nPwmDuty );

public slots:

    void processStandbyChanged( const bool bStandbyActive );

    void updateStartStopPwm();

    void processStandbyFaderTimeout();

private:

    bool              m_bStandbyActive { false };
    int               m_nStartStopPwmDuty { StartStopDutyIdle };

    QTimer            m_timerStandbyFader;

    StandbyFaderState m_nStandbyFaderState { StandbyFaderState::Idle };

};

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#endif // StartStopLightDriver_h
