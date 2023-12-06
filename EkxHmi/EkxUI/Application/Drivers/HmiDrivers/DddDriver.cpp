///////////////////////////////////////////////////////////////////////////////
///
/// @file DddDriver.cpp
///
/// @brief Implementation file of class DddDriver.
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

#include "DddDriver.h"

#include <QDebug>
#include <QFile>
#include <QJsonObject>
#include <QtMath>
#include <QStringList>

#include "PwmCaptureDriver.h"
#include "AdcVoltageInDriver.h"

#define MAX_READ_ERROR_CNT        100    // equal 10sec
#define READ_ERROR_UPPER_LIMIT    70
#define READ_ERROR_LOWER_LIMIT    30

namespace SystemIo
{

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

DddDriver::DddDriver( QObject * pParent )
    : QObject( pParent )
    , m_pPwmCapture( new PwmCaptureDriver() )
    , m_pAdcVoltageIn( new AdcVoltageInDriver() )
{
    m_pAdcVoltageIn->setObjectName( "AdcVoltageInDddAnalog" );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

DddDriver::~DddDriver()
{
    m_pPwmCapture   = nullptr;
    m_pAdcVoltageIn = nullptr;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void DddDriver::create( const QString & strDriverName,
                        const DddType   nDddType,
                        const quint32   u32PwmCaptureRegisterAddress,
                        const int       nPwmCaptureChip,
                        const int       nPwmCaptureChannel,
                        const QString & strAdcSystemDeviceDir,
                        const QString & strAdcChannel,
                        const int       nPollCycleMs,
                        const int       nMaxReadErrorCnt )
{
    qInfo() << "DddDriver::create(): name" << strDriverName << "DDD type" << nDddType;

    m_strDriverName = strDriverName;

    setDddType( nDddType );

#ifdef USE_ARITHMETIC_FILTER
    m_arithmeticFilter.init( MAX_DDD_ANGLE_HISTORY_SIZE, 0 );
#endif

    if ( dddType() == DddType::DddTypePwm )
    {
        m_pPwmCapture->create( strDriverName + "PwmCapture",
                               u32PwmCaptureRegisterAddress,
                               nPwmCaptureChip,
                               nPwmCaptureChannel,
                               nPollCycleMs,
                               nMaxReadErrorCnt,
                               2,
                               true,
                               ( nDddType == DddType::DddTypePwm ) );
    }

    if ( dddType() == DddType::DddTypeAdc )
    {
        m_pAdcVoltageIn->create( strDriverName + "AdcVoltageIn",
                                 strAdcSystemDeviceDir,
                                 strAdcChannel,
                                 true,
                                 c_nDmaAdcFilterSize,
                                 nPollCycleMs,
                                 true );
    }
    m_timerHistoryUpdate.setInterval( 3 );
    m_timerHistoryUpdate.setSingleShot( false );
    m_timerHistoryUpdate.start();

    m_nMaxFilterWindowStableCounter = 2000 / m_timerHistoryUpdate.interval();    // number of iterations for 2000ms

    connect( &m_timerHistoryUpdate, &QTimer::timeout, this, &DddDriver::updateHistory );
    connect( this, &DddDriver::rawAngleChanged, this, &DddDriver::processNewRawAngle );
    connect( this, &DddDriver::statusChanged, this, &DddDriver::setFailed );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void DddDriver::setDddType( const DddType nDddType )
{
    qInfo() << "DddDriver::setDddType()" << nDddType;

    //if ( m_nDddType != nDddType )
    {
        m_nDddType = nDddType;

        if ( m_nDddType == DddType::DddTypePwm )
        {
            disconnect( m_pAdcVoltageIn.get(), &AdcVoltageInDriver::voltageChanged, this, &DddDriver::processAdcVoltage );
            connect( m_pPwmCapture.get(), &PwmCaptureDriver::dutyCycleChanged, this, &DddDriver::processPwmDutyCycle );
            m_pPwmCapture->startThread();
        }
        else // DddTypeAdc
        {
            m_pPwmCapture->stopThread();
            disconnect( m_pPwmCapture.get(), &PwmCaptureDriver::dutyCycleChanged, this, &DddDriver::processPwmDutyCycle );
            connect( m_pAdcVoltageIn.get(), &AdcVoltageInDriver::voltageChanged, this, &DddDriver::processAdcVoltage );
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

DddDriver::DddType DddDriver::dddType() const
{
    return m_nDddType;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void DddDriver::calibrateDdd()
{
    qDebug() << "DddDriver::calibrateDdd()";

    m_nCalibrationAngle10thDeg = m_nFilterAngle10thDeg;

    emit calibrationAngleChanged( m_nCalibrationAngle10thDeg );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void DddDriver::setCalibrationAngle10thDeg( const int nCalibrationAngle10thDeg )
{
    m_nCalibrationAngle10thDeg = nCalibrationAngle10thDeg;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

int DddDriver::calibrationAngle10thDeg() const
{
    return m_nCalibrationAngle10thDeg;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

int DddDriver::filterAngle10thDeg() const
{
    return m_nFilterAngle10thDeg;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

int DddDriver::rawAngle10thDeg() const
{
    return m_nRawAngle10thDeg;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

int DddDriver::calibratedFilterAngle10thDeg() const
{
    return m_nFilterAngle10thDeg - m_nCalibrationAngle10thDeg;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

int DddDriver::calibratedRawAngle10thDeg() const
{
    return m_nCurrentRawAngle10thDeg - m_nCalibrationAngle10thDeg;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

int DddDriver::dddValueMym() const
{
    return m_nDddValueMym;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool DddDriver::isFailed() const
{
    return m_bFailed;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

QSharedPointer<AdcVoltageInDriver> DddDriver::adcVoltageInDriver()
{
    return m_pAdcVoltageIn;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void DddDriver::processNewRawAngle( const int nNewRawAngle10thDeg )
{
    if ( nNewRawAngle10thDeg != m_nCurrentRawAngle10thDeg )
    {
        m_nCurrentRawAngle10thDeg = nNewRawAngle10thDeg;
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void DddDriver::updateHistory()
{
    //qDebug() << "updateHistory() " ;

    // consider only valid values
    if ( m_nCurrentRawAngle10thDeg >= 0
         && m_nCurrentRawAngle10thDeg < 3600 )
    {
        int nSmoothOffsetRawAngle = smoothenRawAngle( m_nCurrentRawAngle10thDeg );
        int nFilterRawAngle       = applyFilterWindow( nSmoothOffsetRawAngle );

        // extract offset
        int nFilterAngle = ( nFilterRawAngle % 3600 );

        if ( nFilterAngle < 0 )
        {
            nFilterAngle += 3600;
        }

        // only report on changes
        setFilterAngle10thDeg( nFilterAngle );

        setDddValueMym( static_cast<int>( static_cast<double>( calibratedFilterAngle10thDeg() ) * c_dFakeGearRatio ) );
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

int DddDriver::smoothenRawAngle( const int nNewRawAngle )
{
    int nOffsetRawAngle = nNewRawAngle;

    // extract offset from previous iteration
    int32_t n32LastSmoothAngle = ( m_nLastOffsetRawAngle10thDeg % 3600 );

    // compensate overflow (359+1=0) and underflow (0-1=359)
    if ( !m_bFirstDddAngle )
    {
        if ( n32LastSmoothAngle >= 0 )
        {
            if ( n32LastSmoothAngle <= 900
                 && nOffsetRawAngle > 2700 )
            {
                m_nOffset36010thDeg -= 3600;
            }
            else if ( n32LastSmoothAngle >= 2700
                      && nOffsetRawAngle < 900 )
            {
                m_nOffset36010thDeg += 3600;
            }
        }
        else
        {
            if ( n32LastSmoothAngle >= -900
                 && nOffsetRawAngle < 900 )
            {
                m_nOffset36010thDeg += 3600;
            }
            else if ( n32LastSmoothAngle <= -2700
                      && nOffsetRawAngle > 2700 )
            {
                m_nOffset36010thDeg -= 3600;
            }
        }
        nOffsetRawAngle += m_nOffset36010thDeg;
    }

    m_nLastOffsetRawAngle10thDeg = nOffsetRawAngle;

    int nSmoothOffsetRawAngle = 0;
    // fill history buffer if we init the application
    if ( m_bFirstDddAngle )
    {
#ifdef USE_ARITHMETIC_FILTER
        m_arithmeticFilter.reset( nOffsetRawAngle );
#endif
        m_bFirstDddAngle = false;

        nSmoothOffsetRawAngle = nOffsetRawAngle;
    }
    else
    {
#ifdef USE_ARITHMETIC_FILTER
        nSmoothOffsetRawAngle = m_arithmeticFilter.append( nOffsetRawAngle );
#endif

#ifdef USE_MEDIAN_FILTER
        nSmoothOffsetRawAngle = m_medianFilter.Insert( nOffsetRawAngle );
#endif
    }

    return nSmoothOffsetRawAngle;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

int DddDriver::applyFilterWindow( const int nAngle )
{
    if ( m_bFilterWindowMove )
    {
        if ( std::abs( nAngle - m_nFilterWindowAngle10thDeg ) < m_cWindowFilterRadius )
        {
            m_nFilterWindowStableCounter++;
        }
        else
        {
            m_nFilterWindowStableCounter = 0;
        }

        if ( m_nFilterWindowStableCounter > m_nMaxFilterWindowStableCounter )
        {
            m_bFilterWindowMove = false;
        }
        m_nFilterWindowAngle10thDeg = nAngle;
    }
    else if ( std::abs( nAngle - m_nFilterWindowAngle10thDeg ) > m_cWindowFilterRadius )
    {
        m_nFilterWindowAngle10thDeg  = nAngle;
        m_bFilterWindowMove          = true;
        m_nFilterWindowStableCounter = 0;
    }
    else
    {
        // don't change filter window
    }

    return m_nFilterWindowAngle10thDeg;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void DddDriver::setFilterAngle10thDeg( const int nFilterAngle10thDeg )
{
    if ( m_nFilterAngle10thDeg != nFilterAngle10thDeg )
    {
        m_nFilterAngle10thDeg = nFilterAngle10thDeg;
        emit filterAngleChanged( m_nFilterAngle10thDeg );
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void DddDriver::setFailed( const bool bFailed )
{
    if ( m_bFailed != bFailed )
    {
        m_bFailed = bFailed;
        emit failedChanged( m_bFailed );
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void DddDriver::processPwmDutyCycle( const double dDutyCycle )
{
    // PWM already implements a hysteresis
    if ( m_pPwmCapture->isFailed() )
    {
        m_nReadErrorCnt = MAX_READ_ERROR_CNT + 1;
        //qInfo() << "readPwmDeviceFile(): failed, error count" << m_nReadErrorCnt;
    }
    else
    {
        static double dPwmMax = 0;
        static double dPwmMin = 99999999;

        m_nReadErrorCnt = 0;

        // derived 1/10° angle - see A1330 datasheet, page 9
        const double dOffset             = 5.0; // 4.97505 measured, but nominal 5.0 (%)
        int          nDddRawAngle10thDeg = static_cast<int>( ( dDutyCycle - dOffset ) * 4.0 * 10.0 );
        //double dAngle = ( dDutyCycle - dOffset ) * 4.0 * 10.0;

        dPwmMin = std::min( dPwmMin, dDutyCycle );
        dPwmMax = std::max( dPwmMax, dDutyCycle );

        //qInfo() << "readPwmDeviceFile(): " << "duty cycle " << dDutyCycle << "%" << nDddRawAngle << dPwmMin << dPwmMax;

        // emit writes value into member variable - thread safe approach
        if ( nDddRawAngle10thDeg < 0 )
        {
            setRawAngle10thDeg( 0 );
        }
        else if ( nDddRawAngle10thDeg > 3599 )
        {
            setRawAngle10thDeg( 3599 );
        }
        else
        {
            setRawAngle10thDeg( nDddRawAngle10thDeg );
        }
    }

    updateStatus();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void DddDriver::processAdcVoltage( const double dAdcVoltageMv )
{
    if ( !m_pAdcVoltageIn->isValid() )
    {
        if ( m_nReadErrorCnt < MAX_READ_ERROR_CNT )
        {
            m_nReadErrorCnt++;
        }
    }
    else
    {
        static double dAdcMax = 0;
        static double dAdcMin = 99999999;

        if ( m_nReadErrorCnt > 0 )
        {
            m_nReadErrorCnt--;
        }

        // derived 1/10° angle - see A1330 datasheet, page 8 regarding clamping area
        int nDddRawAngle = static_cast<int>( ( dAdcVoltageMv - m_dAdcMinVoltageMv ) / ( m_dAdcMaxVoltageMv - m_dAdcMinVoltageMv ) * 360.0 * 10.0 );

        dAdcMin = std::min( dAdcMin, dAdcVoltageMv );
        dAdcMax = std::max( dAdcMax, dAdcVoltageMv );

        // emit writes value into member variable - thread safe approach
        if ( nDddRawAngle < 0 )
        {
            setRawAngle10thDeg( 0 );
        }
        else if ( nDddRawAngle > 3599 )
        {
            setRawAngle10thDeg( 3599 );
        }
        else
        {
            setRawAngle10thDeg( nDddRawAngle );
        }
    }

    updateStatus();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void DddDriver::updateStatus()
{
    if ( m_nReadErrorCnt > READ_ERROR_UPPER_LIMIT )
    {
        setFailed( true );
    }
    else if ( m_nReadErrorCnt < READ_ERROR_LOWER_LIMIT )
    {
        setFailed( false );
    }
    else
    {
        // no sensor status change
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void DddDriver::setRawAngle10thDeg( const int nRawAngle )
{
    if ( m_nRawAngle10thDeg != nRawAngle )
    {
        m_nRawAngle10thDeg = nRawAngle;
        emit rawAngleChanged( m_nRawAngle10thDeg );
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void DddDriver::setDddValueMym( const int nDddValueMym )
{
    if ( m_nDddValueMym != nDddValueMym )
    {
        m_nDddValueMym = nDddValueMym;
        emit dddValueChanged( m_nDddValueMym );
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

} // namespace Systemio
