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
//#include "EkxGlobals.h"
#include "StatisticsBoardResult.h"
#include "RadioButtonMenuItem.h"
#include "SqliteInterface.h"

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

int StatisticsBoard::statisticsBoardResult_typeId = qmlRegisterUncreatableType<StatisticsBoardResult>( "Statistics",
                                                                                                       1,
                                                                                                       0,
                                                                                                       "StatisticsBoardResult",
                                                                                                       "StatisticsBoardResult could not be registered" );

int StatisticsBoard::radioButtonMenuItem_typeId = qmlRegisterUncreatableType<RadioButtonMenuItem>( "Statistics",
                                                                                                   1,
                                                                                                   0,
                                                                                                   "RadioButtonMenuItem",
                                                                                                   "RadioButtonMenuItem could not be registered" );

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

StatisticsBoard::StatisticsBoard( QObject * parent )
    : QObject( parent )
{

    // TODO: m_periodChoicesSetting direkt durch Liste von TimePeriod() ersetzen

    // Define set of choices and their order:
    m_periodChoicesSetting.append( TimePeriod::TimePeriodToday );
    //m_periodChoicesSetting.append(TimePeriod::TimePeriodYesterday);
    m_periodChoicesSetting.append( TimePeriod::TimePeriodCurrentWeek );
    //m_periodChoicesSetting.append(TimePeriod::TimePeriodLastWeek);
    m_periodChoicesSetting.append( TimePeriod::TimePeriodCurrentMonth );
    //m_periodChoicesSetting.append(TimePeriod::TimePeriodLastMonth);
    m_periodChoicesSetting.append( TimePeriod::TimePeriodCurrentYear );
    //m_periodChoicesSetting.append(TimePeriod::TimePeriodLastYear);
    m_nPeriodChoicesIndex = 2;

    // Generate RadioButton choices from defintion:
    for ( int i=0; i < m_periodChoicesSetting.count(); i++ )
    {
        TimePeriod timePeriod( m_periodChoicesSetting.at( i ) );

        RadioButtonMenuItem * radioButtonItem = new RadioButtonMenuItem( timePeriod.name(), this );
        radioButtonItem->setKey( timePeriod.shortcut() );

        m_periodChoices.append( radioButtonItem );
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

StatisticsBoard::~StatisticsBoard()
{

}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void StatisticsBoard::create( SqliteInterface * pSqliteInterface )
{
    m_pSqliteInterface = pSqliteInterface;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void StatisticsBoard::queryResults()
{
    if ( m_bQueryIsOngoing )
    {
        m_pSqliteInterface->abortQueryStatistics();
    }

    TimePeriod timePeriod( m_periodChoicesSetting[m_nPeriodChoicesIndex] );

    m_bQueryIsOngoing = true;
    prepareQueryResults();

    setErrorText( "" );

    // call sqLite Interface object...
    if ( !m_pSqliteInterface->startQueryStatisticsResult( timePeriod ) )
    {
        m_bQueryIsOngoing = false;
        setErrorText( "Error during query!" );
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

QQmlListProperty<StatisticsBoardResult> StatisticsBoard::qlpResults()
{
    return QQmlListProperty<StatisticsBoardResult>( this, this, &cb_namesCount, &cb_namesAt );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

QQmlListProperty<RadioButtonMenuItem> StatisticsBoard::qlpPeriodChoices()
{
    return QQmlListProperty<RadioButtonMenuItem>( this, this, &cb_periodChoicesCount, &cb_periodChoicesAt );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

int StatisticsBoard::periodChoicesIndex()
{
    return m_nPeriodChoicesIndex;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void StatisticsBoard::setPeriodChoicesIndex( int index )
{
    if ( m_nPeriodChoicesIndex != index )
    {
        m_nPeriodChoicesIndex = index;
        emit periodChoicesIndexChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void StatisticsBoard::processQueryAbort()
{
    m_bQueryIsOngoing = false;
    prepareQueryResults();
    setErrorText( "Query aborted!" );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void StatisticsBoard::processQueryFinish()
{
    m_bQueryIsOngoing = false;
    prepareQueryResults();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void StatisticsBoard::setDiskUsageMaxHours( int maxHours )
{
    m_nDiscUsageMaxHours = maxHours;
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
    index = findResult( "COUNTERS", "Disc Usage" );
    if ( index > -1 )
    {
        int nHours = ( static_cast<int>( ceil( m_pSqliteInterface->statisticsResult()[index].grindTime100ths()
                                               / 100.0 / 60 / 60 ) ) );
        int nRestHours = nHours - m_nDiscUsageMaxHours;

        StatisticsBoardResult * result = addResult( "LIFETIME",
                                                    m_pSqliteInterface->statisticsResult()[index].name(),
                                                    QString( "%1h" ).arg( nHours )
                                                    + "\n"
                                                    + QString().sprintf( "%+dh", nRestHours ),
                                                    0 );

        nHours = std::min( m_nDiscUsageMaxHours, nHours );

        result->setHasBar( true );
        result->setBarCriticalPercentage( 0.8 );
        result->setBarCurrentPercentage( static_cast<double>( nHours ) / m_nDiscUsageMaxHours );
    }
    else
    {
        StatisticsBoardResult * result = addResult( "LIFETIME",
                                                    "Disc Usage",
                                                    "",
                                                    0 );
        result->setHasBar( true );
        result->setBarCriticalPercentage( 0.8 );
        result->setBarCurrentPercentage( 0.0 );
    }

    TimePeriod timePeriod( m_periodChoicesSetting[m_nPeriodChoicesIndex] );

    addSummary( "*", "SUMMARY" );
    addSummary( "CLASSIC", "CLASSIC MODE" );
    addSummary( "TIME", "TIME MODE" );

    for ( int i = 0; i < m_pSqliteInterface->statisticsResult().size(); i++ )
    {
        const StatisticsRecord & record = m_pSqliteInterface->statisticsResult()[i];

        if ( record.modeName() == "TIME" )
        {
            addResult( "TIME MODE",
                       m_pSqliteInterface->statisticsResult()[i].name(),
                       formatShots( m_pSqliteInterface->statisticsResult()[i].shotsNum() )
                       + "\n"
                       + formatTime( m_pSqliteInterface->statisticsResult()[i].grindTime100ths() / 100 ),
                       m_pSqliteInterface->statisticsResult()[i].portaFilterIndex() );
        }
    }

    addSummary( "LIBRARY", "LIBRARY MODE" );

    for ( int i = 0; i < m_pSqliteInterface->statisticsResult().size(); i++ )
    {
        const StatisticsRecord & record = m_pSqliteInterface->statisticsResult()[i];

        if ( record.modeName() == "LIBRARY" )
        {
            addResult( "LIBRARY MODE",
                       m_pSqliteInterface->statisticsResult()[i].name(),
                       formatShots( m_pSqliteInterface->statisticsResult()[i].shotsNum() )
                       + "\n"
                       + formatTime( m_pSqliteInterface->statisticsResult()[i].grindTime100ths() / 100 ),
                       m_pSqliteInterface->statisticsResult()[i].portaFilterIndex() );
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
    quint32 shotsNum        = 0;
    quint32 grindTime100ths = 0;

    for ( int i = 0; i < m_pSqliteInterface->statisticsResult().size(); i++ )
    {
        const StatisticsRecord & record = m_pSqliteInterface->statisticsResult()[i];

        if ( ( modeName == record.modeName()
               || ( modeName == "*"
                    && record.modeName() != "COUNTERS" ) )
             && record.name() != "Disc Usage" )
        {
            shotsNum        += m_pSqliteInterface->statisticsResult()[i].shotsNum();
            grindTime100ths += m_pSqliteInterface->statisticsResult()[i].grindTime100ths();
        }
    }

    addResult( sectionTitle,
               "Shots",
               formatShots( shotsNum ),
               0 );
    addResult( sectionTitle,
               "Grind Time",
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
                                                    const int       portaFilterIndex )
{
    StatisticsBoardResult * result = new StatisticsBoardResult( name,
                                                                value,
                                                                section,
                                                                portaFilterIndex,
                                                                m_bQueryIsOngoing,
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

    for ( int i = 0; i < m_pSqliteInterface->statisticsResult().size(); i++ )
    {
        const StatisticsRecord & record = m_pSqliteInterface->statisticsResult()[i];

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


int StatisticsBoard::cb_namesCount( QQmlListProperty<StatisticsBoardResult> * qlp )
{
    StatisticsBoard * statBoard = reinterpret_cast<StatisticsBoard *>( qlp->data );
    return statBoard->m_results.count();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

StatisticsBoardResult * StatisticsBoard::cb_namesAt( QQmlListProperty<StatisticsBoardResult> * qlp,
                                                     int                                       idx )
{
    StatisticsBoard * statBoard = reinterpret_cast<StatisticsBoard *>( qlp->data );
    return statBoard->m_results.at( idx );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

int StatisticsBoard::cb_periodChoicesCount( QQmlListProperty<RadioButtonMenuItem> * qlp )
{
    StatisticsBoard * statBoard = reinterpret_cast<StatisticsBoard *>( qlp->data );
    return statBoard->m_periodChoices.count();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

RadioButtonMenuItem * StatisticsBoard::cb_periodChoicesAt( QQmlListProperty<RadioButtonMenuItem> * qlp,
                                                           int                                     idx )
{
    StatisticsBoard * statBoard = reinterpret_cast<StatisticsBoard *>( qlp->data );
    return statBoard->m_periodChoices.at( idx );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

const QString & StatisticsBoard::errorText() const
{
    return m_strErrorText;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void StatisticsBoard::setErrorText( const QString & error )
{
    if ( m_strErrorText != error )
    {
        m_strErrorText = error;
        emit errorTextChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

