///////////////////////////////////////////////////////////////////////////////
///
/// @file DddCouple.cpp
///
/// @brief Implementation file of class DddCouple.
///
/// @author Ultratronik GmbH
///         Dornierstr. 9
///         D-82205 Gilching
///         Germany
///         http://www.ultratronik.de
///
/// @author written by Gerd Esser, Forschung & Entwicklung, gesser@ultratronik.de
///
/// @date 23.03.2022
///
/// @copyright Copyright 2022 by Hemro International AG
///            Hemro International AG
///            Länggenstrasse 34
///            CH 8184 Bachenbülach
///            Switzerland
///            Homepage: www.hemrogroup.com
///
///////////////////////////////////////////////////////////////////////////////

#include "DddCouple.h"

#include <QDebug>
#include <QJsonObject>
#include <QtMath>
#include <QStringList>

#include "JsonHelper.h"
#include "SettingsSerializer.h"
#include "MainStatemachine.h"
#include "DeviceInfoCollector.h"
#include "SqliteInterface.h"
#include "DddDriver.h"
#include "LockableFile.h"

#ifndef TARGETBUILD
#define DDD_SIMULATION_FILE   "/config/DddSimulation.json"
#endif

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

DddCouple::DddCouple( QObject * pParent )
    : QObject( pParent )
{
#ifndef TARGETBUILD
    MainStatemachine::ensureExistingDirectory( MainStatemachine::rootPath() + DDD_SIMULATION_FILE );
#endif
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

DddCouple::~DddCouple()
{
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void DddCouple::create( MainStatemachine *    pMainStatemachine,
                        SystemIo::DddDriver * pDddCoarseDriver,
                        SystemIo::DddDriver * pDddFineDriver )
{
    m_pMainStatemachine = pMainStatemachine;
    m_pDddCoarseDriver  = pDddCoarseDriver;
    m_pDddFineDriver    = pDddFineDriver;

    readCalibrationValues();

#ifndef TARGETBUILD
    readSimulatedScaleRotation();
#endif
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void DddCouple::calibrateDdd()
{
    qDebug() << "DddCouple::calibrateDdd()";

    m_pDddFineDriver->calibrateDdd();
    m_pDddCoarseDriver->calibrateDdd();

    // update display
    scaleToDddValue();
    scaleRawToDddValue();

    m_pMainStatemachine->settingsSerializer()->setDdd1CalibrationAngle10thDeg( m_pDddCoarseDriver->calibrationAngle10thDeg() );
    m_pMainStatemachine->settingsSerializer()->setDdd2CalibrationAngle10thDeg( m_pDddFineDriver->calibrationAngle10thDeg() );

    m_pMainStatemachine->settingsSerializer()->saveSettings();

    m_pMainStatemachine->sqliteInterface()->addEvent( EkxSqliteTypes::SqliteEventType_DDD_CALIBRATE,
                                                      QString( "1:%1/2:%2" )
                                                      .arg( m_pDddFineDriver->calibrationAngle10thDeg() )
                                                      .arg( m_pDddCoarseDriver->calibrationAngle10thDeg() ) );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void DddCouple::readCalibrationValues()
{
    m_pDddCoarseDriver->setCalibrationAngle10thDeg( m_pMainStatemachine->settingsSerializer()->ddd1CalibrationAngle10thDeg() );
    m_pDddFineDriver->setCalibrationAngle10thDeg( m_pMainStatemachine->settingsSerializer()->ddd2CalibrationAngleq10thDeg() );

    // update display
    scaleToDddValue();
    scaleRawToDddValue();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool DddCouple::isDddFailed() const
{
    return isDdd1Failed()
           || isDdd2Failed();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool DddCouple::isDdd1Failed() const
{
    return m_pDddFineDriver->isFailed();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool DddCouple::isDdd2Failed() const
{
    return m_pDddCoarseDriver->isFailed();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#ifndef TARGETBUILD
void DddCouple::incSimulatedScaleRotation( const double dInc )
{
    setSimulatedScaleRotation( simulatedScaleRotation() + dInc );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

double DddCouple::simulatedScaleRotation() const
{
    return m_dSimulatedScaleRotation;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void DddCouple::setSimulatedScaleRotation( const double dSimulatedScaleRotation )
{
    double dTempSimulatedScaleRotation = dSimulatedScaleRotation;

    //qInfo() << "setSimulatedScaleRotation" << nTempSimulatedScaleRotation;

    if ( dTempSimulatedScaleRotation < 0 )
    {
        dTempSimulatedScaleRotation = 0;
    }

    else if ( dTempSimulatedScaleRotation > MAX_DEGREE_SCALE_FLOAT )
    {
        dTempSimulatedScaleRotation = MAX_DEGREE_SCALE_FLOAT;
    }

    if ( m_dSimulatedScaleRotation != dTempSimulatedScaleRotation )
    {
        m_dSimulatedScaleRotation = dTempSimulatedScaleRotation;
        emit simulatedScaleRotationChanged();

        saveSimulatedScaleRotation();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void DddCouple::saveSimulatedScaleRotation( void )
{
#if defined( __WIN32__ )
    saveSimulatedScaleRotation( DddId::DddId1, "/dev/dddpwm3", simulatedScaleRotation() );
    saveSimulatedScaleRotation( DddId::DddId2, "/dev/dddpwm0", simulatedScaleRotation() );
#else
    saveSimulatedScaleRotation( DddId::DddId1, "/sys/devices/platform/soc/44000000.timer/44000000.timer:pwm/pwm/pwmchip4/pwm3/capture", simulatedScaleRotation() );
    saveSimulatedScaleRotation( DddId::DddId2, "/sys/devices/platform/soc/44000000.timer/44000000.timer:pwm/pwm/pwmchip4/pwm0/capture", simulatedScaleRotation() );
#endif

    QJsonObject json;
    json["simulatedScaleRotation"] = simulatedScaleRotation();

    if ( !JsonHelper::writeJsonFile( MainStatemachine::rootPath() + DDD_SIMULATION_FILE, json ) )
    {
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#ifndef TARGETBUILD

void DddCouple::saveSimulatedScaleRotation( const DddId     nDddId,
                                            const QString & strDeviceFile,
                                            const double    dSimulatedScaleRotation )
{
    {
        LockableFile file( MainStatemachine::rootPath() + strDeviceFile );

        if ( m_pDddCoarseDriver )
        {
            if ( m_pDddCoarseDriver->dddType() == SystemIo::DddDriver::DddType::DddTypePwm )
            {
                double dDuty = 0.0;

                if ( nDddId == DddId::DddIdFine )
                {
                    double dTempSimulatedScaleRotation = std::fmod( dSimulatedScaleRotation * FINE_GEAR_RATIO, 360.0 );

                    // scale the value in range 5% - 95% duty cycle
                    dDuty = dTempSimulatedScaleRotation / 360.0 * 90.0 + 5.0;
                }
                else if ( nDddId == DddId::DddIdCoarse )
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
            else if ( m_pDddCoarseDriver->dddType() == SystemIo::DddDriver::DddType::DddTypeAdc )
            {
                qInfo() << "DddCouple::saveSimulatedScaleRotation(): ADC not implemented yet";
            }
            else
            {
                qInfo() << "DddCouple::saveSimulatedScaleRotation(): Unknown not implemented yet";
            }
        }
    }
}

#endif

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void DddCouple::readSimulatedScaleRotation()
{
    QJsonObject jsonFile;
    if ( JsonHelper::readJsonFile( MainStatemachine::rootPath() + DDD_SIMULATION_FILE, jsonFile ) )
    {
        setSimulatedScaleRotation( JsonHelper::read( jsonFile, "simulatedScaleRotation", simulatedScaleRotation() ) );
    }
}

#endif

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void DddCouple::scaleToDddValue()
{
    setDddValue( sensorAngleToDddValue( m_pDddFineDriver->calibratedFilterAngle10thDeg(),
                                        m_pDddCoarseDriver->calibratedFilterAngle10thDeg() ) );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void DddCouple::scaleRawToDddValue()
{
    setRawDddValue( sensorAngleToDddValue( m_pDddFineDriver->calibratedRawAngle10thDeg(),
                                           m_pDddCoarseDriver->calibratedRawAngle10thDeg() ) );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

int DddCouple::sensorAngleToDddValue( const int nFineSensorAngle10thDeg,
                                      const int nCoarseSensorAngle10thDeg )
{
    int nDddFineFilterAngle   = nFineSensorAngle10thDeg;
    int nDddCoarseFilterAngle = nCoarseSensorAngle10thDeg;

    double dDddFineAngle   = static_cast<double>( nDddFineFilterAngle );
    double dDddCoarseAngle = static_cast<double>( nDddCoarseFilterAngle );

    //qDebug() << "C1" << dDddCorseAngle << "F1" << dDddFineAngle;

    dDddCoarseAngle /= 10.0;
    if ( dDddCoarseAngle < 0 )
    {
        dDddCoarseAngle += 360.0;
    }

    dDddFineAngle /= 10.0;
    if ( dDddFineAngle < 0 )
    {
        dDddFineAngle += 360.0;
    }

    // invert coarse rotate direction
    dDddCoarseAngle = 360.0 - dDddCoarseAngle;

    // compensate transmission in mechanics
    dDddCoarseAngle *= COARSE_GEAR_RATIO;  // => 0,809443508

    // try to find the correct fine tuning segment by using the coarse value
    // this is done as iteration because the values do not exactly map at the
    // overflow areas (due to previous filtering and different tolerances of the tooth wheels)
    for ( int i = 0; i < 6; i++ )
    {
        // 4.462 gear translation for fine tune
        double dTempDddFineAngle = ( i * 360.0 + dDddFineAngle ) / ( FINE_GEAR_RATIO );

        if ( dTempDddFineAngle > dDddCoarseAngle - 10.0
             && dTempDddFineAngle < dDddCoarseAngle + 10.0 )
        {
            dDddFineAngle = dTempDddFineAngle;
            break;
        }
    }

    //qDebug() << "C2" << dDddCorseAngle << "F2" << dDddFineAngle;

    //qDebug() << "calculateDddValue " << dddProcessAngle << " / " << ThisDddCalibrationAngle << " / " << ( int ) dddAngle;

    // ... but finally we use the fine tuning value
    // convert angle to grinding degree/coarseness value
    // 288° rotation to 80.0 coarseness
    //double dDddValue = ( dDddCoarseAngle * 80.0 ) / 288.0;  //* 800/360 --> 800 auf 80.0
    double dDddValue = ( dDddFineAngle * MAX_HEMRO_SCALE_FLOAT ) / MAX_DEGREE_SCALE_FLOAT;  //* 800/360 --> 800 auf 80.0

    if ( dDddValue > ( MAX_HEMRO_SCALE_FLOAT + 10.0 ) )
    {
        dDddValue = 0.0;
    }
    else if ( dDddValue > MAX_HEMRO_SCALE_FLOAT )
    {
        dDddValue = MAX_HEMRO_SCALE_FLOAT;
    }

    return static_cast<int>( dDddValue );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void DddCouple::setDddValue( const int nDddValue )
{
    if ( nDddValue != m_nDddValue )
    {
        m_nDddValue = nDddValue;
        emit dddValueChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

int DddCouple::dddValue() const
{
    int nDddValue = m_nDddValue;

    if ( isDddFailed() )
    {
        nDddValue = 0;
    }

    return nDddValue;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void DddCouple::setRawDddValue( const int nDddValue )
{
    if ( nDddValue != m_nRawDddValue )
    {
        m_nRawDddValue = nDddValue;
        emit rawDddValueChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

int DddCouple::rawDddValue() const
{
    return m_nRawDddValue;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////
