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
#include <QCoreApplication>

#include <cstdlib>

#include "MainStatemachine.h"
#include "TimePeriod.h"
#include "EkxSqliteTypes.h"
#include "SqlQueryWorker.h"
#include "NotificationCenter.h"
#include "Notification.h"
#include "SettingsSerializer.h"
#include "StatisticsRecord.h"
#include "BufferedQueryWriter.h"

#define DB_FILE             "/db_storage/EkxSqlite.db"
#define BUFFERED_WRITE

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

SqliteInterface::SqliteInterface( QObject * pParent )
    : QObject( pParent )
    , m_pSqlQueryWorker( new SqlQueryWorker( this ) )
    , m_pBufferedQueryWriter( new BufferedQueryWriter() )
{
    MainStatemachine::ensureExistingDirectory( dbFile() );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

SqliteInterface::~SqliteInterface()
{
    delete m_pBufferedQueryWriter;
    m_pBufferedQueryWriter = nullptr;

    delete m_pSqlQueryWorker;
    m_pSqlQueryWorker = nullptr;

    m_pNotificationCenter = nullptr;
    m_pSettingsSerializer = nullptr;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SqliteInterface::create( NotificationCenter * pNotificationCenter,
                              SettingsSerializer * pSettingsSerializer )
{
    m_pNotificationCenter = pNotificationCenter;
    m_pSettingsSerializer = pSettingsSerializer;

    m_pBufferedQueryWriter->create( MainStatemachine::rootPath() + "/log/SqlBuffer.sql",
                                    this,
                                    3600 );   // each one hour
    MainStatemachine::ensureExistingDirectory( m_pBufferedQueryWriter->bufferFile() );

    connect( m_pSqlQueryWorker, &SqlQueryWorker::queryFinished, this, &SqliteInterface::processQueryFinished );
    connect( m_pSqlQueryWorker, &SqlQueryWorker::queryAborted, this, &SqliteInterface::processQueryAborted );

    addEvent( EkxSqliteTypes::SqliteEventType_SYSTEM_RESTART );

    initDatabase();
    updateRecipeMap();
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

bool SqliteInterface::startQueryStatisticsResult( TimePeriod & timePeriod )
{
    bool bSuccess = false;

    if ( !m_pSqlQueryWorker->isRunning() )
    {
        m_pBufferedQueryWriter->flush();

        timePeriod.update( m_pSettingsSerializer->isStartOfWeekSun() );

        QString     strSql;
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
            m_pNotificationCenter->raiseNotification( EkxSqliteTypes::SqliteNotificationType::SqliteNotificationType_DB_QUERY_STATISTICS_RESULT );
        }
        else
        {
            m_pNotificationCenter->removeNotification( EkxSqliteTypes::SqliteNotificationType::SqliteNotificationType_DB_QUERY_STATISTICS_RESULT );
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

const QList<StatisticsRecord> & SqliteInterface::statisticsResult() const
{
    return m_pSqlQueryWorker->statisticsResult();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SqliteInterface::changeRecipe( const QString & strUuid,
                                    const QString & strName,
                                    const int       nPortaFilterIndex,
                                    const int       nRecipeModeId )
{
    bool bSuccess = false;

    int nRecipeId = m_mapRecipeIds[strUuid];

    // UUID existing -> update existing recipe
    if ( nRecipeId )
    {
#ifdef BUFFERED_WRITE
        m_pBufferedQueryWriter->prepareQuery( "update recipes set recipe_name=:name, porta_filter_index=:porta_filter_index "
                                              "where recipe_uuid=:recipe_uuid;" );
        m_pBufferedQueryWriter->bindValue( ":recipe_uuid", strUuid );
        m_pBufferedQueryWriter->bindValue( ":name", strName );
        m_pBufferedQueryWriter->bindValue( ":porta_filter_index", nPortaFilterIndex );

        bSuccess = m_pBufferedQueryWriter->addQuery();
#else
        SqliteDatabase db( this );

        QSqlQuery query( db() );

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
                m_pNotificationCenter->removeNotification( EkxSqliteTypes::SqliteNotificationType::SqliteNotificationType_DB_CHANGE_RECIPE );
            }
            else
            {
                qWarning() << "SqliteInterface::changeRecipe(): failed to update changed recipe (" << strName << "/" << strUuid << "), rollback transaction," << query.lastError().text();
                db().rollback();
                m_pNotificationCenter->raiseNotification( EkxSqliteTypes::SqliteNotificationType::SqliteNotificationType_DB_CHANGE_RECIPE );
            }
        }
        else
        {
            qWarning() << "SqliteInterface::changeRecipe(): failed to lock database," << db().lastError().text();
            m_pNotificationCenter->raiseNotification( EkxSqliteTypes::SqliteNotificationType::SqliteNotificationType_DB_CHANGE_RECIPE );
        }
#endif
    }
    else // not existing -> create new record
    {
        SqliteDatabase db( this );

        QSqlQuery query( db() );

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
                m_pNotificationCenter->removeNotification( EkxSqliteTypes::SqliteNotificationType::SqliteNotificationType_DB_INSERT_RECIPE );
            }
            else
            {
                qWarning() << "SqliteInterface::changeRecipe(): failed to insert new recipe (" << strName << "/" << strUuid << "), rollback transaction," << query.lastError().text();
                db().rollback();
                m_pNotificationCenter->raiseNotification( EkxSqliteTypes::SqliteNotificationType::SqliteNotificationType_DB_INSERT_RECIPE );
            }
        }
        else
        {
            qWarning() << "SqliteInterface::changeRecipe(): failed to lock database," << db().lastError().text();
            m_pNotificationCenter->raiseNotification( EkxSqliteTypes::SqliteNotificationType::SqliteNotificationType_DB_INSERT_RECIPE );
        }

        updateRecipeMap();
    }

    return bSuccess;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SqliteInterface::addGrind( const QString & strUuid,
                                const quint32   u32GrindTime100ths )
{
    bool bSuccess = false;

    if ( strUuid.isEmpty() )
    {
        qWarning() << "addGrind(): skip illegal empty recipe UUID";
    }
    else if ( u32GrindTime100ths )
    {
        int nRecipeId = m_mapRecipeIds[strUuid];

        if ( nRecipeId == 0 )
        {
            qWarning() << "SqliteInterface::addGrind(): failed to insert grind, illegal recipe ID 0 for UUID" << strUuid;
            m_pNotificationCenter->raiseNotification( EkxSqliteTypes::SqliteNotificationType::SqliteNotificationType_DB_ADD_GRIND );
        }
        else
        {
#ifdef BUFFERED_WRITE
            m_pBufferedQueryWriter->prepareQuery( "insert into grinds ( recipe_id, timestamp, grind_time_100ths ) "
                                                  "values ( :recipe_id, :datetime, :grind_time );" );
            m_pBufferedQueryWriter->bindValue( ":recipe_id", nRecipeId );
            m_pBufferedQueryWriter->bindValue( ":grind_time", u32GrindTime100ths );
            m_pBufferedQueryWriter->bindValue( ":datetime", QDateTime::currentDateTime().toString( "yyyy-MM-dd hh:mm:ss" ) );

            bSuccess = m_pBufferedQueryWriter->addQuery();
#else
            SqliteDatabase db( this );

            QSqlQuery query( db() );
            query.prepare( "insert into grinds ( recipe_id, timestamp, grind_time_100ths ) "
                           "values ( :recipe_id, datetime('now','localtime'), :grind_time );" );
            query.bindValue( ":recipe_id", nRecipeId );
            query.bindValue( ":grind_time", u32GrindTime100ths );

            if ( db().transaction() )
            {
                if ( query.exec() )
                {
                    db().commit();
                    bSuccess = true;
                    m_pNotificationCenter->removeNotification( EkxSqliteTypes::SqliteNotificationType::SqliteNotificationType_DB_ADD_GRIND );
                }
                else
                {
                    qWarning() << "SqliteInterface::addGrind(): failed to insert grind, rollback transaction," << query.lastError().text();
                    db().rollback();
                    m_pNotificationCenter->raiseNotification( EkxSqliteTypes::SqliteNotificationType::SqliteNotificationType_DB_ADD_GRIND );
                }
            }
            else
            {
                qWarning() << "SqliteInterface::addGrind(): failed to lock database," << db().lastError().text();
                m_pNotificationCenter->raiseNotification( EkxSqliteTypes::SqliteNotificationType::SqliteNotificationType_DB_ADD_GRIND );
            }
#endif
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

bool SqliteInterface::addEvent( const int       nEventId,
                                const QString & strContext )
{
    return addEvent( static_cast<EkxSqliteTypes::SqliteEventTypes>( nEventId ),
                     "",
                     EkxSqliteTypes::SqliteNotificationType_UNKNOWN,
                     strContext );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SqliteInterface::addEvent( const EkxSqliteTypes::SqliteEventTypes       nEventId,
                                const QString &                              strRecipeUuid,
                                const EkxSqliteTypes::SqliteNotificationType nNotificationTypeId,
                                const QString &                              strContext )
{
    bool bSuccess = false;

    int nRecipeId = m_mapRecipeIds[strRecipeUuid];

#ifdef BUFFERED_WRITE
    m_pBufferedQueryWriter->prepareQuery( "insert into events ( timestamp, event_type_id, recipe_id, notification_type_id, context ) "
                                          "values ( :datetime, :event_id, :recipe_id, :notification_type_id, :context );" );
    m_pBufferedQueryWriter->bindValue( ":event_id", nEventId );
    m_pBufferedQueryWriter->bindValue( ":recipe_id", nRecipeId );
    m_pBufferedQueryWriter->bindValue( ":notification_type_id", nNotificationTypeId );
    m_pBufferedQueryWriter->bindValue( ":context", strContext );  // TODO: check to escape it?
    m_pBufferedQueryWriter->bindValue( ":datetime", QDateTime::currentDateTime().toString( "yyyy-MM-dd hh:mm:ss" ) );

    bSuccess = m_pBufferedQueryWriter->addQuery();
#else
    SqliteDatabase db( this );

    QSqlQuery query( db() );
    query.prepare( "insert into events ( timestamp, event_type_id, recipe_id, notification_type_id, context ) "
                   "values ( datetime('now','localtime'), :event_id, :recipe_id, :notification_type_id, :context );" );
    query.bindValue( ":event_id", nEventId );
    query.bindValue( ":recipe_id", nRecipeId );
    query.bindValue( ":notification_type_id", nNotificationTypeId );
    query.bindValue( ":context", strContext );  // TODO: check to escape it?

    if ( db().transaction() )
    {
        if ( query.exec() )
        {
            db().commit();
            bSuccess = true;
            // add event not inserted as DB error because of recursion
        }
        else
        {
            qWarning() << "SqliteInterface::addEvent(): failed to insert event, rollback transaction," << query.lastError().text();
            // add event not inserted as DB error because of recursion
        }
    }
    else
    {
        qWarning() << "SqliteInterface::addEvent(): failed to lock database," << db().lastError().text();
        // add event not inserted as DB error because of recursion
    }
#endif

    return bSuccess;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SqliteInterface::raiseNotification( const EkxSqliteTypes::SqliteNotificationType nNotificationTypeId,
                                         const QString &                              strContext )
{
    return addEvent( EkxSqliteTypes::SqliteEventTypes::SqliteEventType_ERROR_RAISE,
                     "",
                     nNotificationTypeId,
                     strContext );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SqliteInterface::removeNotification( const EkxSqliteTypes::SqliteNotificationType nNotificationTypeId,
                                          const QString &                              strContext )
{
    return addEvent( EkxSqliteTypes::SqliteEventTypes::SqliteEventType_ERROR_GONE,
                     "",
                     nNotificationTypeId,
                     strContext );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SqliteInterface::addDeltaToCounter( const EkxSqliteTypes::SqliteCounters nCounterId,
                                         const quint32                        u32AddValue )
{
    bool bSuccess = false;

#ifdef BUFFERED_WRITE

    m_pBufferedQueryWriter->prepareQuery( "update counters set value=value+:value where counter_id=:counter_id;" );
    m_pBufferedQueryWriter->bindValue( ":counter_id", nCounterId );
    m_pBufferedQueryWriter->bindValue( ":value", u32AddValue );

    bSuccess = m_pBufferedQueryWriter->addQuery();
    if ( !bSuccess )
    {
        qWarning() << "SqliteInterface::addDeltaToCounter(): failed to add query";
    }
#else
    //    SqliteDatabase db( this );

    //    QSqlQuery  query( db() );

    //    query.prepare( "update counters set value=value+:value where counter_id=:counter_id;");
    //    query.bindValue( ":counter_id", nCounterId );
    //    query.bindValue( ":value", u32AddValue );

    //    if ( db().transaction() )
    //    {
    //        if ( query.exec() )
    //        {
    //            db().commit();
    //            bSuccess = true;
    //            m_pNotificationCenter->removeNotification( EkxSqliteTypes::SqliteNotificationType::SqliteNotificationType_DB_ADD_DELTA_TO_COUNTER );
    //        }
    //        else
    //        {
    //            qWarning() << "SqliteInterface::addDeltaToCounter(): failed to update counter, rollback transaction," << query.lastError().text() ;
    //            db().rollback();
    //            m_pNotificationCenter->raiseNotification( EkxSqliteTypes::SqliteNotificationType::SqliteNotificationType_DB_ADD_DELTA_TO_COUNTER );
    //        }
    //    }
    //    else
    //    {
    //        qWarning() << "SqliteInterface::addDeltaToCounter(): failed to lock database," << db().lastError().text() ;
    //        m_pNotificationCenter->raiseNotification( EkxSqliteTypes::SqliteNotificationType::SqliteNotificationType_DB_ADD_DELTA_TO_COUNTER );
    //    }
#endif

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

    QSqlQuery query( db() );

    query.prepare( "update counters set value=0 where counter_id=:counter_id;" );
    query.bindValue( ":counter_id", nCounterId );

    if ( db().transaction() )
    {
        if ( query.exec() )
        {
            db().commit();
            bSuccess = true;
            m_pNotificationCenter->removeNotification( EkxSqliteTypes::SqliteNotificationType::SqliteNotificationType_DB_RESET_COUNTER );
        }
        else
        {
            qWarning() << "SqliteInterface::resetCounter(): failed to update counter, rollback transaction," << query.lastError().text();
            db().rollback();
            m_pNotificationCenter->raiseNotification( EkxSqliteTypes::SqliteNotificationType::SqliteNotificationType_DB_RESET_COUNTER );
        }
    }
    else
    {
        qWarning() << "SqliteInterface::resetCounter(): failed to lock database," << db().lastError().text();
        m_pNotificationCenter->raiseNotification( EkxSqliteTypes::SqliteNotificationType::SqliteNotificationType_DB_RESET_COUNTER );
    }

    return bSuccess;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SqliteInterface::queryCounter( const EkxSqliteTypes::SqliteCounters nCounterId,
                                    quint32 &                            u32Value )
{
    bool bSuccess = false;

    u32Value = -1;

    SqliteDatabase db( this );

    QSqlQuery query( db() );

    query.prepare( "select * from counters where counter_id=:counter_id;" );
    query.bindValue( ":counter_id", nCounterId );

    if ( query.exec() )
    {
        if ( query.first() )
        {
            u32Value = query.value( "value" ).toUInt();
            bSuccess = true;
            m_pNotificationCenter->removeNotification( EkxSqliteTypes::SqliteNotificationType::SqliteNotificationType_DB_QUERY_COUNTER );
        }
    }
    else
    {
        qWarning() << "SqliteInterface::queryCounter(): failed to read counter, " << query.lastError().text();
        db().rollback();
        m_pNotificationCenter->raiseNotification( EkxSqliteTypes::SqliteNotificationType::SqliteNotificationType_DB_QUERY_COUNTER );
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

    QSqlQuery query( db() );
    query.prepare( "select * from recipes;" );

    if ( query.exec() )
    {
        int idRecipeId   = query.record().indexOf( "recipe_id" );
        int idRecipeUuid = query.record().indexOf( "recipe_uuid" );

        while ( query.next() )
        {
            int     nRecipeId     = query.value( idRecipeId ).toInt();
            QString strRecipeUuid = query.value( idRecipeUuid ).toString();

            m_mapRecipeIds[strRecipeUuid] = nRecipeId;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SqliteInterface::incrementSystemOnTimeCounterByMinute()
{
    addDeltaToCounter( EkxSqliteTypes::SqliteCounters::SqliteCounter_SYSTEM_ON_TIME, 60 );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SqliteInterface::incrementStandbyTimeCounterByMinute()
{
    addDeltaToCounter( EkxSqliteTypes::SqliteCounters::SqliteCounter_STANDBY_TIME, 60 );
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

        QSqlQuery query( db() );
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

                bool bUpdateRequired = false;

                if ( nVersionIndex > EkxSqliteTypes::SqliteVersion::SqliteVersionIndex )
                {
                    qCritical() << "migrateDatabaseScheme(): database version too big";
                    bSuccess = false;
                }
                else if ( nVersionIndex < EkxSqliteTypes::SqliteVersion::SqliteVersionIndex )
                {
                    qWarning() << "migrateDatabaseScheme(): database update required to version " << EkxSqliteTypes::SqliteVersion::SqliteVersionIndex;
                    bUpdateRequired = true;
                }
                else
                {
                    qInfo() << "migrateDatabaseScheme(): no update required";
                    bUpdateRequired = false;
                }

                nVersionIndex = std::max( nVersionIndex, 0 );

                if ( bSuccess
                     && bUpdateRequired )
                {
                    bSuccess = doDatabaseUpdate( nVersionIndex );
                }
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
        bSuccess = executeSqlFile( ":/DatabaseUpgrades/EkxSqliteMaster.sql" );

        if ( bSuccess )
        {
            m_pNotificationCenter->raiseNotification( EkxSqliteTypes::SqliteNotificationType::SqliteNotificationType_DB_CREATE_NEW_SCHEME );
            m_pNotificationCenter->removeNotification( EkxSqliteTypes::SqliteNotificationType::SqliteNotificationType_DB_CREATE_NEW_SCHEME_FAILED );
        }
        else
        {
            m_pNotificationCenter->raiseNotification( EkxSqliteTypes::SqliteNotificationType::SqliteNotificationType_DB_CREATE_NEW_SCHEME_FAILED );
            m_pNotificationCenter->removeNotification( EkxSqliteTypes::SqliteNotificationType::SqliteNotificationType_DB_CREATE_NEW_SCHEME );
        }
    }

    return bSuccess;
}

/////////////////////////////////////////////////////////////////
//
//
/////////////////////////////////////////////////////////////////

bool SqliteInterface::doDatabaseUpdate( const int nFromDbVersion )
{
    bool bSuccess = true;

    for ( int i = nFromDbVersion + 1; i <= EkxSqliteTypes::SqliteVersion::SqliteVersionIndex; i++ )
    {
        QString strUpgradeName = QString( ":/DatabaseUpgrades/db_upgrade_%1.sql" ).arg( i );

        if ( !executeSqlFile( strUpgradeName ) )
        {
            bSuccess = false;
            break;
        }
    }

    if ( bSuccess )
    {
        m_pNotificationCenter->removeNotification( EkxSqliteTypes::SqliteNotificationType::SqliteNotificationType_DB_UPDATE_FAILED );
    }
    else
    {
        m_pNotificationCenter->raiseNotification( EkxSqliteTypes::SqliteNotificationType::SqliteNotificationType_DB_UPDATE_FAILED );
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
        QString queryStr( sqlFile.readAll() );
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
    QString   queryStr = strSql;

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
        QStringList qList = queryStr.split( ';', Qt::SkipEmptyParts );

        if ( qList.size() )
        {
            //Initialize regular expression for detecting special queries (`begin transaction` and `commit`).
            //NOTE: I used new regular expression for Qt5 as recommended by Qt documentation.
            QRegularExpression re_transaction( "\\bbegin.transaction.*",
                                               QRegularExpression::CaseInsensitiveOption );
            QRegularExpression re_commit( "\\bcommit.*",
                                          QRegularExpression::CaseInsensitiveOption );

            //Check if query file is already wrapped with a transaction
            bool isStartedWithTransaction = re_transaction.match( qList.at( 0 ) ).hasMatch();
            if ( !isStartedWithTransaction )
            {
                db().transaction();     //<=== not wrapped with a transaction, so we wrap it with a transaction.
            }

            // Execute each individual queries
            foreach( const QString & s, qList )
            {
                if ( re_transaction.match( s ).hasMatch() )    //<== detecting special query
                {
                    db().transaction();
                }
                else if ( re_commit.match( s ).hasMatch() )    //<== detecting special query
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
        QStringList qList = queryStr.split( ';', Qt::SkipEmptyParts );
        foreach( const QString & s, qList )
        {
            query.exec( s );

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

const SqliteInterface::StringMap SqliteInterface::getRecipeNames()
{
    StringMap list;

    {
        SqliteDatabase db( this );

        QSqlQuery query( db() );
        query.prepare( "select recipe_id key, recipe_name value from recipes order by recipe_name;" );

        if ( query.exec() )
        {
            int idKey   = query.record().indexOf( "key" );
            int idValue = query.record().indexOf( "value" );

            while ( query.next() )
            {
                int     nKey     = query.value( idKey ).toInt();
                QString strValue = query.value( idValue ).toString();

                list[nKey] = strValue;
            }
        }
        else
        {
            qWarning() << "getRecipeNames(): query failed" << query.lastError();
        }
    }

    return list;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

const SqliteInterface::StringMap SqliteInterface::getRecipeModes()
{
    StringMap list;

    {
        SqliteDatabase db( this );

        QSqlQuery query( db() );
        query.prepare( "select recipe_mode_id key, recipe_mode_name value from recipe_modes order by recipe_mode_name;" );

        if ( query.exec() )
        {
            int idKey   = query.record().indexOf( "key" );
            int idValue = query.record().indexOf( "value" );

            while ( query.next() )
            {
                int     nKey     = query.value( idKey ).toInt();
                QString strValue = query.value( idValue ).toString();

                list[nKey] = strValue;
            }
        }
        else
        {
            qWarning() << "getRecipeModes(): query failed" << query.lastError();
        }
    }

    return list;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

const SqliteInterface::StringMap SqliteInterface::getEventTypes()
{
    StringMap list;

    {
        SqliteDatabase db( this );

        QSqlQuery query( db() );
        query.prepare( "select event_type_id key, event_name value from event_types order by event_name;" );

        if ( query.exec() )
        {
            int idKey   = query.record().indexOf( "key" );
            int idValue = query.record().indexOf( "value" );

            while ( query.next() )
            {
                int     nKey     = query.value( idKey ).toInt();
                QString strValue = query.value( idValue ).toString();

                list[nKey] = strValue;
            }
        }
        else
        {
            qWarning() << "getEventTypes(): query failed" << query.lastError();
        }
    }

    return list;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

const SqliteInterface::StringMap SqliteInterface::getNotificationTypes()
{
    StringMap list;

    {
        SqliteDatabase db( this );

        QSqlQuery query( db() );
        query.prepare( "select notification_type_id key, notification_name value from notification_types order by notification_name;" );

        if ( query.exec() )
        {
            int idKey   = query.record().indexOf( "key" );
            int idValue = query.record().indexOf( "value" );

            while ( query.next() )
            {
                int     nKey     = query.value( idKey ).toInt();
                QString strValue = query.value( idValue ).toString();

                list[nKey] = strValue;
            }
        }
        else
        {
            qWarning() << "getNotificationTypes(): query failed" << query.lastError();
        }
    }

    return list;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SqliteInterface::flushBufferFile()
{
    m_pBufferedQueryWriter->flush();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

SqliteInterface::SqliteDatabase::SqliteDatabase( SqliteInterface * pInterface )
{
    m_db = QSqlDatabase::database( "SqliteDatabase" );

    if ( !m_db.isValid() )
    {
        m_db = QSqlDatabase::addDatabase( "QSQLITE", "SqliteDatabase" );
    }

    m_db.setDatabaseName( pInterface->dbFile() );

    if ( m_bOpen
         || pInterface->m_pSqlQueryWorker->isRunning() )
    {
        qDebug() << "SqliteDatabase(): connection blocked, waiting to continue";

        while ( m_bOpen
                || pInterface->m_pSqlQueryWorker->isRunning() )
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

