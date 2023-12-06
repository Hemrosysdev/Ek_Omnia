///////////////////////////////////////////////////////////////////////////////
///
/// @file UpppHmi.cpp
///
/// @brief Implementation file of class UpppHmi.
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

#include "UpppHmi.h"
#include "UpppLevel3Msg.h"

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

UpppHmi::UpppHmi( QObject * parent )
    : UpppLevel3( parent )
{
    setBaudRate( QSerialPort::Baud115200 );
    setDataBits( QSerialPort::Data8 );
    setStopBits( QSerialPort::StopBits::OneStop );
    setParity( QSerialPort::Parity::NoParity );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

UpppHmi::~UpppHmi()
{
    close();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool UpppHmi::open( const QString & strPortName )
{
    bool bSuccess = false;

    if ( !isOpen() )
    {
        setPortName( strPortName );

        bSuccess = QSerialPort::open( QIODevice::ReadWrite );
    }

    return bSuccess;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

UpppLevel3Msg UpppHmi::requestStatusMsg()
{
    UpppLevel3Msg msg;
    msg.setMsgType( Level3MsgTypeRequest );
    msg << static_cast<quint8>( Level3MsgTypeStatus );

    return msg;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

// static
UpppLevel3Msg UpppHmi::sendMcuStatusMsg( const bool bHmiAliveStatus,
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
                                         const bool bRotorLocked )
{
    UpppLevel3Msg msg;
    msg.setMsgType( Level3MsgTypeStatus );
    msg.setRepeatSend( false );
    quint8 u8SystemStatus = 0;
    u8SystemStatus |= ( bHmiAliveStatus ? McuSystemStatusAlive : 0 );
    u8SystemStatus |= ( bMotorRunning ? McuSystemStatusMotorRunning : 0 );
    u8SystemStatus |= ( bSimulation ? McuSystemStatusSimulation : 0 );
    u8SystemStatus |= ( bSafeLock ? McuSystemStatusSafelock : 0 );
    u8SystemStatus |= ( bFanRunning ? McuSystemStatusFanRunning : 0 );

    quint8 u8FaultStatus = 0;
    u8FaultStatus |= ( bBoardOverTemp ? McuFaultStatusBoardOverTemp : 0 );
    u8FaultStatus |= ( bDcOverCurrent ? McuFaultStatusDcOverCurrent : 0 );
    u8FaultStatus |= ( bAcOverVoltage ? McuFaultStatusAcOverVoltage : 0 );
    u8FaultStatus |= ( bAcUnderVoltage ? McuFaultStatusAcUnderVoltage : 0 );
    u8FaultStatus |= ( bHallSensor ? McuFaultStatusHallSensor : 0 );
    u8FaultStatus |= ( bCurrentSensor ? McuFaultStatusCurrentSensor : 0 );
    u8FaultStatus |= ( bRotorLocked ? McuFaultStatusRotorLocked : 0 );

    msg << u8SystemStatus;
    msg << u8FaultStatus;

    return msg;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

UpppLevel3Msg UpppHmi::sendProductIdMsg( const QString & strProductId,
                                         const QString & strSerialNo,
                                         const QString & strHwVersion,
                                         const QString & strSwVersion )
{
    UpppLevel3Msg msg;
    msg.setMsgType( McuMsgTypeProductId );
    msg << strProductId.toLatin1().leftJustified( 20, 0 );
    msg << strSerialNo.toLatin1().leftJustified( 20, 0 );
    msg << strHwVersion.toLatin1().leftJustified( 20, 0 );
    msg << strSwVersion.toLatin1().leftJustified( 20, 0 );

    return msg;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

UpppLevel3Msg UpppHmi::sendMotorInfoMsg( const quint16 u16MotorCurrent,
                                         const quint16 u16MotorSpeed )
{
    UpppLevel3Msg msg;
    msg.setMsgType( McuMsgTypeMotorInfo );
    msg << u16MotorCurrent;
    msg << u16MotorSpeed;

    return msg;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

UpppLevel3Msg UpppHmi::sendMotorTempMsg( const quint8 u8MotorTemp )
{
    UpppLevel3Msg msg;
    msg.setMsgType( McuMsgTypeMotorTemp );
    msg << u8MotorTemp;

    return msg;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

UpppLevel3Msg UpppHmi::sendMcuBoardTempMsg( const quint8 u8McuBoardTemp )
{
    UpppLevel3Msg msg;
    msg.setMsgType( McuMsgTypeMcuBoardTemp );
    msg << u8McuBoardTemp;

    return msg;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

UpppLevel3Msg UpppHmi::sendMotorDcBusVoltageMsg( const quint16 u16MotorDcBusVoltage )
{
    UpppLevel3Msg msg;
    msg.setMsgType( McuMsgTypeMotorDcBusVoltage );
    msg << u16MotorDcBusVoltage;

    return msg;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

//UpppLevel3Msg UpppHmi::sendSimulateAlarmCodeMsg( const bool bSafeLock,
//                                                 const bool bBoardOverTemp,
//                                                 const bool bDcOverCurrent,
//                                                 const bool bAcOverVoltage,
//                                                 const bool bAcUnderVoltage,
//                                                 const bool bHallSensor,
//                                                 const bool bCurrentSensor,
//                                                 const bool bRotorLocked )
//{
//    UpppLevel3Msg msg;
//    msg.setMsgType( McuMsgTypeSimulateAlarmCode );

//    quint8 u8SystemCode = 0;
//    u8SystemCode |= ( bSafeLock ? McuSystemStatusSafelock : 0 );
//    msg << u8SystemCode;

//    quint8 u8FaultCode = 0;
//    u8FaultCode |= ( bBoardOverTemp  ? McuFaultStatusBoardOverTemp : 0 );
//    u8FaultCode |= ( bDcOverCurrent  ? McuFaultStatusDcOverCurrent : 0 );
//    u8FaultCode |= ( bAcOverVoltage  ? McuFaultStatusAcOverVoltage : 0 );
//    u8FaultCode |= ( bAcUnderVoltage ? McuFaultStatusAcUnderVoltage : 0 );
//    u8FaultCode |= ( bHallSensor     ? McuFaultStatusHallSensor : 0 );
//    u8FaultCode |= ( bCurrentSensor  ? McuFaultStatusCurrentSensor : 0 );
//    u8FaultCode |= ( bRotorLocked    ? McuFaultStatusRotorLocked : 0 );
//    msg << u8FaultCode;

//    return msg;
//}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

//bool UpppHmi::sendSwUpdateStart( const quint32 u32ChunkNum,
//                                 const quint32 u32TotalSize )
//{
//    UpppLevel3Msg msg;
//    msg.setMsgType( McuMsgTypeSwUpdateStart );
//    msg << u32ChunkNum;
//    msg << u32TotalSize;

//    return sendMessage( msg );
//}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

//bool UpppHmi::sendSwUpdateData( const quint32 u32ChunkNo,
//                                const QByteArray & chunkData )
//{
//    UpppLevel3Msg msg;
//    msg.setMsgType( McuMsgTypeSwUpdateData );
//    msg << u32ChunkNo;
//    msg << chunkData;

//    return sendMessage( msg );
//}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

//bool UpppHmi::sendSwUpdateFinish( const bool bSuccess )
//{
//    UpppLevel3Msg msg;
//    msg.setMsgType( McuMsgTypeSwUpdateFinish );
//    msg << static_cast<quint8>( bSuccess ? 0x1 : 0x0 );

//    return sendMessage( msg );
//}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

//bool UpppHmi::sendSwUpdateReject()
//{
//    UpppLevel3Msg msg;
//    msg.setMsgType( McuMsgTypeSwUpdateReject );

//    return sendMessage( msg );
//}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void UpppHmi::processLevel3Message( const UpppCoreMsg & coreMsg )
{
    switch ( coreMsg.msgType() )
    {
        case Level3MsgTypeStatus:
            processHmiStatus( coreMsg );
            break;

        case McuMsgTypeMotorConfig:
            processMotorConfig( coreMsg );
            break;

        case McuMsgTypeSimulateAlarmCode:
            processSimulatedAlarmCode( coreMsg );
            break;

        case McuMsgTypeMotorActuation:
            processMotorActuation( coreMsg );
            break;

        case McuMsgTypeSwUpdateStart:
            processSwUpdateStart( coreMsg );
            break;

        case McuMsgTypeSwUpdateData:
            processSwUpdateData( coreMsg );
            break;

        case McuMsgTypeSwUpdateFinish:
            processSwUpdateFinish( coreMsg );
            break;

        case McuMsgTypeSwUpdateReject:
            processSwUpdateReject( coreMsg );
            break;

        case McuMsgTypeMcuReset:
            processMcuReset( coreMsg );
            break;

        case McuMsgTypeMotorTemp:
        case McuMsgTypeProductId:
        case McuMsgTypeMcuBoardTemp:
        case McuMsgTypeMotorDcBusVoltage:
        case McuMsgTypeMotorInfo:
            sendNack( coreMsg.transactionId(),
                      McuNackReasonMessageTypeNotSupported );
            break;

        default:
            sendNack( coreMsg.transactionId(),
                      McuNackReasonInvalidMessageType );
            break;
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void UpppHmi::processHmiStatus( const UpppCoreMsg & coreMsg )
{
    if ( coreMsg.payload().size() != McuMsgLengthHmiStatus )
    {
        sendNack( coreMsg.transactionId(),
                  McuNackReasonInvalidPayloadLength );
    }
    else
    {
        quint8 u8Status = static_cast<quint8>( coreMsg.payload()[0] );

        bool bAlive = ( u8Status & McuSystemStatusAlive );

        emit hmiStatusReceived( coreMsg.transactionId(),
                                bAlive );
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void UpppHmi::processMotorConfig( const UpppCoreMsg & coreMsg )
{
    if ( replyGranted() )
    {
        if ( coreMsg.payload().size() != McuMsgLengthMotorConfig )
        {
            sendNack( coreMsg.transactionId(),
                      McuNackReasonInvalidPayloadLength );
        }
        else
        {
            quint32 u32MaxMotorSpeed = static_cast<quint8>( coreMsg.payload()[3] );
            u32MaxMotorSpeed <<= 8;
            u32MaxMotorSpeed  |= static_cast<quint8>( coreMsg.payload()[2] );
            u32MaxMotorSpeed <<= 8;
            u32MaxMotorSpeed  |= static_cast<quint8>( coreMsg.payload()[1] );
            u32MaxMotorSpeed <<= 8;
            u32MaxMotorSpeed  |= static_cast<quint8>( coreMsg.payload()[0] );

            quint32 u32NominalMotorSpeed = static_cast<quint8>( coreMsg.payload()[7] );
            u32NominalMotorSpeed <<= 8;
            u32NominalMotorSpeed  |= static_cast<quint8>( coreMsg.payload()[6] );
            u32NominalMotorSpeed <<= 8;
            u32NominalMotorSpeed  |= static_cast<quint8>( coreMsg.payload()[5] );
            u32NominalMotorSpeed <<= 8;
            u32NominalMotorSpeed  |= static_cast<quint8>( coreMsg.payload()[4] );

            quint32 u32AccelerationTime = static_cast<quint8>( coreMsg.payload()[11] );
            u32AccelerationTime <<= 8;
            u32AccelerationTime  |= static_cast<quint8>( coreMsg.payload()[10] );
            u32AccelerationTime <<= 8;
            u32AccelerationTime  |= static_cast<quint8>( coreMsg.payload()[9] );
            u32AccelerationTime <<= 8;
            u32AccelerationTime  |= static_cast<quint8>( coreMsg.payload()[8] );

            quint32 u32DecelerationTime = static_cast<quint8>( coreMsg.payload()[15] );
            u32DecelerationTime <<= 8;
            u32DecelerationTime  |= static_cast<quint8>( coreMsg.payload()[14] );
            u32DecelerationTime <<= 8;
            u32DecelerationTime  |= static_cast<quint8>( coreMsg.payload()[13] );
            u32DecelerationTime <<= 8;
            u32DecelerationTime  |= static_cast<quint8>( coreMsg.payload()[12] );

            emit motorConfigReceived( coreMsg.transactionId(),
                                      u32MaxMotorSpeed,
                                      u32NominalMotorSpeed,
                                      u32AccelerationTime,
                                      u32DecelerationTime );
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void UpppHmi::processSimulatedAlarmCode( const UpppCoreMsg & coreMsg )
{
    if ( replyGranted() )
    {
        if ( coreMsg.payload().size() != McuMsgLengthSimulateAlarmCode )
        {
            sendNack( coreMsg.transactionId(),
                      McuNackReasonInvalidPayloadLength );
        }
        else
        {
            quint8 u8SystemStatus = static_cast<quint8>( coreMsg.payload()[0] );
            quint8 u8FaultStatus  = static_cast<quint8>( coreMsg.payload()[1] );

            bool bSafeLock       = ( u8SystemStatus & McuSystemStatusSafelock );
            bool bBoardOverTemp  = ( u8FaultStatus & McuFaultStatusBoardOverTemp );
            bool bDcOverCurrent  = ( u8FaultStatus & McuFaultStatusDcOverCurrent );
            bool bAcOverVoltage  = ( u8FaultStatus & McuFaultStatusAcOverVoltage );
            bool bAcUnderVoltage = ( u8FaultStatus & McuFaultStatusAcUnderVoltage );
            bool bHallSensor     = ( u8FaultStatus & McuFaultStatusHallSensor );
            bool bCurrentSensor  = ( u8FaultStatus & McuFaultStatusCurrentSensor );
            bool bRotorLocked    = ( u8FaultStatus & McuFaultStatusRotorLocked );

            emit simulatedAlarmCodeReceived( coreMsg.transactionId(),
                                             bSafeLock,
                                             bBoardOverTemp,
                                             bDcOverCurrent,
                                             bAcOverVoltage,
                                             bAcUnderVoltage,
                                             bHallSensor,
                                             bCurrentSensor,
                                             bRotorLocked );
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void UpppHmi::processMotorActuation( const UpppCoreMsg & coreMsg )
{
    if ( replyGranted() )
    {
        if ( coreMsg.payload().size() != McuMsgLengthMotorActuation )
        {
            sendNack( coreMsg.transactionId(),
                      McuNackReasonInvalidPayloadLength );
        }
        else
        {
            bool bStartMotor = ( static_cast<quint8>( coreMsg.payload()[0] ) & 0x1 );

            emit motorActuationReceived( coreMsg.transactionId(),
                                         bStartMotor );
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void UpppHmi::processSwUpdateStart( const UpppCoreMsg & coreMsg )
{
    if ( replyGranted() )
    {
        if ( coreMsg.payload().size() != McuMsgLengthSwUpdateStart )
        {
            sendNack( coreMsg.transactionId(),
                      McuNackReasonInvalidPayloadLength );
        }
        else
        {
            quint32 u32ChunkNum = static_cast<quint8>( coreMsg.payload()[3] );
            u32ChunkNum <<= 8;
            u32ChunkNum  |= static_cast<quint8>( coreMsg.payload()[2] );
            u32ChunkNum <<= 8;
            u32ChunkNum  |= static_cast<quint8>( coreMsg.payload()[1] );
            u32ChunkNum <<= 8;
            u32ChunkNum  |= static_cast<quint8>( coreMsg.payload()[0] );

            quint32 u32TotalSize = static_cast<quint8>( coreMsg.payload()[7] );
            u32TotalSize <<= 8;
            u32TotalSize  |= static_cast<quint8>( coreMsg.payload()[6] );
            u32TotalSize <<= 8;
            u32TotalSize  |= static_cast<quint8>( coreMsg.payload()[5] );
            u32TotalSize <<= 8;
            u32TotalSize  |= static_cast<quint8>( coreMsg.payload()[4] );

            emit swUpdateStartReceived( coreMsg.transactionId(),
                                        u32ChunkNum,
                                        u32TotalSize );
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void UpppHmi::processSwUpdateData( const UpppCoreMsg & coreMsg )
{
    if ( replyGranted() )
    {
        if ( coreMsg.payload().size() < McuMsgLengthSwUpdateDataMin
             || coreMsg.payload().size() > McuMsgLengthSwUpdateDataMax )
        {
            sendNack( coreMsg.transactionId(),
                      McuNackReasonInvalidPayloadLength );
        }
        else
        {
            quint32 u32ChunkNo = static_cast<quint8>( coreMsg.payload()[3] );
            u32ChunkNo <<= 8;
            u32ChunkNo  |= static_cast<quint8>( coreMsg.payload()[2] );
            u32ChunkNo <<= 8;
            u32ChunkNo  |= static_cast<quint8>( coreMsg.payload()[1] );
            u32ChunkNo <<= 8;
            u32ChunkNo  |= static_cast<quint8>( coreMsg.payload()[0] );

            QByteArray chunkData = coreMsg.payload().mid( 4 );

            emit swUpdateDataReceived( coreMsg.transactionId(),
                                       u32ChunkNo,
                                       chunkData );
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void UpppHmi::processSwUpdateFinish( const UpppCoreMsg & coreMsg )
{
    if ( replyGranted() )
    {
        if ( coreMsg.payload().size() != McuMsgLengthSwUpdateFinish )
        {
            sendNack( coreMsg.transactionId(),
                      McuNackReasonInvalidPayloadLength );
        }
        else
        {
            bool bSuccess = ( static_cast<quint8>( coreMsg.payload()[0] ) & 0x1 );

            emit swUpdateFinishReceived( coreMsg.transactionId(),
                                         bSuccess );
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void UpppHmi::processSwUpdateReject( const UpppCoreMsg & coreMsg )
{
    if ( replyGranted() )
    {
        if ( coreMsg.payload().size() != McuMsgLengthSwUpdateReject )
        {
            sendNack( coreMsg.transactionId(),
                      McuNackReasonInvalidPayloadLength );
        }
        else
        {
            emit swUpdateRejectReceived( coreMsg.transactionId() );
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void UpppHmi::processMcuReset( const UpppCoreMsg & coreMsg )
{
    if ( replyGranted() )
    {
        if ( coreMsg.payload().size() != McuMsgLengthMcuReset )
        {
            sendNack( coreMsg.transactionId(),
                      McuNackReasonInvalidPayloadLength );
        }
        else
        {
            emit mcuResetReceived( coreMsg.transactionId() );
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////
