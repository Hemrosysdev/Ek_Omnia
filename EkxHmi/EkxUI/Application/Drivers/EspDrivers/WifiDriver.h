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

#include "EspDeviceDriver.h"
#include "EkxProtocol.h"
#include "SettingsSerializer.h"

class MainStatemachine;

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

class WifiDriver : public EspDeviceDriver
{
    Q_OBJECT

public:

    explicit WifiDriver( const EkxProtocol::DriverId u8DriverId );
    ~WifiDriver() override;

    void create( MainStatemachine * pMainStatemachine );

    bool isValid() const;

    const QString & statusApIp() const;

    const QString & statusApMac() const;

    const QString & statusStaIp() const;

    const QString & statusStaMac() const;

    void setConfig( const SettingsSerializer::WifiMode nWifiMode,
                    const QString &                    strApSsid,
                    const QString &                    strApPassword,
                    const QString &                    strApIp,
                    const QString &                    strStaSsid,
                    const QString &                    strStaPassword,
                    const bool                         bStaDhcp,
                    const QString &                    strStaStaticIp,
                    const QString &                    strStaStaticGateway,
                    const QString &                    strStaStaticNetmask,
                    const QString &                    strStaStaticDns );

    void setWifiMode( const SettingsSerializer::WifiMode nWifiMode );

    void setWifiMode( const EkxProtocol::Payload::WifiMode u8WifiMode );

    void setApSsid( const QString & strApSsid );

    void setApPassword( const QString & strApPassword );

    void setApIp( const QString & strApIp );

    void setStaSsid( const QString & strStaSsid );

    void setStaPassword( const QString & strStaPassword );

    void setStaDhcp( const bool bDhcp );

    void setStaStaticIp( const QString & strStaStaticIp );

    void setStaStaticGateway( const QString & strStaStaticGateway );

    void setStaStaticNetmask( const QString & strStaStaticNetmask );

    void setStaStaticDns( const QString & strStaStaticDns );

    void processMessageFrame( const EkxProtocol::MessageFrame & frame ) override;

    void processAlive( const bool bAlive ) override;

    void sendConfig();

public slots:

    void processWifiModeChanged( const SettingsSerializer::WifiMode nWifiMode );

signals:

    void validChanged( const bool bValid );

    void statusApIpChanged( const QString & strIp );

    void statusApMacChanged( const QString & strMac );

    void statusStaIpChanged( const QString & strIp );

    void statusStaMacChanged( const QString & strMac );

    void wifiModeChanged( const EkxProtocol::Payload::WifiMode u8WifiMode );

    void apSsidChanged( const QString & strApSsid );

    void apPasswordChanged( const QString & strApPassword );

    void apIpChanged( const QString & strApIp );

    void staSsidChanged( const QString & strStaSsid );

    void staPasswordChanged( const QString & strStaPassword );

    void staDhcpChanged( const bool bDhcp );

    void staStaticIpChanged( const QString & strStaStaticIp );

    void staStaticGatewayChanged( const QString & strStaStaticGateway );

    void staStaticNetmaskChanged( const QString & strStaStaticNetmask );

    void staStaticDnsChanged( const QString & strStaStaticDns );

private:

    void setValid( const bool bValid );

    void setStatusApIp( const QString & strIp );

    void setStatusApMac( const QString & strMac );

    void setStatusStaIp( const QString & strIp );

    void setStatusStaMac( const QString & strMac );

private:

    MainStatemachine *             m_pMainStatemachine { nullptr };

    EkxProtocol::Payload::WifiMode m_u8WifiMode = EkxProtocol::Payload::WifiMode::Off;

    QString                        m_strApSsid;
    QString                        m_strApPassword;
    QString                        m_strApIp = "192.168.4.1";

    QString                        m_strStaSsid;
    QString                        m_strStaPassword;
    bool                           m_bStaDhcp;
    QString                        m_strStaStaticIp;
    QString                        m_strStaStaticGateway;
    QString                        m_strStaStaticNetmask;
    QString                        m_strStaStaticDns;

    bool                           m_bValid { false };
    QString                        m_strStatusApIp;
    QString                        m_strStatusApMac;
    QString                        m_strStatusStaIp;
    QString                        m_strStatusStaMac;

};

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#endif // WifiDriver_h
