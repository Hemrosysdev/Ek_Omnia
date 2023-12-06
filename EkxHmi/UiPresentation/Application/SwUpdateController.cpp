///////////////////////////////////////////////////////////////////////////////
///
/// @file SwUpdateController.cpp
///
/// @brief Implementation file of class SwUpdateController.
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

#include "SwUpdateController.h"

#include <QQmlContext>
#include <QtDebug>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QFile>
#include <QDir>
#include <QProcess>
#include <QThread>

#include "EspConnectorClient.h"
#include "RecipeControl.h"
#include "SettingsSerializer.h"
#include "DeviceInfoCollector.h"
#include "EspSwUpdateInDriver.h"
#include "EspSwUpdateOutDriver.h"
#include "EspSwUpdateOut.h"
#include "UartDriver.h"
#include "McuDriver.h"
#include "MainStatemachine.h"
#include "SqliteInterface.h"
#include "StandbyController.h"

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#define SWSTORAGE_PATH                  "/storage/"
#define SWSTATE_PATH                    "/storage/SoftwareUpdateState.json"
#define ESPCONFIG_PATH                  "/usr/local/img/EspFirmwareConfig.json"
#define ESP_FIRMWARE                    "/usr/local/img/"
#define MCUCONFIG_PATH                  "/usr/local/img/McuFirmwareConfig.json"
#define ROOTFS_NOM_PATH                 "/storage/rootfs_version"
#define ROOTFS_CUR_PATH                 "/etc/issue"

#define NOT_VALID                       "Invalid"

#define MAX_INTERFACE_CHECKS            (10)
#define MAX_VERSION_CHECKTIME           (10000)
#define MAX_ESP_COM_TIMEOUT             (20000)
#define MAX_MCU_COM_TIMEOUT             (20000)
#define MAX_ESP_UPDATE_TIMEOUT          (20000)
#define MAX_ESP_UPDATE_REPEATS          3
#define MAX_ESP_UPDATE_BEGIN_TIMEOUT    5000
#define MAX_ESP_UPDATE_FINISH_TIMEOUT   15000
#define MAX_ESP_UPDATE_COMMAND_TIMEOUT  200
#define MAX_ESP_UPDATE_REBOOT_TIMEOUT   6000
#define ESP_UPDATE_CHUNK_SIZE           200

#define SWUPDATE_JSON_PROCESS           "Process"
#define SWUPDATE_JSON_STATE             "State"
#define SWUPDATE_JSON_ERROR             "ErrorReason"
#define SWUPDATE_JSON_PROGREQ           "ProgressRequired"
#define SWUPDATE_JSON_PROGVAL           "ProgressValue"

#define SWUPDATE_JSON_MCU_SWVER         "SwVersion"
#define SWUPDATE_JSON_ESP_SWVER         "VERSION_NO"
#define SWUPDATE_JSON_ESP_FILE          "FILE"

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

int SwUpdateController::m_nQmlSwUpdateControllerId = qmlRegisterUncreatableType<SwUpdateController>( "EkxSwUpdateStates", 1, 0, "SwUpdateStatesEnum", "something went wrong" );

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

