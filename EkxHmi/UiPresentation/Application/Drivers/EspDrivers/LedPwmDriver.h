///////////////////////////////////////////////////////////////////////////////
///
/// @file LedPwmDriver.h
///
/// @brief Header file of class LedPwmDriver.
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

#ifndef LedPwmDriver_h
#define LedPwmDriver_h

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#include <QObject>
#include <QTimer>

class EspPwmOut;

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

class LedPwmDriver : public QObject
{
    Q_OBJECT

public:

    enum StartStopDutyCycle
    {
        StartStopIdle = 100,
        StartStopGrinding = 200
    };

    enum SpotlightDutyCycle
    {
        SpotlightIdle = 0,
        SpotlightGrinding = 200
    };

private:

    enum FaderState
    {
        Idle = 0,
        FadeIn,
        FadeOut,
        Pause,
    };

public:

    explicit LedPwmDriver( QObject *parent = 0 );

    virtual ~LedPwmDriver( ) override;

    void create();

    void connectStartStopInterface( EspPwmOut* pEspPwmOut );
    void disconnectStartStopInterface();
    void setStartStopPwm( const int nPwmDuty );

    void connectSpotlightInterface( EspPwmOut* pEspPwmOut );
    void disconnectSpotlightInterface();
    void setSpotlightPwm( const int nPwmDuty );

public slots:

    void processStandbyChanged( const bool bStandbyActive );
    void updateSpotlightPwm();
    void updateStartStopPwm();
    void processFaderTimeout();

private:

    EspPwmOut *  m_pEspPwmOutStartStop { nullptr };
    EspPwmOut *  m_pEspPwmOutSpotlight { nullptr };
    bool         m_bStandbyActive { false };
    int          m_nStartStopPwmDuty { StartStopIdle };
    int          m_nSpotlightPwmDuty { SpotlightIdle };

    QTimer       m_timerFader;

    FaderState   m_nFaderState { Idle };

};

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#endif // LedPwmDriver_h
