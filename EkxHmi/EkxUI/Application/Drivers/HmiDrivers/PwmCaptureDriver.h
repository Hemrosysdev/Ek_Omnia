///////////////////////////////////////////////////////////////////////////////
///
/// @file PwmCaptureDriver.h
///
/// @brief Header file of class PwmCaptureDriver.
///
/// @author Ultratronik GmbH
///         Dornierstr. 9
///         D-82205 Gilching
///         Germany
///         http://www.ultratronik.de
///
/// @author written by Gerd Esser, Forschung & Entwicklung, gesser@ultratronik.de
///
/// @date 17.12.2022
///
/// @copyright Copyright 2022 by Hemro International AG
///            Hemro International AG
///            Länggenstrasse 34
///            CH 8184 Bachenbülach
///            Switzerland
///            Homepage: www.hemrogroup.com
///
///////////////////////////////////////////////////////////////////////////////

#ifndef PwmCaptureDriver_h
#define PwmCaptureDriver_h

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#include <QThread>
#include <QSharedPointer>
#include <QSemaphore>

namespace SystemIo
{
class SystemDeviceFile;

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

class PwmCaptureDriver : public QThread
{
    Q_OBJECT

private:

    static constexpr double c_dNanoSecsPerSec = 1.e9;

public:

    explicit PwmCaptureDriver( QObject * pParent = nullptr );
    ~PwmCaptureDriver() override;

    void create( const QString & strDriverName,
                 const QString & strCaptureDeviceFile,
                 const int       nPollCycleMs,
                 const int       nMaxReadErrorCnt,
                 const int       nErrorCntHysteresis,
                 const bool      bUpdateCyclic,
                 const bool      bStartThread );

    void create( const QString & strDriverName,
                 const quint32   u32PwmCaptureRegisterAddress,
                 const int       nPwmCaptureChip,
                 const int       nPwmCaptureChannel,
                 const int       nPollCycleMs,
                 const int       nMaxReadErrorCnt,
                 const int       nErrorCntHysteresis,
                 const bool      bUpdateCyclic,
                 const bool      bStartThread );

    bool isFailed() const;

    double dutyCycle() const;

    double frequency() const;

    void startThread();

    void stopThread();

signals:

    void failedChanged( const bool bFailed );

    void dutyCycleChanged( const double dDutyCycle );

    void frequencyChanged( const double dFrequency );

private:

    void setFailed( const bool bFailed );

    void readPwmDeviceFile();

    void updateStatus();

    void setDutyCycle( const double dDutyCycle );

    void setFrequency( const double dFrequency );

    void run() override;

private:

    QString                          m_strDriverName;

    QSharedPointer<SystemDeviceFile> m_pSystemFileCapture;

    int                              m_nPollCycleMs { 50 };

    int                              m_nMaxReadErrorCnt { 50 };

    int                              m_nErrorCntHysteresis { 5 };

    int                              m_nMaxReadErrorUpperHystheresis { };

    double                           m_dDutyCycle { 0.0 };

    double                           m_dFrequency { 0.0 };

    int                              m_nReadErrorCnt { 0 };

    bool                             m_bFailed { true };

    bool                             m_bUpdateCyclic { false };

    QSemaphore                       m_semStopThread;

};

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

} // namespace SystemIo

#endif // PwmCaptureDriver_h
