///////////////////////////////////////////////////////////////////////////////
///
/// @file BufferedQueryWriter.cpp
///
/// @brief main application entry point of BufferedQueryWriter.
///
/// @author Ultratronik GmbH
///         Dornierstr. 9
///         D-82205 Gilching
///         Germany
///         http://www.ultratronik.de
///
/// @author written by Gerd Esser, Forschung & Entwicklung, gesser@ultratronik.de
///
/// @date 12.04.2023
///
/// @copyright Copyright 2023 by Hemro International AG
///            Hemro International AG
///            Länggenstrasse 34
///            CH 8184 Bachenbülach
///            Switzerland
///            Homepage: www.hemrogroup.com
///
///////////////////////////////////////////////////////////////////////////////

#include "BufferedQueryWriter.h"

#include <QDebug>
#include <QFile>
#include <QVariant>

#include "SqliteInterface.h"

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

BufferedQueryWriter::BufferedQueryWriter()
{
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

BufferedQueryWriter::~BufferedQueryWriter()
{
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void BufferedQueryWriter::create( const QString &   strBufferFile,
                                  SqliteInterface * pSqliteInterface,
                                  const int         nFlushPeriodSec )
{
    m_strBufferFile    = strBufferFile;
    m_pSqliteInterface = pSqliteInterface;
    m_nFlushPeriodSec  = nFlushPeriodSec;

    connect( &m_timerFlush, &QTimer::timeout, this, &BufferedQueryWriter::processTimeoutFlush );

    m_timerFlush.setInterval( m_nFlushPeriodSec * 1000 );
    m_timerFlush.setSingleShot( false );
    m_timerFlush.start();

    // start one shot timer to ensure, buffer is flushed at least with each start of application
    QTimer::singleShot( 5000, this, &BufferedQueryWriter::processTimeoutFlush );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

const QString & BufferedQueryWriter::bufferFile() const
{
    return m_strBufferFile;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool BufferedQueryWriter::addSql( const QString & strSql )
{
    bool bSuccess = false;

    QString strTempSql = strSql.trimmed();

    if ( !strTempSql.isEmpty() )
    {
        QFile file( m_strBufferFile );

        if ( file.open( QIODevice::Append ) )
        {
            file.write( ( strTempSql + "\n" ).toUtf8() );
            file.flush();
            file.close();

            bSuccess = true;
        }
        else
        {
            qCritical() << "BufferedQueryWriter::addSql() cannot append SQL to buffer file" << m_strBufferFile;
        }
    }

    return bSuccess;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void BufferedQueryWriter::flush()
{
    if ( m_pSqliteInterface->executeSqlFile( m_strBufferFile ) )
    {
        qInfo() << "BufferedQueryWriter::flush() flush SQL buffer file" << m_strBufferFile;
    }
    else
    {
        qCritical() << "BufferedQueryWriter::flush() cannot flush SQL buffer file" << m_strBufferFile;
    }

    QFile::remove( m_strBufferFile );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void BufferedQueryWriter::prepareQuery( const QString & strQuery )
{
    m_strQuery = strQuery;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void BufferedQueryWriter::bindValue( const QString &  strPlaceholder,
                                     const QVariant & value )
{
    switch ( value.userType() )
    {
        case QVariant::Type::Double:
            m_strQuery.replace( strPlaceholder, QString::number( value.toDouble() ) );
            break;

        case QVariant::Type::Int:
            m_strQuery.replace( strPlaceholder, QString::number( value.toInt() ) );
            break;

        case QVariant::Type::LongLong:
            m_strQuery.replace( strPlaceholder, QString::number( value.toLongLong() ) );
            break;

        case QVariant::Type::UInt:
            m_strQuery.replace( strPlaceholder, QString::number( value.toUInt() ) );
            break;

        case QVariant::Type::ULongLong:
            m_strQuery.replace( strPlaceholder, QString::number( value.toULongLong() ) );
            break;

        case QVariant::Type::String:
            m_strQuery.replace( strPlaceholder, "\"" + value.toString() + "\"" );
            break;

        case QVariant::Type::Bool:
            m_strQuery.replace( strPlaceholder, QString::number( value.toBool() ) );
            break;

        default:
            qCritical() << "BufferedQueryWriter::bindValue() unsupported value type" << value.userType();
            break;
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool BufferedQueryWriter::addQuery()
{
    return addSql( m_strQuery );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void BufferedQueryWriter::processTimeoutFlush()
{
    flush();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////
