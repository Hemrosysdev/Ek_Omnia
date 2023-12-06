///////////////////////////////////////////////////////////////////////////////
///
/// @file PwmCaptureDriver.cpp
///
/// @brief Implementation file of class PwmCaptureDriver.
///
/// @author Ultratronik GmbH
///         Dornierstr. 9
///         D-82205 Gilching
///         Germany
///         http://www.ultratronik.de
///
/// @author written by Gerd Esser, Forschung & Entwicklung, gesser@ultratronik.de
///
/// @date 17.12.2022
///
/// @copyright Copyright 2022 by Hemro International AG
///            Hemro International AG
///            Länggenstrasse 34
///            CH 8184 Bachenbülach
///            Switzerland
///            Homepage: www.hemrogroup.com
///
///////////////////////////////////////////////////////////////////////////////

#include "PwmCaptureDriver.h"

#include <QDebug>
#include <QStringList>
#include <cmath>

#include "SystemDeviceFile.h"
#include "MainStatemachine.h"

namespace SystemIo
{

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

PwmCaptureDriver::PwmCaptureDriver( QObject * pParent )
    : QThread( pParent )
    , m_pSystemFileCapture( new SystemDeviceFile() )
    , m_semStopThread( 1 )
{
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

PwmCaptureDriver::~PwmCaptureDriver()
{
    stopThread();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void PwmCaptureDriver::create( const QString & strDriverName,
                               const QString & strCaptureDeviceFile,
                               const int       nPollCycleMs,
                               const int       nMaxReadErrorCnt,
                               const int       nErrorCntHysteresis,
                               const bool      bUpdateCyclic,
                               const bool      bStartThread )
{
    m_strDriverName = strDriverName;
    m_pSystemFileCapture->setFileName( strCaptureDeviceFile );
    m_nPollCycleMs        = nPollCycleMs;
    m_nMaxReadErrorCnt    = nMaxReadErrorCnt;
    m_nErrorCntHysteresis = nErrorCntHysteresis;
    m_bUpdateCyclic       = bUpdateCyclic;

    m_nReadErrorCnt = m_nMaxReadErrorCnt;

    if ( bStartThread )
    {
        startThread();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void PwmCaptureDriver::create( const QString & strDriverName,
                               const quint32   u32PwmCaptureRegisterAddress,
                               const int       nPwmCaptureChip,
                               const int       nPwmCaptureChannel,
                               const int       nPollCycleMs,
                               const int       nMaxReadErrorCnt,
                               const int       nErrorCntHysteresis,
                               const bool      bUpdateCyclic,
                               const bool      bStartThread )
{
#ifdef __WIN32__
    Q_UNUSED( u32PwmCaptureRegisterAddress );
    Q_UNUSED( nPwmCaptureChip );

    QString strSystemDir = QString( "%1/dev" )
                           .arg( MainStatemachine::rootPath() );
    SystemDeviceFile( strSystemDir + "/export" ).writeFile( nPwmCaptureChannel );
    QString strCaptureDeviceFile = QString( "%1/dddpwm%2" ).arg( strSystemDir ).arg( nPwmCaptureChannel );
#else
    QString strSystemDir = QString( "%1/sys/devices/platform/soc/%2.timer/%3.timer:pwm/pwm/pwmchip%4" )
                           .arg( MainStatemachine::rootPath() )
                           .arg( u32PwmCaptureRegisterAddress, 8, 16, QLatin1Char( '0' ) )
                           .arg( u32PwmCaptureRegisterAddress, 8, 16, QLatin1Char( '0' ) )
                           .arg( nPwmCaptureChip );
    SystemDeviceFile( strSystemDir + "/export" ).writeFile( nPwmCaptureChannel );
    QString strCaptureDeviceFile = QString( "%1/pwm%2/capture" ).arg( strSystemDir ).arg( nPwmCaptureChannel );
#endif

    MainStatemachine::ensureExistingDirectory( strCaptureDeviceFile );

    m_strDriverName = strDriverName;
    m_pSystemFileCapture->setFileName( strCaptureDeviceFile );
    m_nPollCycleMs        = nPollCycleMs;
    m_nMaxReadErrorCnt    = nMaxReadErrorCnt;
    m_nErrorCntHysteresis = nErrorCntHysteresis;
    m_bUpdateCyclic       = bUpdateCyclic;

    m_nReadErrorCnt = m_nMaxReadErrorCnt;

    if ( bStartThread )
    {
        startThread();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool PwmCaptureDriver::isFailed() const
{
    return m_bFailed;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

double PwmCaptureDriver::dutyCycle() const
{
    return m_dDutyCycle;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

double SystemIo::PwmCaptureDriver::frequency() const
{
    return m_dFrequency;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void PwmCaptureDriver::startThread()
{
    QThread::start();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void PwmCaptureDriver::stopThread()
{
    m_semStopThread.acquire();
    wait( 500 );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void PwmCaptureDriver::readPwmDeviceFile()
{
    bool    bSuccess = false;
    QString strError;
    double  dDutyCycle = 0.0;
    double  dFrequency = 0.0;

    QString strCapture;

    if ( m_pSystemFileCapture->readFile( strCapture ) )
    {
        QStringList strPwmCouple = strCapture.split( ' ' );

        if ( strPwmCouple.size() != 2 )
        {
            strError = "Error, illegal capture content " + strCapture;
        }
        else
        {
            double dPwmPeriod = strPwmCouple[0].toDouble();
            double dPwmDuty   = strPwmCouple[1].toDouble();

            if ( dPwmPeriod > 1.e-15 )
            {
                bSuccess   = true;
                dDutyCycle = ( dPwmDuty / dPwmPeriod * 100.0 );
                dFrequency = ( c_dNanoSecsPerSec / dPwmPeriod );
            }
            else
            {
                strError = "Error, period is null";
            }
        }
    }
    else
    {
        strError = "Error file read";
    }

    if ( bSuccess )
    {
        if ( m_nReadErrorCnt > 0 )
        {
            m_nReadErrorCnt--;
        }
    }
    else
    {
        if ( m_nReadErrorCnt < m_nMaxReadErrorCnt )
        {
            m_nReadErrorCnt++;
        }

        if ( m_nReadErrorCnt == 1 )
        {
            qCritical() << "PwmCaptureDriver::readPwmDeviceFile():" << m_strDriverName << ":" << strError;
        }
    }

    if ( m_nReadErrorCnt > m_nMaxReadErrorCnt )
    {
        setFailed( true );
    }
    else if ( m_nReadErrorCnt <= m_nMaxReadErrorCnt - m_nErrorCntHysteresis )
    {
        setFailed( false );
    }
    else
    {
        // no sensor status change
    }

    setDutyCycle( dDutyCycle );
    setFrequency( dFrequency );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void PwmCaptureDriver::setFailed( const bool bFailed )
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

void PwmCaptureDriver::setDutyCycle( const double dDutyCycle )
{
    if ( std::fabs( m_dDutyCycle - dDutyCycle ) > 1.e-15 )
    {
        m_dDutyCycle = dDutyCycle;
        emit dutyCycleChanged( m_dDutyCycle );
    }
    else if ( m_bUpdateCyclic )
    {
        emit dutyCycleChanged( m_dDutyCycle );
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void PwmCaptureDriver::setFrequency( const double dFrequency )
{
    if ( std::fabs( m_dFrequency - dFrequency ) > 1.e-15 )
    {
        m_dFrequency = dFrequency;
        emit frequencyChanged( m_dFrequency );
    }
    else if ( m_bUpdateCyclic )
    {
        emit frequencyChanged( m_dFrequency );
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void PwmCaptureDriver::run()
{
    qInfo() << "PwmCaptureDriver::run()" << m_strDriverName;

    while ( m_semStopThread.available() )
    {
        readPwmDeviceFile();
        msleep( m_nPollCycleMs );
    }

    m_semStopThread.release();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

} // namespace Systemio
