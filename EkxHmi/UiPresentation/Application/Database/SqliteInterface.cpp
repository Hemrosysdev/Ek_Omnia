///////////////////////////////////////////////////////////////////////////////
///
/// @file StandbyController.cpp
///
/// @brief main application entry point of StandbyController.
///
/// @author Ultratronik GmbH
///         Dornierstr. 9
///         D-82205 Gilching
///         Germany
///         http://www.ultratronik.de
///
/// @author written by Gerd Esser, Forschung & Entwicklung, gesser@ultratronik.de
///
/// @date 25.01.2021
///
/// @copyright Copyright 2021 by Hemro International AG
///            Hemro International AG
///            Länggenstrasse 34
///            CH 8184 Bachenbülach
///            Switzerland
///            Homepage: www.hemrogroup.com
///
///////////////////////////////////////////////////////////////////////////////

#include "SqliteInterface.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlError>
#include <QSqlDriver>
#include <QDebug>
#include <QFileInfo>
#include <QDir>
#include <QRegularExpression>
#include <QQmlEngine>
#include <QQmlContext>
#include <QCoreApplication>

#include <stdlib.h>

#include "MainStatemachine.h"
#include "TimePeriod.h"
#include "EkxSqliteTypes.h"
#include "SqlQueryWorker.h"
#include "EkxSqlSchemata.h"

#define DB_FILE           "/db_storage/EkxSqlite.db"
//#define SQL_MASTER_FILE   "/usr/local/sql/EkxSqliteMaster.sql"

int SqliteInterface::m_nEkxSqliteTypeId = qmlRegisterUncreatableMetaObject( EkxSqliteTypes::staticMetaObject,   // static meta object
                                                                            "EkxSqliteTypes",                   // import statement
                                                                            1, 0,                               // major and minor version of the import
                                                                            "EkxSqliteTypes",                   // name in QML
                                                                            "Error: only enums" );

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

