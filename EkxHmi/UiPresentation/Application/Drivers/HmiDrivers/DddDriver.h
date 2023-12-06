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

#include <QThread>
#include <QTimer>

class SettingsSerializer;
class QQmlEngine;
class MainStatemachine;

#define MAX_DDD_ANGLE_HISTORY_SIZE  20

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

class DddDriver : public QThread
{
    Q_OBJECT

    Q_PROPERTY( int  dddBigValue             READ getDddBigValue                                           NOTIFY dddBigValueChanged )
    Q_PROPERTY( int  dddCommaValue           READ getDddCommaValue                                         NOTIFY dddCommaValueChanged )

#ifndef TARGETBUILD
    Q_PROPERTY( int  simulatedScaleRotation  READ simulatedScaleRotation  WRITE setSimulatedScaleRotation  NOTIFY simulatedScaleRotationChanged )
#endif

public:

    explicit DddDriver( QQmlEngine* enigne,
                        MainStatemachine * pMainStatemachine );
    virtual ~DddDriver( ) override;

    void create();

    Q_INVOKABLE void calibrateDdd( void );

    void grinderSettingsUpdated( void );

#ifndef TARGETBUILD
    void incSimulatedScaleRotation( const int nInc );
    int simulatedScaleRotation() const;
    void setSimulatedScaleRotation( const int nSimulatedScaleRotation );
#endif

public slots:

    void dddUpdateHistory();
    void processNewRawAngle( const int nNewRawAngle );

signals:

    void dddBigValueChanged();
    void dddCommaValueChanged();
    void dddValueChanged();
    void rawAngleUpdate( const int nNewRawAngle );

#ifndef TARGETBUILD
    void simulatedScaleRotationChanged();
#endif

private:

#ifndef TARGETBUILD
    void saveSimulatedScaleRotation( void );
    void readSimulatedScaleRotation();
#endif

    void readDddDeviceFile();
    int calculateAverageAngle( const int nDddRawAngle);
    void calculateDddValue( const int nDddProcessAngle );

    void setDddBigValue( const int nDddBigValue );
    int getDddBigValue() const;

    void setDddCommaValue( const int nDddCommaValue );
    int getDddCommaValue() const ;

    void run() override;

private:

    MainStatemachine *  m_pMainStatemachine;
    int                 m_nDddBigValue { 0 };
    int                 m_nDddCommaValue { 0 };
    int                 m_nLastDddAngle { 0 };
    bool                m_bFirstDddAngle { true };
    int                 m_nOffset360 { 0 };
    int                 m_nDddAngleHistoryPos { 0 };
    int                 m_nLastDddAvrgAngle { 0 };
    int                 m_nDddAngleHistory[MAX_DDD_ANGLE_HISTORY_SIZE];
    QTimer              m_timerDddHistoryUpdate;
    int                 m_nDddCurrentRawAngle { 0 };
    int                 m_nDddCalibrationAngle { 0 };
#ifndef TARGETBUILD
    int                 m_nSimulatedScaleRotation { 0 };
#endif

};

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#endif // DddDriver_h
