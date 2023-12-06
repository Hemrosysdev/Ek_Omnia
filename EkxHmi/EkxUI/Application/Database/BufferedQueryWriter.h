///////////////////////////////////////////////////////////////////////////////
///
/// @file BufferedQueryWriter.h
///
/// @brief Header file of class BufferedQueryWriter.
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

#ifndef BufferedQueryWriter_h
#define BufferedQueryWriter_h

#include <QObject>
#include <QTimer>

class SqliteInterface;

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

class BufferedQueryWriter : public QObject
{
    Q_OBJECT

public:

    BufferedQueryWriter();

    ~BufferedQueryWriter() override;

    void create( const QString &   strBufferFile,
                 SqliteInterface * pSqliteInterface,
                 const int         nFlushPeriodSec );

    const QString & bufferFile() const;

    bool addSql( const QString & strSql );

    void flush();

    void prepareQuery( const QString & strQuery );

    void bindValue( const QString &  strPlaceholder,
                    const QVariant & value );

    bool addQuery();

private:

    bool writeBufferFile();

    void readBufferFile();

    void processTimeoutFlush();

private:

    QString           m_strBufferFile;

    SqliteInterface * m_pSqliteInterface { nullptr };

    int               m_nFlushPeriodSec { 0 };

    QTimer            m_timerFlush;

    QString           m_strQuery;

};

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#endif // BufferedQueryWriter_h
