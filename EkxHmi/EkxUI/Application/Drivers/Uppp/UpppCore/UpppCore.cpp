///////////////////////////////////////////////////////////////////////////////
///
/// @file UpppCore.cpp
///
/// @brief Implementation file of class UpppCore.
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

#include "UpppCore.h"

#include <QDebug>

#define MAX_MSG_LENGTH 512

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

UpppCore::UpppCore( QObject * parent )
    : QSerialPort( parent )
    , m_nParserState( Idle )
    , m_timerMsgTimeout( parent )
    , m_coreMsg( parent )
{
    connect( this, &UpppCore::readyRead, this, &UpppCore::processReadyRead );

    m_timerMsgTimeout.setInterval( 200 );
    m_timerMsgTimeout.setSingleShot( true );
    connect( &m_timerMsgTimeout, &QTimer::timeout, this, &UpppCore::processMsgTimeout );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

UpppCore::~UpppCore()
{
    UpppCore::close();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void UpppCore::openStreamInterface()
{
    if ( isOpen() )
    {
        close();
    }

    m_bStreamInterfaceOpen = true;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool UpppCore::isOpen() const
{
    bool bOpen = m_bStreamInterfaceOpen;

    if ( !bOpen )
    {
        bOpen = QSerialPort::isOpen();
    }

    return bOpen;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void UpppCore::close()
{
    m_timerMsgTimeout.stop();
    m_nParserState = Idle;

    if ( m_bStreamInterfaceOpen )
    {
        m_bStreamInterfaceOpen = false;
    }
    else
    {
        QSerialPort::close();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void UpppCore::processMessage( const UpppCoreMsg & coreMsg )
{
    Q_UNUSED( coreMsg )

    // null implementation, all stuff done in overloaded classes
    qCritical() << "not overloaded";
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool UpppCore::sendCoreMessage( const UpppCoreMsg & coreMsg )
{
    coreMsg.dump( "Send" );

    return coreMsg.send( *this );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool UpppCore::outputToStream( const QByteArray & data )
{
    bool bSuccess = true;

    if ( m_bStreamInterfaceOpen )
    {
        emit streamOutput( data );
    }
    else
    {
        foreach( char cByte, data )
        {
            bSuccess = putChar( cByte )
                       && bSuccess;
        }
    }

    return bSuccess;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void UpppCore::inputFromStream( const QByteArray & data )
{
    if ( m_bStreamInterfaceOpen )
    {
        foreach( char cByte, data )
        {
            messageParser( cByte );
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void UpppCore::processReadyRead()
{
    while ( bytesAvailable() )
    {
        char c;

        if ( getChar( &c ) )
        {
            messageParser( static_cast<quint8>( c ) );
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void UpppCore::processMsgTimeout()
{
    m_nParserState = Idle;

    qDebug() << "processMsgTimeout()";
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void UpppCore::messageParser( const quint8 u8Byte )
{
    //qDebug() << QString( "%1 %2" ).arg( (int)u8Byte, 2, 16, QLatin1Char( '0' ) ).arg( m_nParserState );
    switch ( m_nParserState )
    {
        case Idle:
            if ( u8Byte == UpppCoreMsg::PreambleByte1 )
            {
                m_timerMsgTimeout.start();
                m_nParserState = Preamble2;
                m_coreMsg.clear();
                m_crcIn.init();
                m_crcIn << u8Byte;
            }
            break;

        case Preamble2:
            if ( u8Byte == UpppCoreMsg::PreambleByte2 )
            {
                m_nParserState = MessageType;
                m_crcIn << u8Byte;
            }
            else
            {
                m_timerMsgTimeout.stop();
                m_nParserState = Idle;
                qWarning() << "messageParser() illegal preamble2 byte";
            }
            break;

        case MessageType:
            m_coreMsg.setMsgType( u8Byte );
            m_nParserState = TransactionId;
            m_crcIn << u8Byte;
            break;

        case TransactionId:
            m_coreMsg.setTransactionId( u8Byte );
            m_nParserState = Length1;
            m_crcIn << u8Byte;
            break;

        case Length1:
            m_nMsgLength   = u8Byte;
            m_nParserState = Length2;
            m_crcIn << u8Byte;
            break;

        case Length2:
            m_nMsgLength  |= ( u8Byte << 8 );
            m_nParserState = Length1;

            if ( m_nMsgLength > MAX_MSG_LENGTH )
            {
                m_timerMsgTimeout.stop();
                m_nParserState = Idle;
                qInfo() << "messageParser(): illegal message length " << m_nMsgLength;
            }
            else
            {
                m_coreMsg.payload().clear();
                m_nCurrentPayloadLength = 0;
                m_crcIn << u8Byte;

                if ( m_nMsgLength == 0 )
                {
                    m_nParserState = Crc1;
                }
                else
                {
                    m_nParserState = Payload;
                }
            }
            break;

        case Payload:
            if ( m_nCurrentPayloadLength < m_nMsgLength )
            {
                m_coreMsg.payload().push_back( static_cast<char>( u8Byte ) );
                m_nCurrentPayloadLength++;
                m_crcIn << u8Byte;

                if ( m_nCurrentPayloadLength == m_nMsgLength )
                {
                    m_nParserState = Crc1;
                }
            }
            break;

        case Crc1:
            m_nMsgCrc      = u8Byte;
            m_nParserState = Crc2;
            break;

        case Crc2:
            m_nMsgCrc     |= ( u8Byte << 8 );
            m_nParserState = Idle;
            m_timerMsgTimeout.stop();

            if ( m_nMsgCrc == m_crcIn.value() )
            {
                m_coreMsg.dump( "Receive" );
                processMessage( m_coreMsg );
            }
            else
            {
                qWarning() << "messageParser(): illegal message CRC " << m_nMsgCrc;
            }
            break;
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////
