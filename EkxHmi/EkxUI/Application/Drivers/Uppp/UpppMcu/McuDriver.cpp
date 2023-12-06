///////////////////////////////////////////////////////////////////////////////
///
/// @file McuDriver.cpp
///
/// @brief Implementation file of class McuDriver.
///
/// @author Ultratronik GmbH
///         Dornierstr. 9
///         D-82205 Gilching
///         Germany
///         http://www.ultratronik.de
///
/// @author written by Gerd Esser, Forschung & Entwicklung, gesser@ultratronik.de
///
/// @date 17.08.2020
///
/// @copyright Copyright 2020 by Hemro International AG
///            Hemro International AG
///            Länggenstrasse 34
///            CH 8184 Bachenbülach
///            Switzerland
///            Homepage: www.hemrogroup.com
///
///////////////////////////////////////////////////////////////////////////////

#include "McuDriver.h"

#include <QJsonObject>
#include <QDebug>
#include <QCoreApplication>

#include "UpppMcu.h"
#include "UpppLevel3Msg.h"
#include "MainStatemachine.h"
#include "JsonHelper.h"

#define MCU_HW_SW_REF_FILE     "/log/McuHwSwRef.json"
#define MCU_FIRMWARE_PATH      "/usr/local/img/"

#define MCU_FIRMWARE_CONFIG_JSON  "McuFirmwareConfig.json"

#define MCU_SW_UPDATE_CHUNK_SIZE  128

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

