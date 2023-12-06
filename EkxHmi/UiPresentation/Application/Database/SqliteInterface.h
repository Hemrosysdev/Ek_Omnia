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

#include "StatisticsRecord.h"
#include "EkxSqliteTypes.h"

class TimePeriod;
class MainStatemachine;
class QQmlEngine;
class SqlQueryWorker;

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

class SqliteInterface
        : public QObject
{
    Q_OBJECT

private:

    class SqliteDatabase
    {
        friend class SqliteInterface;

    public:

          SqliteDatabase( SqliteInterface * pInterface );
          ~SqliteDatabase();

          QSqlDatabase & operator()() { return m_db; };

    private:

          QSqlDatabase  m_db;
          bool          m_bOpen { false };

    };

public:

    explicit SqliteInterface( QQmlEngine * pEngine,
                              MainStatemachine * pMainStatmachine );
    virtual ~SqliteInterface() override;

    QString dbFile() const;

    bool startQueryStatisticsResult( TimePeriod & timePeriod );
    void abortQueryStatistics();

    const QList<StatisticsRecord> & statisticsResult() const;

    bool changeRecipe( const QString & strUuid,
                       const QString & strName,
                       const int nPortaFilterIndex,
                       const int nRecipeModeId );

    bool addGrind( const QString & strUuid,
                   const quint32 u32GrindTime100ths );

    Q_INVOKABLE bool addEvent( const int nEventId,
                               const QString & strContext = "" );

    Q_INVOKABLE bool addEvent( const EkxSqliteTypes::SqliteEventTypes nEventId,
                               const QString & strRecipeUuid,
                               const EkxSqliteTypes::SqliteErrorTypes nErrorId,
                               const QString & strContext );

    bool addDeltaToCounter( const EkxSqliteTypes::SqliteCounters nCounterId,
                            const quint32 u32AddValue );
    bool resetCounter( const EkxSqliteTypes::SqliteCounters nCounterId );
    bool queryCounter( const EkxSqliteTypes::SqliteCounters nCounterId,
                       quint32 &u32Value);

    void updateRecipeMap();

signals:

    void queryFinished();
    void queryAborted();

private slots:

    void processQueryFinished();
    void processQueryAborted();

private:

    bool initDatabase();
    bool migrateDatabaseScheme();

    bool executeSqlFile( const QString & strSqlFile );
    bool executeSqlString( const QString & strSql );

private:

    MainStatemachine * m_pMainStatmachine { nullptr };

    QSqlDatabase m_db;

    QHash<QString,int>    m_mapRecipeIds;

    static int m_nEkxSqliteTypeId;

    SqlQueryWorker * m_pSqlQueryWorker { nullptr };

};

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#endif // SqliteInterface_h
