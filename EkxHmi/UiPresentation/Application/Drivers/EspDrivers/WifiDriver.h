///////////////////////////////////////////////////////////////////////////////
///
/// @file WifiDriver.h
///
/// @brief Header file of class WifiDriver.
///
/// @author Ultratronik GmbH
///         Dornierstr. 9
///         D-82205 Gilching
///         Germany
///         http://www.ultratronik.de
///
/// @author written by Gerd Esser, Forschung & Entwicklung, gesser@ultratronik.de
///
/// @date 10.02.2021
///
/// @copyright Copyright 2021 by Hemro International AG
///            Hemro International AG
///            Länggenstrasse 34
///            CH 8184 Bachenbülach
///            Switzerland
///            Homepage: www.hemrogroup.com
///
///////////////////////////////////////////////////////////////////////////////

#ifndef WifiDriver_h
#define WifiDriver_h

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#include <QObject>

class EspWifiIn;
class EspWifiOut;
class EspDataInterface;
class QQmlEngine;
class MainStatemachine;

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

class WifiDriver : public QObject
{
    Q_OBJECT

    Q_PROPERTY( QString    active                       READ isActive           NOTIFY activeChanged )
    Q_PROPERTY( QString    currentWifiConnection        READ connection         NOTIFY connectionChanged )
    Q_PROPERTY( bool       currentWifiConnectionValid   READ isConnectionValid  NOTIFY connectionValidChanged )

public:

    explicit WifiDriver( QQmlEngine * pEngine,
                         MainStatemachine *pMainStatemachine );
    virtual ~WifiDriver( ) override;

    void create();

    const QString & connection() const;
    bool isConnectionValid() const;

    void connectInInterface( EspWifiIn* pWifiIn );
    void disconnectInInterface();

    void connectOutInterface( EspWifiOut* pWifiOut );
    void disconnectOutInterface();

    bool isActive() const;
    void setActive( const bool bActive );

    void setIsAccesspoint( const bool bIsAccesspoint );
    void setApSsid( const QString & strApSsid );
    void setApPw( const QString & strApPw );
    void setStaSsid( const QString & strStaSsid );
    void setStaPw( const QString & strStaPw );
    void setWifiValid( const bool bValid );

    bool isApConnected() const;

public slots:

    void processDataChanged( EspDataInterface * pData );
    void processValidChanged( EspDataInterface * pData );
    void processStandbyChanged( const bool bActive );

    void updateWifiActive();

signals:

    void connectionChanged();
    void connectionValidChanged();
    void activeChanged();

private:

    void setConnection( const QString & strConnection );
    void setConnectionValid( const bool bValid );

private:

    MainStatemachine *  m_pMainStatemachine { nullptr };
    EspWifiIn*          m_pEspWifiIn { nullptr };
    EspWifiOut*         m_pEspWifiOut { nullptr };
    QString             m_strWifiConnection { "Off" };
    bool                m_bConnectionValid { false };
    bool                m_bActive { false };

};

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#endif // WifiDriver_h
