///////////////////////////////////////////////////////////////////////////////
///
/// @file TimePeriod.cpp
///
/// @brief main application entry point of TimePeriod.
///
/// @author Ultratronik GmbH
///         Dornierstr. 9
///         D-82205 Gilching
///         Germany
///         http://www.ultratronik.de
///
/// @author written by Gerd Esser, Forschung & Entwicklung, gesser@ultratronik.de
///
/// @date 22.03.2021
///
/// @copyright Copyright 2021 by Hemro International AG
///            Hemro International AG
///            Länggenstrasse 34
///            CH 8184 Bachenbülach
///            Switzerland
///            Homepage: www.hemrogroup.com
///
///////////////////////////////////////////////////////////////////////////////

#include "TimePeriod.h"
#include <QDebug>

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

TimePeriod::TimePeriod( const TimePeriodType nTimePeriodType )
    : m_nTimePeriodType( nTimePeriodType )
{

}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

TimePeriod::~TimePeriod()
{

}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

TimePeriod::TimePeriodType TimePeriod::timePeriodType() const
{
    return m_nTimePeriodType;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

QString TimePeriod::name() const
{
    QString strName = "Unknown";

    switch ( m_nTimePeriodType )
    {
        case TimePeriodToday:
        {
            strName = "Today";
        }
        break;

        case TimePeriodYesterday:
        {
            strName = "Yesterday";
        }
        break;

        case TimePeriodCurrentWeek:
        {
            strName = "Current Week";
        }
        break;

        case TimePeriodCurrentMonth:
        {
            strName = "Current Month";
        }
        break;

        case TimePeriodCurrentYear:
        {
            strName = "Current Year";
        }
        break;

        case TimePeriodLastWeek:
        {
            strName = "Last Week";
        }
        break;

        case TimePeriodLastMonth:
        {
            strName = "Last Month";
        }
        break;

        case TimePeriodLastYear:
        {
            strName = "Last Year";
        }
        break;

        case TimePeriodEnumEnd:
        {
            // nothing to do
        }
        break;
    }

    return strName;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

QString TimePeriod::shortcut() const
{
    QString strShortcut = "U";

    switch ( m_nTimePeriodType )
    {
        case TimePeriodToday:
        {
            strShortcut = "D";
        }
        break;

        case TimePeriodYesterday:
        {
            strShortcut = "D\u2212";
        }
        break;

        case TimePeriodCurrentWeek:
        {
            strShortcut = "W";
        }
        break;

        case TimePeriodCurrentMonth:
        {
            strShortcut = "M";
        }
        break;

        case TimePeriodCurrentYear:
        {
            strShortcut = "Y";
        }
        break;

        case TimePeriodLastWeek:
        {
            strShortcut = "W\u2212";
        }
        break;

        case TimePeriodLastMonth:
        {
            strShortcut = "M\u2212";
        }
        break;

        case TimePeriodLastYear:
        {
            strShortcut = "Y\u2212";
        }
        break;

        case TimePeriodEnumEnd:
        {
            // nothing to do
        }
        break;
    }

    return strShortcut;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void TimePeriod::update( const bool bStartOfWeekIsSunday )
{
    QDate  dateNow = QDate::currentDate();

    int nDayOfWeek = dateNow.dayOfWeek();
    if ( bStartOfWeekIsSunday )
    {
        nDayOfWeek = ( nDayOfWeek + 1 ) % 8;
    }

    switch ( m_nTimePeriodType )
    {
        default:
        case TimePeriodToday:
        {
            m_dateFrom = dateNow;
            m_dateTo   = dateNow;
        }
        break;

        case TimePeriodYesterday:
        {
            m_dateFrom = dateNow.addDays( -1 );
            m_dateTo   = m_dateFrom;
        }
        break;

        case TimePeriodCurrentWeek:
        {
            m_dateFrom = dateNow.addDays( -nDayOfWeek + 1 );
            m_dateTo   = dateNow;
        }
        break;

        case TimePeriodCurrentMonth:
        {
            m_dateFrom = dateNow.addDays( -dateNow.day() + 1 );
            m_dateTo   = dateNow;
        }
        break;

        case TimePeriodCurrentYear:
        {
            m_dateFrom = dateNow.addDays( -dateNow.dayOfYear() + 1 );
            m_dateTo   = dateNow;
        }
        break;

        case TimePeriodLastWeek:
        {
            m_dateTo   = dateNow.addDays( -nDayOfWeek );
            m_dateFrom = m_dateTo.addDays( -6 );
        }
        break;

        case TimePeriodLastMonth:
        {
            m_dateTo   = dateNow.addDays( -dateNow.day() );
            m_dateFrom = m_dateTo.addDays( -m_dateTo.day() + 1 );
        }
        break;

        case TimePeriodLastYear:
        {
            m_dateTo   = dateNow.addDays( -dateNow.dayOfYear() );
            m_dateFrom = m_dateTo.addDays( -m_dateTo.dayOfYear() + 1 );
        }
        break;
    }

    //    qInfo() << "dateFrom" << m_dateFrom.toString();
    //    qInfo() << "dateTo  " << m_dateTo.toString();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

QDate TimePeriod::fromDate() const
{
    return m_dateFrom;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

QDate TimePeriod::toDate()
{
    return m_dateTo;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////
