///////////////////////////////////////////////////////////////////////////////
///
/// @file SettingsStatemachine.cpp
///
/// @brief Implementation file of class SettingsStatemachine
///
/// @author Ultratronik GmbH
///         Dornierstr. 9
///         D-82205 Gilching
///         Germany
///         http://www.ultratronik.de
///
/// @author written by Gerd Esser, Forschung & Entwicklung, gesser@ultratronik.de
///
/// @date 25.01.2021
///
/// @copyright Copyright 2021 by Hemro International AG
///            Hemro International AG
///            Länggenstrasse 34
///            CH 8184 Bachenbülach
///            Switzerland
///            Homepage: www.hemrogroup.com
///
///////////////////////////////////////////////////////////////////////////////

#include "SettingsStatemachine.h"

#include <QtDebug>

#include "SettingsMenu.h"
#include "MainStatemachine.h"
#include "StatisticsBoard.h"
#include "DiskUsageTime.h"
#include "AgsaControl.h"
#include "SqliteInterface.h"
#include "SettingsMenuContentFactory.h"

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

//int SettingsStatemachine::SettingsState_QTypeId = qRegisterMetaType<SettingsStatemachine::SettingsState>( "SettingsStatemachine::SettingsState" );
//int SettingsStatemachine::QmlSettingsStateId    = qmlRegisterUncreatableType<SettingsStatemachine>( "SettingsStateEnum", 1, 0, "SettingsStateEnum", "cannot create Object because SettingsStatemachine was registered as SettingsStateEnum to be uncreational " );

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

