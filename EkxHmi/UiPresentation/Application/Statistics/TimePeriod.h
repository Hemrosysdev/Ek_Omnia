///////////////////////////////////////////////////////////////////////////////
///
/// @file TimePeriod.h
///
/// @brief Header file of class TimePeriod.
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

#ifndef TimePeriod_h
#define TimePeriod_h

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#include <QObject>

#include <QDate>

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

class TimePeriod : public QObject
{
    Q_OBJECT

public:

    enum TimePeriodType
    {
        TimePeriodToday = 0,
        TimePeriodYesterday,
        TimePeriodCurrentWeek,
        TimePeriodCurrentMonth,
        TimePeriodCurrentYear,
        TimePeriodLastWeek,
        TimePeriodLastMonth,
        TimePeriodLastYear,
        TimePeriodEnumEnd,
    };

public:

    explicit TimePeriod( const TimePeriodType nTimePeriodType );
    virtual ~TimePeriod() override;

    TimePeriodType timePeriodType() const;

    Q_INVOKABLE QString name() const;

    Q_INVOKABLE QString shortcut() const;

    void update( const bool bStartOfWeekIsMonday );

    QDate  fromDate() const;

    QDate  toDate();

signals:

private:

    TimePeriodType  m_nTimePeriodType { TimePeriodToday };

    QDate           m_dateFrom;

    QDate           m_dateTo;

};

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#endif // TimePeriod_h