SqliteInterface::SqliteInterface( QQmlEngine * pEngine,
                                  MainStatemachine * pMainStatmachine )
    : QObject( pMainStatmachine )
    , m_pMainStatmachine( pMainStatmachine )
    , m_pSqlQueryWorker( new SqlQueryWorker( this ) )
{
    MainStatemachine::ensureExistingDirectory( dbFile() );
    //    MainStatemachine::ensureExistingDirectory( MainStatemachine::rootPath() + SQL_MASTER_FILE );

    pEngine->rootContext()->setContextProperty( "sqliteInterface" , this );

    connect( m_pSqlQueryWorker, &SqlQueryWorker::queryFinished, this, &SqliteInterface::processQueryFinished );
    connect( m_pSqlQueryWorker, &SqlQueryWorker::queryAborted, this, &SqliteInterface::processQueryAborted );

    initDatabase();
    updateRecipeMap();
    addEvent( EkxSqliteTypes::SqliteEventType_SYSTEM_RESTART );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

SqliteInterface::~SqliteInterface()
{
    delete m_pSqlQueryWorker;
    m_pSqlQueryWorker = nullptr;

    m_pMainStatmachine = nullptr;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

QString SqliteInterface::dbFile() const
{
    return MainStatemachine::rootPath() + DB_FILE;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SqliteInterface::startQueryStatisticsResult( TimePeriod &timePeriod )
{
    bool bSuccess = false;

    if ( !m_pSqlQueryWorker->isRunning() )
    {
        timePeriod.update( m_pMainStatmachine->settingsStatemachine()->settingsSerializer()->startOfWeekisSunNotMon() );

        QString strSql;
        QTextStream stream( &strSql );

        stream << "select recipes.recipe_name, "
               << "recipes.porta_filter_index, "
               << "recipe_modes.recipe_mode_name, "
               << "count(*) as shots_num, "
               << "sum( grinds.grind_time_100ths ) as grind_time "
               << "from grinds "
               << "inner join recipes on recipes.recipe_id=grinds.recipe_id "
               << "inner join recipe_modes on recipes.recipe_mode_id=recipe_modes.recipe_mode_id "
               << "where strftime( '%Y%m%d', grinds.timestamp ) >= "
               << "'" << timePeriod.fromDate().toString( "yyyyMMdd" ) << "' "
               << "and strftime( '%Y%m%d', grinds.timestamp ) <= "
               << "'" << timePeriod.toDate().toString( "yyyyMMdd" ) << "' "
               << "group by recipes.recipe_id;";

        if ( !m_pSqlQueryWorker->execThreaded( strSql ) )
        {
            qWarning() << "SqliteInterface::startQueryStatisticsResult(): failed to query statistics";
        }
        else
        {
            bSuccess = true;
        }
    }

    return bSuccess;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SqliteInterface::abortQueryStatistics()
{
    m_pSqlQueryWorker->abort();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SqliteInterface::processQueryFinished()
{
    qInfo() << "processQueryFinished()";

    for ( int i = 0; i < m_pSqlQueryWorker->statisticsResult().size(); i++ )
    {
        const StatisticsRecord & record = m_pSqlQueryWorker->statisticsResult()[i];

        qInfo() << "    Record" << record.name() << record.modeName() << record.grindTime100ths() << record.shotsNum();
    }

    emit queryFinished();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SqliteInterface::processQueryAborted()
{
    emit queryAborted();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

const QList<StatisticsRecord> &SqliteInterface::statisticsResult() const
{
    return m_pSqlQueryWorker->statisticsResult();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SqliteInterface::changeRecipe( const QString &strUuid,
                                    const QString &strName,
                                    const int nPortaFilterIndex,
                                    const int nRecipeModeId )
{
    bool bSuccess = false;

    SqliteDatabase db( this );

    int nRecipeId = m_mapRecipeIds[strUuid];

    QSqlQuery  query( db() );

    // UUID existing -> update existing recipe
    if ( nRecipeId )
    {
        query.prepare( "update recipes set recipe_name=:name, porta_filter_index=:porta_filter_index "
                           "where recipe_uuid=:recipe_uuid;" );
        query.bindValue( ":recipe_uuid", strUuid );
        query.bindValue( ":name", strName );
        query.bindValue( ":porta_filter_index", nPortaFilterIndex );

        if ( db().transaction() )
        {
            if ( query.exec() )
            {
                db().commit();
                bSuccess = true;
            }
            else
            {
                qWarning() << "SqliteInterface::changeRecipe(): failed to update changed recipe (" << strName << "/" << strUuid << "), rollback transaction," << query.lastError().text() ;
                db().rollback();
            }
        }
        else
        {
            qWarning() << "SqliteInterface::changeRecipe(): failed to lock database," << db().lastError().text() ;
        }
    }
    else // not existing -> create new record
    {
        query.prepare( "insert into recipes ( recipe_uuid, recipe_name, porta_filter_index, recipe_mode_id ) "
                          "values ( :recipe_uuid, :name, :porta_filter_index, :recipe_mode_id );" );
        query.bindValue( ":recipe_uuid", strUuid );
        query.bindValue( ":name", strName );
        query.bindValue( ":porta_filter_index", nPortaFilterIndex );
        query.bindValue( ":recipe_mode_id", nRecipeModeId );

        if ( db().transaction() )
        {
            if ( query.exec() )
            {
                db().commit();
                bSuccess = true;
            }
            else
            {
                qWarning() << "SqliteInterface::changeRecipe(): failed to insert new recipe (" << strName << "/" << strUuid << "), rollback transaction," << query.lastError().text() ;
                db().rollback();
            }
        }
        else
        {
            qWarning() << "SqliteInterface::changeRecipe(): failed to lock database," << db().lastError().text() ;
        }

        updateRecipeMap();
    }

    return bSuccess;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SqliteInterface::addGrind( const QString &strUuid,
                                const quint32 u32GrindTime100ths )
{
    bool bSuccess = false;

    if ( strUuid.isEmpty() )
    {
        qWarning() << "addGrind(): skip illegal empty recipe UUID";
    }
    else if ( u32GrindTime100ths )
    {
        {
            SqliteDatabase db( this );

            int nRecipeId = m_mapRecipeIds[strUuid];

            QSqlQuery  query( db() );
            query.prepare( "insert into grinds ( recipe_id, timestamp, grind_time_100ths ) "
                          "values ( :recipe_id, datetime('now','localtime'), :grind_time );" );
            query.bindValue( ":recipe_id", nRecipeId);
            query.bindValue( ":grind_time", u32GrindTime100ths );

            if ( db().transaction() )
            {
                if ( query.exec() )
                {
                    db().commit();
                    bSuccess = true;
                }
                else
                {
                    qWarning() << "SqliteInterface::addGrind(): failed to insert grind, rollback transaction," << query.lastError().text() ;
                    db().rollback();
                }
            }
            else
            {
                qWarning() << "SqliteInterface::addGrind(): failed to lock database," << db().lastError().text() ;
            }
        }

        addDeltaToCounter( EkxSqliteTypes::SqliteCounter_TOTAL_MOTOR_ON_TIME, u32GrindTime100ths );
        addDeltaToCounter( EkxSqliteTypes::SqliteCounter_DISC_USAGE_TIME, u32GrindTime100ths );
        addDeltaToCounter( EkxSqliteTypes::SqliteCounter_TOTAL_GRIND_SHOTS, 1 );
        addDeltaToCounter( EkxSqliteTypes::SqliteCounter_TOTAL_GRIND_TIME, u32GrindTime100ths );
    }

    return bSuccess;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SqliteInterface::addEvent( const int nEventId,
                                const QString &strContext )
{
    return addEvent( static_cast<EkxSqliteTypes::SqliteEventTypes>( nEventId ),
                     "",
                     EkxSqliteTypes::SqliteErrorType_UNKNOWN,
                     strContext );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SqliteInterface::addEvent( const EkxSqliteTypes::SqliteEventTypes nEventId,
                                const QString &strRecipeUuid,
                                const EkxSqliteTypes::SqliteErrorTypes nErrorId,
                                const QString &strContext )
{
    bool bSuccess = false;

    SqliteDatabase db( this );

    int nRecipeId = m_mapRecipeIds[strRecipeUuid];

    QSqlQuery  query( db() );
    query.prepare( "insert into events ( timestamp, event_type_id, recipe_id, error_type_id, context ) "
                          "values ( datetime('now','localtime'), :event_id, :recipe_id, :error_id, :context );" );
    query.bindValue( ":event_id", nEventId );
    query.bindValue( ":recipe_id", nRecipeId );
    query.bindValue( ":error_id", nErrorId );
    query.bindValue( ":context", strContext );  // TODO: check to escape it?

    if ( db().transaction() )
    {
        if ( query.exec() )
        {
            db().commit();
            bSuccess = true;
        }
        else
        {
            qWarning() << "SqliteInterface::addEvent(): failed to insert grind, rollback transaction," << query.lastError().text() ;
            db().rollback();
        }
    }
    else
    {
        qWarning() << "SqliteInterface::addEvent(): failed to lock database," << db().lastError().text() ;
    }

    return bSuccess;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SqliteInterface::addDeltaToCounter( const EkxSqliteTypes::SqliteCounters nCounterId,
                                         const quint32 u32AddValue )
{
    bool bSuccess = false;

    SqliteDatabase db( this );

    QSqlQuery  query( db() );

    query.prepare( "update counters set value=value+:value where counter_id=:counter_id;");
    query.bindValue( ":counter_id", nCounterId );
    query.bindValue( ":value", u32AddValue );

    if ( db().transaction() )
    {
        if ( query.exec() )
        {
            db().commit();
            bSuccess = true;
        }
        else
        {
            qWarning() << "SqliteInterface::addDeltaToCounter(): failed to update counter, rollback transaction," << query.lastError().text() ;
            db().rollback();
        }
    }
    else
    {
        qWarning() << "SqliteInterface::addDeltaToCounter(): failed to lock database," << db().lastError().text() ;
    }

    return bSuccess;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SqliteInterface::resetCounter( const EkxSqliteTypes::SqliteCounters nCounterId )
{
    bool bSuccess = false;

    SqliteDatabase db( this );

    QSqlQuery  query( db() );

    query.prepare( "update counters set value=0 where counter_id=:counter_id;");
    query.bindValue( ":counter_id", nCounterId );

    if ( db().transaction() )
    {
        if ( query.exec() )
        {
            db().commit();
            bSuccess = true;
        }
        else
        {
            qWarning() << "SqliteInterface::resetCounter(): failed to update counter, rollback transaction," << query.lastError().text() ;
            db().rollback();
        }
    }
    else
    {
        qWarning() << "SqliteInterface::resetCounter(): failed to lock database," << db().lastError().text() ;
    }

    return bSuccess;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SqliteInterface::queryCounter( const EkxSqliteTypes::SqliteCounters nCounterId,
                                    quint32 & u32Value )
{
    bool bSuccess = false;

    u32Value = -1;

    SqliteDatabase db( this );

    QSqlQuery  query( db() );

    query.prepare( "select * from counters where counter_id=:counter_id;");
    query.bindValue( ":counter_id", nCounterId );

    if ( query.exec() )
    {
        if ( query.first() )
        {
            u32Value = query.value( "value" ).toUInt();
            bSuccess = true;
        }
    }
    else
    {
        qWarning() << "SqliteInterface::queryCounter(): failed to read counter, " << query.lastError().text() ;
        db().rollback();
    }

    return bSuccess;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SqliteInterface::updateRecipeMap()
{
    m_mapRecipeIds.clear();

    SqliteDatabase db( this );

    QSqlQuery  query( db() );
    query.prepare( "select * from recipes;" );

    if ( query.exec() )
    {
        int idRecipeId = query.record().indexOf( "recipe_id" );
        int idRecipeUuid = query.record().indexOf( "recipe_uuid" );

        while ( query.next() )
        {
            int nRecipeId = query.value( idRecipeId ).toInt();
            QString strRecipeUuid = query.value( idRecipeUuid ).toString();

            m_mapRecipeIds[strRecipeUuid] = nRecipeId;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SqliteInterface::initDatabase()
{
    bool bSuccess = false;

    bSuccess = migrateDatabaseScheme();

    return bSuccess;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SqliteInterface::migrateDatabaseScheme()
{
    bool bSuccess = false;

    {
        SqliteDatabase db( this );

        QSqlQuery  query( db() );
        query.prepare( "select value from settings where key='Version';" );

        if ( query.exec() )
        {
            qInfo() << "migrateDatabaseScheme(): find table settings ... query it";
            int idValue = query.record().indexOf( "value" );
            if ( query.first() )
            {
                bSuccess = true;
                int nVersionIndex = query.value( idValue ).toInt();
                qInfo() << "migrateDatabaseScheme(): identify database version" << nVersionIndex;
                bSuccess = true;
            }
            else
            {
                qWarning() << "migrateDatabaseScheme(): no version record in table settings";
            }
        }
        else
        {
            qWarning() << "migrateDatabaseScheme(): query version failed" << query.lastError();
        }
    }

    if ( !bSuccess )
    {
        qWarning() << "migrateDatabaseScheme(): create new database content from strEkxSqliteMaster";
        bSuccess = executeSqlString( EkxSqlSchemata::strEkxSqliteMaster );
    }

    return bSuccess;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SqliteInterface::executeSqlFile( const QString & strSqlFile )
{
    bool bSuccess = false;

    qInfo() << "executeSqlFile(): run SQL file" << strSqlFile;

    // Read query file content
    QFile sqlFile( strSqlFile );

    if ( !sqlFile.open( QIODevice::ReadOnly ) )
    {
        qCritical() << "executeSqlFile(): error opening SQL file " << strSqlFile;
    }
    else
    {
        QString queryStr( sqlFile.readAll());
        sqlFile.close();

        bSuccess = executeSqlString( queryStr );
    }

    return bSuccess;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

/**
 * @brief executeQueryFile
 * Read a query file and removes some unnecessary characters/strings such as comments,
 * and executes queries.
 * If there is possible to use Transaction, so this method will uses it.
 * Use this gist as MIT License.
 */

bool SqliteInterface::executeSqlString( const QString & strSql )
{
    qInfo() << "executeSqlString()";

    bool bSuccess = true;

    SqliteDatabase db( this );

    QSqlQuery query( db() );
    QString queryStr = strSql;

    //Check if SQL Driver supports Transactions
    if ( db().driver()->hasFeature( QSqlDriver::Transactions ) )
    {
        //Replace comments and tabs and new lines with space
        queryStr.replace( QRegularExpression( "(\\/\\*(.|\\n)*?\\*\\/|^--.*\\n|\\t|\\n)",
                                              QRegularExpression::CaseInsensitiveOption
                                              | QRegularExpression::MultilineOption ),
                          " " );
        //Remove waste spaces
        queryStr = queryStr.trimmed();

        //Extracting queries
        QStringList qList = queryStr.split( ';', QString::SkipEmptyParts );

        //Initialize regular expression for detecting special queries (`begin transaction` and `commit`).
        //NOTE: I used new regular expression for Qt5 as recommended by Qt documentation.
        QRegularExpression re_transaction( "\\bbegin.transaction.*",
                                           QRegularExpression::CaseInsensitiveOption );
        QRegularExpression re_commit( "\\bcommit.*",
                                      QRegularExpression::CaseInsensitiveOption );

        //Check if query file is already wrapped with a transaction
        bool isStartedWithTransaction = re_transaction.match( qList.at(0) ).hasMatch();
        if( !isStartedWithTransaction )
        {
            db().transaction();     //<=== not wrapped with a transaction, so we wrap it with a transaction.
        }

        // Execute each individual queries
        foreach( const QString &s, qList)
        {
            if ( re_transaction.match(s).hasMatch() )    //<== detecting special query
            {
                db().transaction();
            }
            else if( re_commit.match(s).hasMatch() )    //<== detecting special query
            {
                db().commit();
            }
            else
            {
                query.exec( s );                        //<== execute normal query
                if ( query.lastError().type() != QSqlError::NoError )
                {
                    bSuccess = false;
                    qDebug() << "executeSqlString() failed," << query.lastError().text();
                    db().rollback();                    //<== rollback the transaction if there is any problem
                }
            }
        }

        if ( !isStartedWithTransaction )
        {
            db().commit();          //<== ... completing of wrapping with transaction
        }
    }
    else //Sql Driver doesn't supports transaction
    {
        //...so we need to remove special queries (`begin transaction` and `commit`)
        queryStr.replace( QRegularExpression( "(\\bbegin.transaction.*;|\\bcommit.*;|\\/\\*(.|\\n)*?\\*\\/|^--.*\\n|\\t|\\n)",
                                              QRegularExpression::CaseInsensitiveOption
                                              | QRegularExpression::MultilineOption ),
                          " " );
        queryStr = queryStr.trimmed();

        //Execute each individual queries
        QStringList qList = queryStr.split( ';', QString::SkipEmptyParts );
        foreach(const QString &s, qList)
        {
            query.exec(s);

            if ( query.lastError().type() != QSqlError::NoError )
            {
                bSuccess = false;
                qDebug() << query.lastError().text();
            }
        }
    }

    return bSuccess;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

SqliteInterface::SqliteDatabase::SqliteDatabase( SqliteInterface *pInterface )
{
    m_db = QSqlDatabase::database( "SqliteDatabase" );

    if ( !m_db.isValid() )
    {
        m_db = QSqlDatabase::addDatabase( "QSQLITE", "SqliteDatabase" );
    }

    m_db.setDatabaseName( pInterface->dbFile() );

    if ( m_bOpen || pInterface->m_pSqlQueryWorker->isRunning() )
    {
        qDebug() << "SqliteDatabase(): connection blocked, waiting to continue";

        while ( m_bOpen || pInterface->m_pSqlQueryWorker->isRunning() )
        {
            qApp->processEvents();
            QThread::msleep( 20 );
        }

        qDebug() << "SqliteDatabase(): continue from blocked connection";
    }

    if ( !m_db.open() )
    {
        qCritical() << "SqliteDatabase(): cannot open database, error" << m_db.lastError().text();
        m_db = QSqlDatabase::database( "dummy" );
    }
    else
    {
        m_bOpen = true;
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

SqliteInterface::SqliteDatabase::~SqliteDatabase()
{
    m_db.close();
    m_bOpen = false;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

