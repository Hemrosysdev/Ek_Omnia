///////////////////////////////////////////////////////////////////////////////
///
/// @file StatisticsBoard.h
///
/// @brief Header file of class StatisticsBoard.
///
/// @author Ultratronik GmbH
///         Dornierstr. 9
///         D-82205 Gilching
///         Germany
///         http://www.ultratronik.de
///
/// @author written by Xaver Pilz, Forschung & Entwicklung, xpilz@ultratronik.de
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

#ifndef STATISTICS_BOARD_H
#define STATISTICS_BOARD_H

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#include <QObject>
#include <QQmlListProperty>

#include "TimePeriod.h"

class SqliteInterface;
class RadioButtonMenuItem;
class StatisticsBoardResult;

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

class StatisticsBoard : public QObject
{
    Q_OBJECT

    Q_PROPERTY( QQmlListProperty<StatisticsBoardResult> results READ qlpResults NOTIFY resultsChanged )
    Q_PROPERTY( QQmlListProperty<RadioButtonMenuItem>   periodChoices READ qlpPeriodChoices NOTIFY periodChoicesChanged )
    Q_PROPERTY( int periodChoicesIndex READ periodChoicesIndex WRITE setPeriodChoicesIndex NOTIFY periodChoicesIndexChanged )
    Q_PROPERTY( QString errorText READ errorText NOTIFY errorTextChanged )

public:

    explicit StatisticsBoard( QObject * parent = nullptr );
    ~StatisticsBoard() override;

    void create( SqliteInterface * pSqliteInterface );

    Q_INVOKABLE void queryResults();

    void generateDummyResults();

    QQmlListProperty<StatisticsBoardResult> qlpResults();

    QQmlListProperty<RadioButtonMenuItem>   qlpPeriodChoices();

    int periodChoicesIndex();

    void setPeriodChoicesIndex( int index );

signals:

    void resultsChanged();

    void periodChoicesChanged();

    void periodChoicesIndexChanged();

    void errorTextChanged();

public slots:

    void processQueryAbort();

    void processQueryFinish();

    void setDiskUsageMaxHours( int maxHours );

private:

    void prepareQueryResults();

    int findResult( const QString & section,
                    const QString & key ) const;

    StatisticsBoardResult * addResult( const QString & section,
                                       const QString & name,
                                       const QString & value,
                                       const int       portaFilterIndex );

    void addSummary( const QString & modeName,
                     const QString & sectionTitle );

    QString formatShots( const quint32 u32ShotNum ) const;

    QString formatTime( const quint32 u32Seconds ) const;

    static int cb_namesCount( QQmlListProperty<StatisticsBoardResult> * qlp );

    static StatisticsBoardResult * cb_namesAt( QQmlListProperty<StatisticsBoardResult> * qlp,
                                               int                                       idx );

    static int cb_periodChoicesCount( QQmlListProperty<RadioButtonMenuItem> * qlp );

    static RadioButtonMenuItem * cb_periodChoicesAt( QQmlListProperty<RadioButtonMenuItem> * qlp,
                                                     int                                     idx );

    const QString & errorText() const;

    void setErrorText( const QString & error );

private:

    QString                           m_strErrorText;
    bool                              m_bChoicesMenuVisible { false };
    bool                              m_bQueryIsOngoing { false };
    SqliteInterface *                 m_pSqliteInterface { nullptr };

    QList<StatisticsBoardResult *>    m_results;
    QList<RadioButtonMenuItem *>      m_periodChoices;
    QList<TimePeriod::TimePeriodType> m_periodChoicesSetting;

    int                               m_nPeriodChoicesIndex { 0 };
    int                               m_nDiscUsageMaxHours { 120 };

    static int                        statisticsBoardResult_typeId;
    static int                        radioButtonMenuItem_typeId;
};

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#endif // STATISTICS_BOARD_H

