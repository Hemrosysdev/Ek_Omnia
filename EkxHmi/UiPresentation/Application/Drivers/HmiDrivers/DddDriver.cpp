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
#include <QJsonDocument>
#include <QJsonObject>
#include <QtMath>
#include <QStringList>

#include "SettingsSerializer.h"
#include "MainStatemachine.h"
#include "SqliteInterface.h"

#define DDD_DEVICE_FILE       "/dev/ddd"                // /sys/devices/platform/soc/44000000.timer/44000000.timer:pwm/pwm/pwmchip*/pwm0/capture

#ifndef TARGETBUILD
#define DDD_SIMULATION_FILE   "/config/DddSimulation.json"
#endif

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

DddDriver::DddDriver( QQmlEngine * engine,
                      MainStatemachine * pMainStatemachine )
    : QThread( pMainStatemachine )
    , m_pMainStatemachine( pMainStatemachine )
{
    MainStatemachine::ensureExistingDirectory( MainStatemachine::rootPath() + DDD_DEVICE_FILE );
#ifndef TARGETBUILD
    MainStatemachine::ensureExistingDirectory( MainStatemachine::rootPath() + DDD_SIMULATION_FILE );
#endif
    engine->rootContext()->setContextProperty( "dddDriver", this );

    memset( m_nDddAngleHistory, 0, sizeof( m_nDddAngleHistory ) );

    QThread::start();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

DddDriver::~DddDriver()
{
    terminate();
    wait( 500 );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void DddDriver::create()
{
    m_timerDddHistoryUpdate.setInterval( 3 );
    m_timerDddHistoryUpdate.setSingleShot( false );
    m_timerDddHistoryUpdate.start();

    connect( &m_timerDddHistoryUpdate, &QTimer::timeout, this, &DddDriver::dddUpdateHistory );
    connect( this, &DddDriver::rawAngleUpdate, this, &DddDriver::processNewRawAngle );

    grinderSettingsUpdated();

#ifndef TARGETBUILD
    readSimulatedScaleRotation();
#endif
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void DddDriver::calibrateDdd()
{
    qDebug() << "DddDriver::calibrateDdd()";

    m_nDddCalibrationAngle = m_nLastDddAvrgAngle;

    // update display
    calculateDddValue( m_nLastDddAvrgAngle );

    m_pMainStatemachine->settingsStatemachine()->settingsSerializer()->setDddCalibrationAngleDeg( m_nDddCalibrationAngle );
    m_pMainStatemachine->settingsStatemachine()->settingsSerializer()->saveSettings();

    m_pMainStatemachine->sqliteInterface()->addEvent( EkxSqliteTypes::SqliteEventType_DDD_CALIBRATE,
                                                      QString( "%1" ).arg( m_nDddCalibrationAngle ) );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void DddDriver::grinderSettingsUpdated()
{
    m_nDddCalibrationAngle = m_pMainStatemachine->settingsStatemachine()->settingsSerializer()->getDddCalibrationAngleDeg();
    // update display
    calculateDddValue( m_nLastDddAvrgAngle );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#ifndef TARGETBUILD
void DddDriver::incSimulatedScaleRotation( const int nInc )
{
    setSimulatedScaleRotation( simulatedScaleRotation() + nInc );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

int DddDriver::simulatedScaleRotation() const
{
    return m_nSimulatedScaleRotation;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void DddDriver::setSimulatedScaleRotation(const int nSimulatedScaleRotation)
{
    int nTempSimulatedScaleRotation = nSimulatedScaleRotation;

    //qInfo() << "setSimulatedScaleRotation" << nTempSimulatedScaleRotation;

    if ( nTempSimulatedScaleRotation < 0 )
        nTempSimulatedScaleRotation = 0;

    else if ( nTempSimulatedScaleRotation > 288 )
        nTempSimulatedScaleRotation = 288;

    if ( m_nSimulatedScaleRotation != nTempSimulatedScaleRotation )
    {
        m_nSimulatedScaleRotation = nTempSimulatedScaleRotation;
        emit simulatedScaleRotationChanged();

        saveSimulatedScaleRotation();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void DddDriver::saveSimulatedScaleRotation( void )
{
    {
        QFile file( MainStatemachine::rootPath() + DDD_DEVICE_FILE );

        double dDuty = ( 288.0 - simulatedScaleRotation() ) / 288.0 * 90.0 + 5.0;

        if ( file.open( QIODevice::WriteOnly ) )
        {
            QTextStream stream( &file );
            stream << "100.0 " << dDuty;
            file.flush();
            file.close();
        }
    }

    {
        QFile file( MainStatemachine::rootPath() + DDD_SIMULATION_FILE );

        if ( file.open( QIODevice::WriteOnly ) )
        {
            QJsonObject json;

            json["SimulatedScaleRotation"] = simulatedScaleRotation();

            QJsonDocument doc( json );
            file.write( doc.toJson() );
            file.flush();
            file.close();
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void DddDriver::readSimulatedScaleRotation()
{
    QFile file( MainStatemachine::rootPath() + DDD_SIMULATION_FILE );
    if ( file.open(QIODevice::ReadOnly) )
    {
        QByteArray ba = file.readAll();
        file.close();

        QJsonDocument doc = QJsonDocument::fromJson(ba);

        QJsonObject obj = doc.object();

        setSimulatedScaleRotation( obj["SimulatedScaleRotation"].toInt() );
    }
}
#endif

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void DddDriver::readDddDeviceFile()
{
    //qDebug() << "readDddDeviceFile()";

    QFile file( MainStatemachine::rootPath() + DDD_DEVICE_FILE );
    if ( file.open( QIODevice::ReadOnly ) )
    {
        // this is a blocking call, if PWM signal isn't present
        // therefor this call is done a thread
        QByteArray baDddPwm = file.readAll();
        file.close();

        QStringList strDddPwm = QString::fromUtf8(baDddPwm).split( ' ' );

        if ( strDddPwm.size() == 2 )
        {
            double pwmBase  = strDddPwm[0].toDouble();
            double dddPwm   = strDddPwm[1].toDouble();

            double dDutyCycle = dddPwm / pwmBase * 100.0;

            //qDebug() << "readFile(): duty cycle <" << dDutyCycle << ">";

            // derived 1/10° angle - see A1330 datasheet, page 9
            int dddRawAngle = static_cast<int>( ( dDutyCycle - 5.0 ) * 4.0 * 10.0 );

            // invert the rotation for PT-A
            //dddRawAngle = 3599 - dddRawAngle;
            // simple scaling for PT-A
            //dddRawAngle = static_cast<int>( static_cast<double>( dddRawAngle * ( 180.0 / 212.5 ) ) );
            if ( dddRawAngle < 0 )
            {
                emit rawAngleUpdate( 0 );
            }
            else if ( dddRawAngle > 3599 )
            {
                emit rawAngleUpdate( 3599 );
            }
            else
            {
                emit rawAngleUpdate( dddRawAngle );
            }
        }
    }
    else
    {
        //qDebug() << "readFile(): failed to read file <" << DDD_DEVICE_FILE << ">";
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void DddDriver::processNewRawAngle( const int nNewRawAngle )
{
    if ( nNewRawAngle != m_nDddCurrentRawAngle )
    {
        //qDebug() << "processNewRawAngle " << nNewRawAngle;
        m_nDddCurrentRawAngle = nNewRawAngle;
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void DddDriver::dddUpdateHistory()
{
    //qDebug() << "dddUpdateHistory() " ;

    // consider only valid values
    if ( m_nDddCurrentRawAngle >= 0 && m_nDddCurrentRawAngle < 3600 )
    {
        int dddAvrgAngle = calculateAverageAngle( m_nDddCurrentRawAngle );

        //        qDebug() << "avrgAngle " << dddAvrgAngle;
        //        qDebug() << "lastavrgAngle " << ThisLastDddAvrgAngle;

        int dddProcessAngle = ( dddAvrgAngle % 3600 );

        if ( dddProcessAngle < 0 )
        {
            dddProcessAngle += 3600;
        }

        // only report on changes
        if ( dddProcessAngle != m_nLastDddAvrgAngle )
        {
            m_nLastDddAvrgAngle = dddProcessAngle;

            //qDebug() << "calculateDddValue?? " ;

            calculateDddValue( dddProcessAngle );
        }
    }

}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

int DddDriver::calculateAverageAngle( const int nDddRawAngle )
{
    int nDddAngle = nDddRawAngle;

    int32_t n32LastDddAngle = ( m_nLastDddAngle % 3600 );

    // compensate overflow (359+1=0) and underflow (0-1=359)
    if ( !m_bFirstDddAngle )
    {
        if ( n32LastDddAngle >= 0 )
        {
            if ( n32LastDddAngle <= 900 && nDddAngle > 2700 )
            {
                m_nOffset360 -= 3600;
            }
            else if ( n32LastDddAngle >= 2700 && nDddAngle < 900 )
            {
                m_nOffset360 += 3600;
            }
        }
        else
        {
            if ( n32LastDddAngle >= -900 && nDddAngle < 900 )
            {
                m_nOffset360 += 3600;
            }
            else if ( n32LastDddAngle <= -2700 && nDddAngle > 2700 )
            {
                m_nOffset360 -= 3600;
            }
        }
        nDddAngle += m_nOffset360;
    }

    m_nLastDddAngle = nDddAngle;

    // fill history buffer if we init the application
    if ( m_bFirstDddAngle )
    {
        for ( quint8 i = 0; i < MAX_DDD_ANGLE_HISTORY_SIZE; i++ )
        {
            m_nDddAngleHistory[i] = nDddAngle;
        }
        m_bFirstDddAngle = false;
    }
    else
    {
        m_nDddAngleHistory[m_nDddAngleHistoryPos] = nDddAngle;
    }
    m_nDddAngleHistoryPos = ( m_nDddAngleHistoryPos + 1 ) % MAX_DDD_ANGLE_HISTORY_SIZE;

    int dddAvrgAngle = 0;
    for ( quint8 i = 0; i < MAX_DDD_ANGLE_HISTORY_SIZE; i++ )
    {
        dddAvrgAngle += m_nDddAngleHistory[i];
    }
    dddAvrgAngle /= MAX_DDD_ANGLE_HISTORY_SIZE;

    return dddAvrgAngle;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void DddDriver::calculateDddValue( const int nDddProcessAngle )
{
    double dDddAngle = static_cast<double>( nDddProcessAngle - m_nDddCalibrationAngle );

    dDddAngle /= 10.0;

    if ( dDddAngle < 0 )
    {
        dDddAngle += 360.0;
    }

    // turn around rotate direction
    dDddAngle = 360.0 - dDddAngle;
    // compensate transmission in mechanics
    //dDddAngle *= 180.0 / 218.5;
    dDddAngle *= 180.0 / 224.0;

    //qDebug() << "calculateDddValue " << dddProcessAngle << " / " << ThisDddCalibrationAngle << " / " << ( int ) dddAngle;

    // convert angle to grinding degree/coarseness value
    // 288° rotation to 80.0 coarseness
    double dddValue = ( dDddAngle * 80.0 ) / 288.0;  //* 800/360 --> 800 auf 80.0

    //qDebug() << "calculateDddValue: " << dddValue;

    // shift comma to the left
    //dddValue = dddValue / 10.0;

    int bigDddValue = static_cast<int>(qFloor(dddValue));
    int commaDddValue = QString( QString::number(static_cast<int>(( dddValue * 10.0 ))).back()).toInt();

    setDddBigValue(bigDddValue);
    setDddCommaValue(commaDddValue);
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

int DddDriver::getDddBigValue() const
{
    return m_nDddBigValue;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void DddDriver::setDddBigValue( const int nDddBigValue )
{
    if ( nDddBigValue != m_nDddBigValue )
    {
        m_nDddBigValue = nDddBigValue;

        emit dddBigValueChanged();
        emit dddValueChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

int DddDriver::getDddCommaValue() const
{
    return m_nDddCommaValue;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void DddDriver::setDddCommaValue( const int nDddCommaValue )
{
    if ( nDddCommaValue != m_nDddCommaValue )
    {
        m_nDddCommaValue = nDddCommaValue;

        emit dddCommaValueChanged();
        emit dddValueChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void DddDriver::run()
{
    while ( true )
    {
        readDddDeviceFile();
        msleep( 100 );
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////
