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
#include <QtQml>
#include <QtDebug>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QDir>
#include <QProcess>
#include <QThread>

#include "RecipeControl.h"
#include "SettingsSerializer.h"
#include "DeviceInfoCollector.h"
#include "EspSwUpdateDriver.h"
#include "EspMcuDriver.h"
#include "McuDriver.h"
#include "MainStatemachine.h"
#include "SqliteInterface.h"
#include "StandbyController.h"
#include "NotificationCenter.h"
#include "Notification.h"
#include "LockableFile.h"
#include "EspDriver.h"

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#define SWSTORAGE_PATH                  "/storage/"
#define SWSTATE_PATH                    "/storage/SoftwareUpdateState.json"
#define ESPCONFIG_PATH                  "/usr/local/img/EspFirmwareConfig.json"
#define ESP_FIRMWARE                    "/usr/local/img/"
#define MCU_FW_CONFIG_PATH              "/usr/local/img/McuFirmwareConfig.json"
#define ROOTFS_NOM_PATH                 "/storage/rootfs_version"
#define ROOTFS_CUR_PATH                 "/etc/issue"

#define NOT_VALID                       "Invalid"

#define MAX_ESP_INTERFACE_CHECKS        ( 5 )
#define MAX_MCU_INTERFACE_CHECKS        ( 5 )
#define MAX_VERSION_CHECKTIME           ( 10000 )
#define MAX_ESP_COM_TIMEOUT             ( 20000 )
#define MAX_MCU_COM_TIMEOUT             ( 20000 )
#define MAX_ESP_UPDATE_TIMEOUT          ( 20000 )
#define MAX_ESP_UPDATE_REPEATS          10
#define MAX_ESP_UPDATE_BEGIN_TIMEOUT    5000
#define MAX_ESP_UPDATE_FINISH_TIMEOUT   15000
#define MAX_ESP_UPDATE_COMMAND_TIMEOUT  300
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

