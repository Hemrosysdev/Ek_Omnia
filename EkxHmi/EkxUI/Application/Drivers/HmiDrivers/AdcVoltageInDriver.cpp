///////////////////////////////////////////////////////////////////////////////
///
/// @file AdcVoltageInDriver.cpp
///
/// @brief main application entry point of AdcVoltageInDriver.
///
/// @author Ultratronik GmbH
///         Dornierstr. 9
///         D-82205 Gilching
///         Germany
///         http://www.ultratronik.de
///
/// @author written by Gerd Esser, Forschung & Entwicklung, gesser@ultratronik.de
///
/// @date 04.11.2022
///
/// @copyright Copyright 2022 by Hemro International AG
///            Hemro International AG
///            Länggenstrasse 34
///            CH 8184 Bachenbülach
///            Switzerland
///            Homepage: www.hemrogroup.com
///
///////////////////////////////////////////////////////////////////////////////

#include "AdcVoltageInDriver.h"
#include <QDebug>
#include <cmath>

#include "SystemDeviceFile.h"

namespace SystemIo
{

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

AdcVoltageInDriver::AdcVoltageInDriver( QObject * pParent )
    : QObject( pParent )
    , m_pSystemScale( new SystemDeviceFile() )
    , m_pSystemOffset( new SystemDeviceFile() )
    , m_pSystemRaw( new SystemDeviceFile() )
{
    m_timerPolling.setSingleShot( false );

    connect( &m_timerPolling, &QTimer::timeout, this, &AdcVoltageInDriver::processPollingTimeout );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

AdcVoltageInDriver::~AdcVoltageInDriver()
{
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void AdcVoltageInDriver::create( const QString & strDriverName,
                                 const QString & strSystemDeviceDir,
                                 const QString & strAdcChannel,
                                 const bool      bUseDma,
                                 const int       nDmaFilterElementNum,
                                 const int       nPollCycleMs,
                                 const bool      bUpdateCyclic )
{
    Q_ASSERT( !strSystemDeviceDir.isEmpty() );
    Q_ASSERT( !strAdcChannel.isEmpty() );

    m_strDriverName      = strDriverName;
    m_strSystemDeviceDir = strSystemDeviceDir;
    m_strAdcChannel      = strAdcChannel;
    m_bUpdateCyclic      = bUpdateCyclic;

    m_pSystemOffset->setFileName( strSystemDeviceDir + "/in_voltage_offset" );
    m_pSystemScale->setFileName( strSystemDeviceDir + "/in_voltage_scale" );
    m_pSystemRaw->setFileName( strSystemDeviceDir + QString( "/in_voltage%1_raw" ).arg( m_strAdcChannel ) );

    m_bSetupValid = m_pSystemScale->readFile( m_dAdcScale );
    m_bSetupValid = m_pSystemOffset->readFile( m_nAdcOffset )
                    && m_bSetupValid;

    m_dmaFilter.init( nDmaFilterElementNum, 0 );

    if ( nPollCycleMs <= 0 )
    {
        m_timerPolling.stop();
    }
    else
    {
        m_timerPolling.setInterval( nPollCycleMs );
        m_timerPolling.start();
    }

    processPollingTimeout();

    // like a mutex for writes into m_dmaFilter
    m_bUseDma = bUseDma;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

const QString & AdcVoltageInDriver::systemDeviceDir() const
{
    return m_strSystemDeviceDir;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

const QString & AdcVoltageInDriver::adcChannel() const
{
    return m_strAdcChannel;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

double AdcVoltageInDriver::voltageMv() const
{
    return m_dVoltageMv;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool AdcVoltageInDriver::isValid() const
{
    return m_bValid;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void AdcVoltageInDriver::processDmaValue( const int nAdcValue )
{
    if ( m_bUseDma )
    {
        m_u32DmaCounter++;
        m_dmaFilter.append( nAdcValue );
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void AdcVoltageInDriver::processPollingTimeout()
{
    if ( !m_bSetupValid )
    {
        setValid( false );
        m_nAdcRaw = c_nInvalidAdcRaw;
        setVoltageMv( c_dInvalidVoltageMv );
    }
    else if ( m_bUseDma )
    {
        setValid( m_u32DmaCounter > m_dmaFilter.elementNum() / 2 );
        m_nAdcRaw = m_dmaFilter.value();
        setVoltageMv( static_cast<double>( m_nAdcRaw + m_nAdcOffset ) * m_dAdcScale );
        m_u32DmaCounter = 0;
    }
    else
    {
        setValid( m_pSystemRaw->readFile( m_nAdcRaw ) );
        setVoltageMv( static_cast<double>( m_nAdcRaw + m_nAdcOffset ) * m_dAdcScale );
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void AdcVoltageInDriver::setVoltageMv( const double dVoltageMv )
{
    if ( std::fabs( m_dVoltageMv - dVoltageMv ) > 1.e-15
         || m_bUpdateCyclic )
    {
        m_dVoltageMv = dVoltageMv;
        emit voltageChanged( m_dVoltageMv );
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void AdcVoltageInDriver::setValid( const bool bValid )
{
    if ( m_bValid != bValid )
    {
        m_bValid = bValid;
        emit validChanged( m_bValid );
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

} // namespace SystemIo

