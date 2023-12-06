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

#ifndef StatisticsRecord_h
#define StatisticsRecord_h

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#include <QObject>

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

class StatisticsRecord
        : public QObject
{
    Q_OBJECT

public:

    StatisticsRecord();
    StatisticsRecord( const StatisticsRecord & theRight );
    virtual ~StatisticsRecord() override;

    StatisticsRecord & operator=( const StatisticsRecord & theRight );

    Q_INVOKABLE int portaFilterIndex() const;
    void setPortaFilterIndex(int value);

    Q_INVOKABLE const QString & modeName() const;
    void setModeName(const QString &value);

    Q_INVOKABLE const QString & name() const;
    void setName(const QString &value);

    Q_INVOKABLE quint32 shotsNum() const;
    void setShotsNum(const quint32 &value);

    Q_INVOKABLE quint32 grindTime100ths() const;
    void setGrindTime100ths(const quint32 &value);

private:

    int      m_nPortaFilterIndex { 0 };

    QString  m_strModeName;

    QString  m_strName;

    quint32  m_u32ShotsNum { 0 };

    quint32  m_u32GrindTime100ths { 0 };


};

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#endif // StatisticsRecord_h
