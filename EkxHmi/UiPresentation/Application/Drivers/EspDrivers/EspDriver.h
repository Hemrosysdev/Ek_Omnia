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
#include <QList>

class QQmlEngine;
class EspConnectorClient;
class StartStopButtonDriver;
class NtcTempDriver;
class WifiDriver;
class At24c16Driver;
class LedPwmDriver;
class UartDriver;
class EspSwUpdateInDriver;
class EspSwUpdateOutDriver;
class EspConnectorClient;
class MainStatemachine;

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

class EspDriver
        : public QObject
{
    Q_OBJECT

public:

    explicit EspDriver( QQmlEngine* pEngine,
                        MainStatemachine * pMainStatemachine );
    virtual ~EspDriver( ) override;

    void create();

    EspConnectorClient * connectorClient( void );

    StartStopButtonDriver* startStopButtonDriver( void );
    NtcTempDriver *        ntcTempDriver( void );
    WifiDriver*            wifiDriver( void );
    At24c16Driver*         at24c16Driver( void );
    LedPwmDriver*          ledPwmDriver( void );
    UartDriver*            uartDriver( void );
    EspSwUpdateInDriver*   swUpdateInDriver( void );
    EspSwUpdateOutDriver*  swUpdateOutDriver( void );

public slots:

    void processConnectChanged( void );

private:

    MainStatemachine *      m_pMainStatemachine { nullptr };
    EspConnectorClient *    m_pConnectorClient { nullptr };

    StartStopButtonDriver*  m_pStartStopButtonDriver { nullptr };
    NtcTempDriver *         m_pNtcTempDriver { nullptr };
    WifiDriver *            m_pWifiDriver { nullptr };
    At24c16Driver *         m_pEspAt24c16Driver { nullptr };
    LedPwmDriver *          m_pLedPwmDriver { nullptr };
    UartDriver *            m_pUartDriver { nullptr };
    EspSwUpdateInDriver *   m_pSwUpdateInDriver { nullptr };
    EspSwUpdateOutDriver *  m_pSwUpdateOutDriver { nullptr };

};

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#endif // EspDriver_h
