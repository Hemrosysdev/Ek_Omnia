///////////////////////////////////////////////////////////////////////////////
///
/// @file HmiDriver.h
///
/// @brief Header file of class HmiDriver.
///
/// @author Ultratronik GmbH \n
///         Dornierstr. 9 \n
///         D-82205 Gilching \n
///         Germany
///         http://www.ultratronik.de
///
/// @author written by Gerd Esser, Forschung & Entwicklung, gesser@ultratronik.de
///
/// @date 19.08.2020
///
/// @copyright Copyright 2020 by Hemro International AG \n
///            Hemro International AG \n
///            Länggenstrasse 34 \n
///            CH 8184 Bachenbülach \n
///            Switzerland \n
///            Homepage: www.hemrogroup.com
///
///////////////////////////////////////////////////////////////////////////////

#ifndef HmiDriver_H
#define HmiDriver_H

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#include <QThread>
#include <QTimer>
#include <QList>
#include <QMutex>
#include <QSerialPort>

class UpppHmi;
class UpppLevel3Msg;

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

class HmiDriver
    : public QObject
{
    Q_OBJECT

private:

    enum SwUpdateStates
    {
        SwUpdateIdle = 0,
        SwUpdateData,
    };

public:

    HmiDriver();

    virtual ~HmiDriver() override;

    bool open( const QString & strPortName );

    void close();

    bool isOpen() const;

    bool isStatusAcknowledged( void ) const;

    bool isHmiAlive( void ) const;

    bool isMotorConfigValid() const;

    quint32 maxMotorSpeed() const;

    quint32 nominalMotorSpeed() const;

    quint32 accelerationTime() const;

    quint32 decelerationTime() const;

    bool isHmiStatusValid() const;

    QString productId() const;

    void setProductId( const QString & strProductId );

    QString hwVersion() const;

    void setHwVersion( const QString & strHwVersion );

    QString swVersion() const;

    void setSwVersion( const QString & strSwVersion );

    QString serialNo() const;

    void setSerialNo( const QString & strSerialNo );

signals:

    void statusAcknowledgedChanged();

    void hmiAliveChanged();

    void hmiStatusValidChanged();

    void statusMcuAliveChanged();

    void motorConfigValidChanged();

    void maxMotorSpeedChanged();

    void nominalMotorSpeedChanged();

    void accelerationTimeChanged();

    void decelerationTimeChanged();

private slots:

    void processStatusAcknowledged();

    void processTriggerMcuStatus( void );

    void processHmiAliveTimeout( void );

    void processHmiStatusRequestTimeout( void );

    void processHmiAliveChanged( void );

    void processMcuStatusAcknowledged();

    void processHmiStatusMsg( const quint8 u8TransactionId,
                              const bool   bMcuAlive );

    void processMotorActuationMsg( const quint8 u8TransactionId,
                                   const bool   bMotorStart );

    void processSimulatedAlarmCodeMsg( const quint8 u8TransactionId,
                                       const bool   bSimulatedSafeLock,
                                       const bool   bSimulatedBoardOverTemp,
                                       const bool   bSimulatedDcOverCurrent,
                                       const bool   bSimulatedAcOverVoltage,
                                       const bool   bSimulatedAcUnderVoltage,
                                       const bool   bSimulatedHallSensor,
                                       const bool   bSimulatedCurrentSensor,
                                       const bool   bSimulatedRotorLocked );

    void processMotorConfigMsg( const quint8  u8TransactionId,
                                const quint32 u32MaxMotorSpeed,
                                const quint32 u32NominalMotorSpeed,
                                const quint32 u32AccelerationTime,
                                const quint32 u32DecelerationTime );

    void processHmiMsgRequest( const int nTransactionId,
                               const int nRequestMsgType );

    void processMotorRunningUpdate();

    void processSimulationUpdate();

    void processSwUpdateStartMsg( const quint8  u8TransactionId,
                                  const quint32 u32ChunkNum,
                                  const quint32 u32TotalSize );

    void processSwUpdateDataMsg( const quint8       u8TransactionId,
                                 const quint32      u32ChunkNo,
                                 const QByteArray & chunkData );

    void processSwUpdateFinishMsg( const quint8 u8TransactionId,
                                   const bool   bSuccess );

    void processSwUpdateRejectMsg( const quint8 u8TransactionId );

    void processMcuResetMsg( const quint8 u8TransactionId );

    void processSerialPortError( QSerialPort::SerialPortError error );

private:

    void processQueuedMessage();

    void setStatusAcknowledged( const bool bAcknowledged );

    void updateHmiAlive( void );

    void setHmiStatusValid( const bool bValid );

    void setStatusMcuAlive( const bool bAlive );

    void setMotorConfigValid( const bool bValid );

    void setMaxMotorSpeed( const quint32 u32MaxMotorSpeed );

    void setNominalMotorSpeed( const quint32 u32NominalMotorSpeed );

    void setAccelerationTime( const quint32 u32AccelerationTime );

    void setDecelerationTime( const quint32 u32DecelerationTime );

    void queueMessage( const UpppLevel3Msg & msg );

private:

    UpppHmi *              m_pUpppHmi { nullptr };

    bool                   m_bStatusAcknowledged { false };

    bool                   m_bHmiAlive { false };

    QTimer                 m_timerTriggerMcuStatus;

    QTimer                 m_timerHmiAliveTimeout;

    QTimer                 m_timerHmiStatusRequestTimeout;

    QTimer                 m_timerMotorRunningUpdate;

    QTimer                 m_timerSimulationUpdate;

    bool                   m_bHmiStatusValid { false };

    bool                   m_bStatusMcuAlive { false };

    bool                   m_bMotorConfigValid { false };

    quint32                m_u32MaxMotorSpeed { 1900 };

    quint32                m_u32NominalMotorSpeed { 1700 };

    quint32                m_u32AccelerationTime { 200 };

    quint32                m_u32DecelerationTime { 200 };

    QList<UpppLevel3Msg *> m_theMsgQueue;

    bool                   m_bSystemStatusMotorRunning { false };
    bool                   m_bSystemStatusSimulation { false };
    bool                   m_bSystemStatusSafeLock { false };
    bool                   m_bSystemStatusFanRunning { false };
    bool                   m_bFaultStatusBoardOverTemp { false };
    bool                   m_bFaultStatusDcOverCurrent { false };
    bool                   m_bFaultStatusAcOverVoltage { false };
    bool                   m_bFaultStatusAcUnderVoltage { false };
    bool                   m_bFaultStatusHallSensor { false };
    bool                   m_bFaultStatusCurrentSensor { false };
    bool                   m_bFaultStatusRotorLocked { false };

    bool                   m_bSimulatedSafeLock { false };
    bool                   m_bSimulatedBoardOverTemp { false };
    bool                   m_bSimulatedDcOverCurrent { false };
    bool                   m_bSimulatedAcOverVoltage { false };
    bool                   m_bSimulatedAcUnderVoltage { false };
    bool                   m_bSimulatedHallSensor { false };
    bool                   m_bSimulatedCurrentSensor { false };
    bool                   m_bSimulatedRotorLocked { false };

    quint16                m_u16MotorCurrent { 0 };
    quint16                m_u16MotorSpeed { 0 };

    quint8                 m_u8MotorTemp { 0 };

    quint8                 m_u8McuBoardTemp { 0 };

    quint16                m_u16MotorDcBusVoltage { 0 };

    quint32                m_u32ImageChunkNum { 0 };

    quint32                m_u32ImageTotalSize { 0 };

    quint32                m_u32ImageCurrentChunkNo { 0 };

    quint32                m_u32ChunkSize { 0 };

    SwUpdateStates         m_nSwUpdateState { SwUpdateIdle };

    QString                m_strProductId { "PRODID_1234" };
    QString                m_strHwVersion { "SN_1234" };
    QString                m_strSwVersion { "HW_VER_1234" };
    QString                m_strSerialNo { "SW_VER_1234" };

};

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#endif // HmiDriver_H
