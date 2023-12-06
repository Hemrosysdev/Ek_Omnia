///////////////////////////////////////////////////////////////////////////////
///
/// @file AdcVoltageInDriver.h
///
/// @brief Header file of class AdcVoltageInDriver.
///
/// @author Ultratronik GmbH
///         Dornierstr. 9
///         D-82205 Gilching
///         Germany
///         http://www.ultratronik.de
///
/// @author written by Gerd Esser, Forschung & Entwicklung, gesser@ultratronik.de
///
/// @date 04.11.2022
///
/// @copyright Copyright 2022 by Hemro International AG
///            Hemro International AG
///            Länggenstrasse 34
///            CH 8184 Bachenbülach
///            Switzerland
///            Homepage: www.hemrogroup.com
///
///////////////////////////////////////////////////////////////////////////////

#ifndef AdcVoltageInDriver_h
#define AdcVoltageInDriver_h

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#include <QObject>
#include <QTimer>
#include <QSharedPointer>
#include <ArithmeticFilter.h>

namespace SystemIo
{
class SystemDeviceFile;

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

class AdcVoltageInDriver : public QObject
{
    Q_OBJECT

public:

    static constexpr int    c_nInvalidAdcRaw    = -999;
    static constexpr double c_dInvalidVoltageMv = -999999.0;

public:

    explicit AdcVoltageInDriver( QObject * pParent = nullptr );

    ~AdcVoltageInDriver() override;

    void create( const QString & strDriverName,
                 const QString & strSystemDeviceDir,
                 const QString & strAdcChannel,
                 const bool      bUseDma,
                 const int       nDmaFilterElementNum,
                 const int       nPollCycleMs,
                 const bool      bUpdateCyclic = false );

    const QString & systemDeviceDir() const;

    const QString & adcChannel() const;

    double voltageMv() const;

    bool isValid() const;

    void processDmaValue( const int nAdcValue );

public slots:

    void processPollingTimeout();

signals:

    void voltageChanged( const double dVoltageMv );

    void validChanged( const bool bValid );

private:

    void setVoltageMv( const double dVoltageMv );

    void setValid( const bool bValid );

private:

    QString                          m_strDriverName;

    QString                          m_strSystemDeviceDir;

    QString                          m_strAdcChannel;

    bool                             m_bUseDma { false };

    bool                             m_bUpdateCyclic { false };

    int                              m_nAdcRaw { c_nInvalidAdcRaw };

    int                              m_nAdcOffset { 0 };

    double                           m_dAdcScale { 0.0 };

    bool                             m_bSetupValid { false };

    bool                             m_bValid { false };

    double                           m_dVoltageMv { c_dInvalidVoltageMv };

    QSharedPointer<SystemDeviceFile> m_pSystemScale;

    QSharedPointer<SystemDeviceFile> m_pSystemOffset;

    QSharedPointer<SystemDeviceFile> m_pSystemRaw;

    QTimer                           m_timerPolling;

    UxEco::ArithmeticFilter<qint32>  m_dmaFilter;

    quint32                          m_u32DmaCounter { 0 };
};

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

} // namespace SystemIo

#endif // AdcVoltageInDriver_h
