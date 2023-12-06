///////////////////////////////////////////////////////////////////////////////
///
/// @file SpotLightDriver.h
///
/// @brief Header file of class SpotLightDriver.
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

#ifndef SpotLightDriver_h
#define SpotLightDriver_h

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

class SpotLightDriver : public EspDeviceDriver
{
    Q_OBJECT

    Q_PROPERTY( int spotLightDutyCycle READ spotLightDutyCycle WRITE setSpotLightDutyCycle RESET resetSpotLightDutyCycle NOTIFY spotLightDutyCycleChanged )

public:

    enum SpotLightDutyCycle
    {
        SpotLightDutyIdle = 0,
        SpotLightDutyGrinding = 200
    };
    Q_ENUM( SpotLightDutyCycle );

    enum class StandbyFaderState
    {
        Idle = 0,
        FadeIn,
        FadeOut,
        Pause,
    };
    Q_ENUM( StandbyFaderState );

    enum class SpotLightState
    {
        Idle = 0,
        On,
        PostGrind,
        Standby
    };
    Q_ENUM( SpotLightState );

public:

    explicit SpotLightDriver( const EkxProtocol::DriverId u8DriverId );
    ~SpotLightDriver() override;

    int spotLightDutyCycle() const;

    void setSpotLightDutyCycle( const int nPwmDuty );

    void resetSpotLightDutyCycle();

    void setSpotLightState( const SpotLightState nState );

    void startGrinding();

    void stopGrinding();

    void processMessageFrame( const EkxProtocol::MessageFrame & frame ) override;

    void processAlive( const bool bAlive ) override;

Q_SIGNALS:

    void spotLightDutyCycleChanged( const int nPwmDuty );

public slots:

    void processShowSpotLightChanged( const bool bShow );

    void processStandbyChanged( const bool bStandbyActive );

    void processStandbyFaderTimeout();

    void processSpotLightPostGrindTimeout();

private:

    bool              m_bStandbyActive { false };
    int               m_nSpotLightPwmDuty { SpotLightDutyGrinding };

    QTimer            m_timerStandbyFader;

    QTimer            m_timerSpotLightPostGrind;

    StandbyFaderState m_nStandbyFaderState { StandbyFaderState::Idle };

    SpotLightState    m_nSpotLightState { SpotLightState::Idle };

    bool              m_bShowSpotLight { false };

};

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#endif // SpotLightDriver_h
