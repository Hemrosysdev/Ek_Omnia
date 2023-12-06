///////////////////////////////////////////////////////////////////////////////
///
/// @file SqlQueryWorker.h
///
/// @brief Header file of class SqlQueryWorker.
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

#ifndef SqlQueryWorker_h
#define SqlQueryWorker_h

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#include <QThread>
#include <QSqlQuery>

#include "StatisticsRecord.h"

class QThread;
class QSqlDatabase;
class SqliteInterface;

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

class SqlQueryWorker
        : public QThread
{
    Q_OBJECT

public:

    explicit SqlQueryWorker(  SqliteInterface * pSqlite );
    virtual ~SqlQueryWorker() override;

    bool execThreaded( const QString & strSql );

    void abort();

    QSqlQuery & query();

    const QList<StatisticsRecord> & statisticsResult() const;

private:

signals:

    void queryFinished();
    void queryAborted();
    void queryStarted();

private slots:

    void execInWorker();

    void buildResultList();

    void addCounters();

private:

    SqliteInterface * m_pSqlite { nullptr };

    QThread * m_pWorker { nullptr };

    QSqlQuery  m_theQuery;

    QString m_strSql;

    QList<StatisticsRecord>   m_statisticsResult;

};

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#endif // SqlQueryWorker_h
