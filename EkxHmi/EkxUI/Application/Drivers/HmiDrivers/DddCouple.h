///////////////////////////////////////////////////////////////////////////////
///
/// @file DddCouple.h
///
/// @brief Header file of class DddCouple.
///
/// @author Ultratronik GmbH
///         Dornierstr. 9
///         D-82205 Gilching
///         Germany
///         http://www.ultratronik.de
///
/// @author written by Gerd Esser, Forschung & Entwicklung, gesser@ultratronik.de
///
/// @date 23.03.2022
///
/// @copyright Copyright 2022 by Hemro International AG
///            Hemro International AG
///            Länggenstrasse 34
///            CH 8184 Bachenbülach
///            Switzerland
///            Homepage: www.hemrogroup.com
///
///////////////////////////////////////////////////////////////////////////////

#ifndef DddCouple_h
#define DddCouple_h

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#include <QObject>
#include "DddDriver.h"

class SettingsSerializer;
class MainStatemachine;

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

class DddCouple : public QObject
{
    Q_OBJECT

    Q_PROPERTY( int dddValue READ dddValue NOTIFY dddValueChanged )
    Q_PROPERTY( int rawDddValue READ rawDddValue NOTIFY rawDddValueChanged )
    Q_PROPERTY( bool failed READ isDddFailed NOTIFY failedChanged )

#ifndef TARGETBUILD
    Q_PROPERTY( double simulatedScaleRotation READ simulatedScaleRotation WRITE setSimulatedScaleRotation NOTIFY simulatedScaleRotationChanged )
#endif

    enum class DddId
    {
        DddIdUnknown = 0,
        DddId1 = 1,
        DddId2 = 2,

        DddIdCoarse = DddId1,
        DddIdFine = DddId2,
    };

public:

    explicit DddCouple( QObject * pParent = nullptr );
    virtual ~DddCouple() override;

    void create( MainStatemachine *    pMainStatemachine,
                 SystemIo::DddDriver * pDddCoarseDriver,
                 SystemIo::DddDriver * pDddFineDriver );

    Q_INVOKABLE void calibrateDdd( void );

    void readCalibrationValues( void );

    bool isDddFailed() const;

    bool isDdd1Failed() const;

    bool isDdd2Failed() const;

    int dddValue() const;

    int rawDddValue() const;

#ifndef TARGETBUILD

    void incSimulatedScaleRotation( const double dInc );

    double simulatedScaleRotation() const;

    void setSimulatedScaleRotation( const double dSimulatedScaleRotation );

    void saveSimulatedScaleRotation( const DddId     nDddId,
                                     const QString & strDeviceFile,
                                     const double    dSimulatedScaleRotation );

#endif

public slots:

    void scaleToDddValue();

    void scaleRawToDddValue();

signals:

    void dddValueChanged();

    void rawDddValueChanged();

    void failedChanged( const bool bFailed );

#ifndef TARGETBUILD
    void simulatedScaleRotationChanged();

#endif

private slots:

#ifndef TARGETBUILD
    void saveSimulatedScaleRotation( void );

    void readSimulatedScaleRotation();

#endif

private:

    int sensorAngleToDddValue( const int nFineSensorAngle10thDeg,
                               const int nCoarseSensorAngle10thDeg );

    void setDddValue( const int nDddValue );

    void setRawDddValue( const int nDddValue );

private:

    const int          m_cWindowFilterRadius { 10 };            // 1 degree

    MainStatemachine * m_pMainStatemachine;
    int                m_nRawDddValue { 0 };
    int                m_nDddValue { 0 };
#ifndef TARGETBUILD
    double             m_dSimulatedScaleRotation { 0.0 };
#endif

    SystemIo::DddDriver * m_pDddFineDriver { nullptr };
    SystemIo::DddDriver * m_pDddCoarseDriver { nullptr };

    bool                  m_bDdd1Failed { false };
    bool                  m_bDdd2Failed { false };

};

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#endif // DddCouple_h
