///////////////////////////////////////////////////////////////////////////////
///
/// @file SwUpdateController.h
///
/// @brief Header file of class SwUpdateController.
///
/// @author Ultratronik GmbH
///         Dornierstr. 9
///         D-82205 Gilching
///         Germany
///         http://www.ultratronik.de
///
/// @author written by Gerd Esser, Forschung & Entwicklung, gesser@ultratronik.de
///
/// @date 10.02.2021
///
/// @copyright Copyright 2021 by Hemro International AG
///            Hemro International AG
///            Länggenstrasse 34
///            CH 8184 Bachenbülach
///            Switzerland
///            Homepage: www.hemrogroup.com
///
///////////////////////////////////////////////////////////////////////////////

#ifndef SwUpdateController_h
#define SwUpdateController_h

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#include <QObject>
#include <QFileSystemWatcher>

#include "AcyclicNotification.h"

class McuDriver;
class QTimer;
class RecipeControl;
class SettingsSerializer;
class DeviceInfoCollector;
class EspSwUpdateDriver;
class MainStatemachine;

#define USE_TRIGGERED_ESP_UPDATE
#define USE_TRIGGERED_MCU_UPDATE

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

class SwUpdateController : public QObject
{

    Q_OBJECT

    Q_PROPERTY( QString swUpdateInfo READ swUpdateInfo NOTIFY swUpdateInfoChanged )
    Q_PROPERTY( QString swUpdateHeadline READ swUpdateHeadline NOTIFY swUpdateHeadlineChanged )
    Q_PROPERTY( bool showUpdateCompScreen READ isUpdateCompScreen NOTIFY updateCompScreenChanged )
    Q_PROPERTY( bool showProgressBar READ progressRequired NOTIFY swProgressRequiredChanged )
    Q_PROPERTY( int progressValue READ progressValue NOTIFY swProgressValueChanged )
    Q_PROPERTY( int progressBarAutoTimer READ progressBarAutoTimer NOTIFY swProgressBarAutoTimerChanged )

public:

    enum SwUpdateStates
    {
        SWUPDATE_IDLE = 0,
        SWUPDATE_UPLOAD_IN_PROGRESS,
        SWUPDATE_UPLOAD_COMPLETED,
        SWUPDATE_UPLOAD_FAILED,
        SWUPDATE_UPLOAD_ABORTED,
        SWUPDATE_EVALUATE_PACKAGE,
        SWUPDATE_PACKAGE_FAILED,
        SWUPDATE_PACKAGE_NO_UPGRADE,
        SWUPDATE_UNPACK_ROOTFS,
        SWUPDATE_UNPACK_ROOTFS_FAILED,
        SWUPDATE_UNPACK_APPLICATION,
        SWUPDATE_UNPACK_APPLICATION_FAILED,
        SWUPDATE_REBOOT,
        SWUPDATE_STARTUP_CHECK,
        SWUPDATE_CHECK_ROOTFS,
        SWUPDATE_ROOTFS_FAILED,

        SWUPDATE_ESP_CHECK_INTERFACE,
        SWUPDATE_ESP_INTERFACE_FAILED,
        SWUPDATE_ESP_CHECK_VERSION,
        SWUPDATE_ESP_UPDATE_IN_PROGRESS,
        SWUPDATE_ESP_UPDATE_FAILED,
        SWUPDATE_ESP_VALIDATE_VERSION,

        SWUPDATE_MCU_CHECK_INTERFACE,
        SWUPDATE_MCU_INTERFACE_FAILED,
        SWUPDATE_MCU_CHECK_VERSION,
        SWUPDATE_MCU_UPDATE_IN_PROGRESS,
        SWUPDATE_MCU_UPDATE_FAILED,
        SWUPDATE_MCU_VALIDATE_VERSION,
        SWUPDATE_STARTUP_COMPLETED,
        SWUPDATE_STARTUP_FAILED,
        SWUPDATE_UPDATE_COMPLETED,
        SWUPDATE_UPDATE_FAILED,
        SWUPDATE_TIMEOUT,
        SWUPDATE_INIT
    };
    Q_ENUM( SwUpdateStates );

    enum SwUpdateProcesses
    {
        SWU_PROCESS_NORMAL = 0,
        SWU_PROCESS_UPDATE,
        SWU_PROCESS_ROLLBACK
    };
    Q_ENUM( SwUpdateProcesses );

public:

    explicit SwUpdateController( QObject * pParent = nullptr );
    ~SwUpdateController() override;

    void create( MainStatemachine * pMainStatemachine );

    Q_INVOKABLE void userAcknowlegeCall();

    bool isIdle() const;

    void setSwUploadCompletedState();

signals:

    void swUpdateStateChanged();

    void swuProcessChanged();

    void swUpdateInfoChanged();

    void updateCompScreenChanged();

    void swUpdateStarted();

    void swUpdateStopped();

    void swUpdateHeadlineChanged();

    void swProgressRequiredChanged();

    void swProgressValueChanged();

    void swProgressBarAutoTimerChanged();

private slots:

    void swStateFileChanged();

    void storageDirectoryChanged();

    void checkEspInterface();

    void checkEspVersion();

    void validateEspVersion();

    void doEspSwUpdateStep();

    void checkMcuInterface();

    void checkMcuVersion();

    void validateMcuVersion();

    void handleTimeout();

    void handleEspUpdateCommandTimeout();

