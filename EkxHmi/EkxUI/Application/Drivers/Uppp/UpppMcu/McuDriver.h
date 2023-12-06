///////////////////////////////////////////////////////////////////////////////
///
/// @file McuDriver.h
///
/// @brief Header file of class McuDriver.
///
/// @author Ultratronik GmbH \n
///         Dornierstr. 9 \n
///         D-82205 Gilching \n
///         Germany
///         http://www.ultratronik.de
///
/// @author written by Gerd Esser, Forschung & Entwicklung, gesser@ultratronik.de
///
/// @date 18.08.2020
///
/// @copyright Copyright 2020 by Hemro International AG \n
///            Hemro International AG \n
///            Länggenstrasse 34 \n
///            CH 8184 Bachenbülach \n
///            Switzerland \n
///            Homepage: www.hemrogroup.com
///
///////////////////////////////////////////////////////////////////////////////

#ifndef McuDriver_H
#define McuDriver_H

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#include <QThread>
#include <QTimer>
#include <QList>
#include <QMutex>
#include <QFile>

class UpppMcu;
class UpppLevel3Msg;

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

class McuDriver
    : public QObject
{
    Q_OBJECT

    Q_PROPERTY( bool mcuAlive READ isMcuAlive NOTIFY mcuAliveChanged )

    Q_PROPERTY( bool motorTempValid READ isMotorTempValid NOTIFY motorTempValidChanged )
    Q_PROPERTY( int motorTemp READ motorTemp NOTIFY motorTempChanged )

    Q_PROPERTY( bool mcuBoardTempValid READ isMcuBoardTempValid NOTIFY mcuBoardTempValidChanged )
    Q_PROPERTY( int mcuBoardTemp READ mcuBoardTemp NOTIFY mcuBoardTempChanged )

    Q_PROPERTY( bool motorDcBusVoltageValid READ isMotorDcBusVoltageValid NOTIFY motorDcBusVoltageValidChanged )
    Q_PROPERTY( quint16 motorDcBusVoltage READ motorDcBusVoltage NOTIFY motorDcBusVoltageChanged )

    Q_PROPERTY( bool motorInfoValid READ isMotorInfoValid NOTIFY motorInfoValidChanged )
    Q_PROPERTY( quint16 motorCurrent READ motorCurrent NOTIFY motorCurrentChanged )
    Q_PROPERTY( quint16 motorSpeed READ motorSpeed NOTIFY motorSpeedChanged )

    Q_PROPERTY( bool mcuStatusValid READ isMcuStatusValid NOTIFY mcuStatusValidChanged )
    Q_PROPERTY( bool systemStatusMotorRunning READ isSystemStatusMotorRunning NOTIFY systemStatusMotorRunningChanged )
    Q_PROPERTY( bool systemStatusFanRunning READ isSystemStatusFanRunning NOTIFY systemStatusFanRunningChanged )
    Q_PROPERTY( bool systemStatusSimulation READ isSystemStatusSimulation NOTIFY systemStatusSimulationChanged )
    Q_PROPERTY( bool systemStatusHopperDismounted READ isSystemStatusHopperDismounted NOTIFY systemStatusHopperDismountedChanged )
    Q_PROPERTY( bool systemStatusGrinderChamberOpen READ isSystemStatusGrinderChamberOpen NOTIFY systemStatusGrinderChamberOpenChanged )

    Q_PROPERTY( bool faultStatusBoardOverTemp READ isFaultStatusBoardOverTemp NOTIFY faultStatusBoardOverTempChanged )
    Q_PROPERTY( bool faultStatusDcOverCurrent READ isFaultStatusDcOverCurrent NOTIFY faultStatusDcOverCurrentChanged )
    Q_PROPERTY( bool faultStatusAcOverVoltage READ isFaultStatusAcOverVoltage NOTIFY faultStatusAcOverVoltageChanged )
    Q_PROPERTY( bool faultStatusAcUnderVoltage READ isFaultStatusAcUnderVoltage NOTIFY faultStatusAcUnderVoltageChanged )
    Q_PROPERTY( bool faultStatusHallSensor READ isFaultStatusHallSensor NOTIFY faultStatusHallSensorChanged )
    Q_PROPERTY( bool faultStatusCurrentSensor READ isFaultStatusCurrentSensor NOTIFY faultStatusCurrentSensorChanged )
    Q_PROPERTY( bool faultStatusRotorLocked READ isFaultStatusRotorLocked NOTIFY faultStatusRotorLockedChanged )

    Q_PROPERTY( bool faultInconsistentMotorActuation READ isFaultInconsistentMotorActuation NOTIFY faultInconsistentMotorActuationChanged )

private:

    enum SwUpdateStates
    {
        SwUpdateIdle = 0,
        SwUpdateStart,
        SwUpdateData,
        SwUpdateFinish,
        SwUpdateReject,
        SwUpdateReset,
    };

public:

    McuDriver( QObject * parent = nullptr );

    virtual ~McuDriver();

    bool open();

    bool open( const QString & strPortName );

    void close();

    bool isOpen() const;

    bool isStatusAcknowledged( void ) const;

    bool isMcuAlive( void ) const;

    void setMcuConfig( const quint32 u32ConfigMaxMotorSpeed,
                       const quint32 u32ConfigNominalMotorSpeed,
                       const quint32 u32ConfigAccelerationTime,
                       const quint32 u32ConfigDecelerationTime );

    /*
       void enableSimulationMode( const bool bHopperDismounted,
                               const bool bGrinderChamberOpen,
                               const bool bBoardOverTemp,
                               const bool bDcOverCurrent,
                               const bool bAcOverVoltage,
                               const bool bAcUnderVoltage,
                               const bool bHallSensor,
                               const bool bCurrentSensor,
                               const bool bRotorLocked );
       void disableSimulationMode();
     */

    void killMcuAlive();

    bool startMotor();

    bool stopMotor();

    bool startSwUpdate();

    bool rejectSwUpdate();

    bool resetMcu();

    bool isProductIdValid() const;

    const QString & productId() const;

    const QString & serialNo() const;

    const QString & hwVersion() const;

    const QString & swVersion() const;

    bool isMotorTempValid() const;

    int motorTemp() const;

    bool isMcuBoardTempValid() const;

    int mcuBoardTemp() const;

    bool isMotorDcBusVoltageValid() const;

    quint16 motorDcBusVoltage() const;

    bool isMotorInfoValid() const;

    quint16 motorCurrent() const;

    quint16 motorSpeed() const;

    bool isMcuStatusValid() const;

    bool isSystemStatusMotorRunning() const;

    bool isSystemStatusFanRunning() const;

    bool isSystemStatusSimulation() const;

    bool isSystemStatusHopperDismounted() const;

    bool isSystemStatusGrinderChamberOpen() const;

    bool isFaultStatusBoardOverTemp() const;

    bool isFaultStatusDcOverCurrent() const;

    bool isFaultStatusAcOverVoltage() const;

    bool isFaultStatusAcUnderVoltage() const;

    bool isFaultStatusHallSensor() const;

    bool isFaultStatusCurrentSensor() const;

    bool isFaultStatusRotorLocked() const;

    bool isFaultInconsistentMotorActuation() const;

    UpppMcu * upppMcu();

    const UpppMcu * upppMcu() const;

signals:

    void statusAcknowledgedChanged();

    void mcuAliveChanged( const bool bAlive );

    void productIdRequestTimeout();

    void productIdValidChanged();

    void productIdChanged();

    void serialNoChanged();

    void hwVersionChanged();

    void swVersionChanged();

    void motorTempRequestTimeout();

    void motorTempValidChanged();

    void motorTempChanged();

    void mcuBoardTempRequestTimeout();

    void mcuBoardTempValidChanged();

    void mcuBoardTempChanged();

    void motorDcBusVoltageRequestTimeout();

    void motorDcBusVoltageValidChanged();

    void motorDcBusVoltageChanged();

    void motorInfoRequestTimeout();

    void motorInfoValidChanged();

    void motorCurrentChanged();

    void motorSpeedChanged();

    void mcuStatusValidChanged( const bool bValid );

    void systemStatusHmiAliveChanged( const bool bAlive );

    void systemStatusMotorRunningChanged( const bool bRunning );

    void systemStatusFanRunningChanged( const bool bRunning );

    void systemStatusSimulationChanged( const bool bSimulationActive );

    void systemStatusHopperDismountedChanged( const bool bDismounted );

    void systemStatusGrinderChamberOpenChanged( const bool bOpened );

    void faultStatusBoardOverTempChanged();

    void faultStatusDcOverCurrentChanged();

    void faultStatusAcOverVoltageChanged();

    void faultStatusAcUnderVoltageChanged();

    void faultStatusHallSensorChanged();

    void faultStatusCurrentSensorChanged();

    void faultStatusRotorLockedChanged();

    void faultInconsistentMotorActuationChanged();

    void swUpdateFinished( bool bSuccess );

    void swUpdateProgress( quint8 u8ProgressPercent );

private slots:

    void processTriggerHmiStatus( void );

    void processMcuAliveTimeout( void );

    void processMcuStatusRequestTimeout( void );

    void processProductIdRequestTimeout( void );

    void processMotorTempRequestTimeout( void );

    void processMcuBoardTempRequestTimeout( void );

    void processMotorInfoRequestTimeout( void );

    void processMotorDcBusVoltageRequestTimeout( void );

    void processStatusAcknowledged( void );

    void processMcuAliveChanged( void );

    void processHmiStatusAcknowledged();

    void processMcuStatusMsg( const bool bHmiAlive,
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
                              const bool bRotorLocked );

    void processProductIdMsg( const QString & strProductId,
                              const QString & strSerialNo,
                              const QString & strHwVersion,
                              const QString & strSwVersion );

    void processMotorTempMsg( const int nMotorTemp );

    void processMcuBoardTempMsg( const int nMcuBoardTemp );

    void processMotorInfoMsg( const quint16 u16MotorCurrent,
                              const quint16 u16MotorSpeed );

    void processMotorDcBusVoltageMsg( const quint16 u16DcBusVoltage );

    void processMcuMsgRequest( const int nTransactionId,
                               const int nRequestMsgType );

    void processMotorActuationVerify();

    void updateMcuAlive( void );

    void processMsgFinished( const int    nMsgType,
                             const quint8 u8TransactionId,
                             const bool   bSuccess );

    void triggerSwUpdateState( const int    nMsgType,
                               const quint8 u8TransactionId,
                               const bool   bSuccess );

private:

    void setFaultInconsistentMotorActuation( const bool bFault );

    void setStatusAcknowledged( const bool bAcknowledged );

    void setMcuStatusValid( const bool bValid );

    void setSystemStatusHmiAlive( const bool bAlive );

    void setSystemStatusMotorRunning( const bool bRunning );

    void setSystemStatusSimulation( const bool bSimulation );

    void setSystemStatusHopperDismounted( const bool bDismounted );

    void setSystemStatusGrinderChamberOpen( const bool bOpen );

    void setSystemStatusFanRunning( const bool bRunning );

    void setFaultStatusBoardOverTemp( const bool bOverTemp );

    void setFaultStatusDcOverCurrent( const bool bOverCurrent );

    void setFaultStatusAcOverVoltage( const bool bOverVoltage );

    void setFaultStatusAcUnderVoltage( const bool bUnderVoltage );

    void setFaultStatusHallSensor( const bool bHallSensor );

    void setFaultStatusCurrentSensor( const bool bCurrentSensor );

    void setFaultStatusRotorLocked( const bool bRotorLocked );

    void setProductIdValid( const bool bValid );

    void setProductId( const QString & strProductId );

    void setSerialNo( const QString & strSerialNo );

    void setHwVersion( const QString & strHwVersion );

    void setSwVersion( const QString & strSwVersion );

    void setMotorTempValid( const bool bValid );

    void setMotorTemp( const int nMotorTemp );

    void setMcuBoardTempValid( const bool bValid );

    void setMcuBoardTemp( const int nMcuBoardTemp );

    void setMotorDcBusVoltageValid( const bool bValid );

    void setMotorDcBusVoltage( const quint16 u16DcBusVoltage );

    void setMotorInfoValid( const bool bValid );

    void setMotorCurrent( const quint16 u16MotorCurrent );

    void setMotorSpeed( const quint16 u16MotorSpeed );

    //void readMcuConfig();

    void writeMcuHwSwRef();

private:

    UpppMcu *      m_pUpppMcu { nullptr };

    bool           m_bStatusAcknowledged { false };

    bool           m_bMcuAlive { false };

    QTimer         m_timerTriggerHmiStatus;

    QTimer         m_timerMcuAliveTimeout;

    QTimer         m_timerMcuStatusRequestTimeout;
    QTimer         m_timerProductIdRequestTimeout;
    QTimer         m_timerMotorInfoRequestTimeout;
    QTimer         m_timerMotorTempRequestTimeout;
    QTimer         m_timerMcuBoardTempRequestTimeout;
    QTimer         m_timerMotorDcBusVoltageRequestTimeout;

    QTimer         m_timerMotorActuationVerify;

    bool           m_bProductIdValid { false };

    QString        m_strProductId;

    QString        m_strSerialNo;

    QString        m_strHwVersion;

    QString        m_strSwVersion;

    bool           m_bMcuStatusValid { false };

    bool           m_bSystemStatusHmiAlive { false };

    bool           m_bSystemStatusMotorRunning { false };

    bool           m_bSystemStatusSimulation { false };

    bool           m_bSystemStatusHopperDismounted { false };

    bool           m_bSystemStatusGrinderChamberOpen { false };

    bool           m_bSystemStatusFanRunning { false };

    bool           m_bFaultStatusBoardOverTemp { false };

    bool           m_bFaultStatusDcOverCurrent { false };

    bool           m_bFaultStatusAcOverVoltage { false };

    bool           m_bFaultStatusAcUnderVoltage { false };

    bool           m_bFaultStatusHallSensor { false };

    bool           m_bFaultStatusCurrentSensor { false };

    bool           m_bFaultStatusRotorLocked { false };

    bool           m_bMotorTempValid { false };

    int            m_nMotorTemp { -50 };

    bool           m_bMcuBoardTempValid { false };

    int            m_nMcuBoardTemp { -50 };

    bool           m_bMotorInfoValid { false };

    quint16        m_u16MotorCurrent { 0 };

    quint16        m_u16MotorSpeed { 0 };

    bool           m_bMotorDcBusVoltageValid { false };

    quint16        m_u16MotorDcBusVoltage { 0 };

    quint32        m_u32ConfigMaxMotorSpeed { 1500 };

    quint32        m_u32ConfigNominalMotorSpeed { 1500 };

    quint32        m_u32ConfigAccelerationTime { 200 };

    quint32        m_u32ConfigDecelerationTime { 200 };

    bool           m_bCommandedMotorRunning { false };

    bool           m_bFaultInconsistentMotorActuation { false };

    QFile          m_fileSwUpdate;

    qint64         m_n64SwUpdateCurrentChunk { 0 };

    qint64         m_n64SwUpdateFileSize { 0 };

    SwUpdateStates m_nSwUpdateState { SwUpdateIdle };

    bool           m_bSwUpdateFailed { false };

};

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#endif // McuDriver_H
