///////////////////////////////////////////////////////////////////////////////
///
/// @file SqliteInterface.h
///
/// @brief Header file of class SqliteInterface.
///
/// @author Ultratronik GmbH
///         Dornierstr. 9
///         D-82205 Gilching
///         Germany
///         http://www.ultratronik.de
///
/// @author written by Gerd Esser, Forschung & Entwicklung, gesser@ultratronik.de
///
/// @date 21.03.2021
///
/// @copyright Copyright 2021 by Hemro International AG
///            Hemro International AG
///            Länggenstrasse 34
///            CH 8184 Bachenbülach
///            Switzerland
///            Homepage: www.hemrogroup.com
///
///////////////////////////////////////////////////////////////////////////////

#ifndef SqliteInterface_h
#define SqliteInterface_h

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#include <QObject>
#include <QSqlDatabase>
#include <QHash>
#include <QMap>

#include "EkxSqliteTypes.h"

class TimePeriod;
class SqlQueryWorker;
class StatisticsRecord;
class NotificationCenter;
class SettingsSerializer;
class BufferedWriter;
class BufferedQueryWriter;

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

class SqliteInterface
    : public QObject
{
    Q_OBJECT

public:

    using StringMap = QMap<int, QString>;

private:

    class SqliteDatabase
    {
        friend class SqliteInterface;

public:

        SqliteDatabase( SqliteInterface * const pInterface );
        ~SqliteDatabase();

        QSqlDatabase & operator()()
        {
            return m_db;
        };

private:

        QSqlDatabase m_db;
        bool         m_bOpen { false };

    };

public:

    explicit SqliteInterface( QObject * pParent = nullptr );
    ~SqliteInterface() override;

    void create( NotificationCenter * pNotificationCenter,
                 SettingsSerializer * pSettingsSerializer );

    QString dbFile() const;

    bool startQueryStatisticsResult( TimePeriod & timePeriod );

    void abortQueryStatistics();

    const QList<StatisticsRecord> & statisticsResult() const;

    bool changeRecipe( const QString & strUuid,
                       const QString & strName,
                       const int       nPortaFilterIndex,
                       const int       nRecipeModeId );

    bool addGrind( const QString & strUuid,
                   const quint32   u32GrindTime100ths );

    Q_INVOKABLE bool addEvent( const int       nEventId,
                               const QString & strContext = "" );

    Q_INVOKABLE bool addEvent( const EkxSqliteTypes::SqliteEventTypes       nEventId,
                               const QString &                              strRecipeUuid,
                               const EkxSqliteTypes::SqliteNotificationType nNotificationTypeId,
                               const QString &                              strContext );

    Q_INVOKABLE bool raiseNotification( const EkxSqliteTypes::SqliteNotificationType nNotificationTypeId,
                                        const QString &                              strContext );

    Q_INVOKABLE bool removeNotification( const EkxSqliteTypes::SqliteNotificationType nNotificationTypeId,
                                         const QString &                              strContext );

    bool addDeltaToCounter( const EkxSqliteTypes::SqliteCounters nCounterId,
                            const quint32                        u32AddValue );

    bool resetCounter( const EkxSqliteTypes::SqliteCounters nCounterId );

    bool queryCounter( const EkxSqliteTypes::SqliteCounters nCounterId,
                       quint32 &                            u32Value );

    void updateRecipeMap();

    bool executeSqlFile( const QString & strSqlFile );

    bool executeSqlString( const QString & strSql );

    const StringMap getRecipeNames();

    const StringMap getRecipeModes();

    const StringMap getEventTypes();

    const StringMap getNotificationTypes();

    void flushBufferFile();

signals:

    void queryFinished();

    void queryAborted();

public slots:

    void incrementSystemOnTimeCounterByMinute();

    void incrementStandbyTimeCounterByMinute();

private slots:

    void processQueryFinished();

    void processQueryAborted();

private:

    bool initDatabase();

    bool migrateDatabaseScheme();

    bool doDatabaseUpdate( const int nFromDbVersion );

private:

    NotificationCenter *  m_pNotificationCenter { nullptr };

    SettingsSerializer *  m_pSettingsSerializer { nullptr };

    QSqlDatabase          m_db;

    QHash<QString, int>   m_mapRecipeIds;

    SqlQueryWorker *      m_pSqlQueryWorker { nullptr };

    BufferedQueryWriter * m_pBufferedQueryWriter { nullptr };

};

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#endif // SqliteInterface_h
