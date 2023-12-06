///////////////////////////////////////////////////////////////////////////////
///
/// @file UpppCoreMsg.cpp
///
/// @brief Implementation file of class UpppCoreMsg.
///
/// @author Ultratronik GmbH \n
///         Dornierstr. 9 \n
///         D-82205 Gilching \n
///         Germany
///         http://www.ultratronik.de
///
/// @author written by Gerd Esser, Forschung & Entwicklung, gesser@ultratronik.de
///
/// @date 17.08.2020
///
/// @copyright Copyright 2020 by Hemro International AG \n
///            Hemro International AG \n
///            Länggenstrasse 34 \n
///            CH 8184 Bachenbülach \n
///            Switzerland \n
///            Homepage: www.hemrogroup.com
///
///////////////////////////////////////////////////////////////////////////////

#include "UpppCoreMsg.h"
#include "CcittCrc16.h"
#include "UpppCore.h"

#include <QDebug>

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

UpppCoreMsg::UpppCoreMsg(QObject *parent)
    : QObject( parent )
    , m_nMsgType( -1 )
    , m_u8TransactionId( 0xff )
{

}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

UpppCoreMsg::UpppCoreMsg( const UpppCoreMsg &theRight )
    : QObject()
{
    *this = theRight;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

UpppCoreMsg::~UpppCoreMsg()
{

}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

UpppCoreMsg & UpppCoreMsg::operator=(const UpppCoreMsg &theRight)
{
    if ( this != &theRight )
    {
        m_nMsgType        = theRight.m_nMsgType;
        m_u8TransactionId = theRight.m_u8TransactionId;
        m_payload         = theRight.m_payload;
    }

    return *this;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

UpppCoreMsg &UpppCoreMsg::operator<<( const quint8 u8Data )
{
    m_payload.push_back( static_cast<char>( u8Data ) );

    return *this;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

UpppCoreMsg &UpppCoreMsg::operator<<(const quint16 u16Data)
{
    *this << static_cast<quint8>( u16Data & 0xff );
    *this << static_cast<quint8>( ( u16Data >> 8 ) & 0xff );

    return *this;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

UpppCoreMsg &UpppCoreMsg::operator<<(const quint32 u32Data)
{
    *this << static_cast<quint8>( u32Data & 0xff );
    *this << static_cast<quint8>( ( u32Data >> 8 ) & 0xff );
    *this << static_cast<quint8>( ( u32Data >> 16 ) & 0xff );
    *this << static_cast<quint8>( ( u32Data >> 24 ) & 0xff );

    return *this;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

UpppCoreMsg &UpppCoreMsg::operator<<(const QByteArray &data)
{
    m_payload.push_back( data );

    return *this;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void UpppCoreMsg::clear()
{
    m_nMsgType = -1;
    m_u8TransactionId = 0;
    m_payload.clear();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

int UpppCoreMsg::msgType() const
{
    return m_nMsgType;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void UpppCoreMsg::setMsgType(const int nMsgType)
{
    m_nMsgType = nMsgType;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

quint8 UpppCoreMsg::transactionId() const
{
    return m_u8TransactionId;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void UpppCoreMsg::setTransactionId( const quint8 u8TransactionId )
{
    m_u8TransactionId = u8TransactionId;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

const QByteArray &UpppCoreMsg::payload() const
{
    return m_payload;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

QByteArray &UpppCoreMsg::payload()
{
    return m_payload;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

quint16 UpppCoreMsg::crc16() const
{
    CcittCrc16  crc;
    crc.init();

    crc << PreambleByte1;
    crc << PreambleByte2;
    crc << static_cast<quint8>( m_nMsgType );
    crc << m_u8TransactionId;

    tLeWord u16Length;
    u16Length.word = static_cast<quint16>( m_payload.size() );
    crc << u16Length.byte.lsb;
    crc << u16Length.byte.msb;

    crc << m_payload;

    return crc.value();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool UpppCoreMsg::send( UpppCore & core ) const
{
    bool bSuccess = false;

    if ( core.isOpen() )
    {
        QByteArray data;
        data.push_back( static_cast<char>( PreambleByte1 ) );
        data.push_back( static_cast<char>( PreambleByte2 ) );
        data.push_back( static_cast<char>( m_nMsgType ) );
        data.push_back( static_cast<char>( m_u8TransactionId ) );

        tLeWord u16Length;
        u16Length.word = static_cast<quint16>( m_payload.size() );
        data.push_back( static_cast<char>( u16Length.byte.lsb ) );
        data.push_back( static_cast<char>( u16Length.byte.msb ) );

        data.push_back( m_payload );

        tLeWord u16Crc;
        u16Crc.word = crc16();
        data.push_back( static_cast<char>( u16Crc.byte.lsb ) );
        data.push_back( static_cast<char>( u16Crc.byte.msb ) );

        QString strStream;
        QTextStream   stream( &strStream );

        bSuccess = true;
        for ( int i = 0; i < data.size(); i++ )
        {
            bSuccess = core.putChar( data[i] ) && bSuccess;
        }
    }

    return bSuccess;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void UpppCoreMsg::dump( const QString &strPrompt ) const
{
    QByteArray data;
    data.push_back( static_cast<char>( PreambleByte1 ) );
    data.push_back( static_cast<char>( PreambleByte2 ) );
    data.push_back( static_cast<char>( m_nMsgType ) );
    data.push_back( static_cast<char>( m_u8TransactionId ) );

    tLeWord u16Length;
    u16Length.word = static_cast<quint16>( m_payload.size() );
    data.push_back( static_cast<char>( u16Length.byte.lsb ) );
    data.push_back( static_cast<char>( u16Length.byte.msb ) );

    data.push_back( m_payload );

    tLeWord u16Crc;
    u16Crc.word = crc16();
    data.push_back( static_cast<char>( u16Crc.byte.lsb ) );
    data.push_back( static_cast<char>( u16Crc.byte.msb ) );

    QString strStream;
    QTextStream   stream( &strStream );

    for ( int i = 0; i < data.size(); i++ )
    {
        strStream += QString( "%1 " ).arg( static_cast<int>( data[i] ) & 0xff, 2, 16, QLatin1Char( '0' ) );
    }
    qDebug() << strPrompt << strStream;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////