McuDriver::McuDriver( QObject * parent )
    : QObject( parent )
    , m_pUpppMcu( new UpppMcu() )
{
    MainStatemachine::ensureExistingDirectory( MainStatemachine::rootPath() + MCU_HW_SW_REF_FILE );
    MainStatemachine::ensureExistingDirectory( MainStatemachine::rootPath() + MCU_FIRMWARE_PATH + "/dummy" );

    m_timerTriggerHmiStatus.setInterval( 1000 );
    m_timerTriggerHmiStatus.setSingleShot( false );
    connect( &m_timerTriggerHmiStatus, &QTimer::timeout, this, &McuDriver::processTriggerHmiStatus );
    m_timerTriggerHmiStatus.start();

    m_timerMcuAliveTimeout.setInterval( 5500 );
    m_timerMcuAliveTimeout.setSingleShot( true );
    connect( &m_timerMcuAliveTimeout, &QTimer::timeout, this, &McuDriver::processMcuAliveTimeout );

    m_timerMcuStatusRequestTimeout.setInterval( 500 );
    m_timerMcuStatusRequestTimeout.setSingleShot( true );
    connect( &m_timerMcuStatusRequestTimeout, &QTimer::timeout, this, &McuDriver::processMcuStatusRequestTimeout );

    m_timerProductIdRequestTimeout.setInterval( 500 );
    m_timerProductIdRequestTimeout.setSingleShot( true );
    connect( &m_timerProductIdRequestTimeout, &QTimer::timeout, this, &McuDriver::processProductIdRequestTimeout );

    m_timerMotorTempRequestTimeout.setInterval( 500 );
    m_timerMotorTempRequestTimeout.setSingleShot( true );
    connect( &m_timerMotorTempRequestTimeout, &QTimer::timeout, this, &McuDriver::processMotorTempRequestTimeout );

    m_timerMcuBoardTempRequestTimeout.setInterval( 500 );
    m_timerMcuBoardTempRequestTimeout.setSingleShot( true );
    connect( &m_timerMcuBoardTempRequestTimeout, &QTimer::timeout, this, &McuDriver::processMcuBoardTempRequestTimeout );

    m_timerMotorInfoRequestTimeout.setInterval( 500 );
    m_timerMotorInfoRequestTimeout.setSingleShot( true );
    connect( &m_timerMotorInfoRequestTimeout, &QTimer::timeout, this, &McuDriver::processMotorInfoRequestTimeout );

    m_timerMotorDcBusVoltageRequestTimeout.setInterval( 500 );
    m_timerMotorDcBusVoltageRequestTimeout.setSingleShot( true );
    connect( &m_timerMotorDcBusVoltageRequestTimeout, &QTimer::timeout, this, &McuDriver::processMotorDcBusVoltageRequestTimeout );

    m_timerMotorActuationVerify.setInterval( 100 );
    m_timerMotorActuationVerify.setSingleShot( true );
    connect( &m_timerMotorActuationVerify, &QTimer::timeout, this, &McuDriver::processMotorActuationVerify );

    connect( this, &McuDriver::mcuStatusValidChanged, this, &McuDriver::updateMcuAlive );
    connect( this, &McuDriver::systemStatusHmiAliveChanged, this, &McuDriver::updateMcuAlive );
    connect( this, &McuDriver::statusAcknowledgedChanged, this, &McuDriver::processStatusAcknowledged );
    connect( this, &McuDriver::mcuAliveChanged, this, &McuDriver::processMcuAliveChanged );

    connect( m_pUpppMcu, &UpppMcu::statusMsgAcknowledged, this, &McuDriver::processHmiStatusAcknowledged );
    connect( m_pUpppMcu, &UpppMcu::messageInProgressFinished, this, &McuDriver::processMsgFinished );

    connect( m_pUpppMcu, &UpppMcu::requestReceived, this, &McuDriver::processMcuMsgRequest );
    connect( m_pUpppMcu, &UpppMcu::mcuStatusReceived, this, &McuDriver::processMcuStatusMsg );
    connect( m_pUpppMcu, &UpppMcu::productIdReceived, this, &McuDriver::processProductIdMsg );
    connect( m_pUpppMcu, &UpppMcu::motorTempReceived, this, &McuDriver::processMotorTempMsg );
    connect( m_pUpppMcu, &UpppMcu::mcuBoardTempReceived, this, &McuDriver::processMcuBoardTempMsg );
    connect( m_pUpppMcu, &UpppMcu::motorInfoReceived, this, &McuDriver::processMotorInfoMsg );
    connect( m_pUpppMcu, &UpppMcu::motorDcBusVoltageReceived, this, &McuDriver::processMotorDcBusVoltageMsg );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

McuDriver::~McuDriver()
{
    m_timerTriggerHmiStatus.stop();

    close();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool McuDriver::open()
{
    m_pUpppMcu->openStreamInterface();

    return true;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool McuDriver::open( const QString & strPortName )
{
    bool bSuccess = false;

    if ( !isOpen() )
    {
        qInfo() << "McuDriver::open() " << strPortName;

        bSuccess = m_pUpppMcu->open( strPortName );

        if ( !bSuccess )
        {
            qWarning() << "McuDriver::open() failed " << m_pUpppMcu->errorString();
        }
    }

    return bSuccess;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void McuDriver::close()
{
    qInfo() << "McuDriver::close()";

    m_timerMcuAliveTimeout.stop();
    m_timerMcuStatusRequestTimeout.stop();
    m_timerMotorInfoRequestTimeout.stop();
    m_timerMotorTempRequestTimeout.stop();
    m_timerProductIdRequestTimeout.stop();
    m_timerMcuBoardTempRequestTimeout.stop();
    m_timerMotorDcBusVoltageRequestTimeout.stop();
    m_timerMotorActuationVerify.stop();

    m_pUpppMcu->close();

    m_pUpppMcu->setReplyGranted( false );
    setStatusAcknowledged( false );

    setProductIdValid( false );
    setProductId( "" );
    setSerialNo( "" );
    setHwVersion( "" );
    setSwVersion( "" );

    setMotorTempValid( false );
    setMotorTemp( -50 );

    setMcuBoardTempValid( false );
    setMcuBoardTemp( -50 );

    setMotorDcBusVoltageValid( false );
    setMotorDcBusVoltage( 0 );

    setMotorInfoValid( false );
    setMotorCurrent( 0 );
    setMotorSpeed( 0 );

    setMcuStatusValid( false );
    setSystemStatusHmiAlive( false );
    setSystemStatusMotorRunning( false );
    setSystemStatusSimulation( false );
    setSystemStatusHopperDismounted( false );
    setSystemStatusGrinderChamberOpen( false );
    setSystemStatusFanRunning( false );

    setFaultStatusBoardOverTemp( false );
    setFaultStatusDcOverCurrent( false );
    setFaultStatusAcOverVoltage( false );
    setFaultStatusAcUnderVoltage( false );
    setFaultStatusHallSensor( false );
    setFaultStatusCurrentSensor( false );
    setFaultStatusRotorLocked( false );

    m_bCommandedMotorRunning = false;

    setFaultInconsistentMotorActuation( false );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool McuDriver::isOpen() const
{
    return m_pUpppMcu->isOpen();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool McuDriver::isStatusAcknowledged() const
{
    return m_bStatusAcknowledged;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool McuDriver::isMcuAlive() const
{
    return m_bMcuAlive;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void McuDriver::setMcuConfig( const quint32 u32ConfigMaxMotorSpeed,
                              const quint32 u32ConfigNominalMotorSpeed,
                              const quint32 u32ConfigAccelerationTime,
                              const quint32 u32ConfigDecelerationTime )
{
    m_u32ConfigMaxMotorSpeed     = u32ConfigMaxMotorSpeed;
    m_u32ConfigNominalMotorSpeed = u32ConfigNominalMotorSpeed;
    m_u32ConfigAccelerationTime  = u32ConfigAccelerationTime;
    m_u32ConfigDecelerationTime  = u32ConfigDecelerationTime;

    if ( isMcuAlive() )
    {
        m_pUpppMcu->queueMessage( UpppMcu::sendMotorConfigMsg( m_u32ConfigMaxMotorSpeed,
                                                               m_u32ConfigNominalMotorSpeed,
                                                               m_u32ConfigAccelerationTime,
                                                               m_u32ConfigDecelerationTime ) );
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void McuDriver::killMcuAlive()
{
    m_bStatusAcknowledged   = false;
    m_bMcuStatusValid       = false;
    m_bSystemStatusHmiAlive = false;
    updateMcuAlive();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool McuDriver::startMotor()
{
    if ( isMcuAlive() )
    {
        m_bCommandedMotorRunning = true;
        m_pUpppMcu->queueMessage( UpppMcu::sendMotorActuationMsg( m_bCommandedMotorRunning ) );
        m_timerMotorActuationVerify.start();
    }

    return isMcuAlive();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool McuDriver::stopMotor()
{
    if ( isMcuAlive() )
    {
        m_bCommandedMotorRunning = false;
        m_pUpppMcu->queueMessage( UpppMcu::sendMotorActuationMsg( m_bCommandedMotorRunning ) );
        m_timerMotorActuationVerify.start();
    }

    return isMcuAlive();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool McuDriver::startSwUpdate()
{
    bool bSuccess = false;

    if ( isMcuAlive() )
    {
        if ( m_nSwUpdateState == SwUpdateIdle )
        {
            QJsonObject jsonFile;
            if ( JsonHelper::readJsonFile( MainStatemachine::rootPath() + MCU_FIRMWARE_PATH "/" MCU_FIRMWARE_CONFIG_JSON, jsonFile ) )
            {
                QString strFirmwareFile = JsonHelper::read( jsonFile, "FirmwareFile", QString() );

                if ( !strFirmwareFile.isEmpty() )
                {
                    m_fileSwUpdate.close();

                    m_fileSwUpdate.setFileName( QString( MCU_FIRMWARE_PATH ) + "/" + strFirmwareFile );

                    if ( m_fileSwUpdate.open( QIODevice::ReadOnly ) )
                    {
                        m_n64SwUpdateFileSize = m_fileSwUpdate.size();
                        m_nSwUpdateState      = SwUpdateStart;

                        qint64 n64ChunkNum = ( ( m_n64SwUpdateFileSize + MCU_SW_UPDATE_CHUNK_SIZE - 1 ) / MCU_SW_UPDATE_CHUNK_SIZE );

                        qInfo() << "startSwUpdate: start upload of file" << m_fileSwUpdate.fileName() << "size" << m_n64SwUpdateFileSize << "chunks" << n64ChunkNum;

                        // before starting the new update, finish a potentially, previously opened update
                        m_pUpppMcu->queueMessage( UpppMcu::sendSwUpdateFinishMsg( false ) );
                        m_pUpppMcu->queueMessage( UpppMcu::sendSwUpdateStartMsg( static_cast<quint32>( n64ChunkNum ),
                                                                                 static_cast<quint32>( m_n64SwUpdateFileSize ) ) );

                        bSuccess = true;
                    }
                    else
                    {
                        qCritical() << "startSwUpdate: can't open firmware file" << m_fileSwUpdate.fileName();
                    }
                }
                else
                {
                    qCritical() << "startSwUpdate: firmware file name is empty";
                }
            }
            else
            {
                qCritical() << "startSwUpdate: can't open JSON config file" << MCU_FIRMWARE_PATH "/" MCU_FIRMWARE_CONFIG_JSON;
            }
        }
        else
        {
            qWarning() << "startSwUpdate: forbidden, not in IDLE";
        }
    }

    return bSuccess;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool McuDriver::rejectSwUpdate()
{
    if ( isMcuAlive() )
    {
        m_pUpppMcu->queueMessage( UpppMcu::sendSwUpdateRejectMsg() );
    }

    return isMcuAlive();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool McuDriver::resetMcu()
{
    // don't care about alive status - just try it
    m_pUpppMcu->queueMessage( UpppMcu::sendMcuResetMsg() );

    return true;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool McuDriver::isProductIdValid() const
{
    return m_bProductIdValid;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

const QString & McuDriver::productId() const
{
    return m_strProductId;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

const QString & McuDriver::serialNo() const
{
    return m_strSerialNo;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

const QString & McuDriver::hwVersion() const
{
    return m_strHwVersion;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

const QString & McuDriver::swVersion() const
{
    return m_strSwVersion;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool McuDriver::isMotorTempValid() const
{
    return m_bMotorTempValid;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

int McuDriver::motorTemp() const
{
    return m_nMotorTemp;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool McuDriver::isMcuBoardTempValid() const
{
    return m_bMcuBoardTempValid;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

int McuDriver::mcuBoardTemp() const
{
    return m_nMcuBoardTemp;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool McuDriver::isMotorDcBusVoltageValid() const
{
    return m_bMotorDcBusVoltageValid;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

quint16 McuDriver::motorDcBusVoltage() const
{
    return m_u16MotorDcBusVoltage;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool McuDriver::isMotorInfoValid() const
{
    return m_bMotorInfoValid;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

quint16 McuDriver::motorCurrent() const
{
    return m_u16MotorCurrent;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

quint16 McuDriver::motorSpeed() const
{
    return m_u16MotorSpeed;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool McuDriver::isMcuStatusValid() const
{
    return m_bMcuStatusValid;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool McuDriver::isSystemStatusMotorRunning() const
{
    return m_bSystemStatusMotorRunning;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool McuDriver::isSystemStatusFanRunning() const
{
    return m_bSystemStatusFanRunning;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool McuDriver::isSystemStatusSimulation() const
{
    return m_bSystemStatusSimulation;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool McuDriver::isSystemStatusHopperDismounted() const
{
    return m_bSystemStatusHopperDismounted;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool McuDriver::isSystemStatusGrinderChamberOpen() const
{
    return m_bSystemStatusGrinderChamberOpen;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool McuDriver::isFaultStatusBoardOverTemp() const
{
    return m_bFaultStatusBoardOverTemp;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool McuDriver::isFaultStatusDcOverCurrent() const
{
    return m_bFaultStatusDcOverCurrent;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool McuDriver::isFaultStatusAcOverVoltage() const
{
    return m_bFaultStatusAcOverVoltage;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool McuDriver::isFaultStatusAcUnderVoltage() const
{
    return m_bFaultStatusAcUnderVoltage;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool McuDriver::isFaultStatusHallSensor() const
{
    return m_bFaultStatusHallSensor;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool McuDriver::isFaultStatusCurrentSensor() const
{
    return m_bFaultStatusCurrentSensor;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool McuDriver::isFaultStatusRotorLocked() const
{
    return m_bFaultStatusRotorLocked;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool McuDriver::isFaultInconsistentMotorActuation() const
{
    return m_bFaultInconsistentMotorActuation;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

UpppMcu * McuDriver::upppMcu()
{
    return m_pUpppMcu;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

const UpppMcu * McuDriver::upppMcu() const
{
    return m_pUpppMcu;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void McuDriver::processTriggerHmiStatus()
{
    m_pUpppMcu->queueMessage( UpppMcu::sendHmiStatusMsg( isStatusAcknowledged() ) );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void McuDriver::processMcuAliveTimeout()
{
    setStatusAcknowledged( false );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void McuDriver::processMcuStatusRequestTimeout()
{
    setMcuStatusValid( false );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void McuDriver::processProductIdRequestTimeout()
{
    setProductIdValid( false );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void McuDriver::processMotorTempRequestTimeout()
{
    setMotorTempValid( false );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void McuDriver::processMcuBoardTempRequestTimeout()
{
    setMcuBoardTempValid( false );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void McuDriver::processMotorInfoRequestTimeout()
{
    setMotorInfoValid( false );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void McuDriver::processMotorDcBusVoltageRequestTimeout()
{
    setMotorDcBusVoltageValid( false );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void McuDriver::processStatusAcknowledged()
{
    updateMcuAlive();

    if ( isStatusAcknowledged() )
    {
        m_pUpppMcu->queueMessage( UpppMcu::requestStatusMsg() );
        m_timerMcuStatusRequestTimeout.start();
    }
    else
    {
        setMcuStatusValid( false );
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void McuDriver::processMcuAliveChanged()
{
    if ( isMcuAlive() )
    {
        m_pUpppMcu->queueMessage( UpppMcu::sendHmiStatusMsg( isStatusAcknowledged() ) );
        m_pUpppMcu->queueMessage( UpppMcu::requestMcuBoardTempMsg() );
        m_timerMcuBoardTempRequestTimeout.start();
        m_pUpppMcu->queueMessage( UpppMcu::requestMotorTempMsg() );
        m_timerMotorTempRequestTimeout.start();
        m_pUpppMcu->queueMessage( UpppMcu::requestProductIdMsg() );
        m_timerProductIdRequestTimeout.start();
        m_pUpppMcu->queueMessage( UpppMcu::requestMotorDcBusVoltageMsg() );
        m_timerMotorDcBusVoltageRequestTimeout.start();
        m_pUpppMcu->queueMessage( UpppMcu::requestMotorInfoMsg() );
        m_timerMotorInfoRequestTimeout.start();
        m_pUpppMcu->queueMessage( UpppMcu::sendMotorConfigMsg( m_u32ConfigMaxMotorSpeed,
                                                               m_u32ConfigNominalMotorSpeed,
                                                               m_u32ConfigAccelerationTime,
                                                               m_u32ConfigDecelerationTime ) );
    }
    else
    {
        setMotorInfoValid( false );
        setMotorTempValid( false );
        setMcuBoardTempValid( false );
        setMotorDcBusVoltageValid( false );
        setProductIdValid( false );
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void McuDriver::processHmiStatusAcknowledged()
{
    setStatusAcknowledged( true );
    m_timerMcuAliveTimeout.start();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void McuDriver::processMcuStatusMsg( const bool bHmiAlive,
                                     const bool bMotorRunning,
                                     const bool bSimulation,
                                     const bool bHopperDismounted,
                                     const bool bGrinderChamberOpen,
                                     const bool bFanRunning,
                                     const bool bBoardOverTemp,
                                     const bool bDcOverCurrent,
                                     const bool bAcOverVoltage,
                                     const bool bAcUnderVoltage,
                                     const bool bHallSensor,
                                     const bool bCurrentSensor,
                                     const bool bRotorLocked )
{
    //qDebug() << "processMcuStatusMsg() receive";
    m_timerMcuStatusRequestTimeout.stop();
    setSystemStatusHmiAlive( bHmiAlive );
    setSystemStatusMotorRunning( bMotorRunning );
    setSystemStatusSimulation( bSimulation );
    setSystemStatusHopperDismounted( bHopperDismounted );
    setSystemStatusGrinderChamberOpen( bGrinderChamberOpen );
    setSystemStatusFanRunning( bFanRunning );
    setFaultStatusBoardOverTemp( bBoardOverTemp );
    setFaultStatusDcOverCurrent( bDcOverCurrent );
    setFaultStatusAcOverVoltage( bAcOverVoltage );
    setFaultStatusAcUnderVoltage( bAcUnderVoltage );
    setFaultStatusHallSensor( bHallSensor );
    setFaultStatusCurrentSensor( bCurrentSensor );
    setFaultStatusRotorLocked( bRotorLocked );
    setMcuStatusValid( true );

    if ( m_bCommandedMotorRunning == bMotorRunning )
    {
        m_timerMotorActuationVerify.stop();
        setFaultInconsistentMotorActuation( false );
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void McuDriver::processProductIdMsg( const QString & strProductId,
                                     const QString & strSerialNo,
                                     const QString & strHwVersion,
                                     const QString & strSwVersion )
{
    qDebug() << "processProductIdMsg() receive";
    qDebug() << "  ProductId " << strProductId.trimmed();
    qDebug() << "  SerialNo  " << strSerialNo.trimmed();
    qDebug() << "  HwVersion " << strHwVersion.trimmed();
    qDebug() << "  SwVersion " << strSwVersion.trimmed();

    m_timerProductIdRequestTimeout.stop();
    setProductId( strProductId.trimmed() );
    setSerialNo( strSerialNo.trimmed() );
    setHwVersion( strHwVersion.trimmed() );
    setSwVersion( strSwVersion.trimmed() );
    setProductIdValid( true );

    writeMcuHwSwRef();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void McuDriver::processMotorTempMsg( const int nMotorTemp )
{
    qDebug() << "processMotorTempMsg() receive" << nMotorTemp;
    m_timerMotorTempRequestTimeout.stop();
    setMotorTemp( nMotorTemp );
    setMotorTempValid( true );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void McuDriver::processMcuBoardTempMsg( const int nMcuBoardTemp )
{
    qDebug() << "processMcuBoardTempMsg() receive" << nMcuBoardTemp;
    m_timerMcuBoardTempRequestTimeout.stop();
    setMcuBoardTemp( nMcuBoardTemp );
    setMcuBoardTempValid( true );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void McuDriver::processMotorInfoMsg( const quint16 u16MotorCurrent,
                                     const quint16 u16MotorSpeed )
{
    qDebug() << "processMotorInfoMsg() receive: current" << u16MotorCurrent << "speed" << u16MotorSpeed;
    m_timerMotorInfoRequestTimeout.stop();
    setMotorCurrent( u16MotorCurrent );
    setMotorSpeed( u16MotorSpeed );
    setMotorInfoValid( true );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void McuDriver::processMotorDcBusVoltageMsg( const quint16 u16DcBusVoltage )
{
    qDebug() << "processMotorDcBusVoltageMsg() receive" << u16DcBusVoltage;
    m_timerMotorDcBusVoltageRequestTimeout.stop();
    setMotorDcBusVoltage( u16DcBusVoltage );
    setMotorDcBusVoltageValid( true );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void McuDriver::processMcuMsgRequest( const int nTransactionId,
                                      const int nRequestMsgType )
{
    switch ( nRequestMsgType )
    {
        case UpppLevel3::Level3MsgTypeStatus:
            qDebug() << "processMcuMsgRequest() - Status requested";
            processTriggerHmiStatus();
            break;

        default:
            m_pUpppMcu->sendNack( nTransactionId,
                                  UpppMcu::McuNackReasonMessageTypeNotSupported );
            break;
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void McuDriver::processMotorActuationVerify()
{
    if ( isMcuStatusValid()
         && ( m_bCommandedMotorRunning == isSystemStatusMotorRunning() ) )
    {
        // all ok (but why timeout here? Shouldn't happen)
        setFaultInconsistentMotorActuation( false );
    }
    else
    {
        // if motor should have been stopped, retry!
        if ( !m_bCommandedMotorRunning )
        {
            stopMotor();
        }

        setFaultInconsistentMotorActuation( true );
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void McuDriver::setFaultInconsistentMotorActuation( const bool bFault )
{
    if ( m_bFaultInconsistentMotorActuation != bFault )
    {
        m_bFaultInconsistentMotorActuation = bFault;
        emit faultInconsistentMotorActuationChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void McuDriver::setStatusAcknowledged( const bool bAcknowledged )
{
    if ( m_bStatusAcknowledged != bAcknowledged )
    {
        m_bStatusAcknowledged = bAcknowledged;
        emit statusAcknowledgedChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void McuDriver::updateMcuAlive( void )
{
    bool bAlive = m_bStatusAcknowledged
                  && m_bMcuStatusValid
                  && m_bSystemStatusHmiAlive;

    //qInfo() << "updateMcuAlive" << m_bMcuAlive <<  m_bStatusAcknowledged << m_bMcuStatusValid << m_bSystemStatusHmiAlive;

    if ( m_bMcuAlive != bAlive )
    {
        m_bMcuAlive = bAlive;
        if ( m_bMcuAlive )
        {
            qInfo() << "MCU alive!";
        }
        else
        {
            qInfo() << "MCU died!";
        }
        m_pUpppMcu->setReplyGranted( m_bMcuAlive );

        emit mcuAliveChanged( m_bMcuAlive );
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void McuDriver::setMcuStatusValid( const bool bValid )
{
    if ( m_bMcuStatusValid != bValid )
    {
        m_bMcuStatusValid = bValid;
        emit mcuStatusValidChanged( m_bMcuStatusValid );
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void McuDriver::setSystemStatusHmiAlive( const bool bAlive )
{
    if ( m_bSystemStatusHmiAlive != bAlive )
    {
        m_bSystemStatusHmiAlive = bAlive;
        emit systemStatusHmiAliveChanged( m_bSystemStatusHmiAlive );
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void McuDriver::setSystemStatusMotorRunning( const bool bRunning )
{
    if ( m_bSystemStatusMotorRunning != bRunning )
    {
        m_bSystemStatusMotorRunning = bRunning;
        emit systemStatusMotorRunningChanged( m_bSystemStatusMotorRunning );
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void McuDriver::setSystemStatusSimulation( const bool bSimulation )
{
    if ( m_bSystemStatusSimulation != bSimulation )
    {
        m_bSystemStatusSimulation = bSimulation;
        emit systemStatusSimulationChanged( m_bSystemStatusSimulation );
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void McuDriver::setSystemStatusHopperDismounted( const bool bDismounted )
{
    if ( m_bSystemStatusHopperDismounted != bDismounted )
    {
        m_bSystemStatusHopperDismounted = bDismounted;
        emit systemStatusHopperDismountedChanged( m_bSystemStatusHopperDismounted );
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void McuDriver::setSystemStatusGrinderChamberOpen( const bool bOpen )
{
    if ( m_bSystemStatusGrinderChamberOpen != bOpen )
    {
        m_bSystemStatusGrinderChamberOpen = bOpen;
        emit systemStatusGrinderChamberOpenChanged( m_bSystemStatusGrinderChamberOpen );
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void McuDriver::setSystemStatusFanRunning( const bool bRunning )
{
    if ( m_bSystemStatusFanRunning != bRunning )
    {
        m_bSystemStatusFanRunning = bRunning;
        emit systemStatusFanRunningChanged( m_bSystemStatusFanRunning );
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void McuDriver::setFaultStatusBoardOverTemp( const bool bOverTemp )
{
    if ( m_bFaultStatusBoardOverTemp != bOverTemp )
    {
        m_bFaultStatusBoardOverTemp = bOverTemp;
        emit faultStatusBoardOverTempChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void McuDriver::setFaultStatusDcOverCurrent( const bool bOverCurrent )
{
    if ( m_bFaultStatusDcOverCurrent != bOverCurrent )
    {
        m_bFaultStatusDcOverCurrent = bOverCurrent;
        emit faultStatusDcOverCurrentChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void McuDriver::setFaultStatusAcOverVoltage( const bool bOverVoltage )
{
    if ( m_bFaultStatusAcOverVoltage != bOverVoltage )
    {
        m_bFaultStatusAcOverVoltage = bOverVoltage;
        emit faultStatusAcOverVoltageChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void McuDriver::setFaultStatusAcUnderVoltage( const bool bUnderVoltage )
{
    if ( m_bFaultStatusAcUnderVoltage != bUnderVoltage )
    {
        m_bFaultStatusAcUnderVoltage = bUnderVoltage;
        emit faultStatusAcUnderVoltageChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void McuDriver::setFaultStatusHallSensor( const bool bHallSensor )
{
    if ( m_bFaultStatusHallSensor != bHallSensor )
    {
        m_bFaultStatusHallSensor = bHallSensor;
        emit faultStatusHallSensorChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void McuDriver::setFaultStatusCurrentSensor( const bool bCurrentSensor )
{
    if ( m_bFaultStatusCurrentSensor != bCurrentSensor )
    {
        m_bFaultStatusCurrentSensor = bCurrentSensor;
        emit faultStatusCurrentSensorChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void McuDriver::setFaultStatusRotorLocked( const bool bRotorLocked )
{
    if ( m_bFaultStatusRotorLocked != bRotorLocked )
    {
        m_bFaultStatusRotorLocked = bRotorLocked;
        emit faultStatusRotorLockedChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void McuDriver::setProductIdValid( const bool bValid )
{
    if ( m_bProductIdValid != bValid )
    {
        m_bProductIdValid = bValid;
        emit productIdValidChanged();
        emit productIdChanged();
        emit serialNoChanged();
        emit hwVersionChanged();
        emit swVersionChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void McuDriver::setProductId( const QString & strProductId )
{
    if ( m_strProductId != strProductId )
    {
        m_strProductId = strProductId;
        emit productIdChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void McuDriver::setSerialNo( const QString & strSerialNo )
{
    if ( m_strSerialNo != strSerialNo )
    {
        m_strSerialNo = strSerialNo;
        emit serialNoChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void McuDriver::setHwVersion( const QString & strHwVersion )
{
    if ( m_strHwVersion != strHwVersion )
    {
        m_strHwVersion = strHwVersion;
        emit hwVersionChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void McuDriver::setSwVersion( const QString & strSwVersion )
{
    if ( m_strSwVersion != strSwVersion )
    {
        m_strSwVersion = strSwVersion;
        emit swVersionChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void McuDriver::setMotorTempValid( const bool bValid )
{
    if ( m_bMotorTempValid != bValid )
    {
        m_bMotorTempValid = bValid;
        emit motorTempValidChanged();
        emit motorTempChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void McuDriver::setMotorTemp( const int nMotorTemp )
{
    if ( m_nMotorTemp != nMotorTemp )
    {
        m_nMotorTemp = nMotorTemp;
        emit motorTempChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void McuDriver::setMcuBoardTempValid( const bool bValid )
{
    if ( m_bMcuBoardTempValid != bValid )
    {
        m_bMcuBoardTempValid = bValid;
        emit mcuBoardTempValidChanged();
        emit mcuBoardTempChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void McuDriver::setMcuBoardTemp( const int nMcuBoardTemp )
{
    if ( m_nMcuBoardTemp != nMcuBoardTemp )
    {
        m_nMcuBoardTemp = nMcuBoardTemp;
        emit mcuBoardTempChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void McuDriver::setMotorDcBusVoltageValid( const bool bValid )
{
    if ( m_bMotorDcBusVoltageValid != bValid )
    {
        m_bMotorDcBusVoltageValid = bValid;
        emit motorDcBusVoltageValidChanged();
        emit motorDcBusVoltageChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void McuDriver::setMotorDcBusVoltage( const quint16 u16DcBusVoltage )
{
    if ( m_u16MotorDcBusVoltage != u16DcBusVoltage )
    {
        m_u16MotorDcBusVoltage = u16DcBusVoltage;
        emit motorDcBusVoltageChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void McuDriver::setMotorInfoValid( const bool bValid )
{
    if ( m_bMotorInfoValid != bValid )
    {
        m_bMotorInfoValid = bValid;
        emit motorInfoValidChanged();
        emit motorCurrentChanged();
        emit motorSpeedChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void McuDriver::setMotorCurrent( const quint16 u16MotorCurrent )
{
    if ( m_u16MotorCurrent != u16MotorCurrent )
    {
        m_u16MotorCurrent = u16MotorCurrent;
        emit motorCurrentChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void McuDriver::setMotorSpeed( const quint16 u16MotorSpeed )
{
    if ( m_u16MotorSpeed != u16MotorSpeed )
    {
        m_u16MotorSpeed = u16MotorSpeed;
        emit motorSpeedChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void McuDriver::writeMcuHwSwRef()
{
    if ( isProductIdValid() )
    {
        QJsonObject json;
        json["ProductId"] = productId();
        json["SerailNo"]  = serialNo();
        json["HwVersion"] = hwVersion();
        json["SwVersion"] = swVersion();

        if ( !JsonHelper::writeJsonFile( MainStatemachine::rootPath() + MCU_HW_SW_REF_FILE, json ) )
        {
            qCritical() << "writeMcuHwSwRef(): failed to write file <" << MCU_HW_SW_REF_FILE << ">";
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void McuDriver::processMsgFinished( const int    nMsgType,
                                    const quint8 u8TransactionId,
                                    const bool   bSuccess )
{
    triggerSwUpdateState( nMsgType, u8TransactionId, bSuccess );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void McuDriver::triggerSwUpdateState( const int    nMsgType,
                                      const quint8 u8TransactionId,
                                      const bool   bSuccess )
{
    Q_UNUSED( u8TransactionId )

    switch ( m_nSwUpdateState )
    {
        case SwUpdateIdle:
            // skip all
            break;

        case SwUpdateStart:
        {
            switch ( nMsgType )
            {
                case UpppMcu::McuMsgTypeSwUpdateStart:
                {
                    if ( bSuccess )
                    {
                        m_bSwUpdateFailed         = false;
                        m_n64SwUpdateCurrentChunk = 0;
                        m_nSwUpdateState          = SwUpdateData;
                        triggerSwUpdateState( UpppMcu::McuMsgTypeSwUpdateData, u8TransactionId, bSuccess );
                    }
                    else
                    {
                        qWarning() << "triggerSwUpdateState: no response to SW update start, abort";

                        m_bSwUpdateFailed = true;
                        m_nSwUpdateState  = SwUpdateFinish;
                        m_pUpppMcu->queueMessage( UpppMcu::sendSwUpdateFinishMsg( false ) );
                    }
                }
                break;

                default:
                    // skip all others
                    break;
            }
        }
        break;

        case SwUpdateData:
        {
            switch ( nMsgType )
            {
                case UpppMcu::McuMsgTypeSwUpdateData:
                {
                    if ( bSuccess )
                    {
                        qint64 n64ChunkData = m_n64SwUpdateFileSize - ( m_n64SwUpdateCurrentChunk * MCU_SW_UPDATE_CHUNK_SIZE );

                        if ( n64ChunkData > 0 )
                        {
                            if ( n64ChunkData > MCU_SW_UPDATE_CHUNK_SIZE )
                            {
                                n64ChunkData = MCU_SW_UPDATE_CHUNK_SIZE;
                            }
                            QByteArray chunkData;
                            chunkData.resize( static_cast<int>( n64ChunkData ) );

                            qint64 n64ReadData = m_fileSwUpdate.read( chunkData.data(), n64ChunkData );

                            if ( n64ReadData == n64ChunkData )
                            {
                                m_pUpppMcu->queueMessage( UpppMcu::sendSwUpdateDataMsg( static_cast<quint32>( m_n64SwUpdateCurrentChunk ),
                                                                                        chunkData ) );

                                m_n64SwUpdateCurrentChunk++;
                                qint64 n64Progress = ( m_n64SwUpdateCurrentChunk * MCU_SW_UPDATE_CHUNK_SIZE ) * 100 / m_n64SwUpdateFileSize;
                                emit   swUpdateProgress( static_cast<quint8>( n64Progress ) );
                            }
                            else
                            {
                                qWarning() << "triggerSwUpdateState: read from file returns" << n64ReadData << "but expected" << n64ChunkData;

                                m_bSwUpdateFailed = true;
                                m_nSwUpdateState  = SwUpdateFinish;
                                m_pUpppMcu->queueMessage( UpppMcu::sendSwUpdateFinishMsg( false ) );
                            }
                        }
                        else
                        {
                            m_nSwUpdateState = SwUpdateFinish;
                            m_pUpppMcu->queueMessage( UpppMcu::sendSwUpdateFinishMsg( true ) );
                        }
                    }
                    else
                    {
                        qWarning() << "triggerSwUpdateState: no response to SW update data, abort";
                        m_bSwUpdateFailed = true;
                        m_nSwUpdateState  = SwUpdateFinish;
                        m_pUpppMcu->queueMessage( UpppMcu::sendSwUpdateFinishMsg( false ) );
                    }
                }
                break;

                default:
                    // skip all others
                    break;
            }
        }
        break;

        case SwUpdateFinish:
        {
            switch ( nMsgType )
            {
                case UpppMcu::McuMsgTypeSwUpdateFinish:
                {
                    if ( bSuccess )
                    {
                        qInfo() << "triggerSwUpdateState: finish upload";
                        m_fileSwUpdate.close();
                    }
                    else
                    {
                        qWarning() << "triggerSwUpdateState: no response to SW update finish, abort";
                        m_bSwUpdateFailed = true;
                    }

                    if ( m_bSwUpdateFailed )
                    {
                        m_nSwUpdateState = SwUpdateReject;
                        m_pUpppMcu->queueMessage( UpppMcu::sendSwUpdateRejectMsg() );
                    }
                    else
                    {
                        m_nSwUpdateState = SwUpdateReset;
                        m_pUpppMcu->queueMessage( UpppMcu::sendMcuResetMsg() );
                    }
                }
                break;

                default:
                    // skip all others
                    break;
            }
        }
        break;

        case SwUpdateReject:
        {
            switch ( nMsgType )
            {
                case UpppMcu::McuMsgTypeSwUpdateReject:
                {
                    if ( bSuccess )
                    {
                        qInfo() << "triggerSwUpdateState: SW update rejected";
                    }
                    else
                    {
                        qWarning() << "triggerSwUpdateState: no response to SW update reject";
                    }

                    m_nSwUpdateState = SwUpdateReset;
                    m_pUpppMcu->queueMessage( UpppMcu::sendMcuResetMsg() );
                }
                break;

                default:
                    // skip all others
                    break;
            }
        }
        break;

        case SwUpdateReset:
        {
            switch ( nMsgType )
            {
                case UpppMcu::McuMsgTypeMcuReset:
                {
                    if ( bSuccess )
                    {
                        qInfo() << "triggerSwUpdateState: MCU resetted";
                    }
                    else
                    {
                        qWarning() << "triggerSwUpdateState: no response to MCU reset";
                    }

                    killMcuAlive();

                    m_nSwUpdateState = SwUpdateIdle;
                    emit swUpdateFinished( !m_bSwUpdateFailed );
                }
                break;

                default:
                    // skip all others
                    break;
            }
        }
        break;
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////
