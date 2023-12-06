///////////////////////////////////////////////////////////////////////////////
///
/// @file UpppMcu.cpp
///
/// @brief Implementation file of class UpppMcu.
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

#include "UpppMcu.h"
#include "UpppLevel3Msg.h"

#include <QDebug>

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

UpppMcu::UpppMcu(QObject *parent)
    : UpppLevel3( parent )
{
    setBaudRate( QSerialPort::Baud115200 );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

UpppMcu::~UpppMcu()
{
    close();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool UpppMcu::open( const QString & strPortName )
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

UpppLevel3Msg UpppMcu::requestStatusMsg()
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
UpppLevel3Msg UpppMcu::sendHmiStatusMsg( const bool bMcuAliveStatus )
{
    UpppLevel3Msg msg;
    msg.setMsgType( Level3MsgTypeStatus );
    msg.setRepeatSend( false );
    msg << static_cast<quint8>( bMcuAliveStatus ? 0x1 : 0x0 );

    return msg;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

// static
UpppLevel3Msg UpppMcu::sendMotorActuationMsg(const bool bStartMotor)
{
    UpppLevel3Msg msg;
    msg.setMsgType( McuMsgTypeMotorActuation );
    msg << static_cast<quint8>( bStartMotor ? 0x1 : 0x0 );

    return msg;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

// static
UpppLevel3Msg UpppMcu::requestProductIdMsg()
{
    UpppLevel3Msg msg;
    msg.setMsgType( Level3MsgTypeRequest );
    msg << static_cast<quint8>( McuMsgTypeProductId );

    return msg;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

UpppLevel3Msg UpppMcu::sendMotorConfigMsg( const quint32 u32MaxMotorSpeed,
                                           const quint32 u32NominalMotorSpeed,
                                           const quint32 u32AccelerationTime,
                                           const quint32 u32DecelerationTime )
{
    UpppLevel3Msg msg;
    msg.setMsgType( McuMsgTypeMotorConfig );
    msg << u32MaxMotorSpeed;
    msg << u32NominalMotorSpeed;
    msg << u32AccelerationTime;
    msg << u32DecelerationTime;

    return msg;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

// static
UpppLevel3Msg UpppMcu::requestMotorTempMsg()
{
    UpppLevel3Msg msg;
    msg.setMsgType( Level3MsgTypeRequest );
    msg << static_cast<quint8>( McuMsgTypeMotorTemp );

    return msg;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

// static
UpppLevel3Msg UpppMcu::requestMcuBoardTempMsg()
{
    UpppLevel3Msg msg;
    msg.setMsgType( Level3MsgTypeRequest );
    msg << static_cast<quint8>( McuMsgTypeMcuBoardTemp );

    return msg;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

// static
UpppLevel3Msg UpppMcu::requestMotorInfoMsg()
{
    UpppLevel3Msg msg;
    msg.setMsgType( Level3MsgTypeRequest );
    msg << static_cast<quint8>( McuMsgTypeMotorInfo );

    return msg;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

// static
UpppLevel3Msg UpppMcu::requestMotorDcBusVoltageMsg()
{
    UpppLevel3Msg msg;
    msg.setMsgType( Level3MsgTypeRequest );
    msg << static_cast<quint8>( McuMsgTypeMotorDcBusVoltage );

    return msg;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

UpppLevel3Msg UpppMcu::sendSimulateAlarmCodeMsg( const bool bSafeLock,
                                                 const bool bBoardOverTemp,
                                                 const bool bDcOverCurrent,
                                                 const bool bAcOverVoltage,
                                                 const bool bAcUnderVoltage,
                                                 const bool bHallSensor,
                                                 const bool bCurrentSensor,
                                                 const bool bRotorLocked )
{
    UpppLevel3Msg msg;
    msg.setMsgType( McuMsgTypeSimulateAlarmCode );

    quint8 u8SystemCode = 0;
    u8SystemCode |= ( bSafeLock ? McuSystemStatusSafelock : 0 );
    msg << u8SystemCode;

    quint8 u8FaultCode = 0;
    u8FaultCode |= ( bBoardOverTemp  ? McuFaultStatusBoardOverTemp : 0 );
    u8FaultCode |= ( bDcOverCurrent  ? McuFaultStatusDcOverCurrent : 0 );
    u8FaultCode |= ( bAcOverVoltage  ? McuFaultStatusAcOverVoltage : 0 );
    u8FaultCode |= ( bAcUnderVoltage ? McuFaultStatusAcUnderVoltage : 0 );
    u8FaultCode |= ( bHallSensor     ? McuFaultStatusHallSensor : 0 );
    u8FaultCode |= ( bCurrentSensor  ? McuFaultStatusCurrentSensor : 0 );
    u8FaultCode |= ( bRotorLocked    ? McuFaultStatusRotorLocked : 0 );
    msg << u8FaultCode;

    return msg;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

// static
UpppLevel3Msg UpppMcu::sendSwUpdateStartMsg( const quint32 u32ChunkNum,
                                             const quint32 u32TotalSize )
{
    UpppLevel3Msg msg;
    msg.setMsgType( McuMsgTypeSwUpdateStart );
    msg.setTimeout( 950 );
    msg << u32ChunkNum;
    msg << u32TotalSize;

    return msg;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

// static
UpppLevel3Msg UpppMcu::sendSwUpdateDataMsg( const quint32 u32ChunkNo,
                                            const QByteArray & chunkData )
{
    UpppLevel3Msg msg;
    msg.setTimeout( 450 );
    msg.setMsgType( McuMsgTypeSwUpdateData );
    msg << u32ChunkNo;
    msg << chunkData;

    return msg;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

// static
UpppLevel3Msg UpppMcu::sendSwUpdateFinishMsg( const bool bSuccess )
{
    UpppLevel3Msg msg;
    msg.setTimeout( 950 );
    msg.setMsgType( McuMsgTypeSwUpdateFinish );
    msg << static_cast<quint8>( bSuccess ? 0x1 : 0x0 );

    return msg;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

// static
UpppLevel3Msg UpppMcu::sendSwUpdateRejectMsg()
{
    UpppLevel3Msg msg;
    msg.setTimeout( 1000 );
    msg.setMsgType( McuMsgTypeSwUpdateReject );

    return msg;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

// static
UpppLevel3Msg UpppMcu::sendMcuResetMsg()
{
    UpppLevel3Msg msg;
    msg.setMsgType( McuMsgTypeMcuReset );

    return msg;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void UpppMcu::processLevel3Message( const UpppCoreMsg &coreMsg )
{
    switch ( coreMsg.msgType() )
    {
        case Level3MsgTypeStatus:
            processMcuStatus( coreMsg );
        break;

        case McuMsgTypeMotorInfo:
            processMotorInfo( coreMsg );
        break;

        case McuMsgTypeMotorTemp:
            processMotorTemp( coreMsg );
        break;

        case McuMsgTypeProductId:
            processProductId( coreMsg );
        break;

        case McuMsgTypeMcuBoardTemp:
            processMcuBoardTemp( coreMsg );
        break;

        case McuMsgTypeMotorDcBusVoltage:
            processMotorDcBusVoltage( coreMsg );
        break;

        case McuMsgTypeMcuReset:
        case McuMsgTypeMotorConfig:
        case McuMsgTypeSwUpdateData:
        case McuMsgTypeSwUpdateStart:
        case McuMsgTypeMotorActuation:
        case McuMsgTypeSwUpdateFinish:
        case McuMsgTypeSwUpdateReject:
        case McuMsgTypeSimulateAlarmCode:
            qWarning() << "receive unsupported message" << coreMsg.msgType();
            sendNack( coreMsg.transactionId(),
                      McuNackReasonMessageTypeNotSupported );
        break;

        default:
            qWarning() << "receive invalid message type" << coreMsg.msgType();
            sendNack( coreMsg.transactionId(),
                      McuNackReasonInvalidMessageType );
        break;
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void UpppMcu::processMcuStatus( const UpppCoreMsg &coreMsg )
{
    if ( coreMsg.payload().size() != McuMsgLengthMcuStatus )
    {
        qWarning() << "receive invalid payload length" << coreMsg.payload().size();
        sendNack( coreMsg.transactionId(),
                  McuNackReasonInvalidPayloadLength );
    }
    else
    {
        quint8 u8SystemStatus = static_cast<quint8>( coreMsg.payload()[0] );
        quint8 u8FaultStatus  = static_cast<quint8>( coreMsg.payload()[1] );

        bool bAlive          = ( u8SystemStatus & McuSystemStatusAlive );
        bool bMotorRunning   = ( u8SystemStatus & McuSystemStatusMotorRunning );
        bool bSimulation     = ( u8SystemStatus & McuSystemStatusSimulation );
        bool bSafeLock       = ( u8SystemStatus & McuSystemStatusSafelock );
        bool bFanRunning     = ( u8SystemStatus & McuSystemStatusFanRunning );
        bool bBoardOverTemp  = ( u8FaultStatus & McuFaultStatusBoardOverTemp );
        bool bDcOverCurrent  = ( u8FaultStatus & McuFaultStatusDcOverCurrent );
        bool bAcOverVoltage  = ( u8FaultStatus & McuFaultStatusAcOverVoltage );
        bool bAcUnderVoltage = ( u8FaultStatus & McuFaultStatusAcUnderVoltage );
        bool bHallSensor     = ( u8FaultStatus & McuFaultStatusHallSensor );
        bool bCurrentSensor  = ( u8FaultStatus & McuFaultStatusCurrentSensor );
        bool bRotorLocked    = ( u8FaultStatus & McuFaultStatusRotorLocked );

        sendAck( coreMsg.transactionId() );

        emit mcuStatusReceived( bAlive,
                                bMotorRunning,
                                bSimulation,
                                bSafeLock,
                                bFanRunning,
                                bBoardOverTemp,
                                bDcOverCurrent,
                                bAcOverVoltage,
                                bAcUnderVoltage,
                                bHallSensor,
                                bCurrentSensor,
                                bRotorLocked );
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void UpppMcu::processMotorInfo( const UpppCoreMsg &coreMsg )
{
    if ( replyGranted() )
    {
        if ( coreMsg.payload().size() != McuMsgLengthMotorInfo )
        {
            qWarning() << "receive invalid payload length" << coreMsg.payload().size();
            sendNack( coreMsg.transactionId(),
                      McuNackReasonInvalidPayloadLength );
        }
        else
        {
            quint16 u16MotorCurrent = static_cast<quint8>( coreMsg.payload()[1] );
            u16MotorCurrent <<= 8;
            u16MotorCurrent |= static_cast<quint8>( coreMsg.payload()[0] );

            quint16 u16MotorSpeed = static_cast<quint8>( coreMsg.payload()[3] );
            u16MotorSpeed <<= 8;
            u16MotorSpeed |= static_cast<quint8>( coreMsg.payload()[2] );

            sendAck( coreMsg.transactionId() );

            emit motorInfoReceived( u16MotorCurrent,
                                    u16MotorSpeed );
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void UpppMcu::processMotorTemp( const UpppCoreMsg &coreMsg )
{
    if ( replyGranted() )
    {
        if ( coreMsg.payload().size() != McuMsgLengthMotorTemp )
        {
            qWarning() << "receive invalid payload length" << coreMsg.payload().size();
            sendNack( coreMsg.transactionId(),
                      McuNackReasonInvalidPayloadLength );
        }
        else
        {
            quint8 u8MotorTemp = static_cast<quint8>( coreMsg.payload()[0] );

            int nMotorTemp = static_cast<int>( u8MotorTemp ) - 50;

            sendAck( coreMsg.transactionId() );

            emit motorTempReceived( nMotorTemp );
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void UpppMcu::processProductId( const UpppCoreMsg &coreMsg )
{
    if ( replyGranted() )
    {
        if ( coreMsg.payload().size() != McuMsgLengthProductId )
        {
            qWarning() << "receive invalid payload length" << coreMsg.payload().size();
            sendNack( coreMsg.transactionId(),
                      McuNackReasonInvalidPayloadLength );
        }
        else
        {
            QString strProductId = coreMsg.payload().mid(  0, 20 );
            QString strSerialNo  = coreMsg.payload().mid( 20, 20 );
            QString strHwVersion = coreMsg.payload().mid( 40, 20 );
            QString strSwVersion = coreMsg.payload().mid( 60, 20 );

            sendAck( coreMsg.transactionId() );

            emit productIdReceived( strProductId,
                                    strSerialNo,
                                    strHwVersion,
                                    strSwVersion );
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void UpppMcu::processMcuBoardTemp( const UpppCoreMsg &coreMsg )
{
    if ( replyGranted() )
    {
        if ( coreMsg.payload().size() != McuMsgLengthMcuBoardTemp )
        {
            qWarning() << "receive invalid payload length" << coreMsg.payload().size();
            sendNack( coreMsg.transactionId(),
                      McuNackReasonInvalidPayloadLength );
        }
        else
        {
            quint8 u8McuBoardTemp = static_cast<quint8>( coreMsg.payload()[0] );

            int nMcuBoardTemp = static_cast<int>( u8McuBoardTemp ) - 50;

            sendAck( coreMsg.transactionId() );

            emit mcuBoardTempReceived( nMcuBoardTemp );
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void UpppMcu::processMotorDcBusVoltage( const UpppCoreMsg &coreMsg )
{
    if ( replyGranted() )
    {
        if ( coreMsg.payload().size() != McuMsgLengthMotorDcBusVoltage )
        {
            qWarning() << "receive invalid payload length" << coreMsg.payload().size();
            sendNack( coreMsg.transactionId(),
                      McuNackReasonInvalidPayloadLength );
        }
        else
        {
            quint16 u16MotorDcBusVoltage = static_cast<quint8>( coreMsg.payload()[1] );
            u16MotorDcBusVoltage <<= 8;
            u16MotorDcBusVoltage |= static_cast<quint8>( coreMsg.payload()[0] );

            sendAck( coreMsg.transactionId() );

            emit motorDcBusVoltageReceived( u16MotorDcBusVoltage );
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////
