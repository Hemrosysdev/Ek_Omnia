///////////////////////////////////////////////////////////////////////////////
///
/// @file DddDriver.h
///
/// @brief Header file of class DddDriver.
///
/// @author Ultratronik GmbH
///         Dornierstr. 9
///         D-82205 Gilching
///         Germany
///         http://www.ultratronik.de
///
/// @author written by Gerd Esser, Forschung & Entwicklung, gesser@ultratronik.de
///
/// @date 01.02.2021
///
/// @copyright Copyright 2021 by Hemro International AG
///            Hemro International AG
///            Länggenstrasse 34
///            CH 8184 Bachenbülach
///            Switzerland
///            Homepage: www.hemrogroup.com
///
///////////////////////////////////////////////////////////////////////////////

#ifndef DddDriver_h
#define DddDriver_h

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

//#define USE_MEDIAN_FILTER
#define USE_ARITHMETIC_FILTER

#include <QObject>
#include <QTimer>
#include <QSharedPointer>

#ifdef USE_MEDIAN_FILTER
#include "MedianFilter.hpp"
#endif

#ifdef USE_ARITHMETIC_FILTER
#include "ArithmeticFilter.h"
#endif

#define MAX_DDD_ANGLE_HISTORY_SIZE  20
#define MAX_HEMRO_SCALE_FLOAT       ( 800.0 )
#define MAX_DEGREE_SCALE_FLOAT      ( 288.0 )
#define MAX_HEMRO_SCALE             ( 800 )
#define MAX_DEGREE_SCALE            ( 288 )
#define COARSE_GEAR_RATIO           ( MAX_DEGREE_SCALE_FLOAT / 355.8 )
#define FINE_GEAR_RATIO             ( 4.462 )

namespace SystemIo
{
class PwmCaptureDriver;
class AdcVoltageInDriver;

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

class DddDriver : public QObject
{
    Q_OBJECT

    Q_PROPERTY( int filterAngle10thDeg READ filterAngle10thDeg NOTIFY filterAngleChanged )
    Q_PROPERTY( int rawAngle10thDeg READ rawAngle10thDeg NOTIFY rawAngleChanged )
    Q_PROPERTY( bool failed READ isFailed NOTIFY failedChanged )

private:

    static const int c_nDmaAdcFilterSize = 800;

public:

    enum class DddType
    {
        DddTypePwm = 0,
        DddTypeAdc
    };
    Q_ENUM( DddType );

#ifdef USE_MEDIAN_FILTER
    using DddMedianFilter = MedianFilter<int, MAX_DDD_ANGLE_HISTORY_SIZE>;
#endif

#ifdef USE_ARITHMETIC_FILTER
    using DddArithmeticFilter = UxEco::ArithmeticFilter<int>;
#endif

private:

    static constexpr double c_dFakeGearRatio = 2.0;         // TODO: set the correct gear ratio

public:

    explicit DddDriver( QObject * pParent = nullptr );
    ~DddDriver() override;

    void create( const QString & strDriverName,
                 const DddType   nDddType,
                 const quint32   u32PwmCaptureRegisterAddress,
                 const int       nPwmCaptureChip,
                 const int       nPwmCaptureChannel,
                 const QString & strAdcSystemDeviceDir,
                 const QString & strAdcChannel,
                 const int       nPollCycleMs     = 50,
                 const int       nMaxReadErrorCnt = 50 );

    void setDddType( const DddType nDddType );

    DddType dddType() const;

    Q_INVOKABLE void calibrateDdd( void );

    void setCalibrationAngle10thDeg( const int nCalibrationAngle10thDeg );

    int calibrationAngle10thDeg() const;

    int filterAngle10thDeg() const;

    int rawAngle10thDeg() const;

    int calibratedFilterAngle10thDeg() const;

    int calibratedRawAngle10thDeg() const;

    int dddValueMym() const;

    bool isFailed() const;

    QSharedPointer<AdcVoltageInDriver>  adcVoltageInDriver();

public slots:

    void updateHistory();

    void processNewRawAngle( const int nNewRawAngle10thDeg );

    void setFailed( const bool bFailed );

    void processPwmDutyCycle( const double dDutyCycle );

    void processAdcVoltage( const double dAdcVoltageMv );

signals:

    void filterAngleChanged( const int nFilterAngle10thDeg );

    void rawAngleChanged( const int nRawAngle10thDeg );

    void statusChanged( const bool bFailed );

    void failedChanged( const bool bFailed );

    void calibrationAngleChanged( const int nCalibrationAngle10thDeg );

    void dddValueChanged( const int nDddValueMym );

private:

    int smoothenRawAngle( const int nNewRawAngle );

    int applyFilterWindow( const int nAngle );

    void setFilterAngle10thDeg( const int nFilterAngle10thDeg );

    void updateStatus();

    void setRawAngle10thDeg( const int nRawAngle );

    void setDddValueMym( const int nDddValueMym );

private:

    QString                            m_strDriverName;

    const int                          m_cWindowFilterRadius { 10 }; // 1 degree

    DddType                            m_nDddType { DddType::DddTypePwm };

    QSharedPointer<PwmCaptureDriver>   m_pPwmCapture;
    QSharedPointer<AdcVoltageInDriver> m_pAdcVoltageIn;

    int                                m_nReadErrorCnt { 0 };
    bool                               m_bFailed { true };
    int                                m_nLastOffsetRawAngle10thDeg { 0 };
    bool                               m_bFirstDddAngle { true };
    int                                m_nOffset36010thDeg { 0 };
    int                                m_nFilterAngle10thDeg { 0 };
    int                                m_nRawAngle10thDeg { 0 };
    QTimer                             m_timerHistoryUpdate;
    int                                m_nCurrentRawAngle10thDeg { 0 };
    int                                m_nCalibrationAngle10thDeg { 0 };
    int                                m_nFilterWindowAngle10thDeg { 0 };
    bool                               m_bFilterWindowMove { false };
    int                                m_nFilterWindowStableCounter { 0 };
    int                                m_nMaxFilterWindowStableCounter { 0 };
    int                                m_nDddValueMym { 0 };

    double                             m_dAdcMinVoltageMv {  125.0 };
    double                             m_dAdcMaxVoltageMv { 2375.0 };

#ifdef USE_MEDIAN_FILTER
    DddMedianFilter m_medianFilter;
#endif

#ifdef USE_ARITHMETIC_FILTER
    DddArithmeticFilter m_arithmeticFilter;
#endif

};

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

} // namespace SystemIo

#endif // DddDriver_h
