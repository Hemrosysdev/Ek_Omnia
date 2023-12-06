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

#ifndef STATISTICS_BOARD_RESULT_H
#define STATISTICS_BOARD_RESULT_H

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#include <QObject>

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

class StatisticsBoardResult : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString key                   READ key                   NOTIFY keyChanged)
    Q_PROPERTY(QString value                 READ value                 NOTIFY valueChanged)
    Q_PROPERTY(QString sectionTitle          READ sectionTitle          NOTIFY sectionTitleChanged)
    Q_PROPERTY(int     iconIndex             READ iconIndex             NOTIFY iconIndexChanged)
    Q_PROPERTY(bool    isLoading             READ isLoading             NOTIFY isLoadingChanged)
    Q_PROPERTY(bool    hasBar                READ hasBar                NOTIFY hasBarChanged)
    Q_PROPERTY(double  barCurrentPercentage  READ barCurrentPercentage  NOTIFY barCurrentPercentageChanged)
    Q_PROPERTY(double  barCriticalPercentage READ barCriticalPercentage NOTIFY barCriticalPercentageChanged)


public:

    StatisticsBoardResult();
    StatisticsBoardResult( const QString & key,
                           const QString & value,
                           const QString & sectionTitle,
                           const int portaFilterIndex,
                           const bool isLoading,
                           QObject *parent = nullptr );

    QString key();
    void setKey(const QString &key);

    QString value() const;
    void setValue(const QString &value);

    QString sectionTitle() const;
    void setSectionTitle(const QString &sectionTitle);

    int iconIndex() const;
    void setIconIndex(const int iconIndex);

    bool isLoading() const;
    void setIsLoading(bool isLoading);

    bool hasBar() const;
    void setHasBar(bool hasBar);

    double barCurrentPercentage() const;
    void setBarCurrentPercentage(double barCurrentPercentage);

    double barCriticalPercentage() const;
    void setBarCriticalPercentage(double barCriticalPercentage);

signals:
    void keyChanged();
    void valueChanged();
    void sectionTitleChanged();
    void iconIndexChanged();
    void isLoadingChanged();
    void hasBarChanged();
    void barCurrentPercentageChanged();
    void barCriticalPercentageChanged();

private:

    QString m_key;
    QString m_value;
    QString m_sectionTitle;
    int     m_iconIndex;
    bool    m_isLoading;
    bool    m_hasBar;
    double  m_barCurrentPercentage;
    double  m_barCriticalPercentage;

};

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#endif // STATISTICS_BOARD_RESULT_H