SwUpdateController::SwUpdateController( QObject * pParent )
    : QObject( pParent )
    , m_pEspInterfaceCheckTimer( new QTimer( this ) )
    , m_pMcuInterfaceCheckTimer( new QTimer( this ) )
    , m_pTimeoutTimer( new QTimer( this ) )
    , m_pEspUpdateCommandTimer( new QTimer( this ) )
{
    MainStatemachine::ensureExistingDirectory( MainStatemachine::rootPath() + SWSTATE_PATH );
    MainStatemachine::ensureExistingDirectory( MainStatemachine::rootPath() + ESPCONFIG_PATH );
    MainStatemachine::ensureExistingDirectory( MainStatemachine::rootPath() + MCU_FW_CONFIG_PATH );
    MainStatemachine::ensureExistingDirectory( MainStatemachine::rootPath() + ROOTFS_NOM_PATH );
    MainStatemachine::ensureExistingDirectory( MainStatemachine::rootPath() + ROOTFS_CUR_PATH );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

SwUpdateController::~SwUpdateController()
{
    m_theSwUpdateFileObserver.removePath( SWSTATE_PATH );

    delete m_pEspInterfaceCheckTimer;
    m_pEspInterfaceCheckTimer = nullptr;

    delete m_pMcuInterfaceCheckTimer;
    m_pMcuInterfaceCheckTimer = nullptr;

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

void SwUpdateController::create( MainStatemachine * pMainStatemachine )
{
    m_pMainStatemachine = pMainStatemachine;

    m_acycNotificationStateFileCorrupted.create( pMainStatemachine->notificationCenter(),
                                                 EkxSqliteTypes::SqliteNotificationType::SqliteNotificationType_SW_UPDATE_STATE_FILE_CORRUPTED );

    connect( m_pEspInterfaceCheckTimer, &QTimer::timeout, this, &SwUpdateController::checkEspInterface );
    connect( m_pMcuInterfaceCheckTimer, &QTimer::timeout, this, &SwUpdateController::checkMcuInterface );
    connect( m_pTimeoutTimer, &QTimer::timeout, this, &SwUpdateController::handleTimeout );

    m_pEspUpdateCommandTimer->setInterval( MAX_ESP_UPDATE_COMMAND_TIMEOUT );
    m_pEspUpdateCommandTimer->setSingleShot( true );
    connect( m_pEspUpdateCommandTimer, &QTimer::timeout, this, &SwUpdateController::handleEspUpdateCommandTimeout );

    m_theSwUpdateFileObserver.addPath( MainStatemachine::rootPath() + SWSTORAGE_PATH );
    m_theSwUpdateFileObserver.addPath( MainStatemachine::rootPath() + SWSTATE_PATH );
    connect( &m_theSwUpdateFileObserver, &QFileSystemWatcher::fileChanged, this, &SwUpdateController::swStateFileChanged );
    connect( &m_theSwUpdateFileObserver, &QFileSystemWatcher::directoryChanged, this, &SwUpdateController::storageDirectoryChanged );

    readSwUpdateStateFile( true );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SwUpdateController::setProgressRequired( const bool bRequired )
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

void SwUpdateController::setProgressValue( const int nValue )
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

void SwUpdateController::setProgressBarAutoTimer( const int nTimeoutSec )
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

void SwUpdateController::setSwUpdateState( const SwUpdateController::SwUpdateStates nState )
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

bool SwUpdateController::processSwUpdateState( const SwUpdateStates nState )
{
    bool success = true;

    qInfo() << "processSwUpdateState()" << nState;

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
        case SWUPDATE_UPLOAD_ABORTED:
            success = tryActivateUploadAbortedState();
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
            success = tryActivateUnpackApplicationFailedState();
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
            success = tryActivateRootFsFailedState();
            break;

        case SWUPDATE_ESP_CHECK_INTERFACE:
            success = tryActivateEspCheckInterfaceState();
            break;
        case SWUPDATE_ESP_INTERFACE_FAILED:
            success = tryActivateEspInterfaceFailedState();
            break;
        case SWUPDATE_ESP_CHECK_VERSION:
            success = tryActivateEspCheckVersionState();
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

        case SWUPDATE_MCU_CHECK_INTERFACE:
            success = tryActivateMcuCheckInterfaceState();
            break;
        case SWUPDATE_MCU_INTERFACE_FAILED:
            success = tryActivateMcuInterfaceFailedState();
            break;
        case SWUPDATE_MCU_CHECK_VERSION:
            success = tryActivateMcuCheckVersionState();
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
        case SWUPDATE_INIT:
        default:
            success = false;
    }

    if ( !success )
    {
        qWarning() << "processSwUpdateState: Abort SW Update or startup";
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

const QString & SwUpdateController::swUpdateInfo() const
{
    return m_strSwUpdateInfo;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SwUpdateController::setSwUpdateInfo( const QString & strInfo )
{
    if ( m_strSwUpdateInfo != strInfo )
    {
        m_strSwUpdateInfo = strInfo;

        emit swUpdateInfoChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SwUpdateController::isUpdateCompScreen() const
{
    return m_bUpdateCompScreen;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SwUpdateController::setUpdateCompScreen( const bool bNewUpdateCompScreen )
{
    if ( m_bUpdateCompScreen != bNewUpdateCompScreen )
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

    if ( success )
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
        //case SWUPDATE_INIT_UPDATE_FAILED:
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
        case SWUPDATE_IDLE:
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

    if ( success )
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
        case SWUPDATE_UPLOAD_ABORTED:
        case SWUPDATE_UPLOAD_IN_PROGRESS:
        case SWUPDATE_UPLOAD_COMPLETED:
            success = true;
            break;

        default:
            success = false;
            break;
    }

    if ( success )
    {
        setUpdateCompScreen( false );
        m_strCurrentErrorReason = QString( "Package Upload failed" );
        setSwUpdateState( SwUpdateStates::SWUPDATE_UPLOAD_FAILED );
        setProgressRequired( false );

        //triggerSwUpdateFileState( SwUpdateStates::SWUPDATE_INIT_UPDATE_FAILED );

        m_pMainStatemachine->notificationCenter()->raiseNotification( EkxSqliteTypes::SqliteNotificationType::SqliteNotificationType_SW_UPDATE_UPLOAD_FAILED );

        triggerSwUpdateFileState( SWUPDATE_IDLE );
        emit swUpdateStopped();
    }

    return success;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SwUpdateController::tryActivateUploadAbortedState()
{
    bool success;

    switch ( m_nCurrentState )
    {
        case SWUPDATE_UPLOAD_FAILED:
        case SWUPDATE_UPLOAD_IN_PROGRESS:
        case SWUPDATE_UPLOAD_ABORTED:
            success = true;
            break;

        default:
            success = false;
            break;
    }

    if ( success )
    {
        setUpdateCompScreen( false );
        m_strCurrentErrorReason = QString( "Package Upload aborted" );
        setSwUpdateState( SwUpdateStates::SWUPDATE_UPLOAD_ABORTED );
        setProgressRequired( false );

        //triggerSwUpdateFileState( SwUpdateStates::SWUPDATE_INIT_UPDATE_FAILED );

        m_pMainStatemachine->notificationCenter()->raiseNotification( EkxSqliteTypes::SqliteNotificationType::SqliteNotificationType_SW_UPDATE_UPLOAD_ABORTED );

        triggerSwUpdateFileState( SWUPDATE_IDLE );
        emit swUpdateStopped();
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
        case SWUPDATE_IDLE:
        case SWUPDATE_EVALUATE_PACKAGE:
        case SWUPDATE_UPLOAD_IN_PROGRESS:
        case SWUPDATE_UPLOAD_COMPLETED:
            success = true;
            break;

        default:
            success = false;
            break;
    }

    if ( success )
    {
        setUpdateCompScreen( false );
        setSwUpdateState( SwUpdateStates::SWUPDATE_EVALUATE_PACKAGE );
        setSwUpdateInfo( "Checking update package" );
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
        case SWUPDATE_IDLE:
        case SWUPDATE_EVALUATE_PACKAGE:
        case SWUPDATE_PACKAGE_FAILED:
            success = true;
            break;

        default:
            success = false;
            break;
    }

    if ( success )
    {
        setUpdateCompScreen( false );
        m_strCurrentErrorReason = QString( "Update package corrupt" );
        setSwUpdateState( SwUpdateStates::SWUPDATE_PACKAGE_FAILED );
        setProgressRequired( false );
        //triggerSwUpdateFileState( SwUpdateStates::SWUPDATE_INIT_UPDATE_FAILED );

        m_pMainStatemachine->notificationCenter()->raiseNotification( EkxSqliteTypes::SqliteNotificationType::SqliteNotificationType_SW_UPDATE_PACKAGE_FAILED );

        triggerSwUpdateFileState( SWUPDATE_IDLE );
        emit swUpdateStopped();
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
        case SWUPDATE_IDLE:
        case SWUPDATE_EVALUATE_PACKAGE:
        case SWUPDATE_PACKAGE_NO_UPGRADE:
            success = true;
            break;

        default:
            success = false;
            break;
    }

    if ( success )
    {
        setUpdateCompScreen( false );
        m_strCurrentErrorReason = QString( "Update package is no upgrade" );
        setSwUpdateState( SwUpdateStates::SWUPDATE_PACKAGE_NO_UPGRADE );
        setProgressRequired( false );
        //triggerSwUpdateFileState( SwUpdateStates::SWUPDATE_INIT_UPDATE_FAILED );

        m_pMainStatemachine->notificationCenter()->raiseNotification( EkxSqliteTypes::SqliteNotificationType::SqliteNotificationType_SW_UPDATE_PACKAGE_NO_UPGRADE );

        triggerSwUpdateFileState( SWUPDATE_IDLE );
        emit swUpdateStopped();
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
        case SWUPDATE_IDLE:
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

    if ( success )
    {
        setUpdateCompScreen( false );
        setSwUpdateState( SwUpdateStates::SWUPDATE_UNPACK_ROOTFS );
        setSwUpdateInfo( "Unpack image #1 of 2" );
        setProgressRequired( true );
        setProgressValue( 0 );
        setProgressBarAutoTimer( 0 );
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

    if ( success )
    {
        setUpdateCompScreen( false );
        m_strCurrentErrorReason = QString( "Unpacking image #1 failed" );
        setSwUpdateState( SwUpdateStates::SWUPDATE_UNPACK_ROOTFS_FAILED );
        setProgressRequired( false );
        //triggerSwUpdateFileState( SwUpdateStates::SWUPDATE_INIT_UPDATE_FAILED );

        m_pMainStatemachine->notificationCenter()->raiseNotification( EkxSqliteTypes::SqliteNotificationType::SqliteNotificationType_SW_UPDATE_UNPACK_ROOTFS_FAILED );

        triggerSwUpdateFileState( SWUPDATE_IDLE );
        emit swUpdateStopped();
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

    if ( success )
    {
        setUpdateCompScreen( false );
        setSwUpdateState( SwUpdateStates::SWUPDATE_UNPACK_APPLICATION );
        setSwUpdateInfo( "Unpack image #2 of 2" );
        setProgressRequired( true );
        setProgressValue( 0 );
        setProgressBarAutoTimer( 0 );
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

    if ( success )
    {
        setUpdateCompScreen( false );
        m_strCurrentErrorReason = QString( "Unpacking image #2 failed" );
        setSwUpdateState( SwUpdateStates::SWUPDATE_UNPACK_APPLICATION_FAILED );
        setProgressRequired( false );
        //triggerSwUpdateFileState( SwUpdateStates::SWUPDATE_INIT_UPDATE_FAILED );

        m_pMainStatemachine->notificationCenter()->raiseNotification( EkxSqliteTypes::SqliteNotificationType::SqliteNotificationType_SW_UPDATE_UNPACK_APPLICATION_FAILED );

        triggerSwUpdateFileState( SWUPDATE_IDLE );
        emit swUpdateStopped();
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

        QThread::msleep( 2000 );  // give time to recognize the message

#ifdef TARGETBUILD
        if ( !system( "reboot" ) )
        {
            // do nothing
        }
#else
        qInfo() << "SW Update State is REBOOT. Only on Host without real Reboot for Test Issues";
        //QThread::msleep( 500 );
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
                setSwUpdateInfo( "Grinder startup" );
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

    if ( success )
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
                setSwUpdateInfo( "Check image #1" );
                break;
        }

        if ( checkRootfs() )
        {
            triggerSwUpdateFileState( SwUpdateStates::SWUPDATE_ESP_CHECK_INTERFACE );
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

bool SwUpdateController::tryActivateRootFsFailedState()
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

    if ( success )
    {
        setUpdateCompScreen( false );
        setProgressRequired( false );
        m_strCurrentErrorReason = "Update image #1 failed";
        setSwUpdateState( SwUpdateStates::SWUPDATE_ROOTFS_FAILED );
        triggerSwUpdateFileState( SwUpdateStates::SWUPDATE_STARTUP_FAILED );

        //        m_pMainStatemachine->notificationCenter()->raiseNotification( EkxSqliteTypes::SqliteErrorTypes::SqliteNotificationType_STARTUP_ROOTFS_FAILED );

        //        triggerSwUpdateFileState( SWUPDATE_IDLE );
        //        emit swUpdateStopped();
    }

    return success;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SwUpdateController::tryActivateEspCheckInterfaceState()
{
    bool success;

    switch ( m_nCurrentState )
    {
        case SWUPDATE_CHECK_ROOTFS:
        case SWUPDATE_ROOTFS_FAILED:
        case SWUPDATE_ESP_CHECK_INTERFACE:
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
        setSwUpdateState( SwUpdateStates::SWUPDATE_ESP_CHECK_INTERFACE );

        switch ( m_nCurrentProcess )
        {
            case SWU_PROCESS_NORMAL:
                break;

            case SWU_PROCESS_UPDATE:
            case SWU_PROCESS_ROLLBACK:
                setSwUpdateInfo( "Check internal interface" );
                break;
        }

        startPeriodicEspInterfaceCheck();
    }

    return success;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SwUpdateController::tryActivateEspInterfaceFailedState()
{
    bool success;

    switch ( m_nCurrentState )
    {
        case SWUPDATE_ESP_CHECK_INTERFACE:
        case SWUPDATE_ESP_INTERFACE_FAILED:
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
        m_strCurrentErrorReason = "Internal interface failed";

        m_pMainStatemachine->notificationCenter()->raiseNotification( EkxSqliteTypes::SqliteNotificationType::SqliteNotificationType_ESP_INTERFACE_FAILED );

        setSwUpdateState( SwUpdateStates::SWUPDATE_ESP_INTERFACE_FAILED );
        triggerSwUpdateFileState( SwUpdateStates::SWUPDATE_MCU_CHECK_INTERFACE );
    }

    return success;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SwUpdateController::tryActivateMcuCheckInterfaceState()
{
    bool success;

    switch ( m_nCurrentState )
    {
        case SWUPDATE_ESP_CHECK_VERSION:
        case SWUPDATE_ESP_VALIDATE_VERSION:
        case SWUPDATE_ESP_INTERFACE_FAILED:
        case SWUPDATE_MCU_CHECK_INTERFACE:
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
        setSwUpdateState( SwUpdateStates::SWUPDATE_MCU_CHECK_INTERFACE );

        switch ( m_nCurrentProcess )
        {
            case SWU_PROCESS_NORMAL:
                break;

            case SWU_PROCESS_UPDATE:
            case SWU_PROCESS_ROLLBACK:
                setSwUpdateInfo( "Check MCU interface" );
                break;
        }

        startPeriodicMcuInterfaceCheck();
    }

    return success;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SwUpdateController::tryActivateMcuInterfaceFailedState()
{
    bool success;

    switch ( m_nCurrentState )
    {
        case SWUPDATE_MCU_CHECK_INTERFACE:
        case SWUPDATE_MCU_INTERFACE_FAILED:
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
        m_strCurrentErrorReason = "MCU interface failed";

        m_pMainStatemachine->notificationCenter()->raiseNotification( EkxSqliteTypes::SqliteNotificationType::SqliteNotificationType_MCU_INTERFACE_FAILED );

        setSwUpdateState( SwUpdateStates::SWUPDATE_MCU_INTERFACE_FAILED );
        triggerSwUpdateFileState( SwUpdateStates::SWUPDATE_STARTUP_FAILED );
    }

    return success;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SwUpdateController::tryActivateMcuCheckVersionState()
{
    bool success;

    switch ( m_nCurrentState )
    {
        case SWUPDATE_MCU_CHECK_INTERFACE:
        case SWUPDATE_MCU_CHECK_VERSION:
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
        setSwUpdateState( SwUpdateStates::SWUPDATE_MCU_CHECK_VERSION );

        switch ( m_nCurrentProcess )
        {
            case SWU_PROCESS_NORMAL:
                break;

            case SWU_PROCESS_UPDATE:
            case SWU_PROCESS_ROLLBACK:
                setSwUpdateInfo( "Check MCU version" );
                break;
        }

        connect( m_pMainStatemachine->deviceInfoCollector(), &DeviceInfoCollector::mcuInfosChanged, this, &SwUpdateController::checkMcuVersion );

        m_pTimeoutTimer->start( MAX_VERSION_CHECKTIME );

        // check MCU Version;
        if ( m_pMainStatemachine->deviceInfoCollector()->mcuInfosCollected() )
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

bool SwUpdateController::tryActivateMcuUpdateInProgressState()
{
    bool success;

    switch ( m_nCurrentState )
    {
#ifdef USE_TRIGGERED_MCU_UPDATE
        case SWUPDATE_IDLE:   // TODO: this state is inserted for test usages from web site
#endif
        case SWUPDATE_MCU_CHECK_VERSION:
        case SWUPDATE_MCU_UPDATE_IN_PROGRESS:
            success = true;
            break;

        default:
            success = false;
            break;
    }

    if ( success )
    {
        setUpdateCompScreen( false );
        setSwUpdateState( SwUpdateStates::SWUPDATE_MCU_UPDATE_IN_PROGRESS );
        setSwUpdateInfo( "MCU update in progress" );
        setProgressBarAutoTimer( 0 );
        setProgressRequired( true );

#ifdef USE_TRIGGERED_MCU_UPDATE
        emit swUpdateStarted();   // TODO: this state is inserted for test usages from web site
#endif

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

    if ( success )
    {
        m_strCurrentErrorReason = QString( "MCU update failed" );
        setUpdateCompScreen( false );
        setProgressRequired( false );
        setSwUpdateState( SwUpdateStates::SWUPDATE_MCU_UPDATE_FAILED );
        m_pMainStatemachine->notificationCenter()->raiseNotification( EkxSqliteTypes::SqliteNotificationType::SqliteNotificationType_SW_UPDATE_MCU_UPDATE_FAILED );
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

    if ( success )
    {
        setUpdateCompScreen( false );
        setProgressRequired( false );
        setSwUpdateState( SwUpdateStates::SWUPDATE_MCU_VALIDATE_VERSION );
        setSwUpdateInfo( "Validate MCU version" );

        connect( m_pMainStatemachine->deviceInfoCollector(), &DeviceInfoCollector::mcuInfosChanged, this, &SwUpdateController::validateMcuVersion );

        m_pTimeoutTimer->start( MAX_VERSION_CHECKTIME );

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
        case SWUPDATE_ESP_CHECK_INTERFACE:
        case SWUPDATE_ESP_CHECK_VERSION:
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
        setSwUpdateState( SwUpdateStates::SWUPDATE_ESP_CHECK_VERSION );

        switch ( m_nCurrentProcess )
        {
            case SWU_PROCESS_NORMAL:
                break;

            case SWU_PROCESS_UPDATE:
            case SWU_PROCESS_ROLLBACK:
                setSwUpdateInfo( "Check ESP version" );
                break;
        }

        connect( m_pMainStatemachine->deviceInfoCollector(), &DeviceInfoCollector::espInfosChanged, this, &SwUpdateController::checkEspVersion );
        m_pTimeoutTimer->start( MAX_VERSION_CHECKTIME );

        if ( m_pMainStatemachine->deviceInfoCollector()->espInfosCollected() )
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

bool SwUpdateController::tryActivateEspUpdateInProgressState()
{
    bool success;

    switch ( m_nCurrentState )
    {
#ifdef USE_TRIGGERED_ESP_UPDATE
        case SWUPDATE_IDLE:   // TODO: this state is inserted for test usages from web site
#endif
        case SWUPDATE_ESP_CHECK_VERSION:
        case SWUPDATE_ESP_UPDATE_IN_PROGRESS:
            success = true;
            break;

        default:
            success = false;
            break;
    }

    if ( success )
    {
        setUpdateCompScreen( false );
        setSwUpdateState( SwUpdateStates::SWUPDATE_ESP_UPDATE_IN_PROGRESS );
        setSwUpdateInfo( "Driver board update in progress" );
        setProgressValue( 0 );
        setProgressBarAutoTimer( 0 );
        setProgressRequired( true );

#ifdef USE_TRIGGERED_ESP_UPDATE
        emit swUpdateStarted();   // TODO: this state is inserted for test usages from web site
#endif

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

    if ( success )
    {
        m_strCurrentErrorReason = QString( "Driver board update failed" );
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

    if ( success )
    {
        setUpdateCompScreen( false );
        setProgressRequired( false );
        setSwUpdateState( SwUpdateStates::SWUPDATE_ESP_VALIDATE_VERSION );
        setSwUpdateInfo( "Validate ESP version" );

        connect( m_pMainStatemachine->deviceInfoCollector(), &DeviceInfoCollector::espInfosChanged, this, &SwUpdateController::validateEspVersion );

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
        case SWUPDATE_ESP_VALIDATE_VERSION:
        case SWUPDATE_ESP_CHECK_VERSION:
            success = true;
            break;

        default:
            success = false;
            break;
    }

    if ( success )
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
        case SWUPDATE_ESP_INTERFACE_FAILED:
        case SWUPDATE_ESP_UPDATE_FAILED:
        case SWUPDATE_MCU_UPDATE_FAILED:
        case SWUPDATE_TIMEOUT:
            success = true;
            break;

        default:
            success = false;
            break;
    }

    if ( success )
    {
        SwUpdateController::SwUpdateStates oldState = m_nCurrentState;

        setSwUpdateState( SwUpdateStates::SWUPDATE_STARTUP_FAILED );

        switch ( m_nCurrentProcess )
        {
//#ifdef USE_TRIGGERED_ESP_UPDATE
//            case SWU_PROCESS_UPDATE:
//#endif
            case SWU_PROCESS_NORMAL:
                setUpdateCompScreen( false );
                setSwUpdateInfo( QString( "Startup failed!\n%1" ).arg( m_strCurrentErrorReason ) );

                triggerSwUpdateFileState( SWUPDATE_IDLE );
                emit swUpdateStopped();
                break;

            case SWU_PROCESS_ROLLBACK:
                setUpdateCompScreen( false );
                setSwUpdateInfo( QString( "Startup failed!\n%1" ).arg( m_strCurrentErrorReason ) );

                m_pMainStatemachine->notificationCenter()->raiseNotification( EkxSqliteTypes::SqliteNotificationType::SqliteNotificationType_ROLLBACK_STARTUP_FAILED );
                triggerSwUpdateFileState( SWUPDATE_IDLE );
                emit swUpdateStopped();
                break;

//#ifndef USE_TRIGGERED_ESP_UPDATE
            case SWU_PROCESS_UPDATE:
                setUpdateCompScreen( false );
                setSwUpdateProcess( SWU_PROCESS_ROLLBACK );

                m_pMainStatemachine->notificationCenter()->raiseNotification( EkxSqliteTypes::SqliteNotificationType::SqliteNotificationType_SW_UPDATE_STARTUP_FAILED );

                int rollbackAppCode  = 0;
                int rollBackRootCode = 0;

                if ( oldState == SWUPDATE_ROOTFS_FAILED )
                {
                    rollbackAppCode = QProcess::execute( "rollback", QStringList() << "app" );
                }
                else
                {
                    rollbackAppCode  = QProcess::execute( "rollback", QStringList() << "app" );
                    rollBackRootCode = QProcess::execute( "rollback", QStringList() << "rootfs" );
                }

                if ( rollbackAppCode == -2
                     || rollbackAppCode == -1 )
                {
                    qCritical() << "tryActivateStartUpFailedState(): Rollback of app has failed !!";
                    m_pMainStatemachine->notificationCenter()->raiseNotification( EkxSqliteTypes::SqliteNotificationType::SqliteNotificationType_SW_UPDATE_ROLLBACK_APP_FAILED );
                }

                if ( rollBackRootCode == -2
                     || rollBackRootCode == -1 )
                {
                    qCritical() << "tryActivateStartUpFailedState(): Rollback of rootfs has failed !!";
                    m_pMainStatemachine->notificationCenter()->raiseNotification( EkxSqliteTypes::SqliteNotificationType::SqliteNotificationType_SW_UPDATE_ROLLBACK_ROOTFS_FAILED );
                }

                triggerSwUpdateFileState( SWUPDATE_REBOOT );
                break;
//#endif
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

    if ( success )
    {
        qInfo() << "tryActivateUpdateCompletedState";
        m_pMainStatemachine->sqliteInterface()->addEvent( EkxSqliteTypes::SqliteEventType_SW_UPDATE_PACKAGE,
                                                          m_pMainStatemachine->deviceInfoCollector()->packageSwVersion()
                                                          + "/Build "
                                                          + m_pMainStatemachine->deviceInfoCollector()->packageBuildNo() );

        setSwUpdateProcess( SwUpdateProcesses::SWU_PROCESS_NORMAL );
        setSwUpdateState( SwUpdateStates::SWUPDATE_UPDATE_COMPLETED );
        setSwUpdateInfo( "Software update successfully completed" );

        m_pMainStatemachine->recipeControl()->checkAndConvertRecipeConfig();

        m_pMainStatemachine->settingsSerializer()->checkAndConvertSettingConfig();

        //setUpdateCompScreen( true );
        setUpdateCompScreen( false );

        m_pMainStatemachine->notificationCenter()->raiseNotification( EkxSqliteTypes::SqliteNotificationType::SqliteNotificationType_SW_UPDATE_COMPLETED );
        triggerSwUpdateFileState( SwUpdateStates::SWUPDATE_IDLE );
        emit swUpdateStopped();
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

    if ( success )
    {
        setSwUpdateProcess( SwUpdateProcesses::SWU_PROCESS_NORMAL );
        setSwUpdateState( SwUpdateStates::SWUPDATE_UPDATE_FAILED );
        //setUpdateCompScreen( true );
        //setSwUpdateInfo( QString( "Software update failed.\nRoll-back completed!\n%1" ).arg( m_strCurrentErrorReason ) );

        m_pMainStatemachine->notificationCenter()->raiseNotification( EkxSqliteTypes::SqliteNotificationType::SqliteNotificationType_SW_UPDATE_ROLLED_BACK );
        triggerSwUpdateFileState( SwUpdateStates::SWUPDATE_IDLE );
        emit swUpdateStopped();
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

    if ( success )
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

//bool SwUpdateController::tryActivateFatalErrorState()
//{
//    bool success;

//    switch ( m_nCurrentState )
//    {
//        case SWUPDATE_STARTUP_FAILED:
//        case SWUPDATE_FATAL_ERROR:
//            success = true;
//        break;

//        default:
//            success = false;
//        break;
//    }

//    if( success )
//    {
//        setSwUpdateState( SwUpdateStates::SWUPDATE_FATAL_ERROR );
//        setUpdateCompScreen( true );
//        setSwUpdateInfo( QString( "System consistency check failed!\n " + m_strCurrentErrorReason ) );
//    }

//    return success;
//}

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

void SwUpdateController::readSwUpdateStateFile( const bool bStartUp )
{
    //qInfo() << "readSwUpdateFile()";

    LockableFile file( MainStatemachine::rootPath() + SWSTATE_PATH );

    if ( file.open( QIODevice::ReadOnly ) )
    {
        QString strJson = file.readAll();
        file.close();

        //qInfo() << strJson;

        QJsonDocument doc = QJsonDocument::fromJson( strJson.toUtf8() );
        QJsonObject   obj = doc.object();

        if ( !obj.isEmpty() )
        {
            setSwUpdateProcess( convertStringToSwUpdateProcess( obj[SWUPDATE_JSON_PROCESS].toString() ) );
            m_strCurrentErrorReason = obj[SWUPDATE_JSON_ERROR].toString();
            //setProgressRequired( obj[SWUPDATE_JSON_PROGREQ].toBool() );
            setProgressValue( obj[SWUPDATE_JSON_PROGVAL].toInt() );

            SwUpdateStates currentState = convertStringToSwUpdateState( obj[SWUPDATE_JSON_STATE].toString() );
            if ( bStartUp )
            {
                if ( currentState != SWUPDATE_REBOOT )
                {
                    m_nCurrentProcess = SWU_PROCESS_NORMAL;
                }
                currentState = SWUPDATE_STARTUP_CHECK;
            }

            if ( currentState != m_nCurrentState )
            {
                qInfo() << "Update:" << swUpdateProcessString( m_nCurrentProcess ) << swUpdateStateString( currentState );

                processSwUpdateState( currentState );
            }

            m_acycNotificationStateFileCorrupted.deactivate();
        }
        else
        {
            m_acycNotificationStateFileCorrupted.activate();

            // should never happen, but already observed an empty state file!!!
            if ( bStartUp )
            {
                writeSwUpdateStateFile( swUpdateProcessString( SWU_PROCESS_NORMAL ),
                                        swUpdateStateString( SWUPDATE_IDLE ),
                                        "",
                                        false,
                                        0 );
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
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SwUpdateController::writeSwUpdateStateFile( const QString & strProcess,
                                                 const QString & strState,
                                                 const QString & strErrorReason,
                                                 const bool      bProgressReq,
                                                 const int       nProgValue )
{
    bool         success = false;
    LockableFile file( MainStatemachine::rootPath() + SWSTATE_PATH );

    if ( file.open( QIODevice::WriteOnly ) )
    {
        QJsonObject swUpdateObj;

        swUpdateObj[SWUPDATE_JSON_PROCESS] = strProcess;
        swUpdateObj[SWUPDATE_JSON_STATE]   = strState;
        swUpdateObj[SWUPDATE_JSON_ERROR]   = strErrorReason;
        swUpdateObj[SWUPDATE_JSON_PROGREQ] = bProgressReq;
        swUpdateObj[SWUPDATE_JSON_PROGVAL] = nProgValue;

        QJsonDocument jsonDoc( swUpdateObj );

        file.write( jsonDoc.toJson() );
        file.flush();
        file.close();

        success = true;

        qInfo() << "writeSwUpdateStateFile" << strProcess << strState << nProgValue;
    }

    return success;
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
            //case SWUPDATE_INIT_UPDATE_FAILED:
            //case SWUPDATE_FATAL_ERROR:
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
    return m_nCurrentState == SWUPDATE_IDLE;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SwUpdateController::setSwUploadCompletedState()
{
    writeSwUpdateStateFile( swUpdateProcessString( SWU_PROCESS_UPDATE ),
                            swUpdateStateString( SWUPDATE_UPLOAD_COMPLETED ),
                            "",
                            false,
                            0 );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SwUpdateController::setSwUpdateHeadline( const QString & strHeadline )
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

const QString & SwUpdateController::swUpdateHeadline() const
{
    return m_strSwUpdateHeadline;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SwUpdateController::startEspSwUpdate()
{
    if ( m_pMainStatemachine->espDriver()->swUpdateDriver() )
    {
        connect( m_pMainStatemachine->espDriver()->swUpdateDriver(), &EspSwUpdateDriver::statusChanged, this, &SwUpdateController::doEspSwUpdateStep );
        m_pTimeoutTimer->start( MAX_ESP_COM_TIMEOUT );

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
            m_pMainStatemachine->espDriver()->swUpdateDriver()->sendCmd( EkxProtocol::Payload::SwUpdateCmd::Start );
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
    if ( m_pMainStatemachine->espDriver()->espMcuDriver()->mcuDriver()->startSwUpdate() )
    {
        m_pTimeoutTimer->start( MAX_MCU_COM_TIMEOUT );
        connect( m_pMainStatemachine->espDriver()->espMcuDriver()->mcuDriver(), &McuDriver::swUpdateFinished, this, &SwUpdateController::processMcuSwUpdateFinished );
        connect( m_pMainStatemachine->espDriver()->espMcuDriver()->mcuDriver(), &McuDriver::swUpdateProgress, this, &SwUpdateController::processMcuSwUpdateProgress );
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

void SwUpdateController::setSwUpdateProcess( const SwUpdateController::SwUpdateProcesses nProcess )
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
    bool                              bStatusSuccess       = m_pMainStatemachine->espDriver()->swUpdateDriver()->statusSuccess();
    int                               nStatusTransactionId = m_pMainStatemachine->espDriver()->swUpdateDriver()->statusTransactionId();
    EkxProtocol::Payload::SwUpdateCmd nStatusCommand       = m_pMainStatemachine->espDriver()->swUpdateDriver()->statusCmd();

    if ( !bStatusSuccess )
    {
        qWarning() << "SwUpdateController::doEspSwUpdate() - receive reply with no success";

        switch ( nStatusCommand )
        {
            case EkxProtocol::Payload::SwUpdateCmd::Start:
                qCritical() << "SwUpdateController::doEspSwUpdateStep() - SW update package begin not accepted - abort";
                swEspUpdateFailure( false );
                break;

            case EkxProtocol::Payload::SwUpdateCmd::Finish:
                qCritical() << "SwUpdateController::doEspSwUpdateStep() - SW update package not accepted - abort";
                swEspUpdateFailure( false );
                break;

            case EkxProtocol::Payload::SwUpdateCmd::Data:
                qCritical() << "SwUpdateController::doEspSwUpdateStep() - error during writing SW update package - abort";
                swEspUpdateFailure( false );
                break;

            default:
                qCritical() << "SwUpdateController::doEspSwUpdateStep() - no switch case -> handleEspUpdateCommandTimeout";
                handleEspUpdateCommandTimeout();
                break;
        }
    }
    else if ( nStatusTransactionId != m_pMainStatemachine->espDriver()->swUpdateDriver()->sendTransactionId() )
    {
        qWarning() << "SwUpdateController::doEspSwUpdateStep() - transaction ID mismatch in reply, expected " << m_pMainStatemachine->espDriver()->swUpdateDriver()->sendTransactionId() << ", received " << nStatusTransactionId;
        // force a greater time delay for repeat - maybe communication is out of sync
        m_pEspUpdateCommandTimer->start( 3 * MAX_ESP_UPDATE_COMMAND_TIMEOUT );
    }
    else if ( nStatusCommand != m_pMainStatemachine->espDriver()->swUpdateDriver()->sendCmd() )
    {
        qWarning() << "SwUpdateController::doEspSwUpdateStep() - command mismatch in reply, expected " << static_cast<int>( m_pMainStatemachine->espDriver()->swUpdateDriver()->sendCmd() ) << ", received " << static_cast<int>( nStatusCommand );
        m_pEspUpdateCommandTimer->start( 3 * MAX_ESP_UPDATE_COMMAND_TIMEOUT );
    }
    else
    {
        //qInfo() << "SwUpdateController::doEspSwUpdateStep()";

        switch ( nStatusCommand )
        {
            case EkxProtocol::Payload::SwUpdateCmd::Start:
            case EkxProtocol::Payload::SwUpdateCmd::Data:
            {
                m_nEspUpdateRepeatCounter = 0;

                m_nEspUpdateChunkNo++;

                if ( m_nEspUpdateChunkNo * ESP_UPDATE_CHUNK_SIZE > m_baEspFirmware.length() )
                {
                    m_pMainStatemachine->espDriver()->swUpdateDriver()->sendCmd( EkxProtocol::Payload::SwUpdateCmd::Finish );
                    m_pEspUpdateCommandTimer->start( MAX_ESP_UPDATE_FINISH_TIMEOUT );
                }
                else
                {
                    int startValue = m_nEspUpdateChunkNo * ESP_UPDATE_CHUNK_SIZE;
                    int endValue   = startValue + ESP_UPDATE_CHUNK_SIZE;

                    if ( endValue > m_baEspFirmware.length() )
                    {
                        endValue = m_baEspFirmware.length();
                        qInfo() << "calc size: " << ( m_nEspUpdateChunkNo * ESP_UPDATE_CHUNK_SIZE + endValue - startValue ) << " chunks " << m_nEspUpdateChunkNo;
                    }

                    setProgressValue( endValue * 100 / m_baEspFirmware.length() + 1 );

                    //qInfo() << "setData chunk no " << ThisEspUpdateChunkNo << " tid " << ThisEspSwUpdateOut->getCurrentTransactionId();

                    m_pMainStatemachine->espDriver()->swUpdateDriver()->sendChunk( m_nEspUpdateChunkNo,
                                                                                   m_baEspFirmware.mid( startValue, endValue - startValue ) );
                    m_pEspUpdateCommandTimer->start( MAX_ESP_UPDATE_COMMAND_TIMEOUT );
                }
                m_pTimeoutTimer->start( MAX_ESP_UPDATE_TIMEOUT );
            }
            break;

            case EkxProtocol::Payload::SwUpdateCmd::Finish:
            {
                qInfo() << "SwUpdateController::doEspSwUpdateStep() - SwUpdateFinish";

                m_nEspUpdateRepeatCounter = 0;
                m_pMainStatemachine->espDriver()->swUpdateDriver()->sendCmd( EkxProtocol::Payload::SwUpdateCmd::Reboot );

                // we try reconnect after reboot time
                m_pEspUpdateCommandTimer->start( MAX_ESP_UPDATE_REBOOT_TIMEOUT );
                m_pTimeoutTimer->start( MAX_ESP_UPDATE_TIMEOUT );
            }
            break;

            case EkxProtocol::Payload::SwUpdateCmd::Reboot:
            {
                // should never happen
            }
            break;

            default:
                break;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SwUpdateController::handleEspUpdateCommandTimeout()
{
    qInfo() << "SwUpdateController::handleEspUpdateCommandTimeout()";

    EkxProtocol::Payload::SwUpdateCmd curCommand = m_pMainStatemachine->espDriver()->swUpdateDriver()->sendCmd();

    if ( m_nEspUpdateRepeatCounter >= MAX_ESP_UPDATE_REPEATS )
    {
        qCritical() << "handleEspUpdateCommandTimeout() abort update due to reaching max. command repeatitions (cmd=" << static_cast<int>( curCommand ) << ")";
        swEspUpdateFailure( false );
    }
    else
    {
        m_nEspUpdateRepeatCounter++;

        switch ( curCommand )
        {
            case EkxProtocol::Payload::SwUpdateCmd::Start:
                qInfo() << "handleEspUpdateCommandTimeout() repeat begin command";
                m_pMainStatemachine->espDriver()->swUpdateDriver()->sendCmd( EkxProtocol::Payload::SwUpdateCmd::Start );
                m_pEspUpdateCommandTimer->start( MAX_ESP_UPDATE_BEGIN_TIMEOUT );
                break;

            case EkxProtocol::Payload::SwUpdateCmd::Data:
            {
                qInfo() << "handleEspUpdateCommandTimeout() repeat data chunk no " << m_nEspUpdateChunkNo;
                int startValue = m_nEspUpdateChunkNo * ESP_UPDATE_CHUNK_SIZE;
                int endValue   = startValue + ESP_UPDATE_CHUNK_SIZE;

                if ( endValue > m_baEspFirmware.length() )
                {
                    endValue = m_baEspFirmware.length();
                }

                m_pMainStatemachine->espDriver()->swUpdateDriver()->sendChunk( m_nEspUpdateChunkNo,
                                                                               m_baEspFirmware.mid( startValue, endValue - startValue ) );
                m_pEspUpdateCommandTimer->start( MAX_ESP_UPDATE_COMMAND_TIMEOUT );
            }
            break;

            case EkxProtocol::Payload::SwUpdateCmd::Finish:
                qInfo() << "handleEspUpdateCommandTimeout() repeat finish command";
                m_pMainStatemachine->espDriver()->swUpdateDriver()->sendCmd( EkxProtocol::Payload::SwUpdateCmd::Finish );
                m_pEspUpdateCommandTimer->start( MAX_ESP_UPDATE_FINISH_TIMEOUT );
                break;

            case EkxProtocol::Payload::SwUpdateCmd::Abort:
                qInfo() << "handleEspUpdateCommandTimeout() repeat abort command";
                m_pMainStatemachine->espDriver()->swUpdateDriver()->sendCmd( EkxProtocol::Payload::SwUpdateCmd::Abort );
                m_pEspUpdateCommandTimer->start( MAX_ESP_UPDATE_COMMAND_TIMEOUT );
                break;

            case EkxProtocol::Payload::SwUpdateCmd::Reboot:
                qInfo() << "handleEspUpdateCommandTimeout() finalize update process";

                disconnect( m_pMainStatemachine->espDriver()->swUpdateDriver(), &EspSwUpdateDriver::statusChanged, this, &SwUpdateController::doEspSwUpdateStep );
                m_pTimeoutTimer->stop();
                m_pEspUpdateCommandTimer->stop();

                triggerSwUpdateFileState( SWUPDATE_ESP_VALIDATE_VERSION );
                break;

            default:
                qCritical() << "handleEspUpdateCommandTimeout() process illegal command" << static_cast<int>( curCommand );
                swEspUpdateFailure( false );
                break;
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
    disconnect( m_pMainStatemachine->espDriver()->swUpdateDriver(), &EspSwUpdateDriver::sendTransactionIdChanged, this, &SwUpdateController::doEspSwUpdateStep );

    m_nEspUpdateChunkNo = 0;
    m_pMainStatemachine->espDriver()->swUpdateDriver()->sendCmd( EkxProtocol::Payload::SwUpdateCmd::Abort );

    if ( bByTimeout )
    {
        m_strCurrentErrorReason = QString( "Timeout during ESP update" );
        //triggerSwUpdateFileState( SWUPDATE_TIMEOUT );
        // timeout already handled as error notification
    }
    else
    {
        //triggerSwUpdateFileState( SWUPDATE_ESP_UPDATE_FAILED );
        m_pMainStatemachine->notificationCenter()->raiseNotification( EkxSqliteTypes::SqliteNotificationType::SqliteNotificationType_SW_UPDATE_ESP_UPDATE_FAILED );
    }

    triggerSwUpdateFileState( SWUPDATE_ESP_UPDATE_FAILED );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SwUpdateController::processMcuSwUpdateFinished( const bool bSuccess )
{
    disconnect( m_pMainStatemachine->espDriver()->espMcuDriver()->mcuDriver(), &McuDriver::swUpdateFinished, this, &SwUpdateController::processMcuSwUpdateFinished );
    disconnect( m_pMainStatemachine->espDriver()->espMcuDriver()->mcuDriver(), &McuDriver::swUpdateProgress, this, &SwUpdateController::processMcuSwUpdateProgress );
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

void SwUpdateController::processMcuSwUpdateProgress( const quint8 u8ProgressPercent )
{
    m_pTimeoutTimer->start( MAX_MCU_COM_TIMEOUT );
    setProgressValue( static_cast<int>( u8ProgressPercent ) );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SwUpdateController::startPeriodicEspInterfaceCheck()
{
    qInfo() << "startPeriodicEspInterfaceCheck()";
    m_nEspInterfaceCheckCounter = 0;
    m_pEspInterfaceCheckTimer->start( 500 );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SwUpdateController::startPeriodicMcuInterfaceCheck()
{
    qInfo() << "startPeriodicMcuInterfaceCheck()";
    m_nMcuInterfaceCheckCounter = 0;
    m_pMcuInterfaceCheckTimer->start( 500 );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SwUpdateController::handleTimeout()
{
    switch ( m_nCurrentState )
    {
        case SWUPDATE_ESP_CHECK_VERSION:
            qWarning() << "handleTimeout: timeout SWUPDATE_ESP_CHECK_VERSION";
            disconnect( m_pMainStatemachine->deviceInfoCollector(), &DeviceInfoCollector::espInfosChanged, this, &SwUpdateController::checkEspVersion );
            m_pTimeoutTimer->stop();
            m_strCurrentErrorReason = QString( "Timeout: ESP check version" );
            triggerSwUpdateFileState( SWUPDATE_TIMEOUT );
            m_pMainStatemachine->notificationCenter()->raiseNotification( EkxSqliteTypes::SqliteNotificationType::SqliteNotificationType_STARTUP_ESP_CHECK_VERSION_TIMEOUT );
            break;

        case SWUPDATE_MCU_CHECK_VERSION:
            qWarning() << "handleTimeout: timeout SWUPDATE_MCU_CHECK_VERSION";
            disconnect( m_pMainStatemachine->deviceInfoCollector(), &DeviceInfoCollector::mcuInfosChanged, this, &SwUpdateController::checkMcuVersion );
            m_pTimeoutTimer->stop();
            m_strCurrentErrorReason = QString( "Timeout: MCU check version" );
            triggerSwUpdateFileState( SWUPDATE_TIMEOUT /*SWUPDATE_ESP_CHECK_VERSION*/ );
            m_pMainStatemachine->notificationCenter()->raiseNotification( EkxSqliteTypes::SqliteNotificationType::SqliteNotificationType_STARTUP_MCU_CHECK_VERSION_TIMEOUT );
            break;

        case SWUPDATE_ESP_UPDATE_IN_PROGRESS:
            qWarning() << "handleTimeout: timeout SWUPDATE_ESP_UPDATE_IN_PROGRESS";
            swEspUpdateFailure( true );
            m_pMainStatemachine->notificationCenter()->raiseNotification( EkxSqliteTypes::SqliteNotificationType::SqliteNotificationType_SW_UPDATE_ESP_UPDATE_TIMEOUT );
            break;

        case SWUPDATE_MCU_UPDATE_IN_PROGRESS:
            qWarning() << "handleTimeout: timeout SWUPDATE_MCU_UPDATE_IN_PROGRESS";
            disconnect( m_pMainStatemachine->espDriver()->espMcuDriver()->mcuDriver(), &McuDriver::swUpdateFinished, this, &SwUpdateController::processMcuSwUpdateFinished );
            disconnect( m_pMainStatemachine->espDriver()->espMcuDriver()->mcuDriver(), &McuDriver::swUpdateProgress, this, &SwUpdateController::processMcuSwUpdateProgress );
            m_pTimeoutTimer->stop();
            m_strCurrentErrorReason = QString( "Timeout: MCU update in progress" );
            triggerSwUpdateFileState( SWUPDATE_TIMEOUT );
            m_pMainStatemachine->notificationCenter()->raiseNotification( EkxSqliteTypes::SqliteNotificationType::SqliteNotificationType_SW_UPDATE_MCU_UPDATE_TIMEOUT );
            break;

        case SWUPDATE_ESP_VALIDATE_VERSION:
            qWarning() << "handleTimeout: timeout SWUPDATE_ESP_VALIDATE_VERSION";
            disconnect( m_pMainStatemachine->deviceInfoCollector(), &DeviceInfoCollector::espInfosChanged, this, &SwUpdateController::validateEspVersion );
            m_pTimeoutTimer->stop();
            m_strCurrentErrorReason = QString( "Timeout: ESP validate version" );
            triggerSwUpdateFileState( SWUPDATE_TIMEOUT );
            m_pMainStatemachine->notificationCenter()->raiseNotification( EkxSqliteTypes::SqliteNotificationType::SqliteNotificationType_SW_UPDATE_ESP_VALIDATE_VERSION_TIMEOUT );
            break;

        case SWUPDATE_MCU_VALIDATE_VERSION:
            qWarning() << "handleTimeout: timeout SWUPDATE_MCU_VALIDATE_VERSION";
            disconnect( m_pMainStatemachine->deviceInfoCollector(), &DeviceInfoCollector::mcuInfosChanged, this, &SwUpdateController::validateMcuVersion );
            m_pTimeoutTimer->stop();
            m_strCurrentErrorReason = QString( "Timeout: MCU validate version" );
            triggerSwUpdateFileState( SWUPDATE_TIMEOUT );
            m_pMainStatemachine->notificationCenter()->raiseNotification( EkxSqliteTypes::SqliteNotificationType::SqliteNotificationType_SW_UPDATE_MCU_VALIDATE_VERSION_TIMEOUT );
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

void SwUpdateController::checkEspInterface()
{
    qInfo() << "SwUpdateController::checkEspInterface()";
#ifdef TARGETBUILD
    if ( m_nEspInterfaceCheckCounter <= MAX_ESP_INTERFACE_CHECKS )
    {
        m_nEspInterfaceCheckCounter++;

        if ( m_pMainStatemachine->espDriver()->isConnected() )
        {
            m_nEspInterfaceCheckCounter = 0;
            m_pEspInterfaceCheckTimer->stop();

            qInfo() << "checkInternalInterface: successfull!";
            triggerSwUpdateFileState( SWUPDATE_ESP_CHECK_VERSION );
        }
    }
    else
    {
        m_nEspInterfaceCheckCounter = 0;
        m_pEspInterfaceCheckTimer->stop();

        qWarning() << "checkInternalInterface: timeout -> no interface detected";
        triggerSwUpdateFileState( SWUPDATE_ESP_INTERFACE_FAILED );
    }
#else
    m_pEspInterfaceCheckTimer->stop();
    triggerSwUpdateFileState( SWUPDATE_ESP_CHECK_VERSION );
#endif
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SwUpdateController::checkMcuInterface()
{
#ifdef TARGETBUILD
    if ( m_nMcuInterfaceCheckCounter <= MAX_MCU_INTERFACE_CHECKS )
    {
        m_nMcuInterfaceCheckCounter++;

        if ( m_pMainStatemachine->espDriver()->espMcuDriver()->mcuDriver()->isMcuAlive()
             || m_pMainStatemachine->isMcuIgnored() )
        {
            m_nMcuInterfaceCheckCounter = 0;
            m_pMcuInterfaceCheckTimer->stop();

            qInfo() << "checkMcuInterface: successfull!";
            triggerSwUpdateFileState( SWUPDATE_MCU_CHECK_VERSION );
        }
    }
    else
    {
        m_nMcuInterfaceCheckCounter = 0;
        m_pMcuInterfaceCheckTimer->stop();

        qWarning() << "checkMcuInterface: timeout -> no interface detected";
        triggerSwUpdateFileState( SWUPDATE_MCU_INTERFACE_FAILED );
    }
#else
    m_pMcuInterfaceCheckTimer->stop();
    triggerSwUpdateFileState( SWUPDATE_MCU_CHECK_VERSION );
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
        disconnect( m_pMainStatemachine->deviceInfoCollector(), &DeviceInfoCollector::mcuInfosChanged, this, &SwUpdateController::checkMcuVersion );
        m_pTimeoutTimer->stop();
        triggerSwUpdateFileState( SWUPDATE_STARTUP_COMPLETED );
    }
    else
    {
        if ( m_pMainStatemachine->deviceInfoCollector()->mcuInfosCollected() )
        {
            disconnect( m_pMainStatemachine->deviceInfoCollector(), &DeviceInfoCollector::mcuInfosChanged, this, &SwUpdateController::checkMcuVersion );
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
        disconnect( m_pMainStatemachine->deviceInfoCollector(), &DeviceInfoCollector::mcuInfosChanged, this, &SwUpdateController::validateMcuVersion );
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
            m_pMainStatemachine->notificationCenter()->raiseNotification( EkxSqliteTypes::SqliteNotificationType::SqliteNotificationType_SW_UPDATE_MCU_VALIDATE_VERSION_FAILED );
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
#ifdef TARGETBUILD
    if ( m_pMainStatemachine->deviceInfoCollector()->espInfosCollected() )
    {
        disconnect( m_pMainStatemachine->deviceInfoCollector(), &DeviceInfoCollector::espInfosChanged, this, &SwUpdateController::checkEspVersion );
        m_pTimeoutTimer->stop();

        // check Esp Version;
        QString espUpdateVersion  = espUpdateSwVersionString();
        QString espCurrentVersion = m_pMainStatemachine->deviceInfoCollector()->espSwVersion();

        if ( QString::compare( espUpdateVersion, espCurrentVersion ) == 0 )
        {
            triggerSwUpdateFileState( SWUPDATE_MCU_CHECK_INTERFACE );
        }
        else
        {
            triggerSwUpdateFileState( SWUPDATE_ESP_UPDATE_IN_PROGRESS );
        }
    }
    else
    {
        qWarning() << "SwUpdateController::checkEspVersion() infos not collected";
    }
#else
    disconnect( m_pMainStatemachine->deviceInfoCollector(), &DeviceInfoCollector::espInfosChanged, this, &SwUpdateController::checkEspVersion );
    m_pTimeoutTimer->stop();
    triggerSwUpdateFileState( SWUPDATE_MCU_CHECK_INTERFACE );
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
        QString espUpdateVersion  = espUpdateSwVersionString();
        QString espCurrentVersion = m_pMainStatemachine->deviceInfoCollector()->espSwVersion();

        //qInfo() << "validateEspVersion /" << espUpdateVersion << "/" << espCurrentVersion;

        if ( QString::compare( espUpdateVersion, espCurrentVersion ) == 0 )
        {
            triggerSwUpdateFileState( SWUPDATE_STARTUP_COMPLETED );
            //triggerSwUpdateFileState( SWUPDATE_MCU_CHECK_VERSION );
        }
        else
        {
            m_pMainStatemachine->notificationCenter()->raiseNotification( EkxSqliteTypes::SqliteNotificationType::SqliteNotificationType_SW_UPDATE_ESP_VALIDATE_VERSION_FAILED );
            triggerSwUpdateFileState( SWUPDATE_ESP_UPDATE_FAILED );
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SwUpdateController::clearSwUpdatePackage()
{
    QString path = SWSTORAGE_PATH;
    QDir    dir( path );
    dir.setNameFilters( QStringList() << "*.swu" );
    dir.setFilter( QDir::Files );
    foreach( QString dirFile, dir.entryList() )
    {
        dir.remove( dirFile );
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

QString SwUpdateController::mcuUpdateSwVersionString() const
{
    QFile   file( MainStatemachine::rootPath() + MCU_FW_CONFIG_PATH );
    QString swVersion = "";

    if ( file.open( QIODevice::ReadOnly ) )
    {
        QByteArray ba = file.readAll();
        file.close();

        QJsonDocument doc = QJsonDocument::fromJson( ba );

        QJsonObject obj = doc.object();

        swVersion = obj[SWUPDATE_JSON_MCU_SWVER].toString();
    }

    return swVersion;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

QString SwUpdateController::espUpdateSwVersionString() const
{
    QFile   file( MainStatemachine::rootPath() + ESPCONFIG_PATH );
    QString swVersion = "";

    if ( file.open( QIODevice::ReadOnly ) )
    {
        QByteArray ba = file.readAll();
        file.close();

        QJsonDocument doc = QJsonDocument::fromJson( ba );

        QJsonObject obj = doc.object();

        swVersion = obj[SWUPDATE_JSON_ESP_SWVER].toString();
    }

    return swVersion;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

QString SwUpdateController::espUpdateFile() const
{
    QFile   file( MainStatemachine::rootPath() + ESPCONFIG_PATH );
    QString updateFile = "";

    if ( file.open( QIODevice::ReadOnly ) )
    {
        QByteArray ba = file.readAll();
        file.close();

        QJsonDocument doc = QJsonDocument::fromJson( ba );

        QJsonObject obj = doc.object();

        updateFile = obj[SWUPDATE_JSON_ESP_FILE].toString();
    }

    return updateFile;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SwUpdateController::checkRootfs() const
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
        if ( rootfsNomPath.open( QIODevice::ReadOnly ) )
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
        if ( rootfsCurPath.open( QIODevice::ReadOnly ) )
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

SwUpdateController::SwUpdateStates SwUpdateController::convertStringToSwUpdateState( const QString strState ) const
{
    SwUpdateController::SwUpdateStates currentSwUpdateState = SwUpdateStates::SWUPDATE_INIT;

    if ( QString::compare( strState, "IDLE" ) == 0 )
    {
        currentSwUpdateState = SwUpdateStates::SWUPDATE_IDLE;
    }
    else if ( QString::compare( strState, "UPLOAD_IN_PROGRESS" ) == 0 )
    {
        currentSwUpdateState = SwUpdateStates::SWUPDATE_UPLOAD_IN_PROGRESS;
    }
    else if ( QString::compare( strState, "UPLOAD_COMPLETED" ) == 0 )
    {
        currentSwUpdateState = SwUpdateStates::SWUPDATE_UPLOAD_COMPLETED;
    }
    else if ( QString::compare( strState, "UPLOAD_FAILED" ) == 0 )
    {
        currentSwUpdateState = SwUpdateStates::SWUPDATE_UPLOAD_FAILED;
    }
    else if ( QString::compare( strState, "UPLOAD_ABORTED" ) == 0 )
    {
        currentSwUpdateState = SwUpdateStates::SWUPDATE_UPLOAD_ABORTED;
    }
    else if ( QString::compare( strState, "EVALUATE_PACKAGE" ) == 0 )
    {
        currentSwUpdateState = SwUpdateStates::SWUPDATE_EVALUATE_PACKAGE;
    }
    else if ( QString::compare( strState, "PACKAGE_FAILED" ) == 0 )
    {
        currentSwUpdateState = SwUpdateStates::SWUPDATE_PACKAGE_FAILED;
    }
    else if ( QString::compare( strState, "PACKAGE_NO_UPGRADE" ) == 0 )
    {
        currentSwUpdateState = SwUpdateStates::SWUPDATE_PACKAGE_NO_UPGRADE;
    }
    else if ( QString::compare( strState, "UNPACK_ROOTFS" ) == 0 )
    {
        currentSwUpdateState = SwUpdateStates::SWUPDATE_UNPACK_ROOTFS;
    }
    else if ( QString::compare( strState, "UNPACK_ROOTFS_FAILED" ) == 0 )
    {
        currentSwUpdateState = SwUpdateStates::SWUPDATE_UNPACK_ROOTFS_FAILED;
    }
    else if ( QString::compare( strState, "UNPACK_APPLICATION" ) == 0 )
    {
        currentSwUpdateState = SwUpdateStates::SWUPDATE_UNPACK_APPLICATION;
    }
    else if ( QString::compare( strState, "UNPACK_APPLICATION_FAILED" ) == 0 )
    {
        currentSwUpdateState = SwUpdateStates::SWUPDATE_UNPACK_APPLICATION_FAILED;
    }
    //    else if(QString::compare(strState, "INIT_UPDATE_FAILED" ) == 0)
    //    {
    //        currentSwUpdateState = SwUpdateStates::SWUPDATE_INIT_UPDATE_FAILED;
    //    }
    else if ( QString::compare( strState, "REBOOT" ) == 0 )
    {
        currentSwUpdateState = SwUpdateStates::SWUPDATE_REBOOT;
    }
    else if ( QString::compare( strState, "STARTUP_CHECK" ) == 0 )
    {
        currentSwUpdateState = SwUpdateStates::SWUPDATE_STARTUP_CHECK;
    }
    else if ( QString::compare( strState, "CHECK_ROOTFS" ) == 0 )
    {
        currentSwUpdateState = SwUpdateStates::SWUPDATE_CHECK_ROOTFS;
    }
    else if ( QString::compare( strState, "ROOTFS_FAILED" ) == 0 )
    {
        currentSwUpdateState = SwUpdateStates::SWUPDATE_ROOTFS_FAILED;
    }

    else if ( QString::compare( strState, "ESP_CHECK_INTERFACE" ) == 0 )
    {
        currentSwUpdateState = SwUpdateStates::SWUPDATE_ESP_CHECK_INTERFACE;
    }
    else if ( QString::compare( strState, "ESP_INTERFACE_FAILED" ) == 0 )
    {
        currentSwUpdateState = SwUpdateStates::SWUPDATE_ESP_INTERFACE_FAILED;
    }
    else if ( QString::compare( strState, "ESP_CHECK_VERSION" ) == 0 )
    {
        currentSwUpdateState = SwUpdateStates::SWUPDATE_ESP_CHECK_VERSION;
    }
    else if ( QString::compare( strState, "ESP_UPDATE_IN_PROGRESS" ) == 0 )
    {
        currentSwUpdateState = SwUpdateStates::SWUPDATE_ESP_UPDATE_IN_PROGRESS;
    }
    else if ( QString::compare( strState, "ESP_UPDATE_FAILED" ) == 0 )
    {
        currentSwUpdateState = SwUpdateStates::SWUPDATE_ESP_UPDATE_FAILED;
    }
    else if ( QString::compare( strState, "ESP_VALIDATE_VERSION" ) == 0 )
    {
        currentSwUpdateState = SwUpdateStates::SWUPDATE_ESP_VALIDATE_VERSION;
    }

    else if ( QString::compare( strState, "MCU_CHECK_INTERFACE" ) == 0 )
    {
        currentSwUpdateState = SwUpdateStates::SWUPDATE_MCU_CHECK_INTERFACE;
    }
    else if ( QString::compare( strState, "MCU_INTERFACE_FAILED" ) == 0 )
    {
        currentSwUpdateState = SwUpdateStates::SWUPDATE_MCU_INTERFACE_FAILED;
    }
    else if ( QString::compare( strState, "MCU_CHECK_VERSION" ) == 0 )
    {
        currentSwUpdateState = SwUpdateStates::SWUPDATE_MCU_CHECK_VERSION;
    }
    else if ( QString::compare( strState, "MCU_UPDATE_IN_PROGRESS" ) == 0 )
    {
        currentSwUpdateState = SwUpdateStates::SWUPDATE_MCU_UPDATE_IN_PROGRESS;
    }
    else if ( QString::compare( strState, "MCU_UPDATE_FAILED" ) == 0 )
    {
        currentSwUpdateState = SwUpdateStates::SWUPDATE_MCU_UPDATE_FAILED;
    }
    else if ( QString::compare( strState, "MCU_VALIDATE_VERSION" ) == 0 )
    {
        currentSwUpdateState = SwUpdateStates::SWUPDATE_MCU_VALIDATE_VERSION;
    }

    else if ( QString::compare( strState, "STARTUP_COMPLETED" ) == 0 )
    {
        currentSwUpdateState = SwUpdateStates::SWUPDATE_STARTUP_COMPLETED;
    }
    else if ( QString::compare( strState, "STARTUP_FAILED" ) == 0 )
    {
        currentSwUpdateState = SwUpdateStates::SWUPDATE_STARTUP_FAILED;
    }
    else if ( QString::compare( strState, "UPDATE_COMPLETED" ) == 0 )
    {
        currentSwUpdateState = SwUpdateStates::SWUPDATE_UPDATE_COMPLETED;
    }
    else if ( QString::compare( strState, "UPDATE_FAILED" ) == 0 )
    {
        currentSwUpdateState = SwUpdateStates::SWUPDATE_UPDATE_FAILED;
    }
    else if ( QString::compare( strState, "UPDATE_TIMEOUT" ) == 0 )
    {
        currentSwUpdateState = SwUpdateStates::SWUPDATE_TIMEOUT;
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

SwUpdateController::SwUpdateProcesses SwUpdateController::convertStringToSwUpdateProcess( const QString & strProcess ) const
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

QString SwUpdateController::swUpdateStateString( const SwUpdateController::SwUpdateStates nState ) const
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
            strState = "PACKAGE_NO_UPGRADE";
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
        case SWUPDATE_UPLOAD_ABORTED:
            strState = "UPLOAD_ABORTED";
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
        case SWUPDATE_ESP_CHECK_INTERFACE:
            strState = "ESP_CHECK_INTERFACE";
            break;
        case SWUPDATE_ESP_INTERFACE_FAILED:
            strState = "ESP_INTERFACE_FAILED";
            break;

        case SWUPDATE_ESP_CHECK_VERSION:
            strState = "ESP_CHECK_VERSION";
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

        case SWUPDATE_MCU_CHECK_INTERFACE:
            strState = "MCU_CHECK_INTERFACE";
            break;
        case SWUPDATE_MCU_INTERFACE_FAILED:
            strState = "MCU_INTERFACE_FAILED";
            break;
        case SWUPDATE_MCU_CHECK_VERSION:
            strState = "MCU_CHECK_VERSION";
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

