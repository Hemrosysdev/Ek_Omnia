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

#include <QThread>
#include <QTimer>
#include <QSemaphore>

#ifdef USE_MEDIAN_FILTER
#include "MedianFilter.hpp"
#endif

#ifdef USE_ARITHMETIC_FILTER
#include "ArithmeticFilter.h"
#endif

class SettingsSerializer;
class MainStatemachine;
class DddCouple;

#define MAX_DDD_ANGLE_HISTORY_SIZE  20
#define MAX_HEMRO_SCALE_FLOAT       ( 800.0 )
#define MAX_DEGREE_SCALE_FLOAT      ( 288.0 )
#define MAX_HEMRO_SCALE             ( 800 )
#define MAX_DEGREE_SCALE            ( 288 )
#define COARSE_GEAR_RATIO           ( MAX_DEGREE_SCALE_FLOAT / 355.8 )
#define FINE_GEAR_RATIO             ( 4.462 )

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

class DddDriver : public QThread
{
    Q_OBJECT

    Q_PROPERTY( int filterAngle READ filterAngle NOTIFY filterAngleChanged )
    Q_PROPERTY( int rawAngle READ rawAngle NOTIFY rawAngleUpdate )
    Q_PROPERTY( bool failed READ isFailed NOTIFY failedChanged )

public:

    enum class DddType
    {
        DddTypeUnknown = 0,
        DddTypePwm,
        DddTypeAdc
    };

    enum class DddId
    {
        DddIdUnknown = 0,
        DddId1 = 1,
        DddId2 = 2,

        DddIdCoarse = DddId1,
        DddIdFine = DddId2,
    };

#ifdef USE_MEDIAN_FILTER
    using DddMedianFilter = MedianFilter<int, MAX_DDD_ANGLE_HISTORY_SIZE>;
#endif

#ifdef USE_ARITHMETIC_FILTER
    using DddArithmeticFilter = ArithmeticFilter<int, MAX_DDD_ANGLE_HISTORY_SIZE>;
#endif

public:

    explicit DddDriver( QObject * pParent = nullptr );
    ~DddDriver() override;

    void create( MainStatemachine * pMainStatemachine,
                 DddCouple *        pDddCouple,
                 const DddId        nDddId,
                 const DddType      nDddType );

    Q_INVOKABLE void calibrateDdd( void );

    void readCalibrationValues( void );

    int calibrationAngle() const;

    int filterAngle() const;

    int rawAngle() const;

    int calibratedFilterAngle() const;

    int calibratedRawAngle() const;

    bool isFailed() const;

#ifndef TARGETBUILD
    void saveSimulatedScaleRotation( const double dSimulatedScaleRotation );

#endif

public slots:

    void updateHistory();

    void processNewRawAngle( const int nNewRawAngle );

    void setFailed( const bool bFailed );

signals:

    void filterAngleChanged();

    void rawAngleUpdate( const int nNewRawAngle );

    void statusChanged( const bool bFailed );

    void failedChanged();

private:

    void readPwmDeviceFile();

    void readAdcDeviceFile();

    int smoothenRawAngle( const int nNewRawAngle );

    int applyFilterWindow( const int nAngle );

    void setFilterAngle( const int nFilterAngle );

    void updateStatus();

    void setRawAngle( const int nRawAngle );

    void run() override;

private:

    const int          m_cWindowFilterRadius { 10 };            // 1 degree

    MainStatemachine * m_pMainStatemachine { nullptr };
    DddCouple *        m_pDddCouple { nullptr };
    DddType            m_nDddType { DddType::DddTypeUnknown };
    DddId              m_nDddId { DddId::DddIdUnknown };
    QString            m_strDeviceFile;

    int                m_nReadErrorCnt { 0 };
    bool               m_bFailed { false };
    int                m_nLastOffsetRawAngle { 0 };
    bool               m_bFirstDddAngle { true };
    int                m_nOffset360 { 0 };
    int                m_nLastFilterAngle { 0 };
    int                m_nRawAngle { 0 };
    QTimer             m_timerHistoryUpdate;
    int                m_nCurrentRawAngle { 0 };
    int                m_nCalibrationAngle { 0 };
    int                m_nFilterWindowAngle { 0 };
    bool               m_bFilterWindowMove { false };
    int                m_nFilterWindowStableCounter { 0 };
    int                m_nMaxFilterWindowStableCounter { 0 };
#ifndef TARGETBUILD
    int                m_nSimulatedScaleRotation { 0 };
#endif

#define USE_5V

#ifdef USE_5V
    double m_dAdc1MinValue { 0.161135 };
    double m_dAdc1MaxValue { 2.76251 };
    double m_dAdc2MinValue { 0.165264 };
    double m_dAdc2MaxValue { 2.80244 };
#else
    double m_dAdc1MinValue { 0.145727 };
    double m_dAdc1MaxValue { 2.48511 };
    double m_dAdc2MinValue { 0.153079 };
    double m_dAdc2MaxValue { 2.60682 };
#endif
    double m_dAdcMinValue { 0.11924 };
    double m_dAdcMaxValue { 2.08992 };

#ifdef USE_MEDIAN_FILTER
    DddMedianFilter m_medianFilter;
#endif

#ifdef USE_ARITHMETIC_FILTER
    DddArithmeticFilter m_arithmeticFilter;
#endif

    QSemaphore m_semStopThread { 1 };

};

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#endif // DddDriver_h
