///////////////////////////////////////////////////////////////////////////////
///
/// @file UpppMcu.h
///
/// @brief Header file of class UpppMcu.
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

#ifndef UpppMcu_H
#define UpppMcu_H

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#include "UpppLevel3.h"

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

class UpppMcu
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
        McuSystemStatusHopperDismounted = 0x08,
        McuSystemStatusFanRunning = 0x10,
        McuSystemStatusGrinderChamberOpen = 0x20,
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
        McuNackReasonSwUpdateinvalidBinary,
        McuNackReasonSwUpdateNotPossible,
        McuNackReasonMotorStartNotPossible,
        McuNackReasonIllegalMotorConfig,
    };

public:

    UpppMcu( QObject * parent = nullptr );

    virtual ~UpppMcu() override;

    bool open( const QString & strPortName );

    static UpppLevel3Msg requestStatusMsg();

    static UpppLevel3Msg sendHmiStatusMsg( const bool bMcuAliveStatus );

    static UpppLevel3Msg sendMotorActuationMsg( const bool bStartMotor );

    static UpppLevel3Msg requestProductIdMsg();

    static UpppLevel3Msg sendMotorConfigMsg( const quint32 u32MaxMotorSpeed,
                                             const quint32 u32NominalMotorSpeed,
                                             const quint32 u32AccelerationTime,
                                             const quint32 u32DecelerationTime );

    static UpppLevel3Msg requestMotorTempMsg();

    static UpppLevel3Msg requestMcuBoardTempMsg();

    static UpppLevel3Msg requestMotorInfoMsg();

    static UpppLevel3Msg requestMotorDcBusVoltageMsg();

    static UpppLevel3Msg sendSimulateAlarmCodeMsg( const bool bHopperDismounted,
                                                   const bool bGrinderChamberOpen,
                                                   const bool bBoardOverTemp,
                                                   const bool bDcOverCurrent,
                                                   const bool bAcOverVoltage,
                                                   const bool bAcUnderVoltage,
                                                   const bool bHallSensor,
                                                   const bool bCurrentSensor,
                                                   const bool bRotorLocked );

    static UpppLevel3Msg sendSwUpdateStartMsg( const quint32 u32ChunkNum,
                                               const quint32 u32TotalSize );

    static UpppLevel3Msg sendSwUpdateDataMsg( const quint32      u32ChunkNo,
                                              const QByteArray & chunkData );

    static UpppLevel3Msg sendSwUpdateFinishMsg( const bool bSuccess );

    static UpppLevel3Msg sendSwUpdateRejectMsg();

    static UpppLevel3Msg sendMcuResetMsg();

signals:

    void mcuStatusReceived( const bool bAlive,
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

    void productIdReceived( const QString & strProductId,
                            const QString & strSerialNo,
                            const QString & strHwVersion,
                            const QString & strSwVersion );

    void motorTempReceived( const int nMotorTemp );

    void mcuBoardTempReceived( const int nMcuBoardTemp );

    void motorInfoReceived( const quint16 u16MotorCurrent,
                            const quint16 u16MotorSpeed );

    void motorDcBusVoltageReceived( const quint16 u16DcBusVoltage );

public slots:

private:

    void processLevel3Message( const UpppCoreMsg & coreMsg ) override;

    void processMcuStatus( const UpppCoreMsg & coreMsg );

    void processMotorInfo( const UpppCoreMsg & coreMsg );

    void processMotorTemp( const UpppCoreMsg & coreMsg );

    void processProductId( const UpppCoreMsg & coreMsg );

    void processMcuBoardTemp( const UpppCoreMsg & coreMsg );

    void processMotorDcBusVoltage( const UpppCoreMsg & coreMsg );

};

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#endif // UpppMcu_H
