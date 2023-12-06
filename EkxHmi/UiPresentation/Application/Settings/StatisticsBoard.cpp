///////////////////////////////////////////////////////////////////////////////
///
/// @file StatisticsBoard.cpp
///
/// @brief Implementation file of class StatisticsBoard
///
/// @author Ultratronik GmbH
///         Dornierstr. 9
///         D-82205 Gilching
///         Germany
///         http://www.ultratronik.de
///
/// @author written by Xaver Pilz, Forschung & Entwicklung, xpilz@ultratronik.de
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

#include "StatisticsBoard.h"

#include <QtQml> // for: qmlRegisterUncreatableType
#include <QDebug>

#include "StatisticsRecord.h"
#include "EkxGlobals.h"

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

int StatisticsBoard::statisticsBoardResult_typeId  = qmlRegisterUncreatableType<StatisticsBoardResult>( "Statistics",
                                                                                                        1,
                                                                                                        0,
                                                                                                        "StatisticsBoardResult",
                                                                                                        "StatisticsBoardResult could not be registered" );

int StatisticsBoard::radioButtonMenuItem_typeId  = qmlRegisterUncreatableType<RadioButtonMenuItem>( "Statistics",
                                                                                                    1,
                                                                                                    0,
                                                                                                    "RadioButtonMenuItem",
                                                                                                    "RadioButtonMenuItem could not be registered" );

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

