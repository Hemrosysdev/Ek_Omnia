///////////////////////////////////////////////////////////////////////////////
///
/// @file UpppHmi.h
///
/// @brief Header file of class UpppHmi.
///
/// @author Ultratronik GmbH \n
///         Dornierstr. 9 \n
///         D-82205 Gilching \n
///         Germany
///         http://www.ultratronik.de
///
/// @author written by Gerd Esser, Forschung & Entwicklung, gesser@ultratronik.de
///
/// @date 13.08.2020
///
/// @copyright Copyright 2020 by Hemro International AG \n
///            Hemro International AG \n
///            Länggenstrasse 34 \n
///            CH 8184 Bachenbülach \n
///            Switzerland \n
///            Homepage: www.hemrogroup.com
///
///////////////////////////////////////////////////////////////////////////////

#ifndef UpppHmi_H
#define UpppHmi_H

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#include "UpppLevel3.h"

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

class UpppHmi
    : public UpppLevel3
{
    Q_OBJECT

public:

    enum tMcuMsgType
    {
        McuMsgTypeMotorActuation = 0x04,
        McuMsgTypeProductId = 0x05,
        McuMsgTypeMotorConfig = 0x06,
        McuMsgTypeMotorTemp = 0x07,
        McuMsgTypeMcuBoardTemp = 0x08,
        McuMsgTypeMotorInfo = 0x09,
        McuMsgTypeMotorDcBusVoltage = 0x0a,
        McuMsgTypeSimulateAlarmCode = 0x0b,
        McuMsgTypeSwUpdateStart = 0x0c,
        McuMsgTypeSwUpdateData = 0x0d,
        McuMsgTypeSwUpdateFinish = 0x0e,
        McuMsgTypeSwUpdateReject = 0x0f,
        McuMsgTypeMcuReset = 0x10
    };

    enum tMcuMsgLength
    {
        McuMsgLengthHmiStatus = 1,
        McuMsgLengthMcuStatus = 2,
        McuMsgLengthMotorActuation = 1,
        McuMsgLengthProductId = 80,
        McuMsgLengthMotorConfig = 16,
        McuMsgLengthMotorTemp = 1,
        McuMsgLengthMcuBoardTemp = 1,
        McuMsgLengthMotorInfo = 4,
        McuMsgLengthMotorDcBusVoltage = 2,
        McuMsgLengthSimulateAlarmCode = 2,
        McuMsgLengthSwUpdateStart = 8,
        McuMsgLengthSwUpdateDataMin = 5,
        McuMsgLengthSwUpdateDataMax = 132,
        McuMsgLengthSwUpdateFinish = 1,
        McuMsgLengthSwUpdateReject = 0,
        McuMsgLengthMcuReset = 0
    };

    enum tHmiStatus
    {
        HmiStatusAlive = 0x01,
    };

    enum tMcuSystemStatus
    {
        McuSystemStatusAlive = 0x01,
        McuSystemStatusMotorRunning = 0x02,
        McuSystemStatusSimulation = 0x04,
        McuSystemStatusSafelock = 0x08,
        McuSystemStatusFanRunning = 0x10,
    };

    enum tMcuFaultStatus
    {
        McuFaultStatusBoardOverTemp = 0x01,
        McuFaultStatusDcOverCurrent = 0x02,
        McuFaultStatusAcOverVoltage = 0x04,
        McuFaultStatusAcUnderVoltage = 0x08,
        McuFaultStatusHallSensor = 0x10,
        McuFaultStatusCurrentSensor = 0x20,
        McuFaultStatusRotorLocked = 0x40,
    };

    enum tMcuNackReason
    {
        McuNackReasonUnknown = 0,
        McuNackReasonPayloadDataOutOfRange,
        McuNackReasonInvalidCrc,
        McuNackReasonInvalidMessageType,
        McuNackReasonInvalidPayloadLength,
        McuNackReasonMessageTypeNotSupported,
        McuNackReasonSwUpdateNotStarted,
        McuNackReasonSwUpdateWrongChunkSequence,
        McuNackReasonSwUpdateIllegalBinaraySize,
        McuNackReasonSwUpdateChunkProcessingFailed,
        McuNackReasonSwUpdateInvalidBinary,
        McuNackReasonSwUpdateNotPossible,
        McuNackReasonMotorStartNotPossible,
        McuNackReasonIllegalMotorConfig,
    };

public:

    UpppHmi( QObject * parent );

    virtual ~UpppHmi() override;

    bool open( const QString & strPortName );

    static UpppLevel3Msg requestStatusMsg();

    static UpppLevel3Msg sendMcuStatusMsg( const bool bHmiAliveStatus,
                                           const bool bMotorRunning,
                                           const bool bSimulation,
                                           const bool bSafeLock,
                                           const bool bFanRunning,
                                           const bool bBoardOverTemp,
                                           const bool bDcOverCurrent,
                                           const bool bAcOverVoltage,
                                           const bool bAcUnderVoltage,
                                           const bool bHallSensor,
                                           const bool bCurrentSensor,
                                           const bool bRotorLocked );

    static UpppLevel3Msg sendProductIdMsg( const QString & strProductId,
                                           const QString & strSerialNo,
                                           const QString & strHwVersion,
                                           const QString & strSwversion );

    static UpppLevel3Msg sendMotorInfoMsg( const quint16 u16MotorCurrent,
                                           const quint16 u16MotorSpeed );

    static UpppLevel3Msg sendMotorTempMsg( const quint8 u8MotorTemp );

    static UpppLevel3Msg sendMcuBoardTempMsg( const quint8 u8McuBoardTemp );

    static UpppLevel3Msg sendMotorDcBusVoltageMsg( const quint16 u16MotorDcBusVoltage );

signals:

    void hmiStatusReceived( const quint8 u8TransactionId,
                            const bool   bAlive );

    void simulatedAlarmCodeReceived( const quint8 u8TransactionId,
                                     const bool   bSafeLock,
                                     const bool   bBoardOverTemp,
                                     const bool   bDcOverCurrent,
                                     const bool   bAcOverVoltage,
                                     const bool   bAcUnderVoltage,
                                     const bool   bHallSensor,
                                     const bool   bCurrentSensor,
                                     const bool   bRotorLocked );

    void motorConfigReceived( const quint8  u8TransactionId,
                              const quint32 u32MaxMotorSpeed,
                              const quint32 u32NominalMotorSpeed,
                              const quint32 u32AccelerationTime,
                              const quint32 u32DecelerationTime );

    void motorActuationReceived( const quint8 u8TransactionId,
                                 const bool   bStartMotor );

    void swUpdateStartReceived( const quint8  u8TransactionId,
                                const quint32 u32ChunkNum,
                                const quint32 u32TotalSize );

    void swUpdateDataReceived( const quint8       u8TransactionId,
                               const quint32      u32ChunkNo,
                               const QByteArray & chunkData );

    void swUpdateFinishReceived( const quint8 u8TransactionId,
                                 const bool   bSuccess );

    void swUpdateRejectReceived( const quint8 u8TransactionId );

    void mcuResetReceived( const quint8 u8TransactionId );

public slots:

private:

    void processLevel3Message( const UpppCoreMsg & coreMsg ) override;

    void processHmiStatus( const UpppCoreMsg & coreMsg );

    void processMotorConfig( const UpppCoreMsg & coreMsg );

    void processSimulatedAlarmCode( const UpppCoreMsg & coreMsg );

    void processMotorActuation( const UpppCoreMsg & coreMsg );

    void processSwUpdateStart( const UpppCoreMsg & coreMsg );

    void processSwUpdateData( const UpppCoreMsg & coreMsg );

    void processSwUpdateFinish( const UpppCoreMsg & coreMsg );

    void processSwUpdateReject( const UpppCoreMsg & coreMsg );

    void processMcuReset( const UpppCoreMsg & coreMsg );

};

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#endif // UpppHmi_H
