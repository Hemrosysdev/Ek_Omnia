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

#include <QQmlContext>
#include <QDebug>
#include <QFile>
#include <QJsonObject>
#include <QtMath>
#include <QStringList>

#include "SettingsSerializer.h"
#include "MainStatemachine.h"
#include "SqliteInterface.h"
#include "LockableFile.h"

#define MAX_READ_ERROR_CNT        100    // equal 10sec
#define READ_ERROR_UPPER_LIMIT    70
#define READ_ERROR_LOWER_LIMIT    30

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

DddDriver::DddDriver( QObject * pParent )
    : QThread( pParent )
{
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

DddDriver::~DddDriver()
{
    m_semStopThread.acquire();
    wait( 1000 );
    terminate();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void DddDriver::create( MainStatemachine * pMainStatemachine,
                        DddCouple *        pDddCouple,
                        const DddId        nDddId,
                        const DddType      nDddType )
{
    m_pMainStatemachine = pMainStatemachine;
    m_pDddCouple        = pDddCouple;
    m_nDddId            = nDddId;
    m_nDddType          = nDddType;
    m_strDeviceFile     = "/dev/ddd";

    // "/dev/dddpwm1"            // /sys/devices/platform/soc/44000000.timer/44000000.timer:pwm/pwm/pwmchip4/pwm3/capture
    // "/dev/dddpwm2"            // /sys/devices/platform/soc/44000000.timer/44000000.timer:pwm/pwm/pwmchip4/pwm0/capture

    // "/dev/dddadc1"            // /sys/devices/platform/soc/48003000.adc/48003000.adc:adc@100/iio:device1/in_voltage19_raw
    // "/dev/dddadc2"            // /sys/devices/platform/soc/48003000.adc/48003000.adc:adc@100/iio:device1/in_voltage11_raw

    if ( m_nDddType == DddType::DddTypePwm )
    {
        m_strDeviceFile += "pwm";
    }
    else if ( m_nDddType == DddType::DddTypeAdc )
    {
        m_strDeviceFile += "adc";
    }

    if ( m_nDddId == DddId::DddId1 )
    {
        m_strDeviceFile += "1";

        m_dAdcMinValue = m_dAdc1MinValue;
        m_dAdcMaxValue = m_dAdc1MaxValue;
    }
    else if ( m_nDddId == DddId::DddId2 )
    {
        m_strDeviceFile += "2";

        m_dAdcMinValue = m_dAdc2MinValue;
        m_dAdcMaxValue = m_dAdc2MaxValue;
    }

    MainStatemachine::ensureExistingDirectory( MainStatemachine::rootPath() + m_strDeviceFile );

#ifdef USE_ARITHMETIC_FILTER
    m_arithmeticFilter.reset( 0 );
#endif

    QThread::start();

    m_timerHistoryUpdate.setInterval( 3 );
    m_timerHistoryUpdate.setSingleShot( false );
    m_timerHistoryUpdate.start();

    m_nMaxFilterWindowStableCounter = 2000 / m_timerHistoryUpdate.interval();    // number of iterations for 2000ms

    connect( &m_timerHistoryUpdate, &QTimer::timeout, this, &DddDriver::updateHistory );
    connect( this, &DddDriver::rawAngleUpdate, this, &DddDriver::processNewRawAngle );
    connect( this, &DddDriver::statusChanged, this, &DddDriver::setFailed );

    readCalibrationValues();

    qInfo() << "DddDriver::create(): DDD type" << ( int ) m_nDddType << " ID" << ( int ) m_nDddId << " device file" << m_strDeviceFile;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void DddDriver::calibrateDdd()
{
    qDebug() << "DddDriver::calibrateDdd()";

    m_nCalibrationAngle = m_nLastFilterAngle;

    if ( m_nDddId == DddId::DddId1 )
    {
        m_pMainStatemachine->settingsSerializer()->setDdd1CalibrationAngleDeg( m_nCalibrationAngle );
    }
    else if ( m_nDddId == DddId::DddId2 )
    {
        m_pMainStatemachine->settingsSerializer()->setDdd2CalibrationAngleDeg( m_nCalibrationAngle );
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void DddDriver::readCalibrationValues()
{
    if ( m_nDddId == DddId::DddId1 )
    {
        m_nCalibrationAngle = m_pMainStatemachine->settingsSerializer()->ddd1CalibrationAngleDeg();
    }
    else if ( m_nDddId == DddId::DddId2 )
    {
        m_nCalibrationAngle = m_pMainStatemachine->settingsSerializer()->ddd2CalibrationAngleDeg();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

int DddDriver::calibrationAngle() const
{
    return m_nCalibrationAngle;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

int DddDriver::filterAngle() const
{
    return m_nLastFilterAngle;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

int DddDriver::rawAngle() const
{
    return m_nRawAngle;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

int DddDriver::calibratedFilterAngle() const
{
    return m_nLastFilterAngle - m_nCalibrationAngle;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

int DddDriver::calibratedRawAngle() const
{
    return m_nCurrentRawAngle - m_nCalibrationAngle;
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

#ifndef TARGETBUILD

void DddDriver::saveSimulatedScaleRotation( const double dSimulatedScaleRotation )
{
    {
        LockableFile file( MainStatemachine::rootPath() + m_strDeviceFile );

        if ( m_nDddType == DddType::DddTypePwm )
        {
            double dDuty = 0.0;

            if ( m_nDddId == DddId::DddIdFine )
            {
                double dTempSimulatedScaleRotation = std::fmod( dSimulatedScaleRotation * FINE_GEAR_RATIO, 360.0 );

                // scale the value in range 5% - 95% duty cycle
                dDuty = dTempSimulatedScaleRotation / 360.0 * 90.0 + 5.0;
            }
            else if ( m_nDddId == DddId::DddIdCoarse )
            {
                //double dSensorFullScaledRotation = 296.0;  // should be 288 * 360/354 in theory
                double dSensorFullScaledRotation = 360.0 * COARSE_GEAR_RATIO;  // should be 288 * 360/354 in theory

                // scale the value in range 5% - 95% duty cycle
                dDuty = ( dSensorFullScaledRotation - dSimulatedScaleRotation ) / dSensorFullScaledRotation * 90.0 + 5.0;
            }

            if ( file.open( QIODevice::WriteOnly ) )
            {
                QTextStream stream( &file );
                stream << "100.0 " << dDuty;
                file.flush();
                file.close();
            }
        }
        else if ( m_nDddType == DddType::DddTypeAdc )
        {
            qInfo() << "DddDriver::saveSimulatedScaleRotation(): ADC not implemented yet";
        }
        else
        {
            qInfo() << "DddDriver::saveSimulatedScaleRotation(): Unknown not implemented yet";
        }
    }
}

#endif

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void DddDriver::readPwmDeviceFile()
{
    static double dPwmMax = 0;
    static double dPwmMin = 99999999;

    QFile file( MainStatemachine::rootPath() + m_strDeviceFile );
    if ( file.open( QIODevice::ReadOnly | QIODevice::Unbuffered ) )
    {
        // this is a blocking call, if PWM signal isn't present
        // therefor this call is done a thread
        QByteArray baDddPwm = file.readAll();
        file.close();

        QStringList strDddPwm = QString::fromUtf8( baDddPwm ).split( ' ' );

        if ( strDddPwm.size() != 2 )
        {
            if ( m_nReadErrorCnt < MAX_READ_ERROR_CNT )
            {
                m_nReadErrorCnt++;
            }

            if ( m_nReadErrorCnt == 1 )
            {
                qCritical() << "readPwmDeviceFile(): Illegal content on " << static_cast<int>( m_nDddId ) << baDddPwm;
            }
        }
        else
        {
            if ( m_nReadErrorCnt > 0 )
            {
                m_nReadErrorCnt--;
            }

            double dPwmBase = strDddPwm[0].toDouble();
            double dDddPwm  = strDddPwm[1].toDouble();

            double dDutyCycle = dDddPwm / dPwmBase * 100.0;

            // derived 1/10° angle - see A1330 datasheet, page 9
            const double dOffset      = 5.0; // 4.97505 measured, but nominal 5.0 (%)
            int          nDddRawAngle = static_cast<int>( ( dDutyCycle - dOffset ) * 4.0 * 10.0 );
            //double dAngle = ( dDutyCycle - dOffset ) * 4.0 * 10.0;

            dPwmMin = std::min( dPwmMin, dDutyCycle );
            dPwmMax = std::max( dPwmMax, dDutyCycle );

            //            if ( m_nDddId == DddId::DddIdFine )
            //            {
            //                qInfo() << "readPwmDeviceFile(): " << static_cast<int>( m_nDddId ) << " / duty cycle <" << dDutyCycle << ">" << dAngle << dPwmMin << dPwmMax;
            //            }

            // emit wrties value into member variable - thread safe approach
            if ( nDddRawAngle < 0 )
            {
                setRawAngle( 0 );
            }
            else if ( nDddRawAngle > 3599 )
            {
                setRawAngle( 3599 );
            }
            else
            {
                setRawAngle( nDddRawAngle );
            }
        }
    }
    else
    {
        if ( m_nReadErrorCnt < MAX_READ_ERROR_CNT )
        {
            m_nReadErrorCnt++;
        }

        if ( m_nReadErrorCnt == 1 )
        {
            qCritical() << "readPwmDeviceFile(): Error on " << static_cast<int>( m_nDddId );
        }
    }

    updateStatus();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void DddDriver::readAdcDeviceFile()
{
    static double dAdcMax = 0;
    static double dAdcMin = 99999999;

    //qDebug() << "readAdcDeviceFile()" << (int) m_nDddId;

    QFile file( MainStatemachine::rootPath() + m_strDeviceFile );
    if ( file.open( QIODevice::ReadOnly ) )
    {
        if ( m_nReadErrorCnt > 0 )
        {
            m_nReadErrorCnt--;
        }

        // this is a blocking call, if PWM signal isn't present
        // therefor this call is done a thread
        QByteArray baDddAdc = file.readAll();
        file.close();

        QString strDddAdc = QString::fromUtf8( baDddAdc ).trimmed();

        double dAdc = strDddAdc.toDouble();

        // calc analog from ADC 16bit value relating to 3.3V
        const double dLsb = 3.3 / static_cast<double>( static_cast<quint32>( 0x1U << 16 ) - 1 );
        dAdc *= dLsb;

        // derived 1/10° angle - see A1330 datasheet, page 8 regarding clamping area
        int nDddRawAngle = static_cast<int>( ( dAdc - m_dAdcMinValue ) / ( m_dAdcMaxValue - m_dAdcMinValue ) * 360.0 * 10.0 );

        dAdcMin = std::min( dAdcMin, dAdc );
        dAdcMax = std::max( dAdcMax, dAdc );

        if ( m_nDddId == DddId::DddIdFine )
        {
            qInfo() << "readAdcDeviceFile()" << (int) m_nDddId << QString( "%1" ).arg( strDddAdc, 6 ) << dAdc << nDddRawAngle << dAdcMin << dAdcMax;
        }

        // emit wrties value into member variable - thread safe approach
        if ( nDddRawAngle < 0 )
        {
            setRawAngle( 0 );
        }
        else if ( nDddRawAngle > 3599 )
        {
            setRawAngle( 3599 );
        }
        else
        {
            setRawAngle( nDddRawAngle );
        }
    }
    else
    {
        if ( m_nReadErrorCnt < MAX_READ_ERROR_CNT )
        {
            m_nReadErrorCnt++;
        }

        if ( m_nReadErrorCnt == 1 )
        {
            qCritical() << "readAdcDeviceFile(): Error on " << static_cast<int>( m_nDddId );
        }
    }

    updateStatus();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void DddDriver::processNewRawAngle( const int nNewRawAngle )
{
    if ( nNewRawAngle != m_nCurrentRawAngle )
    {
        m_nCurrentRawAngle = nNewRawAngle;
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
    if ( m_nCurrentRawAngle >= 0
         && m_nCurrentRawAngle < 3600 )
    {
        int nSmoothOffsetRawAngle = smoothenRawAngle( m_nCurrentRawAngle );
        int nFilterRawAngle       = applyFilterWindow( nSmoothOffsetRawAngle );

        //        qDebug() << "avrgAngle " << dddAvrgAngle;
        //        qDebug() << "lastavrgAngle " << ThisLastDddAvrgAngle;

        // extract offset
        int nFilterAngle = ( nFilterRawAngle % 3600 );

        if ( nFilterAngle < 0 )
        {
            nFilterAngle += 3600;
        }

        // only report on changes
        setFilterAngle( nFilterAngle );
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
    int32_t n32LastSmoothAngle = ( m_nLastOffsetRawAngle % 3600 );

    // compensate overflow (359+1=0) and underflow (0-1=359)
    if ( !m_bFirstDddAngle )
    {
        if ( n32LastSmoothAngle >= 0 )
        {
            if ( n32LastSmoothAngle <= 900
                 && nOffsetRawAngle > 2700 )
            {
                m_nOffset360 -= 3600;
            }
            else if ( n32LastSmoothAngle >= 2700
                      && nOffsetRawAngle < 900 )
            {
                m_nOffset360 += 3600;
            }
        }
        else
        {
            if ( n32LastSmoothAngle >= -900
                 && nOffsetRawAngle < 900 )
            {
                m_nOffset360 += 3600;
            }
            else if ( n32LastSmoothAngle <= -2700
                      && nOffsetRawAngle > 2700 )
            {
                m_nOffset360 -= 3600;
            }
        }
        nOffsetRawAngle += m_nOffset360;
    }

    m_nLastOffsetRawAngle = nOffsetRawAngle;

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
        if ( std::abs( nAngle - m_nFilterWindowAngle ) < m_cWindowFilterRadius )
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
        m_nFilterWindowAngle = nAngle;
    }
    else if ( std::abs( nAngle - m_nFilterWindowAngle ) > m_cWindowFilterRadius )
    {
        m_nFilterWindowAngle         = nAngle;
        m_bFilterWindowMove          = true;
        m_nFilterWindowStableCounter = 0;
    }
    else
    {
        // don't change filter window
    }

    return m_nFilterWindowAngle;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void DddDriver::setFilterAngle( const int nFilterAngle )
{
    if ( m_nLastFilterAngle != nFilterAngle )
    {
        m_nLastFilterAngle = nFilterAngle;

        emit filterAngleChanged();
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
        emit failedChanged();
    }
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

void DddDriver::setRawAngle( const int nRawAngle )
{
    if ( m_nRawAngle != nRawAngle )
    {
        m_nRawAngle = nRawAngle;
        emit rawAngleUpdate( m_nRawAngle );
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void DddDriver::run()
{
    while ( m_semStopThread.available() )
    {
        if ( m_nDddType == DddType::DddTypePwm )
        {
            readPwmDeviceFile();
        }
        else if ( m_nDddType == DddType::DddTypeAdc )
        {
            readAdcDeviceFile();
        }

        msleep( 50 );
    }

    m_semStopThread.release();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////