SettingsStatemachine::SettingsStatemachine( QObject * pParent )
    : QObject( pParent )
    , m_pStatisticsBoard( new StatisticsBoard() )
    , m_pDiskUsageTime( new DiskUsageTime() )
    , m_theOldStates( 10 )
    , m_theOldPositions( 10 )
    , m_pContentFactory( new SettingsMenuContentFactory() )
{
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

SettingsStatemachine::~SettingsStatemachine()
{
    delete m_pContentFactory;
    m_pContentFactory = nullptr;

    delete m_pDiskUsageTime;
    m_pDiskUsageTime = nullptr;

    delete m_pStatisticsBoard;
    m_pStatisticsBoard = nullptr;

    m_theOldStates.clear();
    m_theOldPositions.clear();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsStatemachine::create( MainStatemachine *   pMainStatemachine,
                                   SettingsSerializer * pSettingsSerializer,
                                   SqliteInterface *    pSqliteInterface )
{
    m_pMainStatemachine   = pMainStatemachine;
    m_pSettingsSerializer = pSettingsSerializer;

    setSettingsState( SettingsStatemachine::SETTING_MAIN, false );

    m_pStatisticsBoard->create( pSqliteInterface );
    m_pDiskUsageTime->create( pSqliteInterface );
    m_pContentFactory->create( this,
                               m_pMainStatemachine,
                               m_pSettingsSerializer );

    m_pStatisticsBoard->setDiskUsageMaxHours( m_pSettingsSerializer->discUsageMaxHours() );
    m_pStatisticsBoard->setPeriodChoicesIndex( m_pSettingsSerializer->statisticsPeriodIndex() );
    m_pDiskUsageTime->setMaxDiscUsageTimeHours( m_pSettingsSerializer->discUsageMaxHours() );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

SettingsStatemachine::SettingsState SettingsStatemachine::settingsState() const
{
    return m_nCurrentState;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsStatemachine::navigateToMenu( const SettingsStatemachine::SettingsState nSettingsState )
{
    setSettingsState( nSettingsState );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsStatemachine::back()
{
    if ( !m_theOldStates.isEmpty() )
    {
        setSettingsState( m_theOldStates.last(), true );
        m_theOldStates.removeLast();

        emit navigatedPrevious();
        emit popNextRecoverActionStep();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsStatemachine::saveMenuPosition( const double dPosition )
{
    m_theOldPositions.append( dPosition );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

double SettingsStatemachine::savedMenuPostion()
{
    double position = 0.0;
    if ( !m_theOldPositions.isEmpty() )
    {
        position = m_theOldPositions.last();
        m_theOldPositions.removeLast();
    }

    return position;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsStatemachine::backToMainMenu()
{
    if ( !m_theOldStates.isEmpty() )
    {
        m_theOldStates.clear();
        m_theOldPositions.clear();
        setSettingsState( SETTING_MAIN );
        emit popNextRecoverActionStep();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SettingsStatemachine::checkPinCode( const QString &                    strPin,
                                         const SettingsSerializer::UserRole nUserRole,
                                         const bool                         bLogIn )
{
    bool pinCheckSuccess = false;

    switch ( nUserRole )
    {
        case SettingsSerializer::UserRole::StoreOwner:
            if ( strPin == m_pSettingsSerializer->userPinForOwner() )
            {
                pinCheckSuccess = true;
                if ( bLogIn )
                {
                    m_pSettingsSerializer->setCurrentLoggedInUserRole( SettingsSerializer::UserRole::StoreOwner );
                    m_pSettingsSerializer->saveSettings();
                }
            }
            break;

        case SettingsSerializer::UserRole::Technician:
            if ( strPin == m_pSettingsSerializer->userPinForTechnician() )
            {
                pinCheckSuccess = true;
                if ( bLogIn )
                {
                    m_pSettingsSerializer->setCurrentLoggedInUserRole( SettingsSerializer::UserRole::Technician );
                    m_pSettingsSerializer->saveSettings();
                }
            }
            break;

        case SettingsSerializer::UserRole::NoRole:
        default:
            pinCheckSuccess = false;
            break;

    }
    qDebug() << "checkPin returning" << pinCheckSuccess;
    return pinCheckSuccess;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SettingsStatemachine::checkPinCode( QString pin,
                                         int     user,
                                         bool    logIn )
{
    return checkPinCode( pin, static_cast<SettingsSerializer::UserRole>( user ), logIn );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SettingsStatemachine::savePinCode( const QString &                    strPin,
                                        const SettingsSerializer::UserRole nUserRole )
{
    bool pinSaveSuccess = false;

    switch ( nUserRole )
    {
        case SettingsSerializer::UserRole::StoreOwner:
            m_pSettingsSerializer->setUserPinForOwner( strPin );
            m_pMainStatemachine->sqliteInterface()->addEvent( EkxSqliteTypes::SqliteEventTypes::SqliteEventType_OWNER_PIN_CHANGE, "" );
            pinSaveSuccess = true;
            break;
        case SettingsSerializer::UserRole::Technician:
            m_pSettingsSerializer->setUserPinForTechnician( strPin );
            m_pMainStatemachine->sqliteInterface()->addEvent( EkxSqliteTypes::SqliteEventTypes::SqliteEventType_TECHNICIAN_PIN_CHANGE, "" );
            pinSaveSuccess = true;
            break;
        case SettingsSerializer::UserRole::NoRole:
        default:
            pinSaveSuccess = false;

    }
    if ( pinSaveSuccess )
    {
        m_pSettingsSerializer->saveSettings();
    }
    return pinSaveSuccess;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SettingsStatemachine::savePinCode( QString pin,
                                        int     user )
{
    return savePinCode( pin, static_cast<SettingsSerializer::UserRole>( user ) );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsStatemachine::logout()
{
    if ( m_pSettingsSerializer->currentLoggedInUserRole() != SettingsSerializer::UserRole::NoRole )
    {
        m_pSettingsSerializer->setCurrentLoggedInUserRole( SettingsSerializer::UserRole::NoRole );
        m_pSettingsSerializer->saveSettings();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsStatemachine::menuToggleSwitchChangeHandler( const SettingsMenu::ToggleSwitchTypes nToggleSwitchType,
                                                          const bool                            bCurrentValue )
{
    switch ( nToggleSwitchType )
    {
        case SettingsMenu::TOGGLE_SWITCH_FOR_SHOWTEMP:
            m_pSettingsSerializer->setShowTemp( bCurrentValue );
            m_pSettingsSerializer->saveSettings();
            m_pMainStatemachine->sqliteInterface()->addEvent( EkxSqliteTypes::SqliteEventTypes::SqliteEventType_CHANGE_SHOW_TEMP, ( bCurrentValue ? "Show" : "Hide" ) );
            break;

        case SettingsMenu::TOGGLE_SWITCH_FOR_WARNINGTEMP:
            setSettingsState( m_nCurrentState, true );
            m_pSettingsSerializer->setShowTempWarning( bCurrentValue );
            m_pSettingsSerializer->saveSettings();
            m_pMainStatemachine->sqliteInterface()->addEvent( EkxSqliteTypes::SqliteEventTypes::SqliteEventType_CHANGE_TEMP_WARNING, ( bCurrentValue ? "Show" : "Hide" ) );
            break;

        case SettingsMenu::TOGGLE_SWITCH_FOR_LIBRARYMODE_ENABLING:
            m_pSettingsSerializer->setLibraryModeIsEnabled( bCurrentValue );
            m_pSettingsSerializer->saveSettings();
            m_pMainStatemachine->sqliteInterface()->addEvent( EkxSqliteTypes::SqliteEventTypes::SqliteEventType_CHANGE_SHOW_LIBRARY_MODE, ( bCurrentValue ? "Show" : "Hide" ) );
            break;

        case SettingsMenu::TOGGLE_SWITCH_FOR_AGSA_ENABLE:
            m_pSettingsSerializer->setAgsaEnabled( bCurrentValue );
            m_pSettingsSerializer->saveSettings();
            m_pMainStatemachine->sqliteInterface()->addEvent( EkxSqliteTypes::SqliteEventTypes::SqliteEventType_CHANGE_ENABLE_AGSA, ( bCurrentValue ? "Enable" : "Disable" ) );
            break;

        case SettingsMenu::TOGGLE_SWITCH_FOR_TIMEMODE_ENABLING:
            m_pSettingsSerializer->setTimeModeIsEnabled( bCurrentValue );
            m_pSettingsSerializer->saveSettings();
            m_pMainStatemachine->sqliteInterface()->addEvent( EkxSqliteTypes::SqliteEventTypes::SqliteEventType_CHANGE_SHOW_TIME_MODE, ( bCurrentValue ? "Show" : "Hide" ) );
            break;

        case SettingsMenu::TOGGLE_SWITCH_FOR_DOSING_TIME:
            m_pSettingsSerializer->setShowDosingTime( bCurrentValue );
            m_pSettingsSerializer->saveSettings();
            m_pMainStatemachine->sqliteInterface()->addEvent( EkxSqliteTypes::SqliteEventTypes::SqliteEventType_CHANGE_SHOW_DOSING_TIME, ( bCurrentValue ? "Show" : "Hide" ) );
            break;

        case SettingsMenu::TOGGLE_SWITCH_FOR_SHOWSPOT:
            m_pSettingsSerializer->setShowSpotLight( bCurrentValue );
            m_pSettingsSerializer->saveSettings();
            m_pMainStatemachine->sqliteInterface()->addEvent( EkxSqliteTypes::SqliteEventTypes::SqliteEventType_CHANGE_SHOW_SPOTLIGHT, ( bCurrentValue ? "Show" : "Hide" ) );
            break;

        case SettingsMenu::TOGGLE_SWITCH_FOR_WIFI:
            m_pSettingsSerializer->setWifiMode( bCurrentValue ? SettingsSerializer::WifiMode::MODE_AP : SettingsSerializer::WifiMode::MODE_OFF );
            m_pSettingsSerializer->saveSettings();
            setSettingsState( m_nCurrentState, true );   // force updating dependent toggles
            m_pMainStatemachine->sqliteInterface()->addEvent( EkxSqliteTypes::SqliteEventTypes::SqliteEventType_CHANGE_WIFI_ON, ( bCurrentValue ? "On" : "Off" ) );
            break;

        case SettingsMenu::TOGGLE_SWITCH_FOR_AUTOLOG:
            m_pSettingsSerializer->setUserAutoLogoutEnable( bCurrentValue );
            m_pSettingsSerializer->saveSettings();
            m_pMainStatemachine->sqliteInterface()->addEvent( EkxSqliteTypes::SqliteEventTypes::SqliteEventType_CHANGE_AUTO_LOGOUT, ( bCurrentValue ? "On" : "Off" ) );
            break;

        // development issue
        case SettingsMenu::TOGGLE_SWITCH_FOR_AGSA_BLOCKAGE_DETECT:
            m_pMainStatemachine->agsaControl()->setTestBlockageDetect( bCurrentValue );
            break;

        // development issue
        case SettingsMenu::TOGGLE_SWITCH_FOR_AGSA_LIVE_MODE:
            m_pMainStatemachine->agsaControl()->setTestLiveMode( bCurrentValue );
            break;

        default:
            //do nothing;
            break;
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SettingsStatemachine::setSettingsState( const SettingsState nSettingsState,
                                             const bool          bBackUsed )
{
    bool success = true;

    if ( !bBackUsed )
    {
        m_theOldStates.append( m_nCurrentState );
    }

    m_pContentFactory->setNavAvailable( true );

    switch ( nSettingsState )
    {
        case SETTING_MAIN:
            success = setMainMenuState();
            break;

        case SETTING_SERVICE:
            success = setServiceState();
            break;
        case SETTING_SERVICE_DISC_CALIB:
            success = setServiceDiscCalibrationState();
            break;
        case SETTING_SERVICE_DISC_USAGE_TIME:
            success = setServiceDiscUsageTimeState();
            break;
        case SETTING_SERVICE_DISPLAY_TEST:
            success = setServiceDisplayTestState();
            break;
        case SETTING_SERVICE_MOTOR_TEST:
            success = setServiceMotorTestState();
            break;
        case SETTING_SERVICE_MOTOR_TEST_AGSA:
            success = setServiceMotorTestAgsaState();
            break;
        case SETTING_SERVICE_MOTOR_TEST_AGSA_MOVE_TO_DDD:
            success = setServiceMotorTestAgsaMoveToDddState();
            break;
        case SETTING_SERVICE_MOTOR_TEST_AGSA_NUM_STEPS:
            success = setServiceMotorTestAgsaNumStepsState();
            break;
        case SETTING_SERVICE_MOTOR_TEST_AGSA_FREQUENCY:
            success = setServiceMotorTestAgsaFrequencyState();
            break;
        case SETTING_SERVICE_MOTOR_TEST_PDU:
            success = setServiceMotorTestPduState();
            break;
        case SETTING_SERVICE_MOTOR_TEST_AGSA_LONG_DURATION:
            success = setServiceMotorTestAgsaLongDurationState();
            break;
        case SETTING_SERVICE_DDD_TEST:
            success = setServiceDddTestState();
            break;
        case SETTING_SERVICE_MCU_CONFIG:
            success = setServiceMcuConfigState();
            break;
        case SETTING_SERVICE_MCU_CONFIG_MAX_MOTOR_SPEED:
            success = setServiceMcuConfigMaxMotorSpeedState();
            break;
        case SETTING_SERVICE_MCU_CONFIG_NOMINAL_MOTOR_SPEED:
            success = setServiceMcuConfigNominalMotorSpeedState();
            break;
        case SETTING_SERVICE_MCU_CONFIG_ACCELERATION_TIME:
            success = setServiceMcuConfigAccelerationTimeState();
            break;
        case SETTING_SERVICE_MCU_CONFIG_DECELERATION_TIME:
            success = setServiceMcuConfigDecelerationTimeState();
            break;
        case SETTING_SERVICE_SYSTEM_SETUP:
            success = setServiceSystemSetupState();
            break;

        case SETTING_SYSTEM:
            success = setSystemState();
            break;
        case SETTING_SYSTEM_WIFI_SETTING:
            success = setSystemWifiState();
            break;
        case SETTING_SYSTEM_WIFI_SETTING_IP:
            success = setSystemWifiIpState();
            break;
        case SETTING_SYSTEM_WIFI_SETTING_PW:
            success = setSystemWifiPasswordState();
            break;
        case SETTING_SYSTEM_WIFI_SETTING_SSID:
            success = setSystemWifiSsidState();
            break;
        case SETTING_SYSTEM_TEMP_UNIT:
            success = setSystemTempUnitState();
            break;
        case SETTING_SYSTEM_DATE:
            success = setSystemDateState();
            break;
        case SETTING_SYSTEM_TIME:
            success = setSystemTimeState();
            break;
        case SETTING_SYSTEM_GRINDERNAME:
            success = setSystemGrinderNameState();
            break;
        case SETTING_SYSTEM_FACTORY_RESET:
            success = setSystemFactoryResState();
            break;
        case SETTING_SYSTEM_START_OF_WEEK:
            success = setSystemStartOfWeekState();
            break;

        case SETTING_CUSTOMIZE:
            success = setCustomizeState();
            break;
        case SETTING_CUSTOMIZE_TEMP:
            success = setCustomizeTempState();
            break;
        case SETTING_CUSTOMIZE_TEMP_THRESH:
            success = setCustomizeTempThreshState();
            break;
        case SETTING_CUSTOMIZE_BRIGHTNESS:
            success = setCustomizeBrightnessState();
            break;
        case SETTING_CUSTOMIZE_STDBY_ADJ:
            success = setCustomizeStdbyAdjState();
            break;
        case SETTING_CUSTOMIZE_SETTINGS_MODE_EXIT_TIMEOUT_ADJ:
            success = setCustomizeSettingsModeExitTimeoutAdjState();
            break;
        case SETTING_CUSTOMIZE_AGSA_DELAY:
            success = setCustomizeAgsaDelayState();
            break;

        case SETTING_USER_CPC:
            success = setUserCPCState();
            break;
        case SETTING_USER_CPC_STORE:
            success = setUserCPCStoreState();
            break;
        case SETTING_USER_CPC_TECH:
            success = setUserCPCTechState();
            break;
        case SETTING_USER:
            success = setUserState();
            break;
        case SETTING_USER_LOGIN:
            success = setUserLoginState();
            break;
        case SETTING_USER_LOGOUT:
            success = setUserLogoutState();
            break;
        case SETTING_USER_STORE:
            success = setUserStoreState();
            break;
        case SETTING_USER_TECHNICIAN:
            success = setUserTechState();
            break;

        case SETTING_INFO:
            success = setInfoState();
            break;
        case SETTING_INFO_STATISTICS:
            success = setInfoStatisticsState();
            break;
        case SETTING_INFO_NOTIFICATION_CENTER:
            success = setInfoNotificationCenterState();
            break;
        case SETTING_INFO_MACHINE_INFO:
            success = setInfoMachineInfoState();
            break;
        case SETTING_INFO_OWNERS_MANUAL:
            success = setInfoOwnersManualState();
            break;
        case SETTING_INFO_GRINDER_TUTORIAL:
            success = setInfoGrinderTutorialState();
            break;
        case SETTING_INFO_PDU_TUTORIAL:
            success = setInfoPduTutorialState();
            break;
        default:
            success = false;
    }

    if ( !success )
    {
        if ( !m_theOldStates.isEmpty()
             && !bBackUsed )
        {
            m_theOldStates.removeLast();
        }
    }

    return success;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SettingsStatemachine::setMainMenuState()
{
    bool success;

    switch ( m_nCurrentState )
    {
        case SETTING_MAIN:
        case SETTING_SERVICE:
        case SETTING_SERVICE_DISC_CALIB:
        case SETTING_SERVICE_DISC_USAGE_TIME:
        case SETTING_SERVICE_DISPLAY_TEST:
        case SETTING_SERVICE_MOTOR_TEST:
        case SETTING_SERVICE_MOTOR_TEST_AGSA:
        case SETTING_SERVICE_MOTOR_TEST_AGSA_MOVE_TO_DDD:
        case SETTING_SERVICE_MOTOR_TEST_AGSA_NUM_STEPS:
        case SETTING_SERVICE_MOTOR_TEST_AGSA_FREQUENCY:
        case SETTING_SERVICE_MOTOR_TEST_PDU:
        case SETTING_SERVICE_MOTOR_TEST_AGSA_LONG_DURATION:
        case SETTING_SERVICE_DDD_TEST:
        case SETTING_SERVICE_MCU_CONFIG:
        case SETTING_SERVICE_MCU_CONFIG_MAX_MOTOR_SPEED:
        case SETTING_SERVICE_MCU_CONFIG_NOMINAL_MOTOR_SPEED:
        case SETTING_SERVICE_MCU_CONFIG_ACCELERATION_TIME:
        case SETTING_SERVICE_MCU_CONFIG_DECELERATION_TIME:
        case SETTING_SERVICE_SYSTEM_SETUP:
        case SETTING_SYSTEM:
        case SETTING_SYSTEM_WIFI_SETTING:
        case SETTING_SYSTEM_WIFI_SETTING_IP:
        case SETTING_SYSTEM_WIFI_SETTING_PW:
        case SETTING_SYSTEM_WIFI_SETTING_SSID:
        case SETTING_SYSTEM_DATE:
        case SETTING_SYSTEM_TIME:
        case SETTING_SYSTEM_START_OF_WEEK:
        case SETTING_SYSTEM_GRINDERNAME:
        case SETTING_SYSTEM_FACTORY_RESET:
        case SETTING_SYSTEM_TEMP_UNIT:
        case SETTING_CUSTOMIZE:
        case SETTING_CUSTOMIZE_TEMP:
        case SETTING_CUSTOMIZE_TEMP_THRESH:
        case SETTING_CUSTOMIZE_BRIGHTNESS:
        case SETTING_USER_CPC:
        case SETTING_USER_CPC_STORE:
        case SETTING_USER_CPC_TECH:
        case SETTING_USER:
        case SETTING_USER_LOGIN:
        case SETTING_USER_LOGOUT:
        case SETTING_USER_STORE:
        case SETTING_USER_TECHNICIAN:
        case SETTING_CUSTOMIZE_STDBY_ADJ:
        case SETTING_CUSTOMIZE_SETTINGS_MODE_EXIT_TIMEOUT_ADJ:
        case SETTING_CUSTOMIZE_AGSA_DELAY:
        case SETTING_INFO:
        case SETTING_INFO_STATISTICS:
        case SETTING_INFO_NOTIFICATION_CENTER:
        case SETTING_INFO_MACHINE_INFO:
        case SETTING_INFO_OWNERS_MANUAL:
        case SETTING_INFO_GRINDER_TUTORIAL:
        case SETTING_INFO_PDU_TUTORIAL:
            success = true;
            break;
        default:
            success = false;
    }

    if ( success )
    {
        m_nCurrentState = SettingsState::SETTING_MAIN;
        //m_pContentFactory->buildUpMainState();

        m_pContentFactory->setMenuTitle( "Settings" );
        m_pContentFactory->setButtonPreviousVisible( false );
        m_pContentFactory->setButtonOkVisible( false );
        m_pContentFactory->setNavAvailable( false );

        emit settingsStateChanged();
    }

    return success;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SettingsStatemachine::setServiceState()
{
    bool success;

    switch ( m_nCurrentState )
    {
        case SETTING_MAIN:
        case SETTING_SERVICE:
        case SETTING_SERVICE_DISC_CALIB:
        case SETTING_SERVICE_DISC_USAGE_TIME:
        case SETTING_SERVICE_DISPLAY_TEST:
        case SETTING_SERVICE_MOTOR_TEST:
        case SETTING_SERVICE_DDD_TEST:
        case SETTING_SERVICE_MCU_CONFIG:
        case SETTING_SERVICE_MCU_CONFIG_MAX_MOTOR_SPEED:
        case SETTING_SERVICE_MCU_CONFIG_NOMINAL_MOTOR_SPEED:
        case SETTING_SERVICE_MCU_CONFIG_ACCELERATION_TIME:
        case SETTING_SERVICE_MCU_CONFIG_DECELERATION_TIME:
        case SETTING_SERVICE_SYSTEM_SETUP:
            success = true;
            break;
        default:
            success = false;
    }

    if ( success )
    {
        m_nCurrentState = SettingsState::SETTING_SERVICE;

        m_pContentFactory->buildUpService();

        emit settingsStateChanged();
    }

    return success;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SettingsStatemachine::setCustomizeState()
{
    bool success;

    switch ( m_nCurrentState )
    {
        case SETTING_MAIN:
        case SETTING_CUSTOMIZE:
        case SETTING_CUSTOMIZE_TEMP:
        case SETTING_CUSTOMIZE_BRIGHTNESS:
        case SETTING_CUSTOMIZE_STDBY_ADJ:
        case SETTING_CUSTOMIZE_SETTINGS_MODE_EXIT_TIMEOUT_ADJ:
        case SETTING_CUSTOMIZE_AGSA_DELAY:
            success = true;
            break;
        default:
            success = false;
    }

    if ( success )
    {
        m_nCurrentState = SettingsState::SETTING_CUSTOMIZE;

        m_pContentFactory->buildUpCustomize();

        emit settingsStateChanged();
    }

    return success;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SettingsStatemachine::setCustomizeTempState()
{
    bool success;

    switch ( m_nCurrentState )
    {
        case SETTING_CUSTOMIZE:
        case SETTING_CUSTOMIZE_TEMP:
        case SETTING_CUSTOMIZE_TEMP_THRESH:
            success = true;
            break;
        default:
            success = false;
    }

    if ( success )
    {
        m_nCurrentState = SettingsState::SETTING_CUSTOMIZE_TEMP;

        m_pContentFactory->buildUpCustomizeTemp();

        emit settingsStateChanged();
    }

    return success;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SettingsStatemachine::setSystemTempUnitState()
{
    bool success;

    switch ( m_nCurrentState )
    {
        case SETTING_SYSTEM:
        case SETTING_SYSTEM_TEMP_UNIT:
            success = true;
            break;
        default:
            success = false;
    }

    if ( success )
    {
        m_nCurrentState = SettingsState::SETTING_SYSTEM_TEMP_UNIT;

        m_pContentFactory->buildUpSystemTempUnit();

        emit settingsStateChanged();
    }

    return success;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SettingsStatemachine::setCustomizeTempThreshState()
{
    bool success;

    switch ( m_nCurrentState )
    {
        case SETTING_CUSTOMIZE_TEMP:
        case SETTING_CUSTOMIZE_TEMP_THRESH:
            success = true;
            break;
        default:
            success = false;
    }

    if ( success )
    {
        m_nCurrentState = SettingsState::SETTING_CUSTOMIZE_TEMP_THRESH;

        m_pContentFactory->buildUpCustomizeTempThresh();

        emit settingsStateChanged();
    }

    return success;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SettingsStatemachine::setCustomizeBrightnessState()
{
    bool success;

    switch ( m_nCurrentState )
    {
        case SETTING_CUSTOMIZE:
        case SETTING_CUSTOMIZE_BRIGHTNESS:
            success = true;
            break;
        default:
            success = false;
    }

    if ( success )
    {
        m_nCurrentState = SettingsState::SETTING_CUSTOMIZE_BRIGHTNESS;

        m_pContentFactory->buildUpCustomizeBrightness();

        emit settingsStateChanged();
    }

    return success;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SettingsStatemachine::setServiceDiscCalibrationState()
{
    bool success;

    switch ( m_nCurrentState )
    {
        case SETTING_SERVICE:
        case SETTING_SERVICE_DISC_CALIB:
            success = true;
            break;
        default:
            success = false;
    }

    if ( success )
    {
        m_nCurrentState = SettingsState::SETTING_SERVICE_DISC_CALIB;

        m_pContentFactory->buildUpServiceDiscCalibration();

        emit settingsStateChanged();
    }

    return success;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SettingsStatemachine::setServiceDiscUsageTimeState()
{
    bool success;

    switch ( m_nCurrentState )
    {
        case SETTING_SERVICE:
        case SETTING_SERVICE_DISC_USAGE_TIME:
            success = true;
            break;
        default:
            success = false;
    }

    if ( success )
    {
        m_nCurrentState = SettingsState::SETTING_SERVICE_DISC_USAGE_TIME;

        m_pContentFactory->buildUpServiceDiscUsageTime();

        emit settingsStateChanged();
    }

    return success;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SettingsStatemachine::setServiceDisplayTestState()
{
    bool success;

    switch ( m_nCurrentState )
    {
        case SETTING_SERVICE:
        case SETTING_SERVICE_DISPLAY_TEST:
            success = true;
            break;
        default:
            success = false;
    }

    if ( success )
    {
        m_nCurrentState = SettingsState::SETTING_SERVICE_DISPLAY_TEST;
        m_pContentFactory->buildUpSystemDisplayTest();
        emit settingsStateChanged();
    }

    return success;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SettingsStatemachine::setServiceMotorTestState()
{
    bool success;

    switch ( m_nCurrentState )
    {
        case SETTING_SERVICE:
        case SETTING_SERVICE_MOTOR_TEST:
        case SETTING_SERVICE_MOTOR_TEST_AGSA:
        case SETTING_SERVICE_MOTOR_TEST_AGSA_LONG_DURATION:
        case SETTING_SERVICE_MOTOR_TEST_PDU:
            success = true;
            break;
        default:
            success = false;
    }

    if ( success )
    {
        m_nCurrentState = SettingsState::SETTING_SERVICE_MOTOR_TEST;

        m_pContentFactory->buildUpServiceMotorTest();

        emit settingsStateChanged();
    }

    return success;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SettingsStatemachine::setServiceMotorTestAgsaState()
{
    bool success;

    switch ( m_nCurrentState )
    {
        case SETTING_SERVICE:
        case SETTING_SERVICE_MOTOR_TEST:
        case SETTING_SERVICE_MOTOR_TEST_AGSA:
        case SETTING_SERVICE_MOTOR_TEST_AGSA_MOVE_TO_DDD:
        case SETTING_SERVICE_MOTOR_TEST_AGSA_NUM_STEPS:
        case SETTING_SERVICE_MOTOR_TEST_AGSA_FREQUENCY:
            success = true;
            break;

        default:
            success = false;
            break;
    }

    if ( success )
    {
        m_nCurrentState = SettingsState::SETTING_SERVICE_MOTOR_TEST_AGSA;

        m_pContentFactory->buildUpServiceMotorTestAgsa();

        emit settingsStateChanged();
    }

    return success;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SettingsStatemachine::setServiceMotorTestAgsaMoveToDddState()
{
    bool success;

    switch ( m_nCurrentState )
    {
        case SETTING_SERVICE:
        case SETTING_SERVICE_MOTOR_TEST_AGSA:
        case SETTING_SERVICE_MOTOR_TEST_AGSA_MOVE_TO_DDD:
            success = true;
            break;

        default:
            success = false;
            break;
    }

    if ( success )
    {
        m_nCurrentState = SettingsState::SETTING_SERVICE_MOTOR_TEST_AGSA_MOVE_TO_DDD;

        m_pContentFactory->buildUpServiceMotorTestAgsaMoveToDdd();

        emit settingsStateChanged();
    }

    return success;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SettingsStatemachine::setServiceMotorTestAgsaNumStepsState()
{
    bool success;

    switch ( m_nCurrentState )
    {
        case SETTING_SERVICE:
        case SETTING_SERVICE_MOTOR_TEST_AGSA:
        case SETTING_SERVICE_MOTOR_TEST_AGSA_NUM_STEPS:
            success = true;
            break;

        default:
            success = false;
            break;
    }

    if ( success )
    {
        m_nCurrentState = SettingsState::SETTING_SERVICE_MOTOR_TEST_AGSA_NUM_STEPS;

        m_pContentFactory->buildUpServiceMotorTestAgsaNumSteps();

        emit settingsStateChanged();
    }

    return success;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SettingsStatemachine::setServiceMotorTestAgsaFrequencyState()
{
    bool success;

    switch ( m_nCurrentState )
    {
        case SETTING_SERVICE:
        case SETTING_SERVICE_MOTOR_TEST_AGSA:
        case SETTING_SERVICE_MOTOR_TEST_AGSA_FREQUENCY:
            success = true;
            break;

        default:
            success = false;
            break;
    }

    if ( success )
    {
        m_nCurrentState = SettingsState::SETTING_SERVICE_MOTOR_TEST_AGSA_FREQUENCY;

        m_pContentFactory->buildUpServiceMotorTestAgsaFrequency();

        emit settingsStateChanged();
    }

    return success;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SettingsStatemachine::setServiceMotorTestPduState()
{
    bool success;

    switch ( m_nCurrentState )
    {
        case SETTING_SERVICE:
        case SETTING_SERVICE_MOTOR_TEST:
        case SETTING_SERVICE_MOTOR_TEST_PDU:
            success = true;
            break;
        default:
            success = false;
    }

    if ( success )
    {
        m_nCurrentState = SettingsState::SETTING_SERVICE_MOTOR_TEST_PDU;

        m_pContentFactory->buildUpServiceMotorTestPdu();

        emit settingsStateChanged();
    }

    return success;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SettingsStatemachine::setServiceMotorTestAgsaLongDurationState()
{
    bool success;

    switch ( m_nCurrentState )
    {
        case SETTING_SERVICE:
        case SETTING_SERVICE_MOTOR_TEST:
        case SETTING_SERVICE_MOTOR_TEST_AGSA_LONG_DURATION:
            success = true;
            break;
        default:
            success = false;
    }

    if ( success )
    {
        m_nCurrentState = SettingsState::SETTING_SERVICE_MOTOR_TEST_AGSA_LONG_DURATION;

        m_pContentFactory->buildUpServiceMotorTestAgsaLongDuration();

        emit settingsStateChanged();
    }

    return success;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SettingsStatemachine::setServiceDddTestState()
{
    bool success;

    switch ( m_nCurrentState )
    {
        case SETTING_SERVICE:
        case SETTING_SERVICE_DDD_TEST:
            success = true;
            break;

        default:
            success = false;
            break;
    }

    if ( success )
    {
        m_nCurrentState = SettingsState::SETTING_SERVICE_DDD_TEST;

        m_pContentFactory->buildUpServiceDddTest();

        emit settingsStateChanged();
    }

    return success;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SettingsStatemachine::setServiceMcuConfigState()
{
    bool success;

    switch ( m_nCurrentState )
    {
        case SETTING_SERVICE:
        case SETTING_SERVICE_MCU_CONFIG:
        case SETTING_SERVICE_MCU_CONFIG_MAX_MOTOR_SPEED:
        case SETTING_SERVICE_MCU_CONFIG_NOMINAL_MOTOR_SPEED:
        case SETTING_SERVICE_MCU_CONFIG_ACCELERATION_TIME:
        case SETTING_SERVICE_MCU_CONFIG_DECELERATION_TIME:
            success = true;
            break;

        default:
            success = false;
            break;
    }

    if ( success )
    {
        m_nCurrentState = SettingsState::SETTING_SERVICE_MCU_CONFIG;

        m_pContentFactory->buildUpServiceMcuConfig();

        emit settingsStateChanged();
    }

    return success;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SettingsStatemachine::setServiceMcuConfigMaxMotorSpeedState()
{
    bool success = false;

    switch ( m_nCurrentState )
    {
        case SETTING_SERVICE_MCU_CONFIG:
        case SETTING_SERVICE_MCU_CONFIG_MAX_MOTOR_SPEED:
            success = true;
            break;
        default:
            success = false;
            break;
    }

    if ( success )
    {
        m_nCurrentState = SettingsState::SETTING_SERVICE_MCU_CONFIG_MAX_MOTOR_SPEED;

        m_pContentFactory->buildUpServiceMcuConfigMaxMotorSpeed();

        emit settingsStateChanged();
    }

    return success;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SettingsStatemachine::setServiceMcuConfigNominalMotorSpeedState()
{
    bool success;

    switch ( m_nCurrentState )
    {
        case SETTING_SERVICE_MCU_CONFIG:
        case SETTING_SERVICE_MCU_CONFIG_NOMINAL_MOTOR_SPEED:
            success = true;
            break;
        default:
            success = false;
    }

    if ( success )
    {
        m_nCurrentState = SettingsState::SETTING_SERVICE_MCU_CONFIG_NOMINAL_MOTOR_SPEED;

        m_pContentFactory->buildUpServiceMcuConfigNominalMotorSpeed();

        emit settingsStateChanged();
    }

    return success;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SettingsStatemachine::setServiceMcuConfigAccelerationTimeState()
{
    bool success;

    switch ( m_nCurrentState )
    {
        case SETTING_SERVICE_MCU_CONFIG:
        case SETTING_SERVICE_MCU_CONFIG_ACCELERATION_TIME:
            success = true;
            break;
        default:
            success = false;
    }

    if ( success )
    {
        m_nCurrentState = SettingsState::SETTING_SERVICE_MCU_CONFIG_ACCELERATION_TIME;

        m_pContentFactory->buildUpServiceMcuConfigAccelerationTime();

        emit settingsStateChanged();
    }

    return success;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SettingsStatemachine::setServiceMcuConfigDecelerationTimeState()
{
    bool success;

    switch ( m_nCurrentState )
    {
        case SETTING_SERVICE_MCU_CONFIG:
        case SETTING_SERVICE_MCU_CONFIG_DECELERATION_TIME:
            success = true;
            break;
        default:
            success = false;
    }

    if ( success )
    {
        m_nCurrentState = SettingsState::SETTING_SERVICE_MCU_CONFIG_DECELERATION_TIME;

        m_pContentFactory->buildUpServiceMcuConfigDecelerationTime();

        emit settingsStateChanged();
    }

    return success;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SettingsStatemachine::setServiceSystemSetupState()
{
    bool success;

    switch ( m_nCurrentState )
    {
        case SETTING_SERVICE:
        case SETTING_SERVICE_SYSTEM_SETUP:
            success = true;
            break;

        default:
            success = false;
            break;
    }

    if ( success )
    {
        m_nCurrentState = SettingsState::SETTING_SERVICE_SYSTEM_SETUP;

        m_pContentFactory->buildUpServiceSystemSetup();

        emit settingsStateChanged();
    }

    return success;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SettingsStatemachine::setUserCPCState()
{
    bool success;

    switch ( m_nCurrentState )
    {
        case SETTING_USER:
        case SETTING_USER_CPC:
        case SETTING_USER_CPC_STORE:
        case SETTING_USER_CPC_TECH:
            success = true;
            break;
        default:
            success = false;
    }

    if ( success )
    {
        m_nCurrentState = SettingsState::SETTING_USER_CPC;

        m_pContentFactory->buildUpUserCpc();

        emit settingsStateChanged();
    }

    return success;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SettingsStatemachine::setUserCPCStoreState()
{
    bool success;

    switch ( m_nCurrentState )
    {

        case SETTING_USER_CPC:
        case SETTING_USER_CPC_STORE:
            success = true;
            break;
        default:
            success = false;
    }

    if ( success )
    {
        m_nCurrentState = SettingsState::SETTING_USER_CPC_STORE;

        m_pContentFactory->buildUpUserCpcStore();

        emit settingsStateChanged();
    }

    return success;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SettingsStatemachine::setUserCPCTechState()
{
    bool success;

    switch ( m_nCurrentState )
    {

        case SETTING_USER_CPC:
        case SETTING_USER_CPC_TECH:
            success = true;
            break;

        default:
            success = false;
    }

    if ( success )
    {
        m_nCurrentState = SettingsState::SETTING_USER_CPC_TECH;

        m_pContentFactory->buildUpUserCpcTech();

        emit settingsStateChanged();
    }

    return success;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SettingsStatemachine::setSystemWifiState()
{
    bool success;

    switch ( m_nCurrentState )
    {
        case SETTING_SYSTEM:
        case SETTING_SYSTEM_WIFI_SETTING:
        case SETTING_SYSTEM_WIFI_SETTING_IP:
        case SETTING_SYSTEM_WIFI_SETTING_PW:
        case SETTING_SYSTEM_WIFI_SETTING_SSID:
            success = true;
            break;
        default:
            success = false;
    }

    if ( success )
    {
        m_nCurrentState = SettingsState::SETTING_SYSTEM_WIFI_SETTING;

        m_pContentFactory->buildUpSystemWifi();

        emit settingsStateChanged();
    }

    return success;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SettingsStatemachine::setSystemWifiIpState()
{
    bool success;

    switch ( m_nCurrentState )
    {
        case SETTING_SYSTEM_WIFI_SETTING:
        case SETTING_SYSTEM_WIFI_SETTING_IP:
            success = true;
            break;
        default:
            success = false;
    }

    if ( success )
    {
        m_nCurrentState = SettingsState::SETTING_SYSTEM_WIFI_SETTING_IP;

        m_pContentFactory->buildUpSystemWifiIp();

        emit settingsStateChanged();
    }

    return success;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SettingsStatemachine::setSystemWifiPasswordState()
{
    bool success;

    switch ( m_nCurrentState )
    {
        case SETTING_SYSTEM_WIFI_SETTING:
        case SETTING_SYSTEM_WIFI_SETTING_PW:
            success = true;
            break;
        default:
            success = false;
    }

    if ( success )
    {
        m_nCurrentState = SettingsState::SETTING_SYSTEM_WIFI_SETTING_PW;

        m_pContentFactory->buildUpSystemWifiPassword();

        emit settingsStateChanged();
    }

    return success;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SettingsStatemachine::setSystemWifiSsidState()
{
    bool success;

    switch ( m_nCurrentState )
    {
        case SETTING_SYSTEM_WIFI_SETTING:
        case SETTING_SYSTEM_WIFI_SETTING_SSID:
            success = true;
            break;
        default:
            success = false;
    }

    if ( success )
    {
        m_nCurrentState = SettingsState::SETTING_SYSTEM_WIFI_SETTING_SSID;

        m_pContentFactory->buildUpSystemWifiSsid();

        emit settingsStateChanged();
    }

    return success;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SettingsStatemachine::setUserState()
{
    bool success;

    switch ( m_nCurrentState )
    {
        case SETTING_MAIN:
        case SETTING_USER:
        case SETTING_USER_CPC:
        case SETTING_USER_LOGIN:
        case SETTING_USER_LOGOUT:
            success = true;
            break;
        default:
            success = false;
    }

    if ( success )
    {
        m_nCurrentState = SettingsState::SETTING_USER;
        m_pContentFactory->buildUpUser();
        emit settingsStateChanged();
    }

    return success;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SettingsStatemachine::setUserLoginState()
{
    bool success;

    switch ( m_nCurrentState )
    {

        case SETTING_USER:
        case SETTING_USER_LOGIN:
        case SETTING_USER_STORE:
        case SETTING_USER_TECHNICIAN:
            success = true;
            break;
        default:
            success = false;
    }

    if ( success )
    {
        m_nCurrentState = SettingsState::SETTING_USER_LOGIN;
        m_pContentFactory->buildUpUserLogin();
        emit settingsStateChanged();
    }

    return success;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SettingsStatemachine::setUserLogoutState()
{
    bool success;

    switch ( m_nCurrentState )
    {

        case SETTING_USER:
        case SETTING_USER_LOGOUT:
            success = true;
            break;
        default:
            success = false;
    }

    if ( success )
    {
        m_nCurrentState = SettingsState::SETTING_USER_LOGOUT;
        m_pContentFactory->buildUpUserLogout();
        emit settingsStateChanged();
    }

    return success;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SettingsStatemachine::setUserStoreState()
{
    bool success;

    switch ( m_nCurrentState )
    {
        case SETTING_USER_LOGIN:
        case SETTING_USER_STORE:
            success = true;
            break;
        default:
            success = false;
    }

    if ( success )
    {
        m_nCurrentState = SettingsState::SETTING_USER_STORE;
        m_pContentFactory->buildUpUserStore();
        emit settingsStateChanged();
    }

    return success;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SettingsStatemachine::setUserTechState()
{
    bool success;

    switch ( m_nCurrentState )
    {
        case SETTING_USER_LOGIN:
        case SETTING_USER_TECHNICIAN:
            success = true;
            break;
        default:
            success = false;
    }

    if ( success )
    {
        m_nCurrentState = SettingsState::SETTING_USER_TECHNICIAN;
        m_pContentFactory->buildUpUserTech();
        emit settingsStateChanged();
    }

    return success;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SettingsStatemachine::setSystemState()
{
    bool success;

    switch ( m_nCurrentState )
    {
        case SETTING_MAIN:
        case SETTING_SYSTEM:
        case SETTING_SYSTEM_WIFI_SETTING:
        case SETTING_SYSTEM_DATE:
        case SETTING_SYSTEM_TIME:
        case SETTING_SYSTEM_GRINDERNAME:
        case SETTING_SYSTEM_FACTORY_RESET:
        case SETTING_SYSTEM_TEMP_UNIT:
        case SETTING_SYSTEM_START_OF_WEEK:
            success = true;
            break;
        default:
            success = false;
    }

    if ( success )
    {
        m_nCurrentState = SettingsState::SETTING_SYSTEM;
        m_pContentFactory->buildUpSystem();
        emit settingsStateChanged();
    }

    return success;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SettingsStatemachine::setSystemDateState()
{
    bool success;

    switch ( m_nCurrentState )
    {
        case SETTING_SYSTEM:
        case SETTING_SYSTEM_DATE:
            success = true;
            break;
        default:
            success = false;
    }

    if ( success )
    {
        m_nCurrentState = SettingsState::SETTING_SYSTEM_DATE;
        m_pContentFactory->buildUpSystemDate();
        emit settingsStateChanged();
    }

    return success;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SettingsStatemachine::setSystemTimeState()
{
    bool success;

    switch ( m_nCurrentState )
    {
        case SETTING_SYSTEM:
        case SETTING_SYSTEM_TIME:
            success = true;
            break;
        default:
            success = false;
    }

    if ( success )
    {
        m_nCurrentState = SettingsState::SETTING_SYSTEM_TIME;
        m_pContentFactory->buildUpSystemTime();
        emit settingsStateChanged();
    }

    return success;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SettingsStatemachine::setSystemGrinderNameState()
{
    bool success;

    switch ( m_nCurrentState )
    {
        case SETTING_SYSTEM:
        case SETTING_SYSTEM_GRINDERNAME:
            success = true;
            break;
        default:
            success = false;
    }

    if ( success )
    {
        m_nCurrentState = SettingsState::SETTING_SYSTEM_GRINDERNAME;
        m_pContentFactory->buildUpSystemGrindername();
        emit settingsStateChanged();
    }

    return success;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SettingsStatemachine::setSystemFactoryResState()
{
    bool success;

    switch ( m_nCurrentState )
    {
        case SETTING_SYSTEM:
        case SETTING_SYSTEM_FACTORY_RESET:
            success = true;
            break;
        default:
            success = false;
    }

    if ( success )
    {
        m_nCurrentState = SettingsState::SETTING_SYSTEM_FACTORY_RESET;
        m_pContentFactory->buildUpSystemFactoryReset();
        emit settingsStateChanged();
    }

    return success;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SettingsStatemachine::setSystemStartOfWeekState()
{
    bool success;

    switch ( m_nCurrentState )
    {
        case SETTING_SYSTEM:
        case SETTING_SYSTEM_START_OF_WEEK:
            success = true;
            break;
        default:
            success = false;
    }

    if ( success )
    {
        m_nCurrentState = SettingsState::SETTING_SYSTEM_START_OF_WEEK;

        m_pContentFactory->buildUpSystemStartOfWeek();

        emit settingsStateChanged();
    }

    return success;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SettingsStatemachine::setCustomizeStdbyAdjState()
{
    bool success;

    switch ( m_nCurrentState )
    {
        case SETTING_CUSTOMIZE:
        case SETTING_CUSTOMIZE_STDBY_ADJ:
            success = true;
            break;
        default:
            success = false;
    }

    if ( success )
    {
        m_nCurrentState = SettingsState::SETTING_CUSTOMIZE_STDBY_ADJ;

        m_pContentFactory->buildUpCustomizeStandbyAdj();

        emit settingsStateChanged();
    }

    return success;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SettingsStatemachine::setCustomizeSettingsModeExitTimeoutAdjState()
{
    bool success;

    switch ( m_nCurrentState )
    {
        case SETTING_CUSTOMIZE:
        case SETTING_CUSTOMIZE_SETTINGS_MODE_EXIT_TIMEOUT_ADJ:
            success = true;
            break;
        default:
            success = false;
    }

    if ( success )
    {
        m_nCurrentState = SettingsState::SETTING_CUSTOMIZE_SETTINGS_MODE_EXIT_TIMEOUT_ADJ;

        m_pContentFactory->buildUpCustomizeSettingsModeExitTimeoutAdj();

        emit settingsStateChanged();
    }

    return success;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SettingsStatemachine::setCustomizeAgsaDelayState()
{
    bool success;

    switch ( m_nCurrentState )
    {
        case SETTING_CUSTOMIZE:
        case SETTING_CUSTOMIZE_AGSA_DELAY:
            success = true;
            break;
        default:
            success = false;
    }

    if ( success )
    {
        m_nCurrentState = SettingsState::SETTING_CUSTOMIZE_AGSA_DELAY;

        m_pContentFactory->buildUpCustomizeAgsaDelay();

        emit settingsStateChanged();
    }

    return success;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SettingsStatemachine::setInfoState()
{
    bool success;

    switch ( m_nCurrentState )
    {
        case SETTING_MAIN:
        case SETTING_INFO:
        case SETTING_INFO_STATISTICS:
        case SETTING_INFO_NOTIFICATION_CENTER:
        case SETTING_INFO_MACHINE_INFO:
        case SETTING_INFO_OWNERS_MANUAL:
        case SETTING_INFO_GRINDER_TUTORIAL:
        case SETTING_INFO_PDU_TUTORIAL:
            success = true;
            break;
        default:
            success = false;
    }

    if ( success )
    {
        m_nCurrentState = SettingsState::SETTING_INFO;

        m_pContentFactory->buildUpInfo();

        emit settingsStateChanged();
    }

    return success;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SettingsStatemachine::setInfoStatisticsState()
{
    bool success;

    switch ( m_nCurrentState )
    {

        case SETTING_INFO:
        case SETTING_INFO_STATISTICS:
            success = true;
            break;
        default:
            success = false;
    }

    if ( success )
    {
        m_nCurrentState = SettingsState::SETTING_INFO_STATISTICS;

        m_pContentFactory->buildUpInfoStatistics();

        emit settingsStateChanged();
    }

    return success;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SettingsStatemachine::setInfoNotificationCenterState()
{
    bool success;

    switch ( m_nCurrentState )
    {
        case SETTING_INFO:
        case SETTING_INFO_NOTIFICATION_CENTER:
            success = true;
            break;
        default:
            success = false;
    }

    if ( success )
    {
        m_nCurrentState = SettingsState::SETTING_INFO_NOTIFICATION_CENTER;

        m_pContentFactory->buildUpInfoNotificationCenter();

        emit settingsStateChanged();
    }

    return success;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SettingsStatemachine::setInfoMachineInfoState()
{
    bool success;

    switch ( m_nCurrentState )
    {
        case SETTING_INFO:
        case SETTING_INFO_MACHINE_INFO:
            success = true;
            break;
        default:
            success = false;
    }

    if ( success )
    {
        m_nCurrentState = SettingsState::SETTING_INFO_MACHINE_INFO;
        m_pContentFactory->buildUpInfoMachineInfo();
        emit settingsStateChanged();
    }

    return success;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SettingsStatemachine::setInfoOwnersManualState()
{
    bool success;

    switch ( m_nCurrentState )
    {
        case SETTING_INFO:
        case SETTING_INFO_MACHINE_INFO:
            success = true;
            break;
        default:
            success = false;
    }

    if ( success )
    {
        m_nCurrentState = SettingsState::SETTING_INFO_OWNERS_MANUAL;
        m_pContentFactory->buildUpInfoOwnersManual();
        emit settingsStateChanged();
    }

    return success;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SettingsStatemachine::setInfoGrinderTutorialState()
{
    bool success;

    switch ( m_nCurrentState )
    {
        case SETTING_INFO:
        case SETTING_INFO_GRINDER_TUTORIAL:
            success = true;
            break;
        default:
            success = false;
    }

    if ( success )
    {
        m_nCurrentState = SettingsState::SETTING_INFO_GRINDER_TUTORIAL;
        m_pContentFactory->buildUpInfoGrinderTutorial();
        emit settingsStateChanged();
    }

    return success;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SettingsStatemachine::setInfoPduTutorialState()
{
    bool success;

    switch ( m_nCurrentState )
    {
        case SETTING_INFO:
        case SETTING_INFO_PDU_TUTORIAL:
            success = true;
            break;
        default:
            success = false;
    }

    if ( success )
    {
        m_nCurrentState = SettingsState::SETTING_INFO_PDU_TUTORIAL;
        m_pContentFactory->buildUpInfoPduTutorial();
        emit settingsStateChanged();
    }

    return success;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

DiskUsageTime & SettingsStatemachine::diskUsageTime()
{
    return *m_pDiskUsageTime;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

SettingsMenuContentFactory & SettingsStatemachine::contentFactory()
{
    return *m_pContentFactory;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

StatisticsBoard & SettingsStatemachine::statisticsBoard()
{
    return *m_pStatisticsBoard;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsStatemachine::processMainstateChanged()
{
    backToMainMenu();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////
