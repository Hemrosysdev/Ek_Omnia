///////////////////////////////////////////////////////////////////////////////
///
/// @file EspDriver.h
///
/// @brief Header file of class EspDriver.
///
/// @author Ultratronik GmbH
///         Dornierstr. 9
///         D-82205 Gilching
///         Germany
///         http://www.ultratronik.de
///
/// @author written by Florian Sonntag, Forschung & Entwicklung, gesser@ultratronik.de
///
/// @date 31.03.2021
///
/// @copyright Copyright 2021 by Hemro International AG
///            Hemro International AG
///            Länggenstrasse 34
///            CH 8184 Bachenbülach
///            Switzerland
///            Homepage: www.hemrogroup.com
///
///////////////////////////////////////////////////////////////////////////////

#ifndef EspDriver_h
#define EspDriver_h

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#include <QObject>

class At24c16Driver;
class EspDcHallMotorDriver;
class EspMcuDriver;
class EspProtocolStack;
class EspStepperMotorDriver;
class EspSwUpdateDriver;
class HttpServerDriver;
class MainStatemachine;
class NtcTempDriver;
class SpotLightDriver;
class StartStopButtonDriver;
class StartStopLightDriver;
class WifiDriver;
class JsonApi;

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

class EspDriver : public QObject
{
    Q_OBJECT

public:

    explicit EspDriver();
    ~EspDriver() override;

    void create( MainStatemachine * pMainStatemachine );

    const EspProtocolStack * protocolStack( void ) const;

    EspProtocolStack * protocolStack( void );

    At24c16Driver * at24c16Driver( void );

    EspDcHallMotorDriver * pduDcHallMotorDriver( void );

    EspMcuDriver * espMcuDriver( void );

    EspStepperMotorDriver * agsaStepperMotorDriver( void );

    EspSwUpdateDriver * swUpdateDriver( void );

    HttpServerDriver * httpServerDriver();

    NtcTempDriver * ntcTempDriver( void );

    SpotLightDriver * spotLightDriver( void );

    StartStopButtonDriver * startStopButtonDriver( void );

    StartStopLightDriver * startStopLightDriver( void );

    WifiDriver * wifiDriver( void );

    JsonApi * jsonApi();

    bool isConnected() const;

signals:

    void connectedChanged( const bool bConnected );

private:

    MainStatemachine *      m_pMainStatemachine { nullptr };
    EspProtocolStack *      m_pProtocolStack { nullptr };

    At24c16Driver *         m_pEspAt24c16Driver { nullptr };
    EspDcHallMotorDriver *  m_pPduDcHallMotorDriver { nullptr };
    EspMcuDriver *          m_pEspMcuDriver { nullptr };
    EspStepperMotorDriver * m_pAgsaStepperMotorDriver { nullptr };
    EspSwUpdateDriver *     m_pSwUpdateDriver { nullptr };
    HttpServerDriver *      m_pHttpServerDriver { nullptr };
    NtcTempDriver *         m_pNtcTempDriver { nullptr };
    SpotLightDriver *       m_pSpotLightDriver { nullptr };
    StartStopButtonDriver * m_pStartStopButtonDriver { nullptr };
    StartStopLightDriver *  m_pStartStopLightDriver { nullptr };
    WifiDriver *            m_pWifiDriver { nullptr };
    JsonApi *               m_pJsonApi { nullptr };

};

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#endif // EspDriver_h
