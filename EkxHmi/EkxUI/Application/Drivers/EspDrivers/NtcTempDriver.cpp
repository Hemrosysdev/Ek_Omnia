///////////////////////////////////////////////////////////////////////////////
///
/// @file NtcTempDriver.cpp
///
/// @brief main application entry point of NtcTempDriver.
///
/// @author Ultratronik GmbH
///         Dornierstr. 9
///         D-82205 Gilching
///         Germany
///         http://www.ultratronik.de
///
/// @author written by Florian Sonntag, Forschung & Entwicklung, gesser@ultratronik.de
///
/// @date 01.02.2021
///
/// @copyright Copyright 2020 by Hemro International AG
///            Hemro International AG
///            Länggenstrasse 34
///            CH 8184 Bachenbülach
///            Switzerland
///            Homepage: www.hemrogroup.com
///
///////////////////////////////////////////////////////////////////////////////

#include "NtcTempDriver.h"

#include <QDebug>

#include "MainStatemachine.h"
#include "EspDriver.h"
#include "SettingsSerializer.h"

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

NtcTempDriver::NtcTempDriver( const EkxProtocol::DriverId u8DriverId )
    : EspDeviceDriver( u8DriverId )
{
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

NtcTempDriver::~NtcTempDriver()
{
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void NtcTempDriver::create( MainStatemachine * pMainStatemachine )
{
    m_pMainStatemachine = pMainStatemachine;

    EspDeviceDriver::create( m_pMainStatemachine->espDriver()->protocolStack() );
    updateOvertempWarning();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

int NtcTempDriver::adcToTemperature( const quint32 u32NtcValue ) const
{
    /*
     * For a 10K NTC (B=3988K) combined with a 10K Pullup to 3V3, the following applies between 1°C and 70°C:
     *
     *  T = ( (segOffset - AINmV) * 10 / segInclination ) + segStartTemp
     *
     *  AIN [mV]    segOffset   segInclination  segStartTemp
     *  ==========+===========+===============+==============
     *  2496        2464        266             2
     *  2194        2159        326             11
     *  1831        1795        323             21
     *  1474        1440        291             31
     *  1154        1124        244             41
     *  885         862         194             51
     *  672         654         149             61
     *
     *  AIN will then be 2,496V @1°C down to 0,509V @70°C with max 1°C deviation
     *  If higher temperatures will be measured the error will increase if not
     *  another segment gets introduced. At 90°C the error will be -5°C.
     *
     *  ATTENTION: the pullup should be choosen to get a maximum signal swing and
     *             not overloading the AIN (currently 3V0 max can be measured).
     *
     *  For a pullup of 3K3 - with less signal swing - the following parameters must be used:

     *  AIN [mV]    segOffset   segSteigung segStartT
     *  ==========+===========+===============+==============
     *  2983        2968        134             2
     *  2830        2810        196             11
     *  2609        2585        238             21
     *  2343        2314        267             31
     *  2045        2014        277             41
     *  1737        1707        267             51
     *  1441        1413        242             61
     *
     */

    // mv < 2983//2496
    qint32 segOffsetmV    = 2968; //2464;
    qint32 segInclination = 134;  //266;
    qint32 segStartTempC  = 2;

    if ( u32NtcValue < 1441 )//672 )
    {
        segOffsetmV    = 1413; //653;
        segInclination = 242;  //149;
        segStartTempC  = 61;
    }
    else if ( u32NtcValue < 1737 )//885 )
    {
        segOffsetmV    = 1707; //862;
        segInclination = 267;  //194;
        segStartTempC  = 51;
    }
    else if ( u32NtcValue < 2045 )//1154 )
    {
        segOffsetmV    = 2014; //1124;
        segInclination = 277;  //244;
        segStartTempC  = 41;
    }
    else if ( u32NtcValue < 2343 )//1474 )
    {
        segOffsetmV    = 2314; //1440;
        segInclination = 267;  //291;
        segStartTempC  = 31;
    }
    else if ( u32NtcValue < 2609 )//1831 )
    {
        segOffsetmV    = 2585; //1795;
        segInclination = 238;  //323;
        segStartTempC  = 21;
    }
    else if ( u32NtcValue < 2830 )//2194 )
    {
        segOffsetmV    = 2810; //2159;
        segInclination = 196;  //326;
        segStartTempC  = 11;
    }

    qint32 temp = ( ( ( ( segOffsetmV - static_cast<qint32>( u32NtcValue ) ) * 10 ) / segInclination ) + segStartTempC ) - 1;

    return static_cast<int>( temp );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool NtcTempDriver::overtempWarningActive() const
{
    return m_bOvertempWarningActive;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void NtcTempDriver::setOvertempWarningActive( const bool bActive )
{
    if ( m_bOvertempWarningActive != bActive )
    {
        m_bOvertempWarningActive = bActive;

        emit overtempWarningActiveChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

int NtcTempDriver::currentTemp() const
{
    return m_nCurrentTemp;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void NtcTempDriver::setCurrentTemp( const int nTemp )
{
    if ( nTemp != m_nCurrentTemp )
    {
        m_nCurrentTemp = nTemp;

        updateOvertempWarning();

        emit currentTempChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool NtcTempDriver::currentTempValid() const
{
    return m_bCurrentTempValid;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void NtcTempDriver::setCurrentTempValid( const bool bTempValid )
{
    if ( bTempValid != m_bCurrentTempValid )
    {
        m_bCurrentTempValid = bTempValid;

        updateOvertempWarning();

        emit currentTempValidChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void NtcTempDriver::updateOvertempWarning()
{
    setOvertempWarningActive( m_bCurrentTempValid
                              && ( m_pMainStatemachine->settingsSerializer()->showTempWarning() )
                              && ( m_nCurrentTemp >= ( m_pMainStatemachine->settingsSerializer()->getTempThresValueX100() / 100.0 ) ) );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void NtcTempDriver::processMessageFrame( const EkxProtocol::MessageFrame & frame )
{
    const EkxProtocol::Payload::AdcStatus * pPayload = dynamic_cast<const EkxProtocol::Payload::AdcStatus *>( frame.payloadRef().value() );

    if ( pPayload )
    {
        setCurrentTempValid( pPayload->valid().value() );
        setCurrentTemp( adcToTemperature( pPayload->adcVoltage().value() ) );

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

void NtcTempDriver::processAlive( const bool bAlive )
{
    if ( !bAlive )
    {
        setCurrentTempValid( false );
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

