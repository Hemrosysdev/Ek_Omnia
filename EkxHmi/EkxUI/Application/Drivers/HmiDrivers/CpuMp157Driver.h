///////////////////////////////////////////////////////////////////////////////
///
/// @file CpuMp157Driver.h
///
/// @brief Header file of class CpuMp157Driver.
///
/// @author Ultratronik GmbH
///         Dornierstr. 9
///         D-82205 Gilching
///         Germany
///         http://www.ultratronik.de
///
/// @author written by Gerd Esser, Forschung & Entwicklung, gesser@ultratronik.de
///
/// @date 22.11.2022
///
/// @copyright Copyright 2022 by Hemro International AG
///            Hemro International AG
///            Länggenstrasse 34
///            CH 8184 Bachenbülach
///            Switzerland
///            Homepage: www.hemrogroup.com
///
///////////////////////////////////////////////////////////////////////////////

#ifndef CpuMp157Driver_h
#define CpuMp157Driver_h

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#include "DeviceMemoryIo.h"
#include <QObject>
#include <QSharedPointer>

namespace SystemIo
{
class GpioOut;
class SystemDeviceFile;

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

class CpuMp157Driver : public QObject, public DeviceMemoryIo
{
    Q_OBJECT

private:

    static constexpr double c_dAdcConversionTimeTicks = 8.5;

public:

    CpuMp157Driver();
    ~CpuMp157Driver() override;

    void logParameters();

    void readAdcParameters();

    void sequenceStartAdcOnly();

    void sequenceStartGrind();

    void sequenceAdcReset();

    void sequenceTimerReset();

    void sequenceDisableTimer();

    void sequenceUpdateTimer( const quint32 u32ClockTicks );

    void setAdc1SampleTimeTicks( const double dTicks );

    double adc1SampleTimeTicks() const;

    void setAdc2SampleTimeTicks( const double dTicks );

    double adc2SampleTimeTicks() const;

    void setAdc1Oversampling( const quint32 u32Oversampling );

    quint32 adc1Oversampling() const;

    void setAdc2Oversampling( const quint32 u32Oversampling );

    quint32 adc2Oversampling() const;

    void setAdc1LeftShiftFactor( const double dFactor );

    double adc1LeftShiftFactor() const;

    void setAdc2LeftShiftFactor( const double dFactor );

    double adc2LeftShiftFactor() const;

    void setAdcClockHz( const double dClockHz );

    double adcClockHz() const;

    void setAdc1SamplingFreqHz( const double dAdcSamplingFreqHz );

    double adc1SamplingFreqHz() const;

    void setAdc2SamplingFreqHz( const double dAdcSamplingFreqHz );

    double adc2SamplingFreqHz() const;

    double adc1DeltaTimeSec() const;

    double adc2DeltaTimeSec() const;

    double adcConversionTimeTicks() const;

    void activateAdcSampleTimeTicks();

    static bool isDoubleChanged( const double dValue1,
                                 const double dValue2 );

    QString toJson() const;

public slots:

    void activateAdcOversampling();

signals:

    void logInfoDouble( const QString & strGroup,
                        const QString & strKey,
                        const double    dValue );

    void adcClockHzChanged();

    void adc1SampleTimeTicksChanged();

    void adc1OversamplingChanged();

    void adc1LeftShiftfactorChanged();

    void adc1SamplingFreqHzChanged();

    void adc1DeltaTimeSecChanged();

    void adc2SampleTimeTicksChanged();

    void adc2OversamplingChanged();

    void adc2LeftShiftfactorChanged();

    void adc2SamplingFreqHzChanged();

    void adc2DeltaTimeSecChanged();

private:

    bool                             m_bOnePulseDriver          { true };

    double                           m_dAdcClockHz              { 0.0 };

    quint32                          m_u32Adc1Oversampling      { 0 };
    double                           m_dAdc1SampleTimeTicks     { 0.0 };
    double                           m_dAdc1LeftShiftFactor     { 0.0 };
    double                           m_dAdc1SamplingFreqHz      { 0.0 };
    double                           m_dAdc1DeltaTimeSec        { 0.0 };

    quint32                          m_u32Adc2Oversampling      { 0 };
    double                           m_dAdc2SampleTimeTicks     { 0.0 };
    double                           m_dAdc2LeftShiftFactor     { 0.0 };
    double                           m_dAdc2SamplingFreqHz      { 0.0 };
    double                           m_dAdc2DeltaTimeSec        { 0.0 };

    QSharedPointer<GpioOut>          m_pGpioAdc0BufferEnable;

    QSharedPointer<GpioOut>          m_pGpioOnePulseTimer44006000Prepare;
    QSharedPointer<GpioOut>          m_pGpioOnePulseTimer40000000Prepare;

    QSharedPointer<GpioOut>          m_pGpioOnePulseTimer44006000Disable;
    QSharedPointer<GpioOut>          m_pGpioOnePulseTimer40000000Disable;

    QSharedPointer<SystemDeviceFile> m_pSystemFilePulseLength;

    QSharedPointer<GpioOut>          m_pGpioStartGrind;

};

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

} // namespace SystemIo

#endif // CpuMp157Driver_h
