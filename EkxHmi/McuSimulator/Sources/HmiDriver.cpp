///////////////////////////////////////////////////////////////////////////////
///
/// @file HmiDriver.cpp
///
/// @brief Implementation file of class HmiDriver.
///
/// @author Ultratronik GmbH
///         Dornierstr. 9
///         D-82205 Gilching
///         Germany
///         http://www.ultratronik.de
///
/// @author written by Gerd Esser, Forschung & Entwicklung, gesser@ultratronik.de
///
/// @date 19.08.2020
///
/// @copyright Copyright 2020 by Hemro International AG
///            Hemro International AG
///            Länggenstrasse 34
///            CH 8184 Bachenbülach
///            Switzerland
///            Homepage: www.hemrogroup.com
///
///////////////////////////////////////////////////////////////////////////////

#include "HmiDriver.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>
#include <QCoreApplication>

#include "UpppHmi.h"
#include "UpppLevel3Msg.h"

#define MCU_IMAGE_TEMP_FILE  "/tmp/McuReceivedImage.blob"

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

HmiDriver::HmiDriver()
    : m_pUpppHmi( new UpppHmi( nullptr ) )
{
    m_timerTriggerMcuStatus.setInterval( 1000 );
    m_timerTriggerMcuStatus.setSingleShot( false );
    connect( &m_timerTriggerMcuStatus, &QTimer::timeout, this, &HmiDriver::processTriggerMcuStatus );
    m_timerTriggerMcuStatus.start();

    m_timerSimulationUpdate.setInterval( 100 );
    m_timerSimulationUpdate.setSingleShot( false );
    connect( &m_timerSimulationUpdate, &QTimer::timeout, this, &HmiDriver::processSimulationUpdate );
    m_timerSimulationUpdate.start();

    m_timerHmiAliveTimeout.setInterval( 5500 );
    m_timerHmiAliveTimeout.setSingleShot( true );
    connect( &m_timerHmiAliveTimeout, &QTimer::timeout, this, &HmiDriver::processHmiAliveTimeout );

    m_timerHmiStatusRequestTimeout.setInterval( 500 );
    m_timerHmiStatusRequestTimeout.setSingleShot( true );
    connect( &m_timerHmiStatusRequestTimeout, &QTimer::timeout, this, &HmiDriver::processHmiStatusRequestTimeout );

    m_timerMotorRunningUpdate.setInterval( 100 );
    m_timerMotorRunningUpdate.setSingleShot( false );
    connect( &m_timerMotorRunningUpdate, &QTimer::timeout, this, &HmiDriver::processMotorRunningUpdate );

    connect( this, &HmiDriver::statusMcuAliveChanged, this, &HmiDriver::updateHmiAlive );
    connect( this, &HmiDriver::hmiStatusValidChanged, this, &HmiDriver::updateHmiAlive );
    connect( this, &HmiDriver::statusAcknowledgedChanged, this, &HmiDriver::processStatusAcknowledged );
    connect( this, &HmiDriver::hmiAliveChanged, this, &HmiDriver::processHmiAliveChanged );

    connect( m_pUpppHmi, &UpppHmi::requestReceived, this, &HmiDriver::processHmiMsgRequest );
    connect( m_pUpppHmi, &UpppHmi::hmiStatusReceived, this, &HmiDriver::processHmiStatusMsg );
    connect( m_pUpppHmi, &UpppHmi::motorConfigReceived, this, &HmiDriver::processMotorConfigMsg );
    connect( m_pUpppHmi, &UpppHmi::motorActuationReceived, this, &HmiDriver::processMotorActuationMsg );
    connect( m_pUpppHmi, &UpppHmi::simulatedAlarmCodeReceived, this, &HmiDriver::processSimulatedAlarmCodeMsg );
    connect( m_pUpppHmi, &UpppHmi::swUpdateStartReceived, this, &HmiDriver::processSwUpdateStartMsg );
    connect( m_pUpppHmi, &UpppHmi::swUpdateDataReceived, this, &HmiDriver::processSwUpdateDataMsg );
    connect( m_pUpppHmi, &UpppHmi::swUpdateFinishReceived, this, &HmiDriver::processSwUpdateFinishMsg );
    connect( m_pUpppHmi, &UpppHmi::swUpdateRejectReceived, this, &HmiDriver::processSwUpdateRejectMsg );
    connect( m_pUpppHmi, &UpppHmi::mcuResetReceived, this, &HmiDriver::processMcuResetMsg );

    connect( m_pUpppHmi, &UpppHmi::statusMsgAcknowledged, this, &HmiDriver::processMcuStatusAcknowledged );
    connect( m_pUpppHmi, &UpppHmi::messageInProgressFinished, this, &HmiDriver::processQueuedMessage );

    connect( m_pUpppHmi, &UpppHmi::errorOccurred, this, &HmiDriver::processSerialPortError );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

HmiDriver::~HmiDriver()
{
    close();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool HmiDriver::open( const QString & strPortName )
{
    bool bSuccess = false;

    if ( !isOpen() )
    {
        qInfo() << "HmiDriver::open() " << strPortName;

        bSuccess = m_pUpppHmi->open( strPortName );

        if ( !bSuccess )
        {
            qCritical() << "HmiDriver::open() failed " << m_pUpppHmi->errorString();
        }
    }

    return bSuccess;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void HmiDriver::close()
{
    qInfo() << "HmiDriver::close()";

    m_timerTriggerMcuStatus.stop();
    m_timerSimulationUpdate.stop();
    m_timerHmiAliveTimeout.stop();
    m_timerHmiStatusRequestTimeout.stop();
    m_timerMotorRunningUpdate.stop();

    m_pUpppHmi->close();

    m_pUpppHmi->setReplyGranted( false );
    setStatusAcknowledged( false );

    setMotorConfigValid( false );
    setMaxMotorSpeed( 0 );
    setNominalMotorSpeed( 0 );
    setAccelerationTime( 0 );
    setDecelerationTime( 0 );

    setHmiStatusValid( false );
    setStatusMcuAlive( false );

    while ( m_theMsgQueue.size() )
    {
        delete m_theMsgQueue.takeFirst();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool HmiDriver::isOpen() const
{
    return m_pUpppHmi->isOpen();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool HmiDriver::isStatusAcknowledged() const
{
    return m_bStatusAcknowledged;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool HmiDriver::isHmiAlive() const
{
    return m_bHmiAlive;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool HmiDriver::isMotorConfigValid() const
{
    return m_bMotorConfigValid;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

quint32 HmiDriver::maxMotorSpeed() const
{
    return m_u32MaxMotorSpeed;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

quint32 HmiDriver::nominalMotorSpeed() const
{
    return m_u32NominalMotorSpeed;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

quint32 HmiDriver::accelerationTime() const
{
    return m_u32AccelerationTime;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

quint32 HmiDriver::decelerationTime() const
{
    return m_u32DecelerationTime;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool HmiDriver::isHmiStatusValid() const
{
    return m_bHmiStatusValid;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void HmiDriver::processStatusAcknowledged()
{
    updateHmiAlive();

    if ( isStatusAcknowledged() )
    {
        queueMessage( UpppHmi::requestStatusMsg() );
        m_timerHmiStatusRequestTimeout.start();
    }
    else
    {
        setHmiStatusValid( false );
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void HmiDriver::processTriggerMcuStatus()
{
    queueMessage( UpppHmi::sendMcuStatusMsg( isStatusAcknowledged(),
                                             m_bSystemStatusMotorRunning,
                                             m_bSystemStatusSimulation,
                                             m_bSystemStatusSafeLock | m_bSimulatedSafeLock,
                                             m_bSystemStatusFanRunning,
                                             m_bFaultStatusBoardOverTemp | m_bSimulatedBoardOverTemp,
                                             m_bFaultStatusDcOverCurrent | m_bSimulatedDcOverCurrent,
                                             m_bFaultStatusAcOverVoltage | m_bSimulatedAcOverVoltage,
                                             m_bFaultStatusAcUnderVoltage | m_bSimulatedAcUnderVoltage,
                                             m_bFaultStatusHallSensor | m_bSimulatedHallSensor,
                                             m_bFaultStatusCurrentSensor | m_bSimulatedCurrentSensor,
                                             m_bFaultStatusRotorLocked | m_bSimulatedRotorLocked ) );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void HmiDriver::processHmiAliveTimeout()
{
    m_pUpppHmi->setReplyGranted( false );
    setStatusAcknowledged( false );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void HmiDriver::processHmiStatusRequestTimeout()
{
    setHmiStatusValid( false );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void HmiDriver::processHmiAliveChanged()
{
    if ( isHmiAlive() )
    {
        processTriggerMcuStatus();
        //        queueMessage( m_pUpppHmi->requestStatusMsg() );
        //        m_timerHmiStatusRequestTimeout.start();
    }
    else
    {
        setHmiStatusValid( false );
        setMotorConfigValid( false );
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void HmiDriver::processMcuStatusAcknowledged()
{
    m_pUpppHmi->setReplyGranted( true );
    setStatusAcknowledged( true );
    m_timerHmiAliveTimeout.start();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void HmiDriver::processHmiStatusMsg( const quint8 u8TransactionId,
                                     const bool   bMcuAlive )
{
    m_pUpppHmi->sendAck( u8TransactionId );
    m_timerHmiStatusRequestTimeout.stop();
    setStatusMcuAlive( bMcuAlive );
    setHmiStatusValid( true );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void HmiDriver::processMotorActuationMsg( const quint8 u8TransactionId,
                                          const bool   bMotorStart )
{
    qInfo() << "processMotorActuationMsg() received " << bMotorStart;

    if ( isHmiAlive() )
    {
        m_pUpppHmi->sendAck( u8TransactionId );
        m_bSystemStatusMotorRunning = bMotorStart;

        processTriggerMcuStatus();

        if ( bMotorStart )
        {
            m_timerMotorRunningUpdate.start();
        }
        else
        {
            m_timerMotorRunningUpdate.stop();
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void HmiDriver::processSimulatedAlarmCodeMsg( const quint8 u8TransactionId,
                                              const bool   bSimulatedSafeLock,
                                              const bool   bSimulatedBoardOverTemp,
                                              const bool   bSimulatedDcOverCurrent,
                                              const bool   bSimulatedAcOverVoltage,
                                              const bool   bSimulatedAcUnderVoltage,
                                              const bool   bSimulatedHallSensor,
                                              const bool   bSimulatedCurrentSensor,
                                              const bool   bSimulatedRotorLocked )
{
    qInfo() << "processSimulatedAlarmCodeMsg() received ";

    if ( isHmiAlive() )
    {
        m_pUpppHmi->sendAck( u8TransactionId );
        m_bSimulatedSafeLock       = bSimulatedSafeLock;
        m_bSimulatedBoardOverTemp  = bSimulatedBoardOverTemp;
        m_bSimulatedDcOverCurrent  = bSimulatedDcOverCurrent;
        m_bSimulatedAcOverVoltage  = bSimulatedAcOverVoltage;
        m_bSimulatedAcUnderVoltage = bSimulatedAcUnderVoltage;
        m_bSimulatedHallSensor     = bSimulatedHallSensor;
        m_bSimulatedCurrentSensor  = bSimulatedCurrentSensor;
        m_bSimulatedRotorLocked    = bSimulatedRotorLocked;

        m_bSystemStatusSimulation = m_bSimulatedSafeLock
                                    | m_bSimulatedBoardOverTemp
                                    | m_bSimulatedDcOverCurrent
                                    | m_bSimulatedAcOverVoltage
                                    | m_bSimulatedAcUnderVoltage
                                    | m_bSimulatedHallSensor
                                    | m_bSimulatedCurrentSensor
                                    | m_bSimulatedRotorLocked;

        processTriggerMcuStatus();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void HmiDriver::processMotorConfigMsg( const quint8  u8TransactionId,
                                       const quint32 u32MaxMotorSpeed,
                                       const quint32 u32NominalMotorSpeed,
                                       const quint32 u32AccelerationTime,
                                       const quint32 u32DecelerationTime )
{
    qInfo() << "processMotorConfigMsg() received ";
    qInfo() << "  MaxMotorSpeed     " << u32MaxMotorSpeed;
    qInfo() << "  NominalMotorSpeed " << u32NominalMotorSpeed;
    qInfo() << "  AccelerationTime  " << u32AccelerationTime;
    qInfo() << "  DecelerationTime  " << u32DecelerationTime;

    if ( isHmiAlive() )
    {
        m_pUpppHmi->sendAck( u8TransactionId );
        setMaxMotorSpeed( u32MaxMotorSpeed );
        setNominalMotorSpeed( u32NominalMotorSpeed );
        setAccelerationTime( u32AccelerationTime );
        setDecelerationTime( u32DecelerationTime );
        setMotorConfigValid( true );
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void HmiDriver::processHmiMsgRequest( const int nTransactionId,
                                      const int nRequestMsgType )
{
    switch ( nRequestMsgType )
    {
        case UpppLevel3::Level3MsgTypeStatus:
            qDebug() << "processHmiMsgRequest() - Status requested";
            processTriggerMcuStatus();
            break;

        case UpppHmi::McuMsgTypeMotorInfo:
            qInfo() << "processHmiMsgRequest() - Motor info requested";
            if ( isHmiAlive() )
            {
                queueMessage( UpppHmi::sendMotorInfoMsg( m_u16MotorCurrent,
                                                         m_u16MotorSpeed ) );
            }
            break;

        case UpppHmi::McuMsgTypeMotorTemp:
            qInfo() << "processHmiMsgRequest() - Motor temp requested";
            if ( isHmiAlive() )
            {
                queueMessage( UpppHmi::sendMotorTempMsg( m_u8MotorTemp ) );
            }
            break;

        case UpppHmi::McuMsgTypeProductId:
            qInfo() << "processHmiMsgRequest() - Product ID requested";
            if ( isHmiAlive() )
            {
                queueMessage( UpppHmi::sendProductIdMsg( productId(),
                                                         serialNo(),
                                                         hwVersion(),
                                                         swVersion() ) );
            }
            break;

        case UpppHmi::McuMsgTypeMcuBoardTemp:
            qInfo() << "processHmiMsgRequest() - MCU board temp requested";
            if ( isHmiAlive() )
            {
                queueMessage( UpppHmi::sendMcuBoardTempMsg( m_u8McuBoardTemp ) );
            }
            break;

        case UpppHmi::McuMsgTypeMotorActuation:
            qInfo() << "processHmiMsgRequest() - Motor actuation requested";
            break;

        case UpppHmi::McuMsgTypeMotorDcBusVoltage:
            qInfo() << "processHmiMsgRequest() - Motor DC bus voltage requested";
            if ( isHmiAlive() )
            {
                queueMessage( UpppHmi::sendMotorDcBusVoltageMsg( m_u16MotorDcBusVoltage ) );
            }
            break;

        case UpppHmi::McuMsgTypeMcuReset:
        default:
            m_pUpppHmi->sendNack( nTransactionId,
                                  UpppHmi::McuNackReasonMessageTypeNotSupported );
            break;
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void HmiDriver::processMotorRunningUpdate()
{
    if ( isHmiAlive() )
    {
        queueMessage( m_pUpppHmi->sendMotorInfoMsg( m_u16MotorCurrent,
                                                    m_u16MotorSpeed ) );
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void HmiDriver::processSimulationUpdate()
{
    if ( m_bSystemStatusMotorRunning )
    {
        m_u16MotorSpeed   = 1600;
        m_u16MotorCurrent = 5000;
    }
    else
    {
        m_u16MotorSpeed   = 0;
        m_u16MotorCurrent = 5;
    }

    m_u8MotorTemp          = 73;
    m_u8McuBoardTemp       = 80;
    m_u16MotorDcBusVoltage = 200;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void HmiDriver::processSwUpdateStartMsg( const quint8  u8TransactionId,
                                         const quint32 u32ChunkNum,
                                         const quint32 u32TotalSize )
{
    if ( isHmiAlive() )
    {
        qInfo() << "processSwUpdateStartMsg received: ID" << u8TransactionId << "u32ChunkNum" << u32ChunkNum << "u32TotalSize" << u32TotalSize;

        if ( m_nSwUpdateState == SwUpdateIdle )
        {
            m_u32ImageChunkNum       = u32ChunkNum;
            m_u32ImageTotalSize      = u32TotalSize;
            m_u32ImageCurrentChunkNo = 0;
            m_nSwUpdateState         = SwUpdateData;
            m_u32ChunkSize           = 0;

            QFile file( MCU_IMAGE_TEMP_FILE );
            if ( file.open( QIODevice::WriteOnly ) )
            {
                file.close();
            }

            m_pUpppHmi->sendAck( u8TransactionId );
        }
        else
        {
            qWarning() << "processSwUpdateStartMsg: McuNackReasonSwUpdateNotPossible";
            m_pUpppHmi->sendNack( u8TransactionId,
                                  UpppHmi::McuNackReasonSwUpdateNotPossible );
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void HmiDriver::processSwUpdateDataMsg( const quint8       u8TransactionId,
                                        const quint32      u32ChunkNo,
                                        const QByteArray & chunkData )
{
    if ( isHmiAlive() )
    {
        qDebug() << "processSwUpdateDataMsg received: ID" << u8TransactionId << "u32ChunkNo" << u32ChunkNo << "chunkData" << chunkData.size();

        if ( m_nSwUpdateState == SwUpdateData )
        {
            if ( ( m_u32ImageCurrentChunkNo == u32ChunkNo )
                 || ( m_u32ImageCurrentChunkNo - 1 == u32ChunkNo ) )
            {
                if ( u32ChunkNo <= m_u32ImageChunkNum )
                {
                    m_u32ImageCurrentChunkNo = u32ChunkNo + 1;

                    if ( u32ChunkNo == 0 )
                    {
                        m_u32ChunkSize = static_cast<quint32>( chunkData.size() );
                    }

                    QThread::msleep( 20 );

                    m_pUpppHmi->sendAck( u8TransactionId );

                    QFile file( MCU_IMAGE_TEMP_FILE );
                    if ( file.open( QIODevice::WriteOnly ) )
                    {
                        file.seek( static_cast<qint64>( u32ChunkNo ) * m_u32ChunkSize );
                        file.write( chunkData );
                        file.close();
                    }
                }
                else
                {
                    qWarning() << "processSwUpdateDataMsg: McuNackReasonSwUpdateChunkProcessingFailed";
                    m_pUpppHmi->sendNack( u8TransactionId,
                                          UpppHmi::McuNackReasonSwUpdateChunkProcessingFailed );
                }
            }
            else
            {
                qWarning() << "processSwUpdateDataMsg: McuNackReasonSwUpdateWrongChunkSequence, expected" << m_u32ImageCurrentChunkNo;
                m_pUpppHmi->sendNack( u8TransactionId,
                                      UpppHmi::McuNackReasonSwUpdateWrongChunkSequence );
            }
        }
        else
        {
            qWarning() << "processSwUpdateDataMsg: McuNackReasonSwUpdateNotStarted";
            m_pUpppHmi->sendNack( u8TransactionId,
                                  UpppHmi::McuNackReasonSwUpdateNotStarted );
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void HmiDriver::processSwUpdateFinishMsg( const quint8 u8TransactionId,
                                          const bool   bSuccess )
{
    if ( isHmiAlive() )
    {
        qInfo() << "processSwUpdateFinishMsg received: ID" << u8TransactionId << "/ Success" << bSuccess;

        if ( bSuccess )
        {
            if ( m_nSwUpdateState == SwUpdateData )
            {
                if ( m_u32ImageCurrentChunkNo == m_u32ImageChunkNum )
                {
                    QFile file( MCU_IMAGE_TEMP_FILE );
                    if ( file.size() == m_u32ImageTotalSize )
                    {
                        m_pUpppHmi->sendAck( u8TransactionId );
                    }
                    else
                    {
                        qWarning() << "processSwUpdateFinishMsg: failed, binary size wrong";
                        m_pUpppHmi->sendNack( u8TransactionId,
                                              UpppHmi::McuNackReasonSwUpdateIllegalBinaraySize );
                    }
                }
                else
                {
                    qWarning() << "processSwUpdateFinishMsg: failed, chunk sizes differ";
                    m_pUpppHmi->sendNack( u8TransactionId,
                                          UpppHmi::McuNackReasonSwUpdateWrongChunkSequence );
                }
            }
            else
            {
                qWarning() << "processSwUpdateFinishMsg: update not started";
                m_pUpppHmi->sendNack( u8TransactionId,
                                      UpppHmi::McuNackReasonSwUpdateNotStarted );
            }
        }
        else
        {
            qWarning() << "processSwUpdateFinishMsg: update cancelled by remote";
        }
        m_nSwUpdateState = SwUpdateIdle;
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void HmiDriver::processSwUpdateRejectMsg( const quint8 u8TransactionId )
{
    m_pUpppHmi->sendAck( u8TransactionId );
    qInfo() << "processSwUpdateRejectMsg received";
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void HmiDriver::processMcuResetMsg( const quint8 u8TransactionId )
{
    m_pUpppHmi->sendAck( u8TransactionId );

    setStatusAcknowledged( false );
    setHmiStatusValid( false );
    setStatusMcuAlive( false );

    QThread::sleep( 1 );

    qInfo() << "receive MCU reset";
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void HmiDriver::processSerialPortError( QSerialPort::SerialPortError error )
{
    switch ( error )
    {
        case QSerialPort::ResourceError:
        {
            qCritical() << "processSerialPortError: fatal " << error;
            qApp->exit( 1 );
        }
        break;

        default:
        {
            // do nothing
        }
        break;
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void HmiDriver::processQueuedMessage()
{
    if ( !m_pUpppHmi->isServerMsgInProgress() )
    {
        if ( m_theMsgQueue.size() )
        {
            UpppLevel3Msg * pMsg = m_theMsgQueue.takeFirst();

            if ( pMsg )
            {
                m_pUpppHmi->sendMessage( *pMsg );
                delete pMsg;
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void HmiDriver::setStatusAcknowledged( const bool bAcknowledged )
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

void HmiDriver::updateHmiAlive( void )
{
    bool bAlive = m_bStatusAcknowledged
                  && m_bHmiStatusValid
                  && m_bStatusMcuAlive;
    //qInfo() << "trigger HMI alive update" << m_bHmiAlive << m_bStatusAcknowledged << m_bHmiStatusValid << m_bStatusMcuAlive;

    if ( m_bHmiAlive != bAlive )
    {
        m_bHmiAlive = bAlive;
        qInfo() << "change HMI alive to " << bAlive;
        emit hmiAliveChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void HmiDriver::setHmiStatusValid( const bool bValid )
{
    if ( m_bHmiStatusValid != bValid )
    {
        m_bHmiStatusValid = bValid;
        emit hmiStatusValidChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void HmiDriver::setStatusMcuAlive( const bool bAlive )
{
    if ( m_bStatusMcuAlive != bAlive )
    {
        m_bStatusMcuAlive = bAlive;
        emit statusMcuAliveChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void HmiDriver::setMotorConfigValid( const bool bValid )
{
    if ( m_bMotorConfigValid != bValid )
    {
        m_bMotorConfigValid = bValid;
        emit motorConfigValidChanged();
        emit maxMotorSpeedChanged();
        emit nominalMotorSpeedChanged();
        emit accelerationTimeChanged();
        emit decelerationTimeChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void HmiDriver::setMaxMotorSpeed( const quint32 u32MaxMotorSpeed )
{
    if ( m_u32MaxMotorSpeed != u32MaxMotorSpeed )
    {
        m_u32MaxMotorSpeed = u32MaxMotorSpeed;
        emit maxMotorSpeedChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void HmiDriver::setNominalMotorSpeed( const quint32 u32NominalMotorSpeed )
{
    if ( m_u32NominalMotorSpeed != u32NominalMotorSpeed )
    {
        m_u32NominalMotorSpeed = u32NominalMotorSpeed;
        emit nominalMotorSpeedChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void HmiDriver::setAccelerationTime( const quint32 u32AccelerationTime )
{
    if ( m_u32AccelerationTime != u32AccelerationTime )
    {
        m_u32AccelerationTime = u32AccelerationTime;
        emit accelerationTimeChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void HmiDriver::setDecelerationTime( const quint32 u32DecelerationTime )
{
    if ( m_u32DecelerationTime != u32DecelerationTime )
    {
        m_u32DecelerationTime = u32DecelerationTime;
        emit decelerationTimeChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void HmiDriver::queueMessage( const UpppLevel3Msg & msg )
{
    m_theMsgQueue.push_back( new UpppLevel3Msg( msg ) );
    processQueuedMessage();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

QString HmiDriver::serialNo() const
{
    return m_strSerialNo;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void HmiDriver::setSerialNo( const QString & strSerialNo )
{
    m_strSerialNo = strSerialNo;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

QString HmiDriver::swVersion() const
{
    return m_strSwVersion;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void HmiDriver::setSwVersion( const QString & strSwVersion )
{
    m_strSwVersion = strSwVersion;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

QString HmiDriver::hwVersion() const
{
    return m_strHwVersion;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void HmiDriver::setHwVersion( const QString & strHwVersion )
{
    m_strHwVersion = strHwVersion;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

QString HmiDriver::productId() const
{
    return m_strProductId;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void HmiDriver::setProductId( const QString & strProductId )
{
    m_strProductId = strProductId;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

