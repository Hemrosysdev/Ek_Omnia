///////////////////////////////////////////////////////////////////////////////
///
/// @file DeviceMemoryIo.cpp
///
/// @brief main application entry point of DeviceMemoryIo.
///
/// @author Ultratronik GmbH
///         Dornierstr. 9
///         D-82205 Gilching
///         Germany
///         http://www.ultratronik.de
///
/// @author written by Gerd Esser, Forschung & Entwicklung, gesser@ultratronik.de
///
/// @date 22.09.2022
///
/// @copyright Copyright 2022 by Hemro International AG
///            Hemro International AG
///            Länggenstrasse 34
///            CH 8184 Bachenbülach
///            Switzerland
///            Homepage: www.hemrogroup.com
///
///////////////////////////////////////////////////////////////////////////////

#include "CpuMp157Driver.h"

#include <QDebug>
#include <QProcess>
#include <QJsonObject>
#include <QJsonDocument>
#include <QFileInfo>
#include <QThread>

#include <cmath>

#include "GpioOut.h"
#include "SystemDeviceFile.h"

namespace SystemIo
{

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

CpuMp157Driver::CpuMp157Driver()
    : m_pGpioAdc0BufferEnable( new GpioOut() )
    , m_pGpioOnePulseTimer44006000Prepare( new GpioOut() )
    , m_pGpioOnePulseTimer40000000Prepare( new GpioOut() )
    , m_pGpioOnePulseTimer44006000Disable( new GpioOut() )
    , m_pGpioOnePulseTimer40000000Disable( new GpioOut() )
    , m_pSystemFilePulseLength( new SystemDeviceFile() )
    , m_pGpioStartGrind( new GpioOut() )
{
#ifdef TARGETBUILD
    loadRegisterMap( c_strCpuStm32Mp157 );

//    m_pGpioAdc0BufferEnable->setFileName( "/sys/bus/iio/devices/iio:device5/buffer/enable" );

//    m_pGpioOnePulseTimer44006000Prepare->setFileName( "/sys/bus/platform/drivers/stm32-one-pulse/44006000.timer:op/prepare" );
//    m_pGpioOnePulseTimer40000000Prepare->setFileName( "/sys/bus/platform/drivers/stm32-one-pulse/40000000.timer:op/prepare" );
//    m_pGpioOnePulseTimer44006000Disable->setFileName( "/sys/bus/platform/drivers/stm32-one-pulse/44006000.timer:op/disable" );
//    m_pGpioOnePulseTimer40000000Disable->setFileName( "/sys/bus/platform/drivers/stm32-one-pulse/40000000.timer:op/disable" );
//    m_pSystemFilePulseLength->setFileName( "/sys/bus/platform/drivers/stm32-one-pulse/40000000.timer:op/pulse_length" );
//    m_pGpioStartGrind->setFileName( "/sys/bus/platform/drivers/stm32-timers/40001000.timer/40001000.timer:counter/counter0/count0/enable" );

//    sequenceStartAdcOnly();
#endif

    connect( this, &CpuMp157Driver::adc1OversamplingChanged, this, &CpuMp157Driver::activateAdcOversampling );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

CpuMp157Driver::~CpuMp157Driver()
{

}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void CpuMp157Driver::readAdcParameters()
{
    // see RM0436 Rev 6, page 1590
    QVector<double> adcSampleTimeTicksList;
    adcSampleTimeTicksList << 1.5;
    adcSampleTimeTicksList << 2.5;
    adcSampleTimeTicksList << 8.5;
    adcSampleTimeTicksList << 16.5;
    adcSampleTimeTicksList << 32.5;
    adcSampleTimeTicksList << 64.5;
    adcSampleTimeTicksList << 387.5;
    adcSampleTimeTicksList << 810.5;

    QVector<int> adcDataResolutions;
    adcDataResolutions << 16;
    adcDataResolutions << 14;
    adcDataResolutions << 12;
    adcDataResolutions << 10;
    adcDataResolutions << 8;

    QVector<int> adcPrescalers;
    adcPrescalers << 0;
    adcPrescalers << 2;
    adcPrescalers << 4;
    adcPrescalers << 5;
    adcPrescalers << 8;
    adcPrescalers << 10;
    adcPrescalers << 12;
    adcPrescalers << 16;
    adcPrescalers << 32;
    adcPrescalers << 64;
    adcPrescalers << 128;
    adcPrescalers << 256;

    QVector<QString> rccAdcClockSelectionStrings;
    rccAdcClockSelectionStrings << "pll4_r_ck (37.125MHz)";
    rccAdcClockSelectionStrings << "per_ck (24MHz)";
    rccAdcClockSelectionStrings << "pll3_q_ck";

    QVector<double> rccAdcClockSelections;
    rccAdcClockSelections << 37125.e3;
    rccAdcClockSelections << 24.e6;
    rccAdcClockSelections << 0.0;   // ???

    QVector<QString> vrefbufVrsStrings;
    vrefbufVrsStrings << "2.5V";
    vrefbufVrsStrings << "2.048V";
    vrefbufVrsStrings << "1.8V";
    vrefbufVrsStrings << "1.5V";

    setAdcClockHz( rccAdcClockSelections[( read32bit( "RCC", "RCC_ADCCKSELR" ) >> 0 ) & 0x3] );

    setAdc1Oversampling( ( ( read32bit( "ADC1", "ADC_CFGR2" ) >> 16 ) & 0x3ff ) + 1 );
    setAdc1LeftShiftFactor( ( ( read32bit( "ADC1", "ADC_CFGR2" ) >> 5 ) & 0xf ) );
    setAdc1SampleTimeTicks( adcSampleTimeTicksList[( read32bit( "ADC1", "ADC_SMPR1" ) >> 3 ) & 0x7] );

    setAdc2Oversampling( ( ( read32bit( "ADC2", "ADC_CFGR2" ) >> 16 ) & 0x3ff ) + 1 );
    setAdc2LeftShiftFactor( ( ( read32bit( "ADC2", "ADC_CFGR2" ) >> 5 ) & 0xf ) );
    setAdc2SampleTimeTicks( adcSampleTimeTicksList[( read32bit( "ADC2", "ADC_SMPR1" ) >> 24 ) & 0x7] );        // channel 9

    if ( adc1Oversampling() > 0 )
    {
        setAdc1SamplingFreqHz( m_dAdcClockHz / ( m_dAdc1SampleTimeTicks + c_dAdcConversionTimeTicks ) / static_cast<double>( m_u32Adc1Oversampling ) );
    }
    if ( adc2Oversampling() > 0 )
    {
        setAdc2SamplingFreqHz( m_dAdcClockHz / ( m_dAdc2SampleTimeTicks + c_dAdcConversionTimeTicks ) / static_cast<double>( m_u32Adc2Oversampling ) );
    }

    qInfo() << "CpuMp157Driver::readAdcParameters():";
    qInfo() << "    ADC conversion time ticks         " << adcConversionTimeTicks() << "clock cycles";
    qInfo() << "    ADC prescaler                     " << adcPrescalers[( read32bit( "ADC", "ADC_CCR" ) >> 18 ) & 0xf];
    qInfo() << "    RCC ADC clock selection           " << rccAdcClockSelectionStrings[( read32bit( "RCC", "RCC_ADCCKSELR" ) >> 0 ) & 0x3];
    qInfo() << "    VREFBUF VRS                       " << vrefbufVrsStrings[( read32bit( "VREFBUF", "VREFBUF_CSR" ) >> 3 ) & 0x7];
    qInfo() << "    VREFBUF ENVR                      " << ( ( read32bit( "VREFBUF", "VREFBUF_CSR" ) >> 0 ) & 0x1 );

    qInfo() << "    ADC1 oversampling ratio           " << adc1Oversampling();
    qInfo() << "    ADC1 left shift factor            " << adc1LeftShiftFactor() << "bits";
    qInfo() << "    ADC1 oversampling right shift     " << ( ( read32bit( "ADC1", "ADC_CFGR2" ) >> 5 ) & 0xf ) << "bits";
    qInfo() << "    ADC1 sample time ticks (channel 1)" << adc1SampleTimeTicks() << "clock cycles";
    qInfo() << "    ADC1 data resolution              " << adcDataResolutions[( read32bit( "ADC1", "ADC_CFGR" ) >> 2 ) & 0x7] << "bits";
    qInfo() << "    ADC1 sampling frequency           " << adc1SamplingFreqHz() << "Hz";
    qInfo() << "    ADC1 delta sample time            " << adc1DeltaTimeSec() << "sec";

    qInfo() << "    ADC2 oversampling ratio           " << adc2Oversampling();
    qInfo() << "    ADC2 left shift factor            " << adc2LeftShiftFactor() << "bits";
    qInfo() << "    ADC2 oversampling right shift     " << ( ( read32bit( "ADC2", "ADC_CFGR2" ) >> 5 ) & 0xf ) << "bits";
    qInfo() << "    ADC2 sample time ticks (channel 9)" << adc2SampleTimeTicks() << "clock cycles";
    qInfo() << "    ADC2 data resolution              " << adcDataResolutions[( read32bit( "ADC2", "ADC_CFGR" ) >> 2 ) & 0x7] << "bits";
    qInfo() << "    ADC2 sampling frequency           " << adc2SamplingFreqHz() << "Hz";
    qInfo() << "    ADC2 delta sample time            " << adc2DeltaTimeSec() << "sec";

    //for ( int i = 0; i < 10; i++ )
    //{
    //    qInfo() << "    ADC1 sample time ticks SMPR1, channel" << i << adcSampleTimeTicksList[( read32bit( "ADC1", "ADC_SMPR1" ) >> (i*3) ) & 0x7] << "clock cycles";
    //}
    //for ( int i = 0; i < 10; i++ )
    //{
    //    qInfo() << "    ADC1 sample time ticks SMPR2, channel" << (i+10) << adcSampleTimeTicksList[( read32bit( "ADC1", "ADC_SMPR2" ) >> (i*3) ) & 0x7] << "clock cycles";
    //}
    //for ( int i = 0; i < 10; i++ )
    //{
    //    qInfo() << "    ADC2 sample time ticks SMPR1, channel" << i << adcSampleTimeTicksList[( read32bit( "ADC2", "ADC_SMPR1" ) >> (i*3) ) & 0x7] << "clock cycles";
    //}
    //for ( int i = 0; i < 10; i++ )
    //{
    //    qInfo() << "    ADC2 sample time ticks SMPR2, channel" << (i+10) << adcSampleTimeTicksList[( read32bit( "ADC2", "ADC_SMPR2" ) >> (i*3) ) & 0x7] << "clock cycles";
    //}
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void CpuMp157Driver::setAdc1SampleTimeTicks( const double dTicks )
{
    if ( isDoubleChanged( m_dAdc1SampleTimeTicks, dTicks ) )
    {
        m_dAdc1SampleTimeTicks = dTicks;
        emit adc1SampleTimeTicksChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

double CpuMp157Driver::adc1SampleTimeTicks() const
{
    return m_dAdc1SampleTimeTicks;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void CpuMp157Driver::setAdc2SampleTimeTicks( const double dTicks )
{
    if ( isDoubleChanged( m_dAdc2SampleTimeTicks, dTicks ) )
    {
        m_dAdc2SampleTimeTicks = dTicks;
        emit adc2SampleTimeTicksChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

double CpuMp157Driver::adc2SampleTimeTicks() const
{
    return m_dAdc2SampleTimeTicks;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void CpuMp157Driver::activateAdcSampleTimeTicks()
{
    double dSampleRate = std::floor( m_dAdc1SampleTimeTicks - 1 ) / m_dAdcClockHz;
    int    nSampleRate = static_cast<int>( dSampleRate * 1.0e9 ) + 1;

    qInfo() << "CpuMp157Driver::activateAdcSampleTimeTicks()" << m_dAdc1SampleTimeTicks << nSampleRate;

    qInfo() << "CpuMp157Driver::activateAdcSampleclocks() fw_setenv" << "samplerate" << QString( "0x%1" ).arg( nSampleRate, 8, 16, QLatin1Char( '0' ) );
    qInfo() << "CpuMp157Driver::activateAdcSampleclocks() reboot";

    QProcess proc;
    proc.start( "fw_setenv", QStringList() << "samplerate" << QString( "0x%1" ).arg( nSampleRate, 8, 16, QLatin1Char( '0' ) ) );
    proc.waitForFinished( 1000 );

    proc.start( "reboot", QStringList() );
    proc.waitForFinished( 1000 );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void CpuMp157Driver::activateAdcOversampling()
{
    if ( m_u32Adc1Oversampling > 0
         && m_u32Adc1Oversampling <= 1024 )
    {
        /*
           qInfo() << "CpuMp157Driver::activateAdcOversampling()" << m_u32AdcOversampling;

           SystemDeviceFile bufferEnableFile( "/sys/bus/iio/devices/iio:device5/buffer/enable" );

           bufferEnableFile.writeFile( 1 );
           QThread::msleep( 1000 );

           write32bitOr( 0x48003008, 0x10 );       // ADC_CR
           write32bitOr( 0x4800300c, 0x2000 );     // ADC_CFGR

           quint32 u32NewOsvr = ( ( m_u32AdcOversampling - 1 ) & 0x3ff ) << 16;
           quint32 u32CurAdcCfgr2 = read32bit( 0x48003010 );
           write32bit( 0x48003010, u32NewOsvr | ( u32CurAdcCfgr2 & 0xfc00ffff ) );      // ADC_CFGR2

           QThread::msleep( 1000 );
           bufferEnableFile.writeFile( 0 );
           QThread::msleep( 1000 );
           SystemDeviceFile( "/sys/bus/iio/devices/iio:device5/trigger/current_trigger" ).writeFile( "tim3_trgo" );
           QThread::msleep( 1000 );
           bufferEnableFile.writeFile( 1 );

           readAdcParameters();
         */
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void CpuMp157Driver::setAdc1Oversampling( const quint32 u32Oversampling )
{
    if ( m_u32Adc1Oversampling != u32Oversampling )
    {
        m_u32Adc1Oversampling = u32Oversampling;
        emit adc1OversamplingChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

quint32 CpuMp157Driver::adc1Oversampling() const
{
    return m_u32Adc1Oversampling;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void CpuMp157Driver::setAdc2Oversampling( const quint32 u32Oversampling )
{
    if ( m_u32Adc2Oversampling != u32Oversampling )
    {
        m_u32Adc2Oversampling = u32Oversampling;
        emit adc2OversamplingChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

quint32 CpuMp157Driver::adc2Oversampling() const
{
    return m_u32Adc2Oversampling;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void CpuMp157Driver::setAdc1LeftShiftFactor( const double dFactor )
{
    if ( m_dAdc1LeftShiftFactor != dFactor )
    {
        m_dAdc1LeftShiftFactor = dFactor;
        emit adc1LeftShiftfactorChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

double CpuMp157Driver::adc1LeftShiftFactor() const
{
    return m_dAdc1LeftShiftFactor;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void CpuMp157Driver::setAdc2LeftShiftFactor( const double dFactor )
{
    if ( m_dAdc2LeftShiftFactor != dFactor )
    {
        m_dAdc2LeftShiftFactor = dFactor;
        emit adc2LeftShiftfactorChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

double CpuMp157Driver::adc2LeftShiftFactor() const
{
    return m_dAdc2LeftShiftFactor;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void CpuMp157Driver::setAdcClockHz( const double dClockHz )
{
    if ( isDoubleChanged( m_dAdcClockHz, dClockHz ) )
    {
        m_dAdcClockHz = dClockHz;
        emit adcClockHzChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

double CpuMp157Driver::adcClockHz() const
{
    return m_dAdcClockHz;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void CpuMp157Driver::setAdc1SamplingFreqHz( const double dAdcSamplingFreqHz )
{
    if ( isDoubleChanged( m_dAdc1SamplingFreqHz, dAdcSamplingFreqHz ) )
    {
        m_dAdc1SamplingFreqHz = dAdcSamplingFreqHz;

        if ( std::fabs( m_dAdc1SamplingFreqHz ) > 1.e-10 )
        {
            m_dAdc1DeltaTimeSec = 1.0 / m_dAdc1SamplingFreqHz;
        }

        emit adc1SamplingFreqHzChanged();
        emit adc1DeltaTimeSecChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

double CpuMp157Driver::adc1SamplingFreqHz() const
{
    return m_dAdc1SamplingFreqHz;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void CpuMp157Driver::setAdc2SamplingFreqHz( const double dAdcSamplingFreqHz )
{
    if ( isDoubleChanged( m_dAdc2SamplingFreqHz, dAdcSamplingFreqHz ) )
    {
        m_dAdc2SamplingFreqHz = dAdcSamplingFreqHz;

        if ( std::fabs( m_dAdc2SamplingFreqHz ) > 1.e-10 )
        {
            m_dAdc2DeltaTimeSec = 1.0 / m_dAdc2SamplingFreqHz;
        }

        emit adc2SamplingFreqHzChanged();
        emit adc2DeltaTimeSecChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

double CpuMp157Driver::adc2SamplingFreqHz() const
{
    return m_dAdc2SamplingFreqHz;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

double CpuMp157Driver::adc1DeltaTimeSec() const
{
    return m_dAdc1DeltaTimeSec;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

double CpuMp157Driver::adc2DeltaTimeSec() const
{
    return m_dAdc2DeltaTimeSec;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

double CpuMp157Driver::adcConversionTimeTicks() const
{
    return c_dAdcConversionTimeTicks;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

// static
bool CpuMp157Driver::isDoubleChanged( const double dValue1,
                                      const double dValue2 )
{
    return std::fabs( dValue1 - dValue2 ) > 1.0e-10;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

QString CpuMp157Driver::toJson() const
{
    QJsonObject jsonObj;
    jsonObj["adcConversionTimeTicks"] = adcConversionTimeTicks();
    jsonObj["adcClockHz"]             = adcClockHz();

    jsonObj["adcSamplingFreqHz"]  = adc1SamplingFreqHz();
    jsonObj["samplingFreqHz"]     = adc1SamplingFreqHz();            // legacy support
    jsonObj["adcLeftShiftFactor"] = adc1LeftShiftFactor();
    jsonObj["adcOversampling"]    = static_cast<int>( adc1Oversampling() );
    jsonObj["adcSampleTimeTicks"] = adc1SampleTimeTicks();
    jsonObj["adcDeltaTimeSec"]    = adc1DeltaTimeSec();
    jsonObj["deltaTimeSec"]       = adc1DeltaTimeSec();              // legacy support

    jsonObj["adc2SamplingFreqHz"]  = adc2SamplingFreqHz();
    jsonObj["adc2LeftShiftFactor"] = adc2LeftShiftFactor();
    jsonObj["adc2Oversampling"]    = static_cast<int>( adc2Oversampling() );
    jsonObj["adc2SampleTimeTicks"] = adc2SampleTimeTicks();
    jsonObj["adc2DeltaTimeSec"]    = adc2DeltaTimeSec();

    return QJsonDocument( jsonObj ).toJson();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void CpuMp157Driver::sequenceStartAdcOnly()
{
    //#!/bin/sh

    //#TIMER3
    //ENABLE=0x40001000

    //CR1=0x0
    //CR2=0x4
    //SMCR=0x8
    //CCMR1=0x18
    //CCMR2=0x1C
    //CCER=0x20
    //PSC=0x28

    //modify_write () {
    //	reg="$(( $1 | $2 ))"
    //	val=$(devmem $reg)
    //	val="$(( $val | $3 ))"
    //	devmem $reg 32 $val
    //}

    //echo 1 > /sys/bus/platform/drivers/stm32-one-pulse/44006000.timer:op/disable
    //echo 1 > /sys/bus/platform/drivers/stm32-one-pulse/40000000.timer:op/disable
    //modify_write $ENABLE $CR1 0x1

    sequenceDisableTimer();

    write32bitOr( 0x40001000, 0x1 );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

// static
void CpuMp157Driver::sequenceStartGrind()
{
    //#!/bin/sh

    //TIMER3_PATH=/sys/bus/platform/drivers/stm32-timers/40001000.timer/40001000.timer:counter/counter*/count0
    //cd $TIMER3_PATH;echo 1 > enable
    m_pGpioStartGrind->setHigh();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void CpuMp157Driver::sequenceAdcReset()
{
    //#!/bin/sh
    //all_iio=$(ls -d /sys/bus/iio/devices/iio:device*)
    //for device in $all_iio; do
    //	name=$(cat $device/name)
    //	if [ "$name" == "48003000.adc:adc@0" ]; then
    //		PATH_ADC0=$device
    //		break;
    //	fi
    //done
    //#PATH_ADC0=/sys/bus/iio/devices/iio:device4/

    //echo 0 > $PATH_ADC0/buffer/enable
    //echo 1 > $PATH_ADC0/buffer/enable

    m_pGpioAdc0BufferEnable->setLow();
    m_pGpioAdc0BufferEnable->setHigh();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void CpuMp157Driver::sequenceTimerReset()
{
    //#!/bin/sh
    //#Stop ENABLE timer
    //devmem 0x40001000 32 0x82

    //echo 1 > /sys/bus/platform/drivers/stm32-one-pulse/44006000.timer:op/prepare
    //echo 1 > /sys/bus/platform/drivers/stm32-one-pulse/40000000.timer:op/prepare

    //# set grindtime
    //#echo 2000 > /sys/bus/platform/drivers/stm32-one-pulse/40000000.timer:op/pulse_length

    write32bit( 0x40001000, 0x82 );         // stop ENABLE timer

    m_pGpioOnePulseTimer44006000Prepare->setHigh();
    m_pGpioOnePulseTimer40000000Prepare->setHigh();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void CpuMp157Driver::sequenceDisableTimer()
{
    m_pGpioOnePulseTimer44006000Disable->setHigh();
    m_pGpioOnePulseTimer40000000Disable->setHigh();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void CpuMp157Driver::sequenceUpdateTimer( const quint32 u32ClockTicks )
{
    m_pSystemFilePulseLength->writeFile( u32ClockTicks );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void CpuMp157Driver::logParameters()
{
    emit logInfoDouble( "Config", "AdcClockHz", m_dAdcClockHz );

    emit logInfoDouble( "Config", "SamplingFreqHz", m_dAdc1SamplingFreqHz );
    emit logInfoDouble( "Config", "DeltaTimeSec", m_dAdc1DeltaTimeSec );

    emit logInfoDouble( "Config", "AdcSampleTimeTicks", m_dAdc1SampleTimeTicks );
    emit logInfoDouble( "Config", "AdcOversampling", m_u32Adc1Oversampling );
    emit logInfoDouble( "Config", "AdcLeftShiftFactor", m_dAdc1LeftShiftFactor );
    emit logInfoDouble( "Config", "AdcDeltaTimeSec", m_dAdc1DeltaTimeSec );
    emit logInfoDouble( "Config", "AdcSamplingFreqHz", m_dAdc1SamplingFreqHz );

    emit logInfoDouble( "Config", "Adc2SampleTimeTicks", m_dAdc2SampleTimeTicks );
    emit logInfoDouble( "Config", "Adc2Oversampling", m_u32Adc2Oversampling );
    emit logInfoDouble( "Config", "Adc2LeftShiftFactor", m_dAdc2LeftShiftFactor );
    emit logInfoDouble( "Config", "Adc2DeltaTimeSec", m_dAdc2DeltaTimeSec );
    emit logInfoDouble( "Config", "Adc2SamplingFreqHz", m_dAdc2SamplingFreqHz );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

} // namespace SystemIo
