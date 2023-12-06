///////////////////////////////////////////////////////////////////////////////
///
/// @file SqlQueryWorker.cpp
///
/// @brief main application entry point of SqlQueryWorker.
///
/// @author Ultratronik GmbH
///         Dornierstr. 9
///         D-82205 Gilching
///         Germany
///         http://www.ultratronik.de
///
/// @author written by Gerd Esser, Forschung & Entwicklung, gesser@ultratronik.de
///
/// @date 25.03.2021
///
/// @copyright Copyright 2021 by Hemro International AG
///            Hemro International AG
///            Länggenstrasse 34
///            CH 8184 Bachenbülach
///            Switzerland
///            Homepage: www.hemrogroup.com
///
///////////////////////////////////////////////////////////////////////////////

#include "SqlQueryWorker.h"

#include <QThread>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlError>
#include <QDebug>
#include <QSqlDatabase>
#include "SqliteInterface.h"

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

SqlQueryWorker::SqlQueryWorker( SqliteInterface * pSqlite )
    : m_pSqlite( pSqlite )
{
    connect( this, &SqlQueryWorker::queryStarted, this, &SqlQueryWorker::execInWorker );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

SqlQueryWorker::~SqlQueryWorker()
{
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SqlQueryWorker::execThreaded( const QString & strSql )
{
    bool bSuccess = false;

    m_statisticsResult.clear();

    m_theQuery.clear();

    if ( !isRunning() )
    {
        start();

        bSuccess = isRunning();

        if ( bSuccess )
        {
            moveToThread( this );

            m_strSql = strSql;
            emit queryStarted();
            bSuccess = true;
        }
    }
    else
    {
        qCritical() << "execThreaded(): thread is already running";
    }

    return bSuccess;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SqlQueryWorker::abort()
{
    quit();
    emit queryAborted();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

QSqlQuery &SqlQueryWorker::query()
{
    return m_theQuery;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

const QList<StatisticsRecord> & SqlQueryWorker::statisticsResult() const
{
    return m_statisticsResult;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SqlQueryWorker::execInWorker()
{
    // this must be another own connection because we are not in main thread
    QSqlDatabase db = QSqlDatabase::database( "SqlQueryWorker" );

    if ( !db.isValid() )
    {
        db = QSqlDatabase::addDatabase( "QSQLITE", "SqlQueryWorker" );
    }
    db.setDatabaseName( m_pSqlite->dbFile() );

    if ( db.open() )
    {
        m_theQuery = QSqlQuery( db );
        m_theQuery.prepare( m_strSql );

        if ( m_theQuery.exec() )
            m_theQuery.prepare( m_strSql );

       // QThread::msleep( 3000 );     // <<<<<< TODO remove later - just for testing animation

        if ( m_theQuery.exec() )
        {
            buildResultList();

            addCounters();

            emit queryFinished();
        }
        else
        {
            qWarning() << "execInWorker(): error in query" << m_theQuery.lastError().text();
            emit queryAborted();
        }

        db.close();
    }
    else
    {
        qCritical() << "execInWorker(): cannot open database" << db.lastError().text();
        emit queryAborted();
    }

    quit();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SqlQueryWorker::buildResultList()
{
    while ( m_theQuery.next() )
    {
        int idName = m_theQuery.record().indexOf( "recipe_name" );
        int idRecipeModeName = m_theQuery.record().indexOf( "recipe_mode_name" );
        int idShotsNum = m_theQuery.record().indexOf( "shots_num" );
        int idGrindTime = m_theQuery.record().indexOf( "grind_time" );
        int idPortaFilterIndex = m_theQuery.record().indexOf( "porta_filter_index" );

        StatisticsRecord  record;

        record.setName( m_theQuery.value( idName ).toString() );
        record.setModeName( m_theQuery.value( idRecipeModeName ).toString() );
        record.setGrindTime100ths( m_theQuery.value( idGrindTime ).toUInt() );
        record.setPortaFilterIndex( m_theQuery.value( idPortaFilterIndex ).toInt() );
        record.setShotsNum( m_theQuery.value( idShotsNum ).toUInt() );

        m_statisticsResult << record;
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SqlQueryWorker::addCounters()
{
    m_theQuery.clear();
    m_theQuery.prepare( "select * from counters where "
                        "counter_name='DISC_USAGE_TIME' "
                        "or counter_name='TOTAL_GRIND_TIME' "
                        "or counter_name='TOTAL_GRIND_SHOTS' "
                        ";" );

    if ( m_theQuery.exec() )
    {
        while ( m_theQuery.next() )
        {
            StatisticsRecord  record;

            QString strCounterName = m_theQuery.value( "counter_name" ).toString();
            if ( strCounterName == "DISC_USAGE_TIME" )
            {
                record.setName( "Frog Usage" );
                record.setGrindTime100ths( m_theQuery.value( "value" ).toUInt() );
                record.setShotsNum( 0 );
            }
            else if ( strCounterName == "TOTAL_GRIND_TIME" )
            {
                record.setName( "Smile Time" );
                record.setGrindTime100ths( m_theQuery.value( "value" ).toUInt() );
                record.setShotsNum( 0 );
            }
            else if ( strCounterName == "TOTAL_GRIND_SHOTS" )
            {
                record.setName( "Hunters" );
                record.setGrindTime100ths( 0 );
                record.setShotsNum( m_theQuery.value( "value" ).toUInt() );
            }

            record.setModeName( "COUNTERS" );
            record.setPortaFilterIndex( 0 );

            m_statisticsResult << record;
        }
    }
    else
    {
        qWarning() << "execInWorker(): error in counters query" << m_theQuery.lastError().text();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////