SwUpdateController::SwUpdateController( QQmlEngine* pEngine,
                                        MainStatemachine *pMainStatemachine)
    : QObject(pMainStatemachine)
    , m_pMainStatemachine( pMainStatemachine )
    , m_nCurrentState( SwUpdateStates::SWUPDATE_INIT )
    , m_nCurrentProcess( SWU_PROCESS_NORMAL )
{
    MainStatemachine::ensureExistingDirectory( MainStatemachine::rootPath() + SWSTATE_PATH );
    MainStatemachine::ensureExistingDirectory( MainStatemachine::rootPath() + ESPCONFIG_PATH );
    MainStatemachine::ensureExistingDirectory( MainStatemachine::rootPath() + MCUCONFIG_PATH );
    MainStatemachine::ensureExistingDirectory( MainStatemachine::rootPath() + ROOTFS_NOM_PATH );
    MainStatemachine::ensureExistingDirectory( MainStatemachine::rootPath() + ROOTFS_CUR_PATH );

    pEngine->rootContext()->setContextProperty("ekxSwUpdateController" , this);

    m_pInterfaceCheckTimer   = new QTimer(this);
    m_nInterfaceCheckCounter = 0;
    connect( m_pInterfaceCheckTimer, &QTimer::timeout, this, &SwUpdateController::checkInternalInterface );
    m_pTimeoutTimer          = new QTimer(this);
    connect( m_pTimeoutTimer, &QTimer::timeout, this, &SwUpdateController::handleTimeout );

    m_pEspUpdateCommandTimer        = new QTimer(this);
    m_pEspUpdateCommandTimer->setInterval( MAX_ESP_UPDATE_COMMAND_TIMEOUT );
    m_pEspUpdateCommandTimer->setSingleShot( true );
    connect( m_pEspUpdateCommandTimer, &QTimer::timeout, this, &SwUpdateController::handleEspUpdateCommandTimeout );

    m_theSwUpdateFileObserver.addPath( MainStatemachine::rootPath() + SWSTORAGE_PATH );
    m_theSwUpdateFileObserver.addPath( MainStatemachine::rootPath() + SWSTATE_PATH );
    connect( &m_theSwUpdateFileObserver, &QFileSystemWatcher::fileChanged, this, &SwUpdateController::swStateFileChanged );
    connect( &m_theSwUpdateFileObserver, &QFileSystemWatcher::directoryChanged, this, &SwUpdateController::storageDirectoryChanged );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

SwUpdateController::~SwUpdateController()
{
    m_theSwUpdateFileObserver.removePath(SWSTATE_PATH);

    delete m_pInterfaceCheckTimer;
    m_pInterfaceCheckTimer = nullptr;

    delete m_pTimeoutTimer;
    m_pTimeoutTimer = nullptr;

    delete m_pEspUpdateCommandTimer;
    m_pEspUpdateCommandTimer = nullptr;

    m_pMainStatemachine = nullptr;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SwUpdateController::setProgressRequired(const bool bRequired)
{
    if ( m_bCurrentProgressRequired != bRequired )
    {
        m_bCurrentProgressRequired = bRequired;
        emit swProgressRequiredChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SwUpdateController::setProgressValue(const int nValue)
{
    if ( m_nCurrentProgressValue != nValue )
    {
        m_nCurrentProgressValue = nValue;
        emit swProgressValueChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

int SwUpdateController::progressBarAutoTimer() const
{
    return m_nProgressBarAutoTimerSec;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SwUpdateController::setProgressBarAutoTimer(const int nTimeoutSec)
{
    if ( m_nProgressBarAutoTimerSec != nTimeoutSec )
    {
        m_nProgressBarAutoTimerSec = nTimeoutSec;
        emit swProgressBarAutoTimerChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

SwUpdateController::SwUpdateStates SwUpdateController::swUpdateState() const
{
    return m_nCurrentState;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SwUpdateController::setSwUpdateState(const SwUpdateController::SwUpdateStates nState)
{
    if ( m_nCurrentState != nState )
    {
        m_nCurrentState = nState;
        emit swUpdateStateChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SwUpdateController::processSwUpdateState(const SwUpdateStates nState)
{
    bool success = true;

    m_pMainStatemachine->standbyController()->wakeUp();

    switch ( nState )
    {
        case SWUPDATE_IDLE:
            success = tryActivateIdleState();
        break;
        case SWUPDATE_UPLOAD_IN_PROGRESS:
            success = tryActivateUploadInProgressState();
        break;
        case SWUPDATE_UPLOAD_COMPLETED:
            success = tryActivateUploadCompletedState();
        break;
        case SWUPDATE_UPLOAD_FAILED:
            success = tryActivateUploadFailedState();
        break;
        case SWUPDATE_EVALUATE_PACKAGE:
            success = tryActivateEvaluatePackageState();
        break;
        case SWUPDATE_PACKAGE_FAILED:
            success = tryActivatePackageFailedState();
        break;
        case SWUPDATE_PACKAGE_NO_UPGRADE:
            success = tryActivatePackageNoUpgradeState();
        break;
        case SWUPDATE_UNPACK_ROOTFS:
            success = tryActivateUnpackRootfsState();
        break;
        case SWUPDATE_UNPACK_ROOTFS_FAILED:
            success = tryActivateUnpackRootfsFailedState();
        break;
        case SWUPDATE_UNPACK_APPLICATION:
            success = tryActivateUnpackApplicationState();
        break;
        case SWUPDATE_UNPACK_APPLICATION_FAILED:
            success = tryActivateUnpackApplicationState();
        break;
        case SWUPDATE_INIT_UPDATE_FAILED:
            success = tryActivateInitUpdateFailedState();
        break;
        case SWUPDATE_REBOOT:
            success = tryActivateSwUpdateRebootState();
        break;
        case SWUPDATE_STARTUP_CHECK:
            success = tryActivateStartupCheckState();
        break;
        case SWUPDATE_CHECK_ROOTFS:
            success = tryActivateCheckRootFsState();
        break;
        case SWUPDATE_ROOTFS_FAILED:
            success = tryActivateRootFsFailedtState();
        break;
        case SWUPDATE_CHECK_INTERNAL_INTERFACE:
            success = tryActivateCheckInternalInterfaceState();
        break;
        case SWUPDATE_INTERNAL_INTERFACE_FAILED:
            success = tryActivateInternalInterfaceFailedState();
        break;
        case SWUPDATE_MCU_CHECK_VERSION:
            success = tryActivateCheckMcuVersionState();
        break;
        case SWUPDATE_MCU_NOT_AVAILABLE:
            success = tryActivateMcuNotAvailableState();
        break;
        case SWUPDATE_MCU_UPDATE_IN_PROGRESS:
            success =  tryActivateMcuUpdateInProgressState();
        break;
        case SWUPDATE_MCU_UPDATE_FAILED:
            success = tryActivateMcuUpdateFailedState();
        break;
        case SWUPDATE_MCU_VALIDATE_VERSION:
            success = tryActivateMcuValidateVersionState();
        break;
        case SWUPDATE_ESP_CHECK_VERSION:
            success = tryActivateEspCheckVersionState();
        break;
        case SWUPDATE_ESP_NOT_AVAILABLE:
            success = tryActivateEspNotAvailableState();
        break;
        case SWUPDATE_ESP_UPDATE_IN_PROGRESS:
            success = tryActivateEspUpdateInProgressState();
        break;
        case SWUPDATE_ESP_UPDATE_FAILED:
            success = tryActivateEspUpdateFailedState();
        break;
        case SWUPDATE_ESP_VALIDATE_VERSION:
            success = tryActivateEspValidateVersionState();
        break;
        case SWUPDATE_STARTUP_COMPLETED:
            success = tryActivateStartUpCompletedState();
        break;
        case SWUPDATE_STARTUP_FAILED:
            success = tryActivateStartUpFailedState();
        break;
        case SWUPDATE_UPDATE_COMPLETED:
            success = tryActivateUpdateCompletedState();
        break;
        case SWUPDATE_UPDATE_FAILED:
            success = tryActivateUpdateFailedState();
        break;
        case SWUPDATE_TIMEOUT:
            success = tryActivateTimeoutState();
        break;
        case SWUPDATE_FATAL_ERROR:
            success = tryActivateFatalErrorState();
        break;
        default:
            success = false;
    }

    if ( !success )
    {
        qInfo() << "processSwUpdateState: Abort SW Update";
        QThread::msleep( 500 );
        setSwUpdateProcess( SWU_PROCESS_NORMAL );
        triggerSwUpdateFileState( SWUPDATE_IDLE );
    }

    return success;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

const QString &SwUpdateController::swUpdateInfo() const
{
    return m_strSwUpdateInfo;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SwUpdateController::setSwUpdateInfo(const QString &strInfo )
{
    if( m_strSwUpdateInfo != strInfo )
    {
        m_strSwUpdateInfo = strInfo;

        emit swUpdateInfoChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SwUpdateController::updateCompScreen() const
{
    return m_bUpdateCompScreen;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SwUpdateController::setUpdateCompScreen(const bool bNewUpdateCompScreen )
{
    if( m_bUpdateCompScreen != bNewUpdateCompScreen )
    {
        m_bUpdateCompScreen = bNewUpdateCompScreen;

        emit updateCompScreenChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SwUpdateController::tryActivateIdleState()
{
    bool success;

    switch ( m_nCurrentState )
    {
        default:
            success = true;
        break;
    }

    if( success )
    {
        setUpdateCompScreen( false );
        m_strCurrentErrorReason = "";
        setSwUpdateInfo( "" );
        setProgressRequired( false );
        setSwUpdateProcess( SWU_PROCESS_NORMAL );
        setSwUpdateState( SwUpdateStates::SWUPDATE_IDLE );

        emit swUpdateStopped();
    }

    return success;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SwUpdateController::tryActivateUploadInProgressState()
{
    bool success;

    switch ( m_nCurrentState )
    {
        case SWUPDATE_IDLE:
        case SWUPDATE_UPLOAD_IN_PROGRESS:
            success = true;
        break;

            /* TODO: kann weg, wenn mal alles richtig läuft */
        case SWUPDATE_UPLOAD_COMPLETED:
        case SWUPDATE_UPLOAD_FAILED:
        case SWUPDATE_EVALUATE_PACKAGE:
        case SWUPDATE_PACKAGE_FAILED:
        case SWUPDATE_PACKAGE_NO_UPGRADE:
        case SWUPDATE_INIT_UPDATE_FAILED:
        case SWUPDATE_UNPACK_ROOTFS_FAILED:
        case SWUPDATE_UNPACK_ROOTFS:
        case SWUPDATE_UNPACK_APPLICATION_FAILED:
        case SWUPDATE_UNPACK_APPLICATION:
        return true;
        break;

        default:
            success = false;
        break;
    }

    if ( success )
    {
        setUpdateCompScreen( false );
        setSwUpdateState( SwUpdateStates::SWUPDATE_UPLOAD_IN_PROGRESS );
        setSwUpdateInfo( "Upload in progress" );
        setProgressRequired( true );

        emit swUpdateStarted();
    }

    return success;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SwUpdateController::tryActivateUploadCompletedState()
{
    bool success;

    switch ( m_nCurrentState )
    {
        case SWUPDATE_UPLOAD_IN_PROGRESS:
        case SWUPDATE_UPLOAD_COMPLETED:
            success = true;
        break;

        default:
            // work araound TODO - remove later
        return true;
        //success = false;
        break;
    }

    if( success )
    {
        setUpdateCompScreen( false );
        setSwUpdateState( SwUpdateStates::SWUPDATE_UPLOAD_COMPLETED );
        setSwUpdateInfo( "Upload completed.\nWaiting for update." );
        setProgressRequired( false );

        emit swUpdateStarted();
    }

    return success;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SwUpdateController::tryActivateUploadFailedState()
{
    bool success;

    switch ( m_nCurrentState )
    {
        case SWUPDATE_UPLOAD_FAILED:
        case SWUPDATE_UPLOAD_IN_PROGRESS:
        case SWUPDATE_UPLOAD_COMPLETED:
            success = true;
        break;

        default:
            success = false;
        break;
    }

    if( success )
    {
        setUpdateCompScreen( false );
        m_strCurrentErrorReason = QString("Package Upload failed");
        setSwUpdateState( SwUpdateStates::SWUPDATE_UPLOAD_FAILED );
        setProgressRequired( false );

        triggerSwUpdateFileState( SwUpdateStates::SWUPDATE_INIT_UPDATE_FAILED );
    }

    return success;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SwUpdateController::tryActivateEvaluatePackageState()
{
    bool success;

    switch ( m_nCurrentState )
    {
        case SWUPDATE_EVALUATE_PACKAGE:
        case SWUPDATE_UPLOAD_IN_PROGRESS:
        case SWUPDATE_UPLOAD_COMPLETED:
            success = true;
        break;

        default:
            success = false;
        break;
    }

    if( success )
    {
        setUpdateCompScreen( false );
        setSwUpdateState( SwUpdateStates::SWUPDATE_EVALUATE_PACKAGE );
        setSwUpdateInfo("Checking update package");
        setProgressRequired( false );
    }

    return success;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SwUpdateController::tryActivatePackageFailedState()
{
    bool success;

    switch ( m_nCurrentState )
    {
        case SWUPDATE_EVALUATE_PACKAGE:
        case SWUPDATE_PACKAGE_FAILED:
            success = true;
        break;

        default:
            success = false;
        break;
    }

    if( success )
    {
        setUpdateCompScreen( false );
        m_strCurrentErrorReason = QString("Update package corrupt");
        setSwUpdateState( SwUpdateStates::SWUPDATE_PACKAGE_FAILED );
        setProgressRequired( false );
        triggerSwUpdateFileState( SwUpdateStates::SWUPDATE_INIT_UPDATE_FAILED );
    }

    return success;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SwUpdateController::tryActivatePackageNoUpgradeState()
{
    bool success;

    switch ( m_nCurrentState )
    {
        case SWUPDATE_EVALUATE_PACKAGE:
        case SWUPDATE_PACKAGE_NO_UPGRADE:
            success = true;
        break;

        default:
            success = false;
        break;
    }

    if( success )
    {
        setUpdateCompScreen( false );
        m_strCurrentErrorReason = QString("Update package is no upgrade");
        setSwUpdateState( SwUpdateStates::SWUPDATE_PACKAGE_NO_UPGRADE );
        setProgressRequired( false );
        triggerSwUpdateFileState( SwUpdateStates::SWUPDATE_INIT_UPDATE_FAILED );
    }

    return success;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SwUpdateController::tryActivateUnpackRootfsState()
{
    bool success;

    switch ( m_nCurrentState )
    {
        case SWUPDATE_UPLOAD_COMPLETED:
        case SWUPDATE_UPLOAD_IN_PROGRESS:
        case SWUPDATE_EVALUATE_PACKAGE:
        case SWUPDATE_UNPACK_ROOTFS:
            success = true;
        break;

        default:
            success = false;
        break;
    }

    if( success )
    {
        setUpdateCompScreen( false );
        setSwUpdateState( SwUpdateStates::SWUPDATE_UNPACK_ROOTFS );
        setSwUpdateInfo( "Unpack image #1 of 2");
        setProgressRequired( true );
        setProgressValue( 0 );
        setProgressBarAutoTimer( 25 );
    }

    return success;
}


///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SwUpdateController::tryActivateUnpackRootfsFailedState()
{
    bool success;

    switch ( m_nCurrentState )
    {
        case SWUPDATE_EVALUATE_PACKAGE:
        case SWUPDATE_UNPACK_ROOTFS:
        case SWUPDATE_UNPACK_ROOTFS_FAILED:
            success = true;
        break;

        default:
            success = false;
        break;
    }

    if( success )
    {
        setUpdateCompScreen( false );
        m_strCurrentErrorReason = QString("Unpacking image #1 failed");
        setSwUpdateState( SwUpdateStates::SWUPDATE_UNPACK_ROOTFS_FAILED );
        setProgressRequired( false );
        triggerSwUpdateFileState( SwUpdateStates::SWUPDATE_INIT_UPDATE_FAILED );
    }

    return success;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SwUpdateController::tryActivateUnpackApplicationState()
{
    bool success;

    switch ( m_nCurrentState )
    {
        case SWUPDATE_UNPACK_ROOTFS:
        case SWUPDATE_EVALUATE_PACKAGE:
        case SWUPDATE_UNPACK_APPLICATION:
            success = true;
        break;

        default:
            success = false;
        break;
    }

    if( success )
    {
        setUpdateCompScreen( false );
        setSwUpdateState( SwUpdateStates::SWUPDATE_UNPACK_APPLICATION );
        setSwUpdateInfo( "Unpack image #2 of 2" );
        setProgressRequired( true );
        setProgressValue( 0 );
        setProgressBarAutoTimer( 6 );
    }

    return success;
}


///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SwUpdateController::tryActivateUnpackApplicationFailedState()
{
    bool success;

    switch ( m_nCurrentState )
    {
        case SWUPDATE_UNPACK_APPLICATION:
        case SWUPDATE_UNPACK_APPLICATION_FAILED:
            success = true;
        break;

        default:
            success = false;
        break;
    }

    if( success )
    {
        setUpdateCompScreen( false );
        m_strCurrentErrorReason = QString("Unpacking image #2 failed");
        setSwUpdateState( SwUpdateStates::SWUPDATE_UNPACK_APPLICATION_FAILED );
        setProgressRequired( false );
        triggerSwUpdateFileState( SwUpdateStates::SWUPDATE_INIT_UPDATE_FAILED );
    }

    return success;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SwUpdateController::tryActivateInitUpdateFailedState()
{
    bool success;

    switch ( m_nCurrentState )
    {
        case SWUPDATE_UPLOAD_FAILED:
        case SWUPDATE_PACKAGE_FAILED:
        case SWUPDATE_PACKAGE_NO_UPGRADE:
        case SWUPDATE_UNPACK_ROOTFS_FAILED:
        case SWUPDATE_UNPACK_APPLICATION_FAILED:
        case SWUPDATE_INIT_UPDATE_FAILED:
            success = true;
        break;

        default:
            success = false;
        break;
    }

    if( success )
    {
        setUpdateCompScreen( true );
        setSwUpdateState( SwUpdateStates::SWUPDATE_INIT_UPDATE_FAILED );
        setSwUpdateInfo( QString( "Software update failed!\n%1" ).arg( m_strCurrentErrorReason ) );
        setProgressRequired( false );
    }

    return success;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SwUpdateController::tryActivateSwUpdateRebootState()
{
    bool success;

    switch ( m_nCurrentState )
    {
        case SWUPDATE_REBOOT:
        case SWUPDATE_UNPACK_APPLICATION:
        case SWUPDATE_STARTUP_FAILED:
            success = true;
        break;

        default:
            success = false;
        break;
    }

    if ( success )
    {
        setUpdateCompScreen( false );
        setSwUpdateState( SwUpdateStates::SWUPDATE_REBOOT );
        setSwUpdateInfo( "Reboot system" );
        setProgressRequired( false );

#ifdef TARGETBUILD
        if ( !system( "reboot" ) )
        {
            // do nothing
        }
#else
        qInfo() << "SW Update State is REBOOT. Only on Host without real Reboot for Test Issues";
        QThread::msleep( 500 );
        setSwUpdateProcess( SWU_PROCESS_NORMAL );
        triggerSwUpdateFileState( SWUPDATE_IDLE );
#endif

    }

    return success;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SwUpdateController::tryActivateStartupCheckState()
{
    bool success;

    switch ( m_nCurrentState )
    {
        case SWUPDATE_INIT:
        case SWUPDATE_STARTUP_CHECK:
            success = true;
        break;

        default:
            success = false;
        break;
    }

    if ( success )
    {
        setUpdateCompScreen( false );
        setProgressRequired( false );

        switch ( m_nCurrentProcess )
        {
            case SWU_PROCESS_NORMAL:
                setSwUpdateInfo( "Smiley startup" );
            break;

            case SWU_PROCESS_UPDATE:
            case SWU_PROCESS_ROLLBACK:
                setSwUpdateInfo( "Continue update after reboot" );
            break;
        }
        clearSwUpdatePackage();
        setSwUpdateState( SwUpdateStates::SWUPDATE_STARTUP_CHECK );

        emit swUpdateStarted();

        triggerSwUpdateFileState( SwUpdateStates::SWUPDATE_CHECK_ROOTFS );
    }

    return success;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SwUpdateController::tryActivateCheckRootFsState()
{
    bool success;

    switch ( m_nCurrentState )
    {
        case SWUPDATE_STARTUP_CHECK:
        case SWUPDATE_CHECK_ROOTFS:
            success = true;
        break;

        default:
            success = false;
        break;
    }

    if( success )
    {
        setUpdateCompScreen( false );
        setSwUpdateState( SwUpdateStates::SWUPDATE_CHECK_ROOTFS );
        setProgressRequired( false );

        switch ( m_nCurrentProcess )
        {
            case SWU_PROCESS_NORMAL:
            break;

            case SWU_PROCESS_UPDATE:
            case SWU_PROCESS_ROLLBACK:
                setSwUpdateInfo("Check image #1");
            break;
        }

        if( checkRootfs() )
        {
            triggerSwUpdateFileState( SwUpdateStates::SWUPDATE_CHECK_INTERNAL_INTERFACE );
        }
        else
        {
            triggerSwUpdateFileState( SwUpdateStates::SWUPDATE_ROOTFS_FAILED );
        }
    }

    return success;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SwUpdateController::tryActivateRootFsFailedtState()
{
    bool success;

    switch ( m_nCurrentState )
    {
        case SWUPDATE_CHECK_ROOTFS:
        case SWUPDATE_ROOTFS_FAILED:
            success = true;
        break;

        default:
            success = false;
        break;
    }

    if( success )
    {
        setUpdateCompScreen( false );
        setProgressRequired( false );
        m_strCurrentErrorReason = "Update image #1 failed";
        setSwUpdateState( SwUpdateStates::SWUPDATE_ROOTFS_FAILED );
        triggerSwUpdateFileState( SwUpdateStates::SWUPDATE_STARTUP_FAILED );
    }

    return success;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SwUpdateController::tryActivateCheckInternalInterfaceState()
{
    bool success;

    switch ( m_nCurrentState )
    {
        case SWUPDATE_CHECK_ROOTFS:
        case SWUPDATE_CHECK_INTERNAL_INTERFACE:
            success = true;
        break;

        default:
            success = false;
        break;
    }

    if( success )
    {
        setUpdateCompScreen( false );
        setProgressRequired( false );
        setSwUpdateState( SwUpdateStates::SWUPDATE_CHECK_INTERNAL_INTERFACE );

        switch ( m_nCurrentProcess )
        {
            case SWU_PROCESS_NORMAL:
            break;

            case SWU_PROCESS_UPDATE:
            case SWU_PROCESS_ROLLBACK:
                setSwUpdateInfo( "Check internal interface" );
            break;
        }

        startPeriodicInterfaceCheck();
    }

    return success;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SwUpdateController::tryActivateInternalInterfaceFailedState()
{
    bool success;

    switch ( m_nCurrentState )
    {
        case SWUPDATE_CHECK_INTERNAL_INTERFACE:
        case SWUPDATE_INTERNAL_INTERFACE_FAILED:
            success = true;
        break;

        default:
            success = false;
        break;
    }

    if( success )
    {
        setUpdateCompScreen( false );
        setProgressRequired( false );
        m_strCurrentErrorReason = "Internal interface failed";
        setSwUpdateState( SwUpdateStates::SWUPDATE_INTERNAL_INTERFACE_FAILED );
        triggerSwUpdateFileState( SwUpdateStates::SWUPDATE_STARTUP_FAILED );
    }

    return success;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SwUpdateController::tryActivateCheckMcuVersionState()
{
    bool success;

    switch ( m_nCurrentState )
    {
        case SWUPDATE_ESP_CHECK_VERSION:
        case SWUPDATE_ESP_VALIDATE_VERSION:
        case SWUPDATE_MCU_CHECK_VERSION:
            success = true;
        break;

        default:
            success = false;
        break;
    }

    if( success )
    {
        setUpdateCompScreen( false );
        setProgressRequired( false );
        setSwUpdateState( SwUpdateStates::SWUPDATE_MCU_CHECK_VERSION );

        switch ( m_nCurrentProcess )
        {
            case SWU_PROCESS_NORMAL:
            break;

            case SWU_PROCESS_UPDATE:
            case SWU_PROCESS_ROLLBACK:
                setSwUpdateInfo("Check MCU version");
            break;
        }

        connect( m_pMainStatemachine->deviceInfoCollector(), &DeviceInfoCollector::mcuInfosChanged, this, &SwUpdateController::checkMcuVersion );

        m_pTimeoutTimer->start(MAX_VERSION_CHECKTIME);

        // check MCU Version;
        if( m_pMainStatemachine->deviceInfoCollector()->mcuInfosCollected() )
        {
            checkMcuVersion();
        }
    }

    return success;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SwUpdateController::tryActivateMcuNotAvailableState()
{
    bool success;

    switch ( m_nCurrentState )
    {
        case SWUPDATE_MCU_CHECK_VERSION:
        case SWUPDATE_MCU_VALIDATE_VERSION:
        case SWUPDATE_MCU_NOT_AVAILABLE:
            success = true;
        break;

        default:
            success = false;
        break;
    }

    if( success )
    {
        m_strCurrentErrorReason = QString( "MCU not available" );
        setUpdateCompScreen( false );
        setProgressRequired( false );
        setSwUpdateState( SwUpdateStates::SWUPDATE_MCU_NOT_AVAILABLE );
        triggerSwUpdateFileState( SwUpdateStates::SWUPDATE_STARTUP_FAILED );
    }

    return success;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SwUpdateController::tryActivateMcuUpdateInProgressState()
{
    bool success;

    switch ( m_nCurrentState )
    {
        case SWUPDATE_IDLE:   // TODO: this state is inserted for test usages
        case SWUPDATE_MCU_CHECK_VERSION:
        case SWUPDATE_MCU_UPDATE_IN_PROGRESS:
            success = true;
        break;

        default:
            success = false;
        break;
    }

    if( success )
    {
        setUpdateCompScreen( false );
        setSwUpdateState( SwUpdateStates::SWUPDATE_MCU_UPDATE_IN_PROGRESS );
        setSwUpdateInfo( "MCU update in progress" );
        setProgressBarAutoTimer( 0 );
        setProgressRequired( true );

        emit swUpdateStarted();

        startMcuSwUpdate();
    }

    return success;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SwUpdateController::tryActivateMcuUpdateFailedState()
{
    bool success;

    switch ( m_nCurrentState )
    {
        case SWUPDATE_MCU_UPDATE_IN_PROGRESS:
        case SWUPDATE_MCU_UPDATE_FAILED:
        case SWUPDATE_MCU_VALIDATE_VERSION:
            success = true;
        break;

        default:
            success = false;
        break;
    }

    if( success )
    {
        m_strCurrentErrorReason = QString( "MCU update failed" );
        setUpdateCompScreen( false );
        setProgressRequired( false );
        setSwUpdateState( SwUpdateStates::SWUPDATE_MCU_UPDATE_FAILED );
        triggerSwUpdateFileState( SwUpdateStates::SWUPDATE_STARTUP_FAILED );
    }

    return success;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SwUpdateController::tryActivateMcuValidateVersionState()
{
    bool success;

    switch ( m_nCurrentState )
    {
        case SWUPDATE_MCU_UPDATE_IN_PROGRESS:
        case SWUPDATE_MCU_VALIDATE_VERSION:
            success = true;
        break;

        default:
            success = false;
        break;
    }

    if( success )
    {
        setUpdateCompScreen( false );
        setProgressRequired( false );
        setSwUpdateState( SwUpdateStates::SWUPDATE_MCU_VALIDATE_VERSION );
        setSwUpdateInfo("Validate MCU version");

        connect( m_pMainStatemachine->deviceInfoCollector(), &DeviceInfoCollector::mcuInfosChanged, this, &SwUpdateController::validateMcuVersion);

        m_pTimeoutTimer->start(MAX_VERSION_CHECKTIME);

        // check MCU Version;
        validateMcuVersion();
    }

    return success;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SwUpdateController::tryActivateEspCheckVersionState()
{
    bool success;

    switch ( m_nCurrentState )
    {
        case SWUPDATE_CHECK_INTERNAL_INTERFACE:
        case SWUPDATE_ESP_CHECK_VERSION:
            success = true;
        break;

        default:
            success = false;
        break;
    }

    if( success )
    {
        setUpdateCompScreen( false );
        setProgressRequired( false );
        setSwUpdateState( SwUpdateStates::SWUPDATE_ESP_CHECK_VERSION );

        switch ( m_nCurrentProcess )
        {
            case SWU_PROCESS_NORMAL:
            break;

            case SWU_PROCESS_UPDATE:
            case SWU_PROCESS_ROLLBACK:
                setSwUpdateInfo("Check ESP version");
            break;
        }

        connect( m_pMainStatemachine->deviceInfoCollector(), &DeviceInfoCollector::espInfosChanged, this, &SwUpdateController::checkEspVersion);
        m_pTimeoutTimer->start(MAX_VERSION_CHECKTIME);

        if( m_pMainStatemachine->deviceInfoCollector()->espInfosCollected() )
        {
            checkEspVersion();
        }
    }

    return success;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SwUpdateController::tryActivateEspNotAvailableState()
{
    bool success;

    switch ( m_nCurrentState )
    {
        case SWUPDATE_ESP_VALIDATE_VERSION:
        case SWUPDATE_ESP_CHECK_VERSION:
        case SWUPDATE_ESP_NOT_AVAILABLE:
            success = true;
        break;

        default:
            success = false;
        break;
    }

    if ( success )
    {
        m_strCurrentErrorReason = QString( "ESP not available" );
        setUpdateCompScreen( false );
        setProgressRequired( false );
        setSwUpdateState( SwUpdateStates::SWUPDATE_ESP_NOT_AVAILABLE );
        triggerSwUpdateFileState( SwUpdateStates::SWUPDATE_STARTUP_FAILED );
    }

    return success;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SwUpdateController::tryActivateEspUpdateInProgressState()
{
    bool success;

    switch ( m_nCurrentState )
    {
        //case SWUPDATE_IDLE:   // for debugging
        case SWUPDATE_ESP_CHECK_VERSION:
        case SWUPDATE_ESP_UPDATE_IN_PROGRESS:
            success = true;
        break;

        default:
            success = false;
        break;
    }

    if( success )
    {
        setUpdateCompScreen( false );
        setSwUpdateState( SwUpdateStates::SWUPDATE_ESP_UPDATE_IN_PROGRESS );
        setSwUpdateInfo( "Wifi board update in progress" );
        setProgressValue( 0 );
        setProgressBarAutoTimer( 0 );
        setProgressRequired( true );
        //emit swUpdateStarted(); // for debugging

        startEspSwUpdate();
    }

    return success;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SwUpdateController::tryActivateEspUpdateFailedState()
{
    bool success;

    switch ( m_nCurrentState )
    {
        case SWUPDATE_ESP_UPDATE_IN_PROGRESS:
        case SWUPDATE_ESP_UPDATE_FAILED:
        case SWUPDATE_ESP_VALIDATE_VERSION:
            success = true;
        break;

        default:
            success = false;
        break;
    }

    if( success )
    {
        m_strCurrentErrorReason = QString( "Wifi board update failed" );
        setUpdateCompScreen( false );
        setProgressRequired( false );
        setSwUpdateState( SwUpdateStates::SWUPDATE_ESP_UPDATE_FAILED );
        triggerSwUpdateFileState( SwUpdateStates::SWUPDATE_STARTUP_FAILED );
    }

    return success;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SwUpdateController::tryActivateEspValidateVersionState()
{
    bool success;

    switch ( m_nCurrentState )
    {
        case SWUPDATE_ESP_UPDATE_IN_PROGRESS:
        case SWUPDATE_ESP_VALIDATE_VERSION:
            success = true;
        break;

        default:
            success = false;
        break;
    }

    if( success )
    {
        setUpdateCompScreen( false );
        setProgressRequired( false );
        setSwUpdateState( SwUpdateStates::SWUPDATE_ESP_VALIDATE_VERSION );
        setSwUpdateInfo( "Validate ESP version" );

        connect( m_pMainStatemachine->deviceInfoCollector(), &DeviceInfoCollector::espInfosChanged, this, &SwUpdateController::validateEspVersion);

        m_pTimeoutTimer->start( MAX_VERSION_CHECKTIME );

        // check ESP Version;
        //qInfo() << "setESPValidateVersionstate -> validateEspVersion ";
        validateEspVersion();
    }

    return success;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SwUpdateController::tryActivateStartUpCompletedState()
{
    bool success;

    switch ( m_nCurrentState )
    {
        case SWUPDATE_STARTUP_COMPLETED:
        case SWUPDATE_MCU_VALIDATE_VERSION:
        case SWUPDATE_MCU_CHECK_VERSION:
            success = true;
        break;

        default:
            success = false;
        break;
    }

    if( success )
    {
        setUpdateCompScreen( false );
        setSwUpdateState( SwUpdateStates::SWUPDATE_STARTUP_COMPLETED );

        switch ( m_nCurrentProcess )
        {
            case SWU_PROCESS_NORMAL:
                triggerSwUpdateFileState( SwUpdateStates::SWUPDATE_IDLE );
            break;

            case SWU_PROCESS_UPDATE:
                triggerSwUpdateFileState( SwUpdateStates::SWUPDATE_UPDATE_COMPLETED );
            break;

            case SWU_PROCESS_ROLLBACK:
                triggerSwUpdateFileState( SwUpdateStates::SWUPDATE_UPDATE_FAILED );
            break;
        }
    }

    return success;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SwUpdateController::tryActivateStartUpFailedState()
{
    bool success;

    switch ( m_nCurrentState )
    {
        case SWUPDATE_STARTUP_FAILED:
        case SWUPDATE_ROOTFS_FAILED:
        case SWUPDATE_INTERNAL_INTERFACE_FAILED:
        case SWUPDATE_MCU_UPDATE_FAILED:
        case SWUPDATE_ESP_UPDATE_FAILED:
        case SWUPDATE_TIMEOUT:
            success = true;
        break;

        default:
            success = false;
        break;
    }

    if( success )
    {
        SwUpdateController::SwUpdateStates oldState = m_nCurrentState;

        setSwUpdateState( SwUpdateStates::SWUPDATE_STARTUP_FAILED );

        switch ( m_nCurrentProcess )
        {
            case SWU_PROCESS_NORMAL:
            case SWU_PROCESS_ROLLBACK:
                setUpdateCompScreen( true );
                setSwUpdateInfo( QString( "Startup failed!\n%1" ).arg( m_strCurrentErrorReason ) );

                triggerSwUpdateFileState( SwUpdateStates::SWUPDATE_FATAL_ERROR );
            break;

            case SWU_PROCESS_UPDATE:
                setUpdateCompScreen( false );
                setSwUpdateProcess( SWU_PROCESS_ROLLBACK );

                int rollbackAppCode = 0;
                int rollBackRootCode = 0;

                if( oldState == SWUPDATE_ROOTFS_FAILED)
                {
                    rollbackAppCode = QProcess::execute("rollback", QStringList() << "app");
                }
                else
                {
                    rollbackAppCode = QProcess::execute("rollback", QStringList() << "app");
                    rollBackRootCode = QProcess::execute("rollback", QStringList() << "rootfs");
                }

                if( rollbackAppCode == -2 || rollbackAppCode == -1 )
                {
                    qCritical() << "Rollback of app has failed !!";
                }

                if( rollBackRootCode == -2 || rollBackRootCode == -1 )
                {
                    qCritical() << "Rollback of rootfs has failed !!";
                }

                triggerSwUpdateFileState( SWUPDATE_REBOOT );
            break;
        }

    }

    return success;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SwUpdateController::tryActivateUpdateCompletedState()
{
    bool success;

    switch ( m_nCurrentState )
    {
        case SWUPDATE_STARTUP_COMPLETED:
        case SWUPDATE_UPDATE_COMPLETED:
            success = true;
        break;

        default:
            success = false;
        break;
    }

    if( success )
    {
        m_pMainStatemachine->sqliteInterface()->addEvent( EkxSqliteTypes::SqliteEventType_SW_UPDATE_PACKAGE,
                                                          m_pMainStatemachine->deviceInfoCollector()->packageSwVersion()
                                                          + "/Build "
                                                          + m_pMainStatemachine->deviceInfoCollector()->packageBuildNo() );

        setSwUpdateProcess( SwUpdateProcesses::SWU_PROCESS_NORMAL );
        setSwUpdateState( SwUpdateStates::SWUPDATE_UPDATE_COMPLETED );
        setSwUpdateInfo( "Software update successfully completed" );

        m_pMainStatemachine->recipeControl()->checkAndConvertRecipeConfig();

        m_pMainStatemachine->settingsStatemachine()->settingsSerializer()->checkAndConvertSettingConfig();

        setUpdateCompScreen( true );
    }

    return success;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SwUpdateController::tryActivateUpdateFailedState()
{
    bool success;

    switch ( m_nCurrentState )
    {
        case SWUPDATE_UPDATE_FAILED:
        case SWUPDATE_STARTUP_COMPLETED:
            success = true;
        break;

        default:
            success = false;
        break;
    }

    if( success )
    {
        setSwUpdateProcess( SwUpdateProcesses::SWU_PROCESS_NORMAL );
        setSwUpdateState( SwUpdateStates::SWUPDATE_UPDATE_FAILED );
        setUpdateCompScreen( true );
        setSwUpdateInfo( QString( "Software update failed.\nRoll-back completed!\n%1" ).arg( m_strCurrentErrorReason ) );
    }

    return success;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SwUpdateController::tryActivateTimeoutState()
{

    bool success;

    switch ( m_nCurrentState )
    {
        case SWUPDATE_MCU_CHECK_VERSION:
        case SWUPDATE_MCU_UPDATE_IN_PROGRESS:
        case SWUPDATE_MCU_VALIDATE_VERSION:
        case SWUPDATE_ESP_CHECK_VERSION:
        case SWUPDATE_ESP_UPDATE_IN_PROGRESS:
        case SWUPDATE_ESP_VALIDATE_VERSION:
            success = true;
        break;

        default:
            success = false;
        break;
    }

    if( success )
    {
        setSwUpdateState( SwUpdateStates::SWUPDATE_TIMEOUT );
        setSwUpdateInfo( "Timeout due to: " + m_strCurrentErrorReason );
        setUpdateCompScreen( false );

        triggerSwUpdateFileState( SWUPDATE_STARTUP_FAILED );
    }

    return success;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SwUpdateController::tryActivateFatalErrorState()
{
    bool success;

    switch ( m_nCurrentState )
    {
        case SWUPDATE_STARTUP_FAILED:
        case SWUPDATE_FATAL_ERROR:
            success = true;
        break;

        default:
            success = false;
        break;
    }

    if( success )
    {
        setSwUpdateState( SwUpdateStates::SWUPDATE_FATAL_ERROR );
        setUpdateCompScreen( true );
        setSwUpdateInfo( QString( "System consistency check failed!\n " + m_strCurrentErrorReason ) );
    }

    return success;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SwUpdateController::swStateFileChanged()
{
    readSwUpdateStateFile( false );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SwUpdateController::storageDirectoryChanged()
{
    m_theSwUpdateFileObserver.removePath( MainStatemachine::rootPath() + SWSTATE_PATH );
    m_theSwUpdateFileObserver.addPath( MainStatemachine::rootPath() + SWSTATE_PATH );

    readSwUpdateStateFile( false );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SwUpdateController::readSwUpdateStateFile(const bool bStartUp )
{
    //qInfo() << "readSwUpdateFile()";

    QFile file( MainStatemachine::rootPath() + SWSTATE_PATH );

    if ( file.open(QIODevice::ReadOnly) )
    {
        QString strJson = file.readAll();
        file.close();

        //qInfo() << strJson;

        QJsonDocument doc = QJsonDocument::fromJson( strJson.toUtf8() );
        QJsonObject obj = doc.object();

        if ( !obj.isEmpty() )
        {
            setSwUpdateProcess( convertStringToSwUpdateProcess( obj[SWUPDATE_JSON_PROCESS].toString() ) );
            m_strCurrentErrorReason       = obj[SWUPDATE_JSON_ERROR].toString();
            //setProgressRequired( obj[SWUPDATE_JSON_PROGREQ].toBool() );
            setProgressValue( obj[SWUPDATE_JSON_PROGVAL].toInt() );

            SwUpdateStates currentState = convertStringToSwUpdateState( obj[SWUPDATE_JSON_STATE].toString() );
            if( bStartUp )
            {
                if ( currentState != SWUPDATE_REBOOT )
                {
                    m_nCurrentProcess = SWU_PROCESS_NORMAL;
                }
                currentState = SWUPDATE_STARTUP_CHECK;
            }

            if ( currentState !=  m_nCurrentState )
            {
                qInfo() << "Update:" << swUpdateProcessString( m_nCurrentProcess ) << swUpdateStateString( currentState );

                processSwUpdateState( currentState );
            }
        }
    }
    else
    {
        writeSwUpdateStateFile( swUpdateProcessString( SWU_PROCESS_NORMAL ),
                                swUpdateStateString( SWUPDATE_IDLE ),
                                "",
                                false,
                                0 );
        readSwUpdateStateFile( bStartUp );
        //emit swUpdateStopped();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SwUpdateController::writeSwUpdateStateFile( const QString &strProcess,
                                                 const QString &strState,
                                                 const QString &strErrorReason,
                                                 const bool bProgressReq,
                                                 const int nProgValue )
{
    bool success = false;
    QFile file( MainStatemachine::rootPath() + SWSTATE_PATH );

    if ( file.open(QIODevice::WriteOnly) )
    {
        QJsonObject swUpdateObj;

        swUpdateObj[SWUPDATE_JSON_PROCESS]   = strProcess;
        swUpdateObj[SWUPDATE_JSON_STATE]     = strState;
        swUpdateObj[SWUPDATE_JSON_ERROR]     = strErrorReason;
        swUpdateObj[SWUPDATE_JSON_PROGREQ]   = bProgressReq;
        swUpdateObj[SWUPDATE_JSON_PROGVAL]   = nProgValue;

        QJsonDocument jsonDoc(swUpdateObj);

        file.write( jsonDoc.toJson() );
        file.flush();
        file.close();

        success = true;
    }

    return success;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SwUpdateController::create()
{
    readSwUpdateStateFile( true );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SwUpdateController::userAcknowlegeCall()
{
    switch ( m_nCurrentState )
    {
        case SWUPDATE_UPDATE_COMPLETED:
        case SWUPDATE_UPDATE_FAILED:
        case SWUPDATE_STARTUP_FAILED:
        case SWUPDATE_INIT_UPDATE_FAILED:
        case SWUPDATE_FATAL_ERROR:
            triggerSwUpdateFileState( SWUPDATE_IDLE );
            emit swUpdateStopped();
        break;

        default:
            qWarning() << "receive acknowledge in illegal state" << swUpdateStateString( m_nCurrentState );
        break;
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SwUpdateController::isIdle() const
{
    return ( m_nCurrentState == SWUPDATE_IDLE );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SwUpdateController::setSwUpdateHeadline(const QString &strHeadline)
{
    if ( m_strSwUpdateHeadline != strHeadline )
    {
        m_strSwUpdateHeadline = strHeadline;
        emit swUpdateHeadlineChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

const QString &SwUpdateController::swUpdateHeadline() const
{
    return m_strSwUpdateHeadline;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SwUpdateController::startEspSwUpdate()
{
    if( m_pMainStatemachine->espDriver()->swUpdateInDriver() )
    {
        connect( m_pMainStatemachine->espDriver()->swUpdateInDriver(), &EspSwUpdateInDriver::transactionIdChanged, this, &SwUpdateController::doEspSwUpdateStep );
        m_pTimeoutTimer->start(MAX_ESP_COM_TIMEOUT);

        QFile file( MainStatemachine::rootPath() + ESP_FIRMWARE + espUpdateFile() );

        if ( file.open( QIODevice::ReadOnly ) )
        {
            m_baEspFirmware = file.readAll();
            file.close();
        }

        if ( !m_baEspFirmware.isEmpty() )
        {
            qInfo() << "startEspSwUpdate: start firmware file '" << file.fileName() << "' upload, size " << file.size();

            m_nEspUpdateRepeatCounter = 0;
            m_pMainStatemachine->espDriver()->swUpdateOutDriver()->setCommand( EspSwUpdateOut::SwUpdateBegin );
            m_pEspUpdateCommandTimer->start( MAX_ESP_UPDATE_BEGIN_TIMEOUT );
            m_pTimeoutTimer->start( MAX_ESP_UPDATE_TIMEOUT );
            m_nEspUpdateChunkNo = -1;
        }
        else
        {
            qCritical() << "startEspSwUpdate: firmware file '" << file.fileName() << "' is empty!";
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SwUpdateController::startMcuSwUpdate()
{
    if ( m_pMainStatemachine->espDriver()->uartDriver()->mcuDriver()->startSwUpdate() )
    {
        m_pTimeoutTimer->start( MAX_MCU_COM_TIMEOUT );
        connect( m_pMainStatemachine->espDriver()->uartDriver()->mcuDriver(), &McuDriver::swUpdateFinished, this, &SwUpdateController::processMcuSwUpdateFinished );
        connect( m_pMainStatemachine->espDriver()->uartDriver()->mcuDriver(), &McuDriver::swUpdateProgress, this, &SwUpdateController::processMcuSwUpdateProgress );
    }
    else
    {
        triggerSwUpdateFileState( SWUPDATE_MCU_UPDATE_FAILED );
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SwUpdateController::setSwUpdateProcess(const SwUpdateController::SwUpdateProcesses nProcess)
{
    if ( m_nCurrentProcess != nProcess )
    {
        m_nCurrentProcess = nProcess;
        emit swuProcessChanged();

        switch ( m_nCurrentProcess )
        {
            case SWU_PROCESS_NORMAL:
                setSwUpdateHeadline( "" );
            break;
            case SWU_PROCESS_UPDATE:
                setSwUpdateHeadline( "Software update:" );
            break;
            case SWU_PROCESS_ROLLBACK:
                setSwUpdateHeadline( "Update roll-back:" );
            break;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

SwUpdateController::SwUpdateProcesses SwUpdateController::swUpdateProcess() const
{
    return m_nCurrentProcess;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SwUpdateController::doEspSwUpdateStep()
{
    if ( !m_pMainStatemachine->espDriver()->swUpdateInDriver() )
    {
        qCritical() << "SwUpdateController::doEspSwUpdateStep() - no interface existing";
        swEspUpdateFailure( false );
    }
    else
    {
        bool success       = m_pMainStatemachine->espDriver()->swUpdateInDriver()->success();
        int  transactionId = m_pMainStatemachine->espDriver()->swUpdateInDriver()->transactionId();
        int  curCommand    = m_pMainStatemachine->espDriver()->swUpdateInDriver()->command();

        if ( !success )
        {
            qWarning() << "SwUpdateController::doEspSwUpdate() - reply with no success";

            switch ( curCommand )
            {
                case EspSwUpdateOut::SwUpdateBegin:
                    qCritical() << "SwUpdateController::doEspSwUpdate() - SW update package begin not accepted - abort";
                    swEspUpdateFailure( false );
                break;

                case EspSwUpdateOut::SwUpdateFinish:
                    qCritical() << "SwUpdateController::doEspSwUpdate() - SW update package not accepted - abort";
                    swEspUpdateFailure( false );
                break;

                case EspSwUpdateOut::SwUpdateData:
                    qCritical() << "SwUpdateController::doEspSwUpdate() - error during writing SW update package - abort";
                    swEspUpdateFailure( false );
                break;

                default:
                    handleEspUpdateCommandTimeout();
                break;
            }
        }
        else if ( transactionId != m_pMainStatemachine->espDriver()->swUpdateOutDriver()->transactionId() )
        {
            qWarning() << "SwUpdateController::doEspSwUpdate() - transaction ID mismatch in reply, expected " << m_pMainStatemachine->espDriver()->swUpdateOutDriver()->transactionId() << ", received " << transactionId;
            // force a greater time delay for repeat - maybe communication is out of sync
            m_pEspUpdateCommandTimer->start( 3 * MAX_ESP_UPDATE_COMMAND_TIMEOUT );
        }
        else if ( curCommand != m_pMainStatemachine->espDriver()->swUpdateOutDriver()->command() )
        {
            qWarning() << "SwUpdateController::doEspSwUpdate() - command mismatch in reply, expected " << m_pMainStatemachine->espDriver()->swUpdateOutDriver()->command() << ", received " << curCommand;
            m_pEspUpdateCommandTimer->start( 3 * MAX_ESP_UPDATE_COMMAND_TIMEOUT );
        }
        else
        {
            //qInfo() << "SwUpdateController::doEspSwUpdateStep()";

            switch ( curCommand )
            {
                case EspSwUpdateOut::SwUpdateBegin:
                case EspSwUpdateOut::SwUpdateData:
                {
                    m_nEspUpdateRepeatCounter = 0;

                    m_nEspUpdateChunkNo++;

                    if ( m_nEspUpdateChunkNo * ESP_UPDATE_CHUNK_SIZE > m_baEspFirmware.length() )
                    {
                        m_pMainStatemachine->espDriver()->swUpdateOutDriver()->setCommand( EspSwUpdateOut::SwUpdateFinish );
                        m_pEspUpdateCommandTimer->start( MAX_ESP_UPDATE_FINISH_TIMEOUT );
                    }
                    else
                    {
                        int startValue = m_nEspUpdateChunkNo * ESP_UPDATE_CHUNK_SIZE;
                        int endValue   = startValue + ESP_UPDATE_CHUNK_SIZE;

                        if( endValue > m_baEspFirmware.length() )
                        {
                            endValue = m_baEspFirmware.length();
                            qInfo() << "calc size: " << ( m_nEspUpdateChunkNo * ESP_UPDATE_CHUNK_SIZE + endValue - startValue ) << " chunks " << m_nEspUpdateChunkNo;
                        }

                        setProgressValue( endValue * 100 / m_baEspFirmware.length() + 1 );

                        //qInfo() << "setData chunk no " << ThisEspUpdateChunkNo << " tid " << ThisEspSwUpdateOut->getCurrentTransactionId();

                        m_pMainStatemachine->espDriver()->swUpdateOutDriver()->setData( m_nEspUpdateChunkNo,
                                                                                        ESP_UPDATE_CHUNK_SIZE,
                                                                                        m_baEspFirmware.mid( startValue, endValue - startValue ) );
                        m_pEspUpdateCommandTimer->start( MAX_ESP_UPDATE_COMMAND_TIMEOUT );
                    }
                    m_pTimeoutTimer->start( MAX_ESP_UPDATE_TIMEOUT );
                }
                break;

                case EspSwUpdateOut::SwUpdateFinish:
                {
                    m_nEspUpdateRepeatCounter = 0;
                    m_pMainStatemachine->espDriver()->swUpdateOutDriver()->setCommand( EspSwUpdateOut::SwUpdateReboot );

                    // we try reconnect after reboot time
                    m_pEspUpdateCommandTimer->start( MAX_ESP_UPDATE_REBOOT_TIMEOUT );
                    m_pTimeoutTimer->start( MAX_ESP_UPDATE_TIMEOUT );
                }
                break;

                case EspSwUpdateOut::SwUpdateReboot:
                {
                    // should never happen
                }
                break;

                default:
                break;
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SwUpdateController::handleEspUpdateCommandTimeout( )
{
    //qInfo() << "SwUpdateController::handleEspUpdateCommandTimeout()";

    if ( !m_pMainStatemachine->espDriver()->swUpdateOutDriver() )
    {
        qCritical() << "handleEspUpdateCommandTimeout() interface lost ThisEspSwUpdateOut";
        swEspUpdateFailure( false );
    }
    else
    {
        int  curCommand = m_pMainStatemachine->espDriver()->swUpdateOutDriver()->command();

        if ( m_nEspUpdateRepeatCounter >= MAX_ESP_UPDATE_REPEATS )
        {
            qCritical() << "handleEspUpdateCommandTimeout() abort update due to reaching max. command repeatitions (cmd=" << curCommand << ")";
            swEspUpdateFailure( false );
        }
        else
        {
            m_nEspUpdateRepeatCounter++;

            switch ( curCommand )
            {
                case EspSwUpdateOut::SwUpdateBegin:
                    qInfo() << "handleEspUpdateCommandTimeout() repeat begin command";
                    m_pMainStatemachine->espDriver()->swUpdateOutDriver()->setCommand( EspSwUpdateOut::SwUpdateBegin );
                    m_pEspUpdateCommandTimer->start( MAX_ESP_UPDATE_BEGIN_TIMEOUT );
                break;

                case EspSwUpdateOut::SwUpdateData:
                {
                    qInfo() << "handleEspUpdateCommandTimeout() repeat data chunk no " << m_nEspUpdateChunkNo;
                    int startValue = m_nEspUpdateChunkNo * ESP_UPDATE_CHUNK_SIZE;
                    int endValue   = startValue + ESP_UPDATE_CHUNK_SIZE;

                    if ( endValue > m_baEspFirmware.length() )
                    {
                        endValue = m_baEspFirmware.length();
                    }

                    m_pMainStatemachine->espDriver()->swUpdateOutDriver()->setData( m_nEspUpdateChunkNo,
                                                                                    ESP_UPDATE_CHUNK_SIZE,
                                                                                    m_baEspFirmware.mid( startValue, endValue - startValue ) );
                    m_pEspUpdateCommandTimer->start( MAX_ESP_UPDATE_COMMAND_TIMEOUT );
                }
                break;

                case EspSwUpdateOut::SwUpdateFinish:
                    qInfo() << "handleEspUpdateCommandTimeout() repeat finish command";
                    m_pMainStatemachine->espDriver()->swUpdateOutDriver()->setCommand( EspSwUpdateOut::SwUpdateFinish );
                    m_pEspUpdateCommandTimer->start( MAX_ESP_UPDATE_FINISH_TIMEOUT );
                break;

                case EspSwUpdateOut::SwUpdateAbort:
                    qInfo() << "handleEspUpdateCommandTimeout() repeat abort command";
                    m_pMainStatemachine->espDriver()->swUpdateOutDriver()->setCommand( EspSwUpdateOut::SwUpdateAbort );
                    m_pEspUpdateCommandTimer->start( MAX_ESP_UPDATE_COMMAND_TIMEOUT );
                break;

                case EspSwUpdateOut::SwUpdateReboot:
                    qInfo() << "handleEspUpdateCommandTimeout() finalize update process";

                    disconnect( m_pMainStatemachine->espDriver()->swUpdateInDriver(), &EspSwUpdateInDriver::transactionIdChanged, this, &SwUpdateController::doEspSwUpdateStep );
                    m_pTimeoutTimer->stop();
                    m_pEspUpdateCommandTimer->stop();

                    triggerSwUpdateFileState( SWUPDATE_ESP_VALIDATE_VERSION );
                break;

                default:
                    qCritical() << "handleEspUpdateCommandTimeout() process illegal command" << curCommand;
                    swEspUpdateFailure( false );
                break;
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SwUpdateController::swEspUpdateFailure( const bool bByTimeout )
{
    qWarning() << "SwUpdateController::swEspUpdateFailure! by timeout = " << bByTimeout;

    //TODO: analyse error case
    m_pEspUpdateCommandTimer->stop();
    m_pTimeoutTimer->stop();
    disconnect( m_pMainStatemachine->espDriver()->swUpdateInDriver(), &EspSwUpdateInDriver::transactionIdChanged, this, &SwUpdateController::doEspSwUpdateStep);

    m_nEspUpdateChunkNo = 0;
    m_pMainStatemachine->espDriver()->swUpdateOutDriver()->setCommand( EspSwUpdateOut::SwUpdateAbort );

    if ( bByTimeout )
    {
        m_strCurrentErrorReason = QString( "Timeout during ESP update" );
        //triggerSwUpdateFileState( SWUPDATE_TIMEOUT );
    }
    else
    {
        //triggerSwUpdateFileState( SWUPDATE_ESP_UPDATE_FAILED );
    }

    triggerSwUpdateFileState( SWUPDATE_ESP_UPDATE_FAILED );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SwUpdateController::processMcuSwUpdateFinished(const bool bSuccess)
{
    disconnect( m_pMainStatemachine->espDriver()->uartDriver()->mcuDriver(), &McuDriver::swUpdateFinished, this, &SwUpdateController::processMcuSwUpdateFinished );
    disconnect( m_pMainStatemachine->espDriver()->uartDriver()->mcuDriver(), &McuDriver::swUpdateProgress, this, &SwUpdateController::processMcuSwUpdateProgress );
    m_pTimeoutTimer->stop();

    if ( bSuccess )
    {
        triggerSwUpdateFileState( SWUPDATE_MCU_VALIDATE_VERSION );
    }
    else
    {
        triggerSwUpdateFileState( SWUPDATE_MCU_UPDATE_FAILED );
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SwUpdateController::processMcuSwUpdateProgress(const quint8 u8ProgressPercent)
{
    m_pTimeoutTimer->start(MAX_MCU_COM_TIMEOUT);
    setProgressValue( static_cast<int>( u8ProgressPercent ) );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SwUpdateController::startPeriodicInterfaceCheck( )
{
    m_pInterfaceCheckTimer->start(500);
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SwUpdateController::handleTimeout( )
{
    switch ( m_nCurrentState )
    {
        case SWUPDATE_ESP_CHECK_VERSION:
            qWarning() << "handleTimeout: timeout SWUPDATE_ESP_CHECK_VERSION";
            disconnect( m_pMainStatemachine->deviceInfoCollector(), &DeviceInfoCollector::espInfosChanged, this, &SwUpdateController::checkEspVersion);
            m_pTimeoutTimer->stop();
            m_strCurrentErrorReason = QString("Timeout: ESP check version");
            triggerSwUpdateFileState( SWUPDATE_TIMEOUT );
        break;

        case SWUPDATE_MCU_CHECK_VERSION:
            qWarning() << "handleTimeout: timeout SWUPDATE_MCU_CHECK_VERSION";
            disconnect( m_pMainStatemachine->deviceInfoCollector(), &DeviceInfoCollector::mcuInfosChanged, this, &SwUpdateController::checkMcuVersion);
            m_pTimeoutTimer->stop();
            m_strCurrentErrorReason = QString("Timeout: MCU check version");
            triggerSwUpdateFileState( SWUPDATE_TIMEOUT/*SWUPDATE_ESP_CHECK_VERSION*/ );
        break;

        case SWUPDATE_ESP_UPDATE_IN_PROGRESS:
            qWarning() << "handleTimeout: timeout SWUPDATE_ESP_UPDATE_IN_PROGRESS";
            swEspUpdateFailure( true );
        break;

        case SWUPDATE_MCU_UPDATE_IN_PROGRESS:
            qWarning() << "handleTimeout: timeout SWUPDATE_MCU_UPDATE_IN_PROGRESS";
            disconnect( m_pMainStatemachine->espDriver()->uartDriver()->mcuDriver(), &McuDriver::swUpdateFinished, this, &SwUpdateController::processMcuSwUpdateFinished );
            disconnect( m_pMainStatemachine->espDriver()->uartDriver()->mcuDriver(), &McuDriver::swUpdateProgress, this, &SwUpdateController::processMcuSwUpdateProgress );
            m_pTimeoutTimer->stop();
            m_strCurrentErrorReason = QString("Timeout: MCU update in progress");
            triggerSwUpdateFileState( SWUPDATE_TIMEOUT );
        break;

        case SWUPDATE_ESP_VALIDATE_VERSION:
            qWarning() << "handleTimeout: timeout SWUPDATE_ESP_VALIDATE_VERSION";
            disconnect( m_pMainStatemachine->deviceInfoCollector(), &DeviceInfoCollector::espInfosChanged, this, &SwUpdateController::validateEspVersion);
            m_pTimeoutTimer->stop();
            m_strCurrentErrorReason = QString("Timeout: ESP validate version");
            triggerSwUpdateFileState( SWUPDATE_TIMEOUT );
        break;

        case SWUPDATE_MCU_VALIDATE_VERSION:
            qWarning() << "handleTimeout: timeout SWUPDATE_MCU_VALIDATE_VERSION";
            disconnect( m_pMainStatemachine->deviceInfoCollector(), &DeviceInfoCollector::mcuInfosChanged, this, &SwUpdateController::validateMcuVersion);
            m_pTimeoutTimer->stop();
            m_strCurrentErrorReason = QString("Timeout: MCU validate version");
            triggerSwUpdateFileState( SWUPDATE_TIMEOUT );
        break;

        default:
            // do nothing
            qWarning() << "handleTimeout: unexpected timeout";
        break;
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SwUpdateController::checkInternalInterface( )
{
#ifndef SIMULATION_BUILD
    if( m_nInterfaceCheckCounter <= MAX_INTERFACE_CHECKS )
    {
        m_nInterfaceCheckCounter++;

        if( m_pMainStatemachine->espDriver()->connectorClient()->isConnected() )
        {
            m_nInterfaceCheckCounter = 0;
            m_pInterfaceCheckTimer->stop();

            qInfo() << "checkInternalInterface: successfull!";
            triggerSwUpdateFileState( SWUPDATE_ESP_CHECK_VERSION );
        }
    }
    else
    {
        m_nInterfaceCheckCounter = 0;
        m_pInterfaceCheckTimer->stop();

        qWarning() << "checkInternalInterface: timeout -> no interface detected";
        triggerSwUpdateFileState( SWUPDATE_INTERNAL_INTERFACE_FAILED );
    }
#else
    m_pInterfaceCheckTimer->stop();
    triggerSwUpdateFileState( SWUPDATE_ESP_CHECK_VERSION );
#endif
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SwUpdateController::checkMcuVersion()
{
    if ( m_pMainStatemachine->isMcuIgnored() )
    {
        disconnect( m_pMainStatemachine->deviceInfoCollector(), &DeviceInfoCollector::mcuInfosChanged, this, &SwUpdateController::checkMcuVersion);
        m_pTimeoutTimer->stop();
        triggerSwUpdateFileState( SWUPDATE_STARTUP_COMPLETED );
    }
    else
    {
        if ( m_pMainStatemachine->deviceInfoCollector()->mcuInfosCollected() )
        {
            disconnect( m_pMainStatemachine->deviceInfoCollector(), &DeviceInfoCollector::mcuInfosChanged, this, &SwUpdateController::checkMcuVersion);
            m_pTimeoutTimer->stop();

            QString mcuUpdateVersion  = mcuUpdateSwVersionString();
            QString mcuCurrentVersion = m_pMainStatemachine->deviceInfoCollector()->mcuSwVersion();

            //qInfo() << "checkMcuVersion to" << mcuUpdateVersion << " from " << mcuCurrentVersion;
            if ( mcuUpdateVersion == mcuCurrentVersion )
            {
                triggerSwUpdateFileState( SWUPDATE_STARTUP_COMPLETED );
            }
            else
            {
                triggerSwUpdateFileState( SWUPDATE_MCU_UPDATE_IN_PROGRESS );
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SwUpdateController::validateMcuVersion()
{
    if ( m_pMainStatemachine->deviceInfoCollector()->mcuInfosCollected() )
    {
        disconnect( m_pMainStatemachine->deviceInfoCollector(), &DeviceInfoCollector::mcuInfosChanged, this, &SwUpdateController::validateMcuVersion);
        m_pTimeoutTimer->stop();

        QString mcuUpdateVersion  = mcuUpdateSwVersionString();
        QString mcuCurrentVersion = m_pMainStatemachine->deviceInfoCollector()->mcuSwVersion();

        //qInfo() << "checkMcuVersion to" << mcuUpdateVersion << " from " << mcuCurrentVersion;

        if ( mcuUpdateVersion == mcuCurrentVersion )
        {
            triggerSwUpdateFileState( SWUPDATE_STARTUP_COMPLETED );
        }
        else
        {
            triggerSwUpdateFileState( SWUPDATE_MCU_UPDATE_FAILED );
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SwUpdateController::checkEspVersion()
{
#ifndef SIMULATION_BUILD
    if ( m_pMainStatemachine->deviceInfoCollector()->espInfosCollected() )
    {
        disconnect( m_pMainStatemachine->deviceInfoCollector(), &DeviceInfoCollector::espInfosChanged, this, &SwUpdateController::checkEspVersion);
        m_pTimeoutTimer->stop();

        // check Esp Version;
        QString espUpdateVersion = espUpdateSwVersionString();
        QString espCurrentVersion = m_pMainStatemachine->deviceInfoCollector()->espSwVersion();

        if (  QString::compare( espUpdateVersion, espCurrentVersion ) == 0 )
        {
            triggerSwUpdateFileState( SWUPDATE_STARTUP_COMPLETED );
            //triggerSwUpdateFileState( SWUPDATE_MCU_CHECK_VERSION );
        }
        else
        {
            triggerSwUpdateFileState( SWUPDATE_ESP_UPDATE_IN_PROGRESS );
        }
    }
#else
    disconnect( m_pMainStatemachine->deviceInfoCollector(), &DeviceInfoCollector::espInfosChanged, this, &SwUpdateController::checkEspVersion);
    m_pTimeoutTimer->stop();
    triggerSwUpdateFileState( SWUPDATE_MCU_CHECK_VERSION );
#endif
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SwUpdateController::validateEspVersion()
{
    if ( m_pMainStatemachine->deviceInfoCollector()->espInfosCollected() )
    {
        disconnect( m_pMainStatemachine->deviceInfoCollector(), &DeviceInfoCollector::espInfosChanged, this, &SwUpdateController::validateEspVersion );
        m_pTimeoutTimer->stop();

        // check Esp Version;
        QString espUpdateVersion = espUpdateSwVersionString();
        QString espCurrentVersion = m_pMainStatemachine->deviceInfoCollector()->espSwVersion();

        //qInfo() << "validateEspVersion /" << espUpdateVersion << "/" << espCurrentVersion;

        if (  QString::compare(espUpdateVersion, espCurrentVersion ) == 0 )
        {
            triggerSwUpdateFileState( SWUPDATE_STARTUP_COMPLETED );
            //triggerSwUpdateFileState( SWUPDATE_MCU_CHECK_VERSION );
        }
        else
        {
            triggerSwUpdateFileState( SWUPDATE_ESP_UPDATE_FAILED );
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SwUpdateController::clearSwUpdatePackage( )
{
    QString path = SWSTORAGE_PATH;
    QDir dir(path);
    dir.setNameFilters(QStringList() << "*.swu");
    dir.setFilter(QDir::Files);
    foreach(QString dirFile, dir.entryList())
    {
        dir.remove(dirFile);
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

QString SwUpdateController::mcuUpdateSwVersionString() const
{
    QFile file( MainStatemachine::rootPath() + MCUCONFIG_PATH );
    QString swVersion = "";

    if ( file.open(QIODevice::ReadOnly) )
    {
        QByteArray ba = file.readAll();
        file.close();

        QJsonDocument doc = QJsonDocument::fromJson(ba);

        QJsonObject obj = doc.object();

        swVersion  = obj[SWUPDATE_JSON_MCU_SWVER].toString();
    }

    return swVersion;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

QString SwUpdateController::espUpdateSwVersionString() const
{
    QFile file( MainStatemachine::rootPath() + ESPCONFIG_PATH );
    QString swVersion = "";

    if ( file.open(QIODevice::ReadOnly) )
    {
        QByteArray ba = file.readAll();
        file.close();

        QJsonDocument doc = QJsonDocument::fromJson(ba);

        QJsonObject obj = doc.object();

        swVersion  = obj[SWUPDATE_JSON_ESP_SWVER].toString();
    }

    return swVersion;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

QString SwUpdateController::espUpdateFile() const
{
    QFile file( MainStatemachine::rootPath() + ESPCONFIG_PATH );
    QString updateFile = "";


    if ( file.open(QIODevice::ReadOnly) )
    {
        QByteArray ba = file.readAll();
        file.close();

        QJsonDocument doc = QJsonDocument::fromJson(ba);

        QJsonObject obj = doc.object();

        updateFile  = obj[SWUPDATE_JSON_ESP_FILE].toString();
    }

    return updateFile;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SwUpdateController::checkRootfs( ) const
{
#ifdef TARGETBUILD
    bool rootfsCheckSuccess = false;

    QString rootFsNomVersion = "";
    QString rootFsCurVersion = "";

    QRegExp reg( "HEM_EKX\\.(\\d+\\-.+)\\-" );

    // file contents are of type:
    // ekx image, VERSION: HEM_EKX.120-rc1-g08d8d12 (built through jenkins@52ef5de26f6f on Thu, 01 Apr 2021 08:19:37 +0000)
    {
        QFile rootfsNomPath( MainStatemachine::rootPath() + ROOTFS_NOM_PATH );
        if ( rootfsNomPath.open(QIODevice::ReadOnly) )
        {
            QString strContent = rootfsNomPath.readAll();
            rootfsNomPath.close();

            if ( reg.indexIn( strContent ) > -1 )
            {
                rootFsNomVersion = reg.cap( 1 );
            }
        }
    }

    {
        QFile rootfsCurPath( MainStatemachine::rootPath() + ROOTFS_CUR_PATH );
        if ( rootfsCurPath.open(QIODevice::ReadOnly) )
        {
            QString strContent = rootfsCurPath.readAll();
            rootfsCurPath.close();

            if ( reg.indexIn( strContent ) > -1 )
            {
                rootFsCurVersion = reg.cap( 1 );
            }
        }
    }

    if ( !rootFsNomVersion.isEmpty()
         && rootFsNomVersion == rootFsCurVersion )
    {
        rootfsCheckSuccess = true;
    }
    else
    {
        qCritical() << "checkRootfs(): failed " << rootFsNomVersion << "!=" << rootFsCurVersion;
    }

    return rootfsCheckSuccess;
#else
    qInfo() << "checkRootfs(): no target build; defaulted to true";

    return true;
#endif
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

SwUpdateController::SwUpdateStates SwUpdateController::convertStringToSwUpdateState(const QString strState ) const
{
    SwUpdateController::SwUpdateStates currentSwUpdateState = SwUpdateStates::SWUPDATE_INIT;

    if(QString::compare(strState, "IDLE" ) == 0)
    {
        currentSwUpdateState = SwUpdateStates::SWUPDATE_IDLE;
    }
    else if(QString::compare(strState, "UPLOAD_IN_PROGRESS" ) == 0)
    {
        currentSwUpdateState = SwUpdateStates::SWUPDATE_UPLOAD_IN_PROGRESS;
    }
    else if(QString::compare(strState, "UPLOAD_COMPLETED" ) == 0)
    {
        currentSwUpdateState = SwUpdateStates::SWUPDATE_UPLOAD_COMPLETED;
    }
    else if(QString::compare(strState, "UPLOAD_FAILED" ) == 0)
    {
        currentSwUpdateState = SwUpdateStates::SWUPDATE_UPLOAD_FAILED;
    }
    else if(QString::compare(strState, "EVALUATE_PACKAGE" ) == 0)
    {
        currentSwUpdateState = SwUpdateStates::SWUPDATE_EVALUATE_PACKAGE;
    }
    else if(QString::compare(strState, "PACKAGE_FAILED" ) == 0)
    {
        currentSwUpdateState = SwUpdateStates::SWUPDATE_PACKAGE_FAILED;
    }
    else if(QString::compare(strState, "PACKAGE_NO_UPGRADE" ) == 0)
    {
        currentSwUpdateState = SwUpdateStates::SWUPDATE_PACKAGE_NO_UPGRADE;
    }
    else if(QString::compare(strState, "UNPACK_ROOTFS" ) == 0)
    {
        currentSwUpdateState = SwUpdateStates::SWUPDATE_UNPACK_ROOTFS;
    }
    else if(QString::compare(strState, "UNPACK_ROOTFS_FAILED" ) == 0)
    {
        currentSwUpdateState = SwUpdateStates::SWUPDATE_UNPACK_ROOTFS_FAILED;
    }
    else if(QString::compare(strState, "UNPACK_APPLICATION" ) == 0)
    {
        currentSwUpdateState = SwUpdateStates::SWUPDATE_UNPACK_APPLICATION;
    }
    else if(QString::compare(strState, "UNPACK_APPLICATION_FAILED" ) == 0)
    {
        currentSwUpdateState = SwUpdateStates::SWUPDATE_UNPACK_APPLICATION_FAILED;
    }
    else if(QString::compare(strState, "INIT_UPDATE_FAILED" ) == 0)
    {
        currentSwUpdateState = SwUpdateStates::SWUPDATE_INIT_UPDATE_FAILED;
    }
    else if(QString::compare(strState, "REBOOT" ) == 0)
    {
        currentSwUpdateState = SwUpdateStates::SWUPDATE_REBOOT;
    }
    else if(QString::compare(strState, "STARTUP_CHECK" ) == 0)
    {
        currentSwUpdateState = SwUpdateStates::SWUPDATE_STARTUP_CHECK;
    }
    else if(QString::compare(strState, "CHECK_ROOTFS" ) == 0)
    {
        currentSwUpdateState = SwUpdateStates::SWUPDATE_CHECK_ROOTFS;
    }
    else if(QString::compare(strState, "ROOTFS_FAILED" ) == 0)
    {
        currentSwUpdateState = SwUpdateStates::SWUPDATE_ROOTFS_FAILED;
    }
    else if(QString::compare(strState, "CHECK_INTERNAL_INTERFACE" ) == 0)
    {
        currentSwUpdateState = SwUpdateStates::SWUPDATE_CHECK_INTERNAL_INTERFACE;
    }
    else if(QString::compare(strState, "INTERNAL_INTERFACE_FAILED" ) == 0)
    {
        currentSwUpdateState = SwUpdateStates::SWUPDATE_INTERNAL_INTERFACE_FAILED;
    }
    else if(QString::compare(strState, "MCU_CHECK_VERSION" ) == 0)
    {
        currentSwUpdateState = SwUpdateStates::SWUPDATE_MCU_CHECK_VERSION;
    }
    else if(QString::compare(strState, "MCU_UPDATE_IN_PROGRESS" ) == 0)
    {
        currentSwUpdateState = SwUpdateStates::SWUPDATE_MCU_UPDATE_IN_PROGRESS;
    }
    else if(QString::compare(strState, "MCU_UPDATE_FAILED" ) == 0)
    {
        currentSwUpdateState = SwUpdateStates::SWUPDATE_MCU_UPDATE_FAILED;
    }
    else if(QString::compare(strState, "MCU_VALIDATE_VERSION" ) == 0)
    {
        currentSwUpdateState = SwUpdateStates::SWUPDATE_MCU_VALIDATE_VERSION;
    }
    else if(QString::compare(strState, "ESP_CHECK_VERSION" ) == 0)
    {
        currentSwUpdateState = SwUpdateStates::SWUPDATE_ESP_CHECK_VERSION;
    }
    else if(QString::compare(strState, "ESP_UPDATE_IN_PROGRESS" ) == 0)
    {
        currentSwUpdateState = SwUpdateStates::SWUPDATE_ESP_UPDATE_IN_PROGRESS;
    }
    else if(QString::compare(strState, "ESP_UPDATE_FAILED" ) == 0)
    {
        currentSwUpdateState = SwUpdateStates::SWUPDATE_ESP_UPDATE_FAILED;
    }
    else if(QString::compare(strState, "ESP_VALIDATE_VERSION" ) == 0)
    {
        currentSwUpdateState = SwUpdateStates::SWUPDATE_ESP_VALIDATE_VERSION;
    }
    else if(QString::compare(strState, "STARTUP_COMPLETED" ) == 0)
    {
        currentSwUpdateState = SwUpdateStates::SWUPDATE_STARTUP_COMPLETED;
    }
    else if(QString::compare(strState, "STARTUP_FAILED" ) == 0)
    {
        currentSwUpdateState = SwUpdateStates::SWUPDATE_STARTUP_FAILED;
    }
    else if(QString::compare(strState, "UPDATE_COMPLETED" ) == 0)
    {
        currentSwUpdateState = SwUpdateStates::SWUPDATE_UPDATE_COMPLETED;
    }
    else if(QString::compare(strState, "UPDATE_FAILED" ) == 0)
    {
        currentSwUpdateState = SwUpdateStates::SWUPDATE_UPDATE_FAILED;
    }
    else if(QString::compare(strState, "UPDATE_TIMEOUT" ) == 0)
    {
        currentSwUpdateState = SwUpdateStates::SWUPDATE_TIMEOUT;
    }
    else if(QString::compare(strState, "FATAL_ERROR" ) == 0)
    {
        currentSwUpdateState = SwUpdateStates::SWUPDATE_FATAL_ERROR;
    }
    else
    {
        qCritical() << "unknown state detected" << strState;
    }

    return currentSwUpdateState;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

SwUpdateController::SwUpdateProcesses SwUpdateController::convertStringToSwUpdateProcess( const QString &strProcess ) const
{
    SwUpdateProcesses nProcess = SWU_PROCESS_NORMAL;

    if ( strProcess == "Normal" )
    {
        nProcess = SWU_PROCESS_NORMAL;
    }
    else if ( strProcess == "Update" )
    {
        nProcess = SWU_PROCESS_UPDATE;
    }
    else if ( strProcess == "Rollback" )
    {
        nProcess = SWU_PROCESS_ROLLBACK;
    }

    return nProcess;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SwUpdateController::progressRequired() const
{
    return m_bCurrentProgressRequired;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

int SwUpdateController::progressValue() const
{
    return m_nCurrentProgressValue;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SwUpdateController::triggerSwUpdateFileState( const SwUpdateController::SwUpdateStates nNextState )
{
    writeSwUpdateStateFile( swUpdateProcessString( m_nCurrentProcess ),
                            swUpdateStateString( nNextState ),
                            m_strCurrentErrorReason,
                            m_bCurrentProgressRequired,
                            m_nCurrentProgressValue );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

QString SwUpdateController::swUpdateStateString(const SwUpdateController::SwUpdateStates nState) const
{
    QString strState = "UNKNOWN";

    switch ( nState )
    {
        case SWUPDATE_INIT:
            strState = "INIT";
        break;
        case SWUPDATE_EVALUATE_PACKAGE:
            strState = "EVALUATE_PACKAGE";
        break;
        case SWUPDATE_PACKAGE_NO_UPGRADE:
            strState = "PACKAGE_NO_UPGRAD";
        break;
        case SWUPDATE_IDLE:
            strState = "IDLE";
        break;
        case SWUPDATE_UPLOAD_IN_PROGRESS:
            strState = "UPLOAD_IN_PROGRESS";
        break;
        case SWUPDATE_UPLOAD_COMPLETED:
            strState = "UPLOAD_COMPLETED";
        break;
        case SWUPDATE_UPLOAD_FAILED:
            strState = "UPLOAD_FAILED";
        break;
        case SWUPDATE_PACKAGE_FAILED:
            strState = "PACKAGE_FAILED";
        break;
        case SWUPDATE_UNPACK_ROOTFS:
            strState = "UNPACK_ROOTFS";
        break;
        case SWUPDATE_UNPACK_ROOTFS_FAILED:
            strState = "UNPACK_ROOTFS_FAILED";
        break;
        case SWUPDATE_UNPACK_APPLICATION:
            strState = "UNPACK_APPLICATION";
        break;
        case SWUPDATE_UNPACK_APPLICATION_FAILED:
            strState = "UNPACK_APPLICATION_FAILED";
        break;
        case SWUPDATE_INIT_UPDATE_FAILED:
            strState = "INIT_UPDATE_FAILED";
        break;
        case SWUPDATE_REBOOT:
            strState = "REBOOT";
        break;
        case SWUPDATE_STARTUP_CHECK:
            strState = "STARTUP_CHECK";
        break;
        case SWUPDATE_CHECK_ROOTFS:
            strState = "CHECK_ROOTFS";
        break;
        case SWUPDATE_ROOTFS_FAILED:
            strState = "ROOTFS_FAILED";
        break;
        case SWUPDATE_CHECK_INTERNAL_INTERFACE:
            strState = "CHECK_INTERNAL_INTERFACE";
        break;
        case SWUPDATE_INTERNAL_INTERFACE_FAILED:
            strState = "INTERNAL_INTERFACE_FAILED";
        break;
        case SWUPDATE_MCU_CHECK_VERSION:
            strState = "MCU_CHECK_VERSION";
        break;
        case SWUPDATE_MCU_NOT_AVAILABLE:
            strState = "MCU_NOT_AVAILABLE";
        break;
        case SWUPDATE_MCU_UPDATE_IN_PROGRESS:
            strState = "MCU_UPDATE_IN_PROGRESS";
        break;
        case SWUPDATE_MCU_UPDATE_FAILED:
            strState = "MCU_UPDATE_FAILED";
        break;
        case SWUPDATE_MCU_VALIDATE_VERSION:
            strState = "MCU_VALIDATE_VERSION";
        break;
        case SWUPDATE_ESP_CHECK_VERSION:
            strState = "ESP_CHECK_VERSION";
        break;
        case SWUPDATE_ESP_NOT_AVAILABLE:
            strState = "ESP_NOT_AVAILABLE";
        break;
        case SWUPDATE_ESP_UPDATE_IN_PROGRESS:
            strState = "ESP_UPDATE_IN_PROGRESS";
        break;
        case SWUPDATE_ESP_UPDATE_FAILED:
            strState = "ESP_UPDATE_FAILED";
        break;
        case SWUPDATE_ESP_VALIDATE_VERSION:
            strState = "ESP_VALIDATE_VERSION";
        break;
        case SWUPDATE_STARTUP_FAILED:
            strState = "STARTUP_FAILED";
        break;
        case SWUPDATE_STARTUP_COMPLETED:
            strState = "STARTUP_COMPLETED";
        break;
        case SWUPDATE_UPDATE_COMPLETED:
            strState = "UPDATE_COMPLETED";
        break;
        case SWUPDATE_UPDATE_FAILED:
            strState = "UPDATE_FAILED";
        break;
        case SWUPDATE_TIMEOUT:
            strState = "UPDATE_TIMEOUT";
        break;
        case SWUPDATE_FATAL_ERROR:
            strState = "FATAL_ERROR";
        break;

        default:
            qCritical( "Unknown state not implemented" );
        break;

    }

    return strState;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

QString SwUpdateController::swUpdateProcessString( const SwUpdateController::SwUpdateProcesses nProcess ) const
{
    QString strProcess = "UNKNOWN";

    switch ( nProcess )
    {
        case SWU_PROCESS_NORMAL:
            strProcess = "Normal";
        break;
        case SWU_PROCESS_UPDATE:
            strProcess = "Update";
        break;
        case SWU_PROCESS_ROLLBACK:
            strProcess = "Rollback";
        break;

        default:
            qCritical( "Unknown process not implemented" );
        break;

    }

    return strProcess;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////


