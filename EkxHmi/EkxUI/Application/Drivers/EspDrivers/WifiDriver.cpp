///////////////////////////////////////////////////////////////////////////////
///
/// @file WifiDriver.cpp
///
/// @brief main application entry point of WifiDriver.
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

#include "WifiDriver.h"

#include <QDebug>

#include "MainStatemachine.h"
#include "SettingsSerializer.h"
#include "EspDriver.h"

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

WifiDriver::WifiDriver( const EkxProtocol::DriverId u8DriverId )
    : EspDeviceDriver( u8DriverId )
{
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

WifiDriver::~WifiDriver()
{
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void WifiDriver::create( MainStatemachine * pMainStatemachine )
{
    m_pMainStatemachine = pMainStatemachine;

    EspDeviceDriver::create( m_pMainStatemachine->espDriver()->protocolStack() );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool WifiDriver::isValid() const
{
    return m_bValid;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

const QString & WifiDriver::statusApIp() const
{
    return m_strStatusApIp;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

const QString & WifiDriver::statusApMac() const
{
    return m_strStatusApMac;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

const QString & WifiDriver::statusStaIp() const
{
    return m_strStatusStaIp;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

const QString & WifiDriver::statusStaMac() const
{
    return m_strStatusStaMac;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void WifiDriver::setConfig( const SettingsSerializer::WifiMode nWifiMode,
                            const QString &                    strApSsid,
                            const QString &                    strApPassword,
                            const QString &                    strApIp,
                            const QString &                    strStaSsid,
                            const QString &                    strStaPassword,
                            const bool                         bStaDhcp,
                            const QString &                    strStaStaticIp,
                            const QString &                    strStaStaticGateway,
                            const QString &                    strStaStaticNetmask,
                            const QString &                    strStaStaticDns )
{
    m_u8WifiMode          = static_cast<EkxProtocol::Payload::WifiMode>( nWifiMode );
    m_strApSsid           = strApSsid;
    m_strApPassword       = strApPassword;
    m_strApIp             = strApIp;
    m_strStaSsid          = strStaSsid;
    m_strStaPassword      = strStaPassword;
    m_bStaDhcp            = bStaDhcp;
    m_strStaStaticIp      = strStaStaticIp;
    m_strStaStaticGateway = strStaStaticGateway;
    m_strStaStaticNetmask = strStaStaticNetmask;
    m_strStaStaticDns     = strStaStaticDns;

    sendConfig();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void WifiDriver::setWifiMode( const SettingsSerializer::WifiMode nWifiMode )
{
    EkxProtocol::Payload::WifiMode u8WifiMode = static_cast<EkxProtocol::Payload::WifiMode>( nWifiMode );

    if ( m_u8WifiMode != u8WifiMode )
    {
        m_u8WifiMode = u8WifiMode;
        emit wifiModeChanged( m_u8WifiMode );
        sendConfig();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void WifiDriver::setWifiMode( const EkxProtocol::Payload::WifiMode u8WifiMode )
{
    if ( m_u8WifiMode != u8WifiMode )
    {
        m_u8WifiMode = u8WifiMode;
        emit wifiModeChanged( m_u8WifiMode );
        sendConfig();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void WifiDriver::setValid( const bool bValid )
{
    if ( bValid != m_bValid )
    {
        m_bValid = bValid;
        emit validChanged( m_bValid );
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void WifiDriver::setStatusApIp( const QString & strIp )
{
    if ( m_strStatusApIp != strIp )
    {
        m_strStatusApIp = strIp;
        emit apSsidChanged( m_strStatusApIp );
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void WifiDriver::setStatusApMac( const QString & strMac )
{
    if ( m_strStatusApMac != strMac )
    {
        m_strStatusApMac = strMac;
        emit apSsidChanged( m_strStatusApMac );
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void WifiDriver::setStatusStaIp( const QString & strIp )
{
    if ( m_strStatusStaIp != strIp )
    {
        m_strStatusStaIp = strIp;
        emit apSsidChanged( m_strStatusStaIp );
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void WifiDriver::setStatusStaMac( const QString & strMac )
{
    if ( m_strStatusStaMac != strMac )
    {
        m_strStatusStaMac = strMac;
        emit statusStaMacChanged( m_strStatusStaMac );
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void WifiDriver::processWifiModeChanged( const SettingsSerializer::WifiMode nWifiMode )
{
    setWifiMode( static_cast<EkxProtocol::Payload::WifiMode>( nWifiMode ) );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void WifiDriver::setApSsid( const QString & strApSsid )
{
    if ( m_strApSsid != strApSsid )
    {
        m_strApSsid = strApSsid;
        emit apSsidChanged( m_strApSsid );
        sendConfig();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void WifiDriver::setApPassword( const QString & strApPassword )
{
    if ( m_strApPassword != strApPassword )
    {
        m_strApPassword = strApPassword;
        emit apPasswordChanged( m_strApPassword );
        sendConfig();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void WifiDriver::setApIp( const QString & strApIp )
{
    if ( m_strApIp != strApIp )
    {
        m_strApIp = strApIp;
        emit apIpChanged( m_strApIp );
        sendConfig();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void WifiDriver::setStaSsid( const QString & strStaSsid )
{
    if ( m_strStaSsid != strStaSsid )
    {
        m_strStaSsid = strStaSsid;
        emit staSsidChanged( m_strStaPassword );
        sendConfig();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void WifiDriver::setStaPassword( const QString & strStaPassword )
{
    if ( m_strStaPassword != strStaPassword )
    {
        m_strStaPassword = strStaPassword;
        emit staPasswordChanged( m_strStaPassword );
        sendConfig();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void WifiDriver::setStaDhcp( const bool bDhcp )
{
    if ( m_bStaDhcp != bDhcp )
    {
        m_bStaDhcp = bDhcp;
        emit staDhcpChanged( m_bStaDhcp );
        sendConfig();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void WifiDriver::setStaStaticIp( const QString & strStaStaticIp )
{
    if ( m_strStaStaticIp != strStaStaticIp )
    {
        m_strStaStaticIp = strStaStaticIp;
        emit staStaticIpChanged( m_strStaStaticIp );
        sendConfig();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void WifiDriver::setStaStaticGateway( const QString & strStaStaticGateway )
{
    if ( m_strStaStaticGateway != strStaStaticGateway )
    {
        m_strStaStaticGateway = strStaStaticGateway;
        emit staStaticGatewayChanged( m_strStaStaticGateway );
        sendConfig();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void WifiDriver::setStaStaticNetmask( const QString & strStaStaticNetmask )
{
    if ( m_strStaStaticNetmask != strStaStaticNetmask )
    {
        m_strStaStaticNetmask = strStaStaticNetmask;
        emit staStaticNetmaskChanged( m_strStaStaticNetmask );
        sendConfig();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void WifiDriver::setStaStaticDns( const QString & strStaStaticDns )
{
    if ( m_strStaStaticDns != strStaStaticDns )
    {
        m_strStaStaticDns = strStaStaticDns;
        emit staStaticDnsChanged( m_strStaStaticDns );
        sendConfig();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void WifiDriver::processMessageFrame( const EkxProtocol::MessageFrame & frame )
{
    const EkxProtocol::Payload::WifiStatus * pPayload = dynamic_cast<const EkxProtocol::Payload::WifiStatus *>( frame.payloadRef().value() );

    if ( pPayload )
    {
        m_bValid          = pPayload->valid().value();
        m_strStatusApIp   = pPayload->apIp().value().c_str();
        m_strStatusApMac  = pPayload->apMac().value().c_str();
        m_strStatusStaIp  = pPayload->staIp().value().c_str();
        m_strStatusStaMac = pPayload->staMac().value().c_str();

        emit validChanged( m_bValid );

        replyAck( frame.driverId().value(),
                  frame.msgCounter().value(),
                  frame.repeatCounter().value() );
    }
    else
    {
        replyNack( frame.driverId().value(),
                   frame.msgCounter().value(),
                   frame.repeatCounter().value(),
                   EkxProtocol::Payload::Nack::NackReason::PayloadBroken );
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void WifiDriver::processAlive( const bool bAlive )
{
    if ( !bAlive )
    {
        setValid( false );
    }
    else
    {
        sendConfig();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void WifiDriver::sendConfig()
{
    EkxProtocol::Payload::WifiConfig * pPayload = new EkxProtocol::Payload::WifiConfig();

    pPayload->wifiMode().setValue( m_u8WifiMode );

    pPayload->apSsid().setValue( m_strApSsid.toStdString() );
    pPayload->apPassword().setValue( m_strApPassword.toStdString() );
    pPayload->apIp().setValue( m_strApIp.toStdString() );

    pPayload->staSsid().setValue( m_strStaSsid.toStdString() );
    pPayload->staPassword().setValue( m_strStaPassword.toStdString() );
    pPayload->staDhcp().setValue( m_bStaDhcp );
    pPayload->staStaticIp().setValue( m_strStaStaticIp.toStdString() );
    pPayload->staStaticGateway().setValue( m_strStaStaticGateway.toStdString() );
    pPayload->staStaticNetmask().setValue( m_strStaStaticNetmask.toStdString() );
    pPayload->staStaticDns().setValue( m_strStaStaticDns.toStdString() );

    qInfo() << "WifiDriver::sendConfig()";

    send( pPayload );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////