StatisticsBoard::StatisticsBoard( QQmlEngine* engine,
                                  SqliteInterface* sqlInterface,
                                  QObject *parent)
    : QObject(parent)
    , m_sqliteInterface(sqlInterface)
    , m_discUsageMaxHours(120)
{
    engine->rootContext()->setContextProperty( "statistics" , this );

    connect(m_sqliteInterface, &SqliteInterface::queryAborted, this, &StatisticsBoard::processQueryAbort);
    connect(m_sqliteInterface, &SqliteInterface::queryFinished,this, &StatisticsBoard::processQueryFinish);

    // TODO: m_periodChoicesSetting direkt durch Liste von TimePeriod() ersetzen

    // Define set of choices and their order:
    m_periodChoicesSetting.append(TimePeriod::TimePeriodToday);
    m_periodChoicesSetting.append(TimePeriod::TimePeriodYesterday);
    m_periodChoicesSetting.append(TimePeriod::TimePeriodCurrentWeek);
    m_periodChoicesSetting.append(TimePeriod::TimePeriodLastWeek);
    m_periodChoicesSetting.append(TimePeriod::TimePeriodCurrentMonth);
    m_periodChoicesSetting.append(TimePeriod::TimePeriodLastMonth);
    m_periodChoicesSetting.append(TimePeriod::TimePeriodCurrentYear);
    m_periodChoicesSetting.append(TimePeriod::TimePeriodLastYear);
    m_periodChoicesIndex = 2;

    // Generate RadioButton choices from defintion:
    for(int i=0; i<m_periodChoicesSetting.count(); i++ )
    {
        TimePeriod timePeriod(m_periodChoicesSetting.at(i));

        RadioButtonMenuItem *radioButtonItem = new RadioButtonMenuItem(timePeriod.name(), this);
        radioButtonItem->setKey(timePeriod.shortcut());

        m_periodChoices.append(radioButtonItem);
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void StatisticsBoard::queryResults()
{
    if ( m_queryIsOngoing )
    {
        m_sqliteInterface->abortQueryStatistics();
    }

    TimePeriod timePeriod( m_periodChoicesSetting[m_periodChoicesIndex] );

    m_queryIsOngoing = true;
    prepareQueryResults();

    setErrorText( "" );

    // call sqLite Interface object...
    if ( !m_sqliteInterface->startQueryStatisticsResult( timePeriod ) )
    {
        m_queryIsOngoing = false;
        setErrorText( "Error during query!" );
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

QQmlListProperty<StatisticsBoardResult> StatisticsBoard::qlpResults()
{
    return( QQmlListProperty<StatisticsBoardResult>(this, this, &cb_namesCount, &cb_namesAt) );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

QQmlListProperty<RadioButtonMenuItem> StatisticsBoard::qlpPeriodChoices()
{
    return( QQmlListProperty<RadioButtonMenuItem>(this, this, &cb_periodChoicesCount, &cb_periodChoicesAt) );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

int StatisticsBoard::periodChoicesIndex()
{
    return(m_periodChoicesIndex);
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void StatisticsBoard::setPeriodChoicesIndex(int index)
{
    if(m_periodChoicesIndex!=index)
    {
        m_periodChoicesIndex = index;
        emit periodChoicesIndexChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void StatisticsBoard::processQueryAbort()
{
    m_queryIsOngoing = false;
    prepareQueryResults();
    setErrorText( "Query aborted!" );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void StatisticsBoard::processQueryFinish()
{
    m_queryIsOngoing = false;
    prepareQueryResults();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void StatisticsBoard::setDiskUsageMaxHours( int maxHours )
{
    m_discUsageMaxHours = maxHours;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void StatisticsBoard::prepareQueryResults()
{
    // delete old result entries:
    while ( m_results.size() )
    {
        m_results.takeFirst()->deleteLater();
    }

    // rebuild complete list
    addSummary( "COUNTERS", "LIFETIME" );

    int index;
    index = findResult( "COUNTERS", "Frog Usage" );
    if ( index > -1 )
    {
        int nHours = ( static_cast<int>( ceil( m_sqliteInterface->statisticsResult()[index].grindTime100ths()
                                               / 100.0 / 60 / 60 ) ) );
        int nRestHours = nHours - m_discUsageMaxHours;

        StatisticsBoardResult * result = addResult( "LIFETIME",
                                                    m_sqliteInterface->statisticsResult()[index].name(),
                                                    QString( "%1h" ).arg( nHours )
                                                    + "\n"
                                                    + QString().sprintf( "%+dh", nRestHours ),
                                                    0 );

        nHours = std::min( m_discUsageMaxHours, nHours );

        result->setHasBar( true );
        result->setBarCriticalPercentage( 0.8 );
        result->setBarCurrentPercentage( static_cast<double>( nHours ) / m_discUsageMaxHours );
    }
    else
    {
        StatisticsBoardResult * result = addResult( "LIFETIME",
                                                    "Frog Usage",
                                                    "",
                                                    0 );
        result->setHasBar( true );
        result->setBarCriticalPercentage( 0.8 );
        result->setBarCurrentPercentage( 0.0 );
    }

    TimePeriod timePeriod( m_periodChoicesSetting[m_periodChoicesIndex] );

    addSummary( "*", "SUMMARY" );
    addSummary( "CLASSIC", "PET MODE" );
    addSummary( "TIME", "DIVER MODE" );

    for ( int i = 0; i < m_sqliteInterface->statisticsResult().size(); i++ )
    {
        const StatisticsRecord & record = m_sqliteInterface->statisticsResult()[i];

        if ( record.modeName() == "TIME" )
        {
            addResult( "TIME MODE",
                       m_sqliteInterface->statisticsResult()[i].name(),
                       formatShots( m_sqliteInterface->statisticsResult()[i].shotsNum() )
                       + "\n"
                       + formatTime( m_sqliteInterface->statisticsResult()[i].grindTime100ths() / 100 ),
                       m_sqliteInterface->statisticsResult()[i].portaFilterIndex() );
        }
    }

    addSummary( "LIBRARY", "SPACE MODE" );

    for ( int i = 0; i < m_sqliteInterface->statisticsResult().size(); i++ )
    {
        const StatisticsRecord & record = m_sqliteInterface->statisticsResult()[i];

        if ( record.modeName() == "LIBRARY" )
        {
            addResult( "LIBRARY MODE",
                       m_sqliteInterface->statisticsResult()[i].name(),
                       formatShots( m_sqliteInterface->statisticsResult()[i].shotsNum() )
                       + "\n"
                       + formatTime( m_sqliteInterface->statisticsResult()[i].grindTime100ths() / 100 ),
                       m_sqliteInterface->statisticsResult()[i].portaFilterIndex() );
        }
    }

    // add a final fake entry as spacer to enable scrolling out of bottom shadow and above "back" button
    addResult( "LIBRARY MODE",
               "",
               "",
               0 );

    emit resultsChanged();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

QString StatisticsBoard::formatShots( const quint32 u32ShotNum ) const
{
    QString value = QString( "%1\u00D7" ).arg( u32ShotNum ); // ( \\u00D7 = cross product symbol)

    return value;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

QString StatisticsBoard::formatTime( const quint32 u32Seconds ) const
{
    QString value;

    if ( u32Seconds / 3600 > 0 )
    {
        value += QString( " %1h" ).arg( u32Seconds / 3600 );
    }

    if ( u32Seconds / 60 > 0 )
    {
        value += QString( " %1m" ).arg( ( u32Seconds % 3600 ) / 60 );
    }

    value += QString( " %1s" ).arg( u32Seconds % 60 );

    return value;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void StatisticsBoard::addSummary( const QString & modeName,
                                  const QString & sectionTitle )
{
    quint32 shotsNum = 0;
    quint32 grindTime100ths = 0;

    for ( int i = 0; i < m_sqliteInterface->statisticsResult().size(); i++ )
    {
        const StatisticsRecord & record = m_sqliteInterface->statisticsResult()[i];

        if ( ( modeName == record.modeName()
               || ( modeName == "*" && record.modeName() != "COUNTERS" ) )
             && record.name() != "Frog Usage" )
        {
            shotsNum += m_sqliteInterface->statisticsResult()[i].shotsNum();
            grindTime100ths += m_sqliteInterface->statisticsResult()[i].grindTime100ths();
        }
    }

    addResult( sectionTitle,
               "Hunters",
               formatShots( shotsNum ),
               0 );
    addResult( sectionTitle,
               "Smile Time",
               formatTime( grindTime100ths / 100 ),
               0 );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

StatisticsBoardResult * StatisticsBoard::addResult( const QString & section,
                                                    const QString & name,
                                                    const QString & value,
                                                    const int portaFilterIndex )
{
    StatisticsBoardResult * result = new StatisticsBoardResult( name,
                                                                value,
                                                                section,
                                                                portaFilterIndex,
                                                                m_queryIsOngoing,
                                                                this );
    m_results.append( result );

    return result;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

int StatisticsBoard::findResult( const QString & section,
                                 const QString & key ) const
{
    int index = -1;

    for ( int i = 0; i < m_sqliteInterface->statisticsResult().size(); i++ )
    {
        const StatisticsRecord & record = m_sqliteInterface->statisticsResult()[i];

        if ( record.modeName() == section
             && record.name() == key )
        {
            index = i;
            break;
        }
    }

    return index;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

// static functions:


int StatisticsBoard::cb_namesCount(QQmlListProperty<StatisticsBoardResult> *qlp)
{
    StatisticsBoard* statBoard = reinterpret_cast<StatisticsBoard*>(qlp->data);
    return( statBoard->m_results.count() );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

StatisticsBoardResult *StatisticsBoard::cb_namesAt(QQmlListProperty<StatisticsBoardResult> *qlp, int idx)
{
    StatisticsBoard* statBoard = reinterpret_cast<StatisticsBoard*>(qlp->data);
    return( statBoard->m_results.at(idx) );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

int StatisticsBoard::cb_periodChoicesCount(QQmlListProperty<RadioButtonMenuItem> *qlp)
{
    StatisticsBoard* statBoard = reinterpret_cast<StatisticsBoard*>(qlp->data);
    return( statBoard->m_periodChoices.count() );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

RadioButtonMenuItem *StatisticsBoard::cb_periodChoicesAt(QQmlListProperty<RadioButtonMenuItem> *qlp, int idx)
{
    StatisticsBoard* statBoard = reinterpret_cast<StatisticsBoard*>(qlp->data);
    return( statBoard->m_periodChoices.at(idx) );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

const QString &StatisticsBoard::errorText() const
{
    return m_errorText;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void StatisticsBoard::setErrorText(const QString &error)
{
    if ( m_errorText != error )
    {
        m_errorText = error;
        emit errorTextChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