    void processMcuSwUpdateFinished( const bool bSuccess );

    void processMcuSwUpdateProgress( const quint8 u8ProgressPercent );

private:

    SwUpdateStates swUpdateState() const;

    void setSwUpdateState( const SwUpdateStates nState );

    bool processSwUpdateState( const SwUpdateStates nState );

    QString swUpdateStateString( const SwUpdateStates nState ) const;

    SwUpdateStates convertStringToSwUpdateState( const QString strState ) const;

    SwUpdateProcesses swUpdateProcess( void ) const;

    void setSwUpdateProcess( const SwUpdateProcesses nProcess );

    QString swUpdateProcessString( const SwUpdateProcesses nProcess ) const;

    SwUpdateProcesses convertStringToSwUpdateProcess( const QString & strProcess ) const;

    bool isUpdateCompScreen() const;

    void setUpdateCompScreen( const bool bNewUpdateCompScreen );

    void startEspSwUpdate();

    void startMcuSwUpdate();

    void triggerSwUpdateFileState( const SwUpdateStates nNextState );

    const QString & swUpdateInfo() const;

    void setSwUpdateInfo( const QString & strInfo );

    const QString & swUpdateHeadline() const;

    void setSwUpdateHeadline( const QString & strHeadline );

    bool progressRequired( void ) const;

    void setProgressRequired( const bool bRequired );

    int progressValue( void ) const;

    void setProgressValue( const int nValue );

    int progressBarAutoTimer() const;

    void setProgressBarAutoTimer( const int nTimeoutSec );

    bool tryActivateIdleState();

    bool tryActivateUploadInProgressState();

    bool tryActivateUploadCompletedState();

    bool tryActivateUploadFailedState();

    bool tryActivateUploadAbortedState();

    bool tryActivateEvaluatePackageState();

    bool tryActivatePackageFailedState();

    bool tryActivatePackageNoUpgradeState();

    bool tryActivateUnpackRootfsState();

    bool tryActivateUnpackRootfsFailedState();

    bool tryActivateUnpackApplicationState();

    bool tryActivateUnpackApplicationFailedState();

    bool tryActivateSwUpdateRebootState();

    bool tryActivateStartupCheckState();

    bool tryActivateCheckRootFsState();

    bool tryActivateRootFsFailedState();

    bool tryActivateMcuCheckInterfaceState();

    bool tryActivateMcuInterfaceFailedState();

    bool tryActivateMcuCheckVersionState();

    bool tryActivateMcuUpdateInProgressState();

    bool tryActivateMcuUpdateFailedState();

    bool tryActivateMcuValidateVersionState();

    bool tryActivateEspCheckInterfaceState();

    bool tryActivateEspInterfaceFailedState();

    bool tryActivateEspCheckVersionState();

    bool tryActivateEspNotAvailableState();

    bool tryActivateEspUpdateInProgressState();

    bool tryActivateEspUpdateFailedState();

    bool tryActivateEspValidateVersionState();

    bool tryActivateStartUpCompletedState();

    bool tryActivateStartUpFailedState();

    bool tryActivateUpdateCompletedState();

    bool tryActivateUpdateFailedState();

    bool tryActivateTimeoutState();

    bool tryActivateFatalErrorState();

    void readSwUpdateStateFile( const bool bStartUp );

    bool writeSwUpdateStateFile( const QString & strProcess,
                                 const QString & strState,
                                 const QString & strErrorReason,
                                 const bool      bProgressReq,
                                 const int       nProgValue );

    void startPeriodicEspInterfaceCheck();

    void startPeriodicMcuInterfaceCheck();

    QString mcuUpdateSwVersionString() const;

    QString espUpdateSwVersionString() const;

    QString espUpdateFile() const;

    bool checkRootfs() const;

    void swEspUpdateFailure( const bool bByTimeout );

    void clearSwUpdatePackage();

private:

    //static int                              m_nQmlSwUpdateControllerId;

    MainStatemachine *                    m_pMainStatemachine { nullptr };
    QTimer *                              m_pEspInterfaceCheckTimer { nullptr };
    int                                   m_nEspInterfaceCheckCounter { 0 };
    QTimer *                              m_pMcuInterfaceCheckTimer { nullptr };
    int                                   m_nMcuInterfaceCheckCounter { 0 };
    QTimer *                              m_pTimeoutTimer { nullptr };
    QTimer *                              m_pEspUpdateCommandTimer { nullptr };

    QFileSystemWatcher                    m_theSwUpdateFileObserver;
    SwUpdateController::SwUpdateStates    m_nCurrentState { SWUPDATE_INIT };
    QString                               m_strSwUpdateInfo;
    bool                                  m_bUpdateCompScreen { false };
    QString                               m_strSwUpdateHeadline;
    SwUpdateController::SwUpdateProcesses m_nCurrentProcess { SWU_PROCESS_NORMAL };
    QString                               m_strCurrentErrorReason;
    bool                                  m_bCurrentProgressRequired { false };
    int                                   m_nCurrentProgressValue { 0 };
    int                                   m_nProgressBarAutoTimerSec { 0 };
    QByteArray                            m_baEspFirmware;
    int                                   m_nEspUpdateChunkNo { 0 };
    int                                   m_nEspUpdateRepeatCounter { 0 };

    AcyclicNotification                   m_acycNotificationStateFileCorrupted;

};

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#endif // SwUpdateController_h
