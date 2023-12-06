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

#include <QQmlContext>
#include <QtDebug>
#include <QDateTime>
#include <QHostAddress>
#include <QNetworkInterface>

#include "SettingsStatemachine.h"
#include "SettingsMenu.h"
#include "MainStatemachine.h"
#include "StatisticsBoard.h"
#include "DiskUsageTime.h"

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

int SettingsStatemachine::SettingStates_QTypeId = qRegisterMetaType<SettingsStatemachine::SettingStates>( "SettingsStatemachine::SettingStates" );
int SettingsStatemachine::QmlSettingStatesId    = qmlRegisterUncreatableType<SettingsStatemachine>( "SettingStatesEnum", 1, 0, "SettingStatesEnum", "cannot create Object because SettingsStatemachine was registered as SettingStatesEnum to be  uncreational " );

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

SettingsStatemachine::SettingsStatemachine( QQmlEngine* engine,
                                            MainStatemachine * pMainStatemachine,
                                            SettingsSerializer* settingsSerializer,
                                            SqliteInterface* sqlInterface )
    : QObject(pMainStatemachine)
    , ThisQMLEngine( engine )
    , m_pMainStatemachine( pMainStatemachine )
    , m_statisticsBoard( new StatisticsBoard(engine, sqlInterface, this) )
    , m_diskUsageTime( new DiskUsageTime(engine, sqlInterface, this) )
    , ThisSettingsSerializer( settingsSerializer )
    , ThisCurrentState(SettingStates::SETTING_MAIN)
    , ThisLastSettingState(SettingStates::SETTING_INVALID)
    , ThisMenuTitle("")
    , ThisNavAvailable( true )
    , m_buttonPreviousVisible(false)
    , m_buttonOkVisible(false)
    , ThisQlpMenuList( this, ThisMenuList )
    , oldStates(10)
    , oldPosition(10)
    

{
    m_statisticsBoard->setDiskUsageMaxHours( ThisSettingsSerializer->getDiscUsageMaxHours() );
    connect(ThisSettingsSerializer, &SettingsSerializer::discUsageMaxHoursChanged, m_statisticsBoard, &StatisticsBoard::setDiskUsageMaxHours );

    m_diskUsageTime->setEstimatedMax( ThisSettingsSerializer->getDiscUsageMaxHours() );
    connect(ThisSettingsSerializer, &SettingsSerializer::discUsageMaxHoursChanged, m_diskUsageTime, &DiskUsageTime::setEstimatedMax );

    ThisQMLEngine->rootContext()->setContextProperty("settingsStateMachine" , this);
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

SettingsStatemachine::~SettingsStatemachine()
{
    emptyMenuList();
    oldStates.clear();
    oldPosition.clear();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsStatemachine::create()
{
    m_statisticsBoard->setPeriodChoicesIndex( ThisSettingsSerializer->statisticsPeriodIndex() );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

SettingsStatemachine::SettingStates SettingsStatemachine::getSettingState()
{
    return ThisCurrentState;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsStatemachine::navigateToMenu(SettingsStatemachine::SettingStates settingState)
{
    setSettingState( settingState );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsStatemachine::back()
{
    if( !oldStates.isEmpty() )
    {
        setSettingState(oldStates.last(), true);
        oldStates.removeLast();

        emit navigatedPrevious();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsStatemachine::saveMenuPosition( double newPosition )
{
    oldPosition.append(newPosition);
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

double SettingsStatemachine::getSavedMenuPostion()
{
    double position = 0.0;
    if( !oldPosition.isEmpty() )
    {
        position = oldPosition.last();
        oldPosition.removeLast();
    }

    return position;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsStatemachine::backToMainMenu()
{    if( !oldStates.isEmpty() )
    {
        oldStates.clear();
        oldPosition.clear();
        setSettingState(SETTING_MAIN);
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool  SettingsStatemachine::getNavAvailable()
{
    return ThisNavAvailable;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsStatemachine::setNavAvailable( bool newNavAvailable  )
{
    if ( newNavAvailable != ThisNavAvailable )
    {
        ThisNavAvailable = newNavAvailable;

        emit navAvailableChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SettingsStatemachine::getButtonPreviousVisible(void)
{
    return(m_buttonPreviousVisible);
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsStatemachine::setButtonPreviousVisible(bool buttonPreviousVisible)
{
    if ( m_buttonPreviousVisible != buttonPreviousVisible )
    {
        m_buttonPreviousVisible = buttonPreviousVisible;

        emit buttonPreviousVisibleChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SettingsStatemachine::getButtonOkVisible(void)
{
    return(m_buttonOkVisible);
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsStatemachine::setButtonOkVisible(bool buttonOkVisible)
{
    if ( m_buttonOkVisible != buttonOkVisible )
    {
        m_buttonOkVisible = buttonOkVisible;

        emit buttonOkVisibleChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

QString  SettingsStatemachine::getMenuTitle()
{
    return ThisMenuTitle;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsStatemachine::setMenuTitle( QString newMenuTitle  )
{
    if ( newMenuTitle != ThisMenuTitle )
    {
        ThisMenuTitle = newMenuTitle;

        emit menuTitleChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SettingsStatemachine::checkPinCode( QString pin,
                                         SettingsSerializer::userRole user,
                                         bool logIn )
{
    bool pinCheckSuccess = false;

    switch(user)
    {
        case SettingsSerializer::LOGGEDIN_STOREOWNER:
            if( pin == ThisSettingsSerializer->getUserPinForOwner() )
            {
                pinCheckSuccess = true;
                if( logIn )
                {
                    ThisSettingsSerializer->setUserAutoLoginAs( SettingsSerializer::LOGGEDIN_STOREOWNER );
                    ThisSettingsSerializer->saveSettings();
                }
            }
        break;
        case SettingsSerializer::LOGGEDIN_TECHNICIAN:
            if( pin == ThisSettingsSerializer->getUserPinForTechnician() )
            {
                pinCheckSuccess = true;
                if( logIn )
                {
                    ThisSettingsSerializer->setUserAutoLoginAs( SettingsSerializer::LOGGEDIN_TECHNICIAN );
                    ThisSettingsSerializer->saveSettings();
                }
            }
        break;
        case SettingsSerializer::LOGGEDIN_NONE:
        default:
            pinCheckSuccess = false;

    }
    qDebug() << "checkPin returning" << pinCheckSuccess;
    return pinCheckSuccess;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SettingsStatemachine::checkPinCode(QString pin, int user, bool logIn)
{
    return( checkPinCode(pin, static_cast<SettingsSerializer::userRole>(user), logIn) );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool  SettingsStatemachine::savePinCode( QString pin,
                                         SettingsSerializer::userRole user)
{
    bool pinSaveSuccess = false;

    switch(user)
    {
        case SettingsSerializer::LOGGEDIN_STOREOWNER:
            ThisSettingsSerializer->setUserPinForOwner( pin );
            pinSaveSuccess = true;
        break;
        case SettingsSerializer::LOGGEDIN_TECHNICIAN:
            ThisSettingsSerializer->setUserPinForTechnician( pin );
            pinSaveSuccess = true;
        break;
        case SettingsSerializer::LOGGEDIN_NONE:
        default:
            pinSaveSuccess = false;

    }
    if( pinSaveSuccess )
    {
        ThisSettingsSerializer->saveSettings();
    }
    return pinSaveSuccess;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SettingsStatemachine::savePinCode(QString pin, int user)
{
    return( savePinCode(pin, static_cast<SettingsSerializer::userRole>(user)) );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsStatemachine::logout()
{
    if( ThisSettingsSerializer->getUserAutoLoginAs() != SettingsSerializer::LOGGEDIN_NONE)
    {
        ThisSettingsSerializer->setUserAutoLoginAs( SettingsSerializer::LOGGEDIN_NONE );
        ThisSettingsSerializer->saveSettings();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsStatemachine::menuToggleswitchChangeHandler( SettingsMenu::ToggleswitchTypes toggleswitchType,
                                                          bool currentValue  )
{
    switch(toggleswitchType)
    {
        case SettingsMenu::TOGGLESWITCH_FOR_SHOWTEMP:
            ThisSettingsSerializer->setShowTemp(currentValue);
            ThisSettingsSerializer->saveSettings();
        break;
        case SettingsMenu::TOGGLESWITCH_FOR_WARNINGTEMP:
            ThisSettingsSerializer->setShowTempWarning(currentValue);
            setSettingState(ThisCurrentState, true);
            ThisSettingsSerializer->saveSettings();
        break;
        case SettingsMenu::TOGGLESWITCH_FOR_WARNINGS:
            ThisSettingsSerializer->setShowWarnings(currentValue);
            ThisSettingsSerializer->saveSettings();
        break;
        case SettingsMenu::TOGGLESWITCH_FOR_LIBRARYMODE_ENABLING:
            ThisSettingsSerializer->setLibraryModeIsEnabled(currentValue);
            ThisSettingsSerializer->saveSettings();
        break;
        case SettingsMenu::TOGGLESWITCH_FOR_TIMEMODE_ENABLING:
            ThisSettingsSerializer->setTimeModeIsEnabled(currentValue);
            ThisSettingsSerializer->saveSettings();
        break;
        case SettingsMenu::TOGGLESWITCH_FOR_SHOW_DDD:
            ThisSettingsSerializer->setShowDdd(currentValue);
            ThisSettingsSerializer->saveSettings();
        break;
        case SettingsMenu::TOGGLESWITCH_FOR_SHOWSPOT:
            ThisSettingsSerializer->setShowSpotlight(currentValue);
            ThisSettingsSerializer->saveSettings();
        break;
        case SettingsMenu::TOGGLESWITCH_FOR_WIFI:
            m_pMainStatemachine->espDriver()->wifiDriver()->setActive( currentValue );
            setSettingState( ThisCurrentState, true );
            ThisSettingsSerializer->setWifiEnable(currentValue);
            ThisSettingsSerializer->saveSettings();
        break;
        case SettingsMenu::TOGGLESWITCH_FOR_AUTOLOG:
            ThisSettingsSerializer->setUserAutoLogoutEnable(currentValue);
            ThisSettingsSerializer->saveSettings();
        break;
        default:
            ;
            //do nothing;
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SettingsStatemachine::setSettingState( SettingsStatemachine::SettingStates newSettingState,
                                            bool backUsed)
{
    bool success = true;

    if( !backUsed )
    {
        oldStates.append(ThisCurrentState);
    }

    setNavAvailable(true);

    switch ( newSettingState )
    {
        case SETTING_MAIN:
            success = setMainMenuState();
        break;
        case SETTING_DISCS:
            success = setDiscsState();
        break;
        case SETTING_DISCS_CALIB:
            success = setDiscsCalibrationState();
        break;
        case SETTING_DISCS_USAGE_TIME:
            success = setDiscsUsageTimeState();
        break;
        case SETTING_SYSTEM:
            success = setSystemState();
        break;
        case SETTING_SYSTEM_WIFI_SETTING:
            success = setSystemWifiSettingState();
        break;
        case SETTING_SYSTEM_WIFI_SETTING_IP:
            success = setSystemWifiIPSettingState();
        break;
        case SETTING_SYSTEM_WIFI_SETTING_PW:
            success = setSystemWifiPWSettingState();
        break;
        case SETTING_SYSTEM_WIFI_SETTING_SSID:
            success = setSystemWifiSSIDSettingState();
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
        case SETTING_SYSTEM_MACHINEINFO:
            success = setSystemMachineInfoState();
        break;
        case SETTING_SYSTEM_FACTORY_RESET:
            success = setSystemFactoryResState();
        break;
        case SETTING_SYSTEM_DISPLAY_TEST:
            success = setSystemDisplayTestState();
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
            success = setCustomizeBrigState();
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
        case SETTING_CUSTOMIZE_STDBY_ADJ:
            success = setCustomizeStdbyAdjState();
        break;
        case SETTING_INFO:
            success = setInfoState();
        break;
        case SETTING_INFO_NOTIF:
            success = setInfoNotifState();
        break;
        case SETTING_INFO_STATISTIC:
            success = setInfoStatisticsState();
        break;
        default:
            success = false;
    }

    if(!success)
    {
        if( !oldStates.isEmpty() && !backUsed )
        {
            oldStates.removeLast();
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

    switch ( ThisCurrentState )
    {
        case SETTING_MAIN:
        case SETTING_DISCS:
        case SETTING_DISCS_CALIB:
        case SETTING_DISCS_USAGE_TIME:
        case SETTING_CUSTOMIZE:
        case SETTING_CUSTOMIZE_TEMP:
        case SETTING_SYSTEM_TEMP_UNIT:
        case SETTING_CUSTOMIZE_TEMP_THRESH:
        case SETTING_CUSTOMIZE_BRIGHTNESS:
        case SETTING_USER_CPC:
        case SETTING_USER_CPC_STORE:
        case SETTING_USER_CPC_TECH:
        case SETTING_SYSTEM_WIFI_SETTING:
        case SETTING_SYSTEM_WIFI_SETTING_IP:
        case SETTING_SYSTEM_WIFI_SETTING_PW:
        case SETTING_SYSTEM_WIFI_SETTING_SSID:
        case SETTING_USER:
        case SETTING_USER_LOGIN:
        case SETTING_USER_LOGOUT:
        case SETTING_USER_STORE:
        case SETTING_USER_TECHNICIAN:
        case SETTING_SYSTEM:
        case SETTING_SYSTEM_DATE:
        case SETTING_SYSTEM_TIME:
        case SETTING_SYSTEM_GRINDERNAME:
        case SETTING_SYSTEM_MACHINEINFO:
        case SETTING_SYSTEM_FACTORY_RESET:
        case SETTING_SYSTEM_DISPLAY_TEST:
        case SETTING_SYSTEM_START_OF_WEEK:
        case SETTING_CUSTOMIZE_STDBY_ADJ:
        case SETTING_INFO:
        case SETTING_INFO_NOTIF:
        case SETTING_INFO_STATISTIC:
            success = true;
        break;
        default:
            success = false;
    }

    if( success )
    {
        ThisCurrentState = SettingStates::SETTING_MAIN;
        //buildUpMainState();

        setMenuTitle("Settings");
        setButtonPreviousVisible(false);
        setButtonOkVisible(false);
        setNavAvailable(false);

        emit settingStateChanged();
    }

    return success;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SettingsStatemachine::setDiscsState()
{
    bool success;

    switch ( ThisCurrentState )
    {
        case SETTING_MAIN:
        case SETTING_DISCS:
        case SETTING_DISCS_CALIB:
        case SETTING_DISCS_USAGE_TIME:
            success = true;
        break;
        default:
            success = false;
    }

    if( success )
    {
        ThisCurrentState = SettingStates::SETTING_DISCS;

        buildUpDiscsSetting();

        emit settingStateChanged();
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

    switch ( ThisCurrentState )
    {
        case SETTING_MAIN:
        case SETTING_CUSTOMIZE:
        case SETTING_CUSTOMIZE_TEMP:
        case SETTING_CUSTOMIZE_BRIGHTNESS:
        case SETTING_CUSTOMIZE_STDBY_ADJ:
            success = true;
        break;
        default:
            success = false;
    }

    if( success )
    {
        ThisCurrentState = SettingStates::SETTING_CUSTOMIZE;

        buildUpCustomizeSetting();

        emit settingStateChanged();
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

    switch ( ThisCurrentState )
    {
        case SETTING_CUSTOMIZE:
        case SETTING_CUSTOMIZE_TEMP:
        case SETTING_CUSTOMIZE_TEMP_THRESH:
            success = true;
        break;
        default:
            success = false;
    }

    if( success )
    {
        ThisCurrentState = SettingStates::SETTING_CUSTOMIZE_TEMP;

        buildUpCustomizeTempSetting();

        emit settingStateChanged();
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

    switch ( ThisCurrentState )
    {
        case SETTING_SYSTEM:
        case SETTING_SYSTEM_TEMP_UNIT:
            success = true;
        break;
        default:
            success = false;
    }

    if( success )
    {
        ThisCurrentState = SettingStates::SETTING_SYSTEM_TEMP_UNIT;

        buildUpSystemTempUnitSetting();

        emit settingStateChanged();
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

    switch ( ThisCurrentState )
    {
        case SETTING_CUSTOMIZE_TEMP:
        case SETTING_CUSTOMIZE_TEMP_THRESH:
            success = true;
        break;
        default:
            success = false;
    }

    if( success )
    {
        ThisCurrentState = SettingStates::SETTING_CUSTOMIZE_TEMP_THRESH;

        buildUpCustomizeTempThreshSetting();

        emit settingStateChanged();
    }

    return success;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SettingsStatemachine::setCustomizeBrigState()
{
    bool success;

    switch ( ThisCurrentState )
    {
        case SETTING_CUSTOMIZE:
        case SETTING_CUSTOMIZE_BRIGHTNESS:
            success = true;
        break;
        default:
            success = false;
    }

    if( success )
    {
        ThisCurrentState = SettingStates::SETTING_CUSTOMIZE_BRIGHTNESS;

        buildUpCustomizeBrigSetting();

        emit settingStateChanged();
    }

    return success;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SettingsStatemachine::setDiscsCalibrationState()
{
    bool success;

    switch ( ThisCurrentState )
    {
        case SETTING_DISCS:
        case SETTING_DISCS_CALIB:
            success = true;
        break;
        default:
            success = false;
    }

    if( success )
    {
        ThisCurrentState = SettingStates::SETTING_DISCS_CALIB;

        buildUpDiscsCalibrationSetting();

        emit settingStateChanged();
    }

    return success;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SettingsStatemachine::setDiscsUsageTimeState()
{
    bool success;

    switch ( ThisCurrentState )
    {
        case SETTING_DISCS:
        case SETTING_DISCS_USAGE_TIME:
            success = true;
        break;
        default:
            success = false;
    }

    if( success )
    {
        ThisCurrentState = SettingStates::SETTING_DISCS_USAGE_TIME;

        buildUpDiscsUsageTimeSetting();

        emit settingStateChanged();
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

    switch ( ThisCurrentState )
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

    if( success )
    {
        ThisCurrentState = SettingStates::SETTING_USER_CPC;

        buildUpUserCpcSetting();

        emit settingStateChanged();
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

    switch ( ThisCurrentState )
    {

        case SETTING_USER_CPC:
        case SETTING_USER_CPC_STORE:
            success = true;
        break;
        default:
            success = false;
    }

    if( success )
    {
        ThisCurrentState = SettingStates::SETTING_USER_CPC_STORE;

        buildUpUserCpcStoreSetting();

        emit settingStateChanged();
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

    switch ( ThisCurrentState )
    {

        case SETTING_USER_CPC:
        case SETTING_USER_CPC_TECH:
            success = true;
        break;

        default:
            success = false;
    }

    if( success )
    {
        ThisCurrentState = SettingStates::SETTING_USER_CPC_TECH;

        buildUpUserCpcTechSetting();

        emit settingStateChanged();
    }

    return success;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SettingsStatemachine::setSystemWifiSettingState()
{
    bool success;

    switch ( ThisCurrentState )
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

    if( success )
    {
        ThisCurrentState = SettingStates::SETTING_SYSTEM_WIFI_SETTING;

        buildUpSystemWifiSettings();

        emit settingStateChanged();
    }

    return success;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SettingsStatemachine::setSystemWifiIPSettingState()
{
    bool success;

    switch ( ThisCurrentState )
    {
        case SETTING_SYSTEM_WIFI_SETTING:
        case SETTING_SYSTEM_WIFI_SETTING_IP:
            success = true;
        break;
        default:
            success = false;
    }

    if( success )
    {
        ThisCurrentState = SettingStates::SETTING_SYSTEM_WIFI_SETTING_IP;

        buildUpSystemWifiIPSettings();

        emit settingStateChanged();
    }

    return success;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SettingsStatemachine::setSystemWifiPWSettingState()
{
    bool success;

    switch ( ThisCurrentState )
    {
        case SETTING_SYSTEM_WIFI_SETTING:
        case SETTING_SYSTEM_WIFI_SETTING_PW:
            success = true;
        break;
        default:
            success = false;
    }

    if( success )
    {
        ThisCurrentState = SettingStates::SETTING_SYSTEM_WIFI_SETTING_PW;

        buildUpSystemWifiPWSettings();

        emit settingStateChanged();
    }

    return success;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SettingsStatemachine::setSystemWifiSSIDSettingState()
{
    bool success;

    switch ( ThisCurrentState )
    {
        case SETTING_SYSTEM_WIFI_SETTING:
        case SETTING_SYSTEM_WIFI_SETTING_SSID:
            success = true;
        break;
        default:
            success = false;
    }

    if( success )
    {
        ThisCurrentState = SettingStates::SETTING_SYSTEM_WIFI_SETTING_SSID;

        buildUpSystemWifiSSIDSettings();

        emit settingStateChanged();
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

    switch ( ThisCurrentState )
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

    if( success )
    {
        ThisCurrentState = SettingStates::SETTING_USER;
        buildUpUserSettings();
        emit settingStateChanged();
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

    switch ( ThisCurrentState )
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

    if( success )
    {
        ThisCurrentState = SettingStates::SETTING_USER_LOGIN;
        buildUpUserLoginSettings();
        emit settingStateChanged();
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

    switch ( ThisCurrentState )
    {

        case SETTING_USER:
        case SETTING_USER_LOGOUT:
            success = true;
        break;
        default:
            success = false;
    }

    if( success )
    {
        ThisCurrentState = SettingStates::SETTING_USER_LOGOUT;
        buildUpUserLogoutSettings();
        emit settingStateChanged();
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

    switch ( ThisCurrentState )
    {
        case SETTING_USER_LOGIN:
        case SETTING_USER_STORE:
            success = true;
        break;
        default:
            success = false;
    }

    if( success )
    {
        ThisCurrentState = SettingStates::SETTING_USER_STORE;
        buildUpUserStoreSettings();
        emit settingStateChanged();
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

    switch ( ThisCurrentState )
    {
        case SETTING_USER_LOGIN:
        case SETTING_USER_TECHNICIAN:
            success = true;
        break;
        default:
            success = false;
    }

    if( success )
    {
        ThisCurrentState = SettingStates::SETTING_USER_TECHNICIAN;
        buildUpUserTechSettings();
        emit settingStateChanged();
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

    switch ( ThisCurrentState )
    {
        case SETTING_MAIN:
        case SETTING_SYSTEM:
        case SETTING_SYSTEM_WIFI_SETTING:
        case SETTING_SYSTEM_DATE:
        case SETTING_SYSTEM_TIME:
        case SETTING_SYSTEM_GRINDERNAME:
        case SETTING_SYSTEM_FACTORY_RESET:
        case SETTING_SYSTEM_DISPLAY_TEST:
        case SETTING_SYSTEM_TEMP_UNIT:
        case SETTING_SYSTEM_START_OF_WEEK:
            success = true;
        break;
        default:
            success = false;
    }

    if( success )
    {
        ThisCurrentState = SettingStates::SETTING_SYSTEM;
        buildUpSystemSettings();
        emit settingStateChanged();
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

    switch ( ThisCurrentState )
    {
        case SETTING_SYSTEM:
        case SETTING_SYSTEM_DATE:
            success = true;
        break;
        default:
            success = false;
    }

    if( success )
    {
        ThisCurrentState = SettingStates::SETTING_SYSTEM_DATE;
        buildUpSystemDateSettings();
        emit settingStateChanged();
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

    switch ( ThisCurrentState )
    {
        case SETTING_SYSTEM:
        case SETTING_SYSTEM_TIME:
            success = true;
        break;
        default:
            success = false;
    }

    if( success )
    {
        ThisCurrentState = SettingStates::SETTING_SYSTEM_TIME;
        buildUpSystemTimeSettings();
        emit settingStateChanged();
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

    switch ( ThisCurrentState )
    {
        case SETTING_SYSTEM:
        case SETTING_SYSTEM_GRINDERNAME:
            success = true;
        break;
        default:
            success = false;
    }

    if( success )
    {
        ThisCurrentState = SettingStates::SETTING_SYSTEM_GRINDERNAME;
        buildUpSystemGrindernameSettings();
        emit settingStateChanged();
    }

    return success;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SettingsStatemachine::setSystemMachineInfoState()
{
    bool success;

    switch ( ThisCurrentState )
    {
        case SETTING_INFO:
        case SETTING_SYSTEM_MACHINEINFO:
            success = true;
        break;
        default:
            success = false;
    }

    if( success )
    {
        ThisCurrentState = SettingStates::SETTING_SYSTEM_MACHINEINFO;
        buildUpSystemMachineInfoSettings();
        emit settingStateChanged();
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

    switch ( ThisCurrentState )
    {
        case SETTING_SYSTEM:
        case SETTING_SYSTEM_FACTORY_RESET:
            success = true;
        break;
        default:
            success = false;
    }

    if( success )
    {
        ThisCurrentState = SettingStates::SETTING_SYSTEM_FACTORY_RESET;
        buildUpSystemFactoryResetSettings();
        emit settingStateChanged();
    }

    return success;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SettingsStatemachine::setSystemDisplayTestState()
{
    bool success;

    switch ( ThisCurrentState )
    {
        case SETTING_SYSTEM:
        case SETTING_SYSTEM_DISPLAY_TEST:
            success = true;
        break;
        default:
            success = false;
    }

    if( success )
    {
        ThisCurrentState = SettingStates::SETTING_SYSTEM_DISPLAY_TEST;
        buildUpSystemDisplayTestSettings();
        emit settingStateChanged();
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

    switch ( ThisCurrentState )
    {
        case SETTING_SYSTEM:
        case SETTING_SYSTEM_START_OF_WEEK:
            success = true;
        break;
        default:
            success = false;
    }

    if( success )
    {
        ThisCurrentState = SettingStates::SETTING_SYSTEM_START_OF_WEEK;

        buildUpSystemStartOfWeekSettings();

        emit settingStateChanged();
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

    switch ( ThisCurrentState )
    {
        case SETTING_CUSTOMIZE:
        case SETTING_CUSTOMIZE_STDBY_ADJ:
            success = true;
        break;
        default:
            success = false;
    }

    if( success )
    {
        ThisCurrentState = SettingStates::SETTING_CUSTOMIZE_STDBY_ADJ;

        buildUpCustomizeStandbyAdjSettings();

        emit settingStateChanged();
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

    switch ( ThisCurrentState )
    {
        case SETTING_MAIN:
        case SETTING_INFO:
        case SETTING_INFO_NOTIF:
        case SETTING_INFO_STATISTIC:
        case SETTING_SYSTEM_MACHINEINFO:
            success = true;
        break;
        default:
            success = false;
    }

    if( success )
    {
        ThisCurrentState = SettingStates::SETTING_INFO;

        buildUpInfoSettings();

        emit settingStateChanged();
    }

    return success;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SettingsStatemachine::setInfoNotifState()
{
    bool success;

    switch ( ThisCurrentState )
    {
        case SETTING_INFO:
        case SETTING_INFO_NOTIF:
            success = true;
        break;
        default:
            success = false;
    }

    if( success )
    {
        ThisCurrentState = SettingStates::SETTING_INFO_NOTIF;

        buildUpInfoNotifSettings();

        emit settingStateChanged();
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

    switch ( ThisCurrentState )
    {

        case SETTING_INFO:
        case SETTING_INFO_STATISTIC:
            success = true;
        break;
        default:
            success = false;
    }

    if( success )
    {
        ThisCurrentState = SettingStates::SETTING_INFO_STATISTIC;

        buildUpInfoStatisticsSettings();

        emit settingStateChanged();
    }

    return success;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsStatemachine::buildUpMainState()
{
    setMenuTitle("Settings");
    setNavAvailable(false);
    setButtonPreviousVisible(false);
    setButtonOkVisible(false);
    emptyMenuList();

    SettingsMenu* grinderSetting = new SettingsMenu( this );
    grinderSetting->setMenuName("Smiley Settings");
    grinderSetting->setMenuType( SettingsMenu::ENTRYTYPE_MAINENTRY );
    grinderSetting->setStringValue("");
    grinderSetting->setToggleswitchValue(false);
    grinderSetting->setLinkedSettingState(SETTING_DISCS);
    ThisMenuList.append(grinderSetting);

    SettingsMenu* userSetting = new SettingsMenu( this );
    userSetting->setMenuName("User Management");
    userSetting->setMenuType( SettingsMenu::ENTRYTYPE_MAINENTRY );
    userSetting->setStringValue("");
    userSetting->setToggleswitchValue(false);
    userSetting->setLinkedSettingState(SETTING_USER);
    ThisMenuList.append(userSetting);

    SettingsMenu* generalSetting = new SettingsMenu( this );
    generalSetting->setMenuName("General Settings");
    generalSetting->setMenuType( SettingsMenu::ENTRYTYPE_MAINENTRY );
    generalSetting->setStringValue("");
    generalSetting->setToggleswitchValue(false);
    generalSetting->setLinkedSettingState(SETTING_SYSTEM);
    ThisMenuList.append(generalSetting);

    emit qlpMenuListChanged();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsStatemachine::buildUpDiscsSetting()
{
    setMenuTitle("Frogs");
    emptyMenuList();

    SettingsSerializer::userRole userRights = ThisSettingsSerializer->getUserAutoLoginAs();

    SettingsMenu* calibrationMenu = new SettingsMenu( this );
    calibrationMenu->setMenuName("Calibration");
    calibrationMenu->setMenuType( SettingsMenu::ENTRYTYPE_NOVALUE );
    calibrationMenu->setLinkedSettingState(SETTING_DISCS_CALIB);
    calibrationMenu->setUserRightsObtained( (    userRights == SettingsSerializer::LOGGEDIN_STOREOWNER )
                                            || ( userRights == SettingsSerializer::LOGGEDIN_TECHNICIAN ) );
    ThisMenuList.append(calibrationMenu);

    SettingsMenu* usageTimeMenu = new SettingsMenu( this );
    usageTimeMenu->setMenuName("Usage Time");
    usageTimeMenu->setMenuType( SettingsMenu::ENTRYTYPE_NOVALUE );
    usageTimeMenu->setLinkedSettingState(SETTING_DISCS_USAGE_TIME);
    usageTimeMenu->setUserRightsObtained( true );
    ThisMenuList.append(usageTimeMenu);

    emit qlpMenuListChanged();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsStatemachine::buildUpCustomizeSetting()
{
    setButtonPreviousVisible(true);
    setButtonOkVisible(false);
    emptyMenuList();

    SettingsSerializer::userRole userRights = ThisSettingsSerializer->getUserAutoLoginAs();

    SettingsMenu* timeModeMenu = new SettingsMenu( this );
    timeModeMenu->setMenuName("Driver Mode");
    timeModeMenu->setMenuType( SettingsMenu::ENTRYTYPE_TOGGLESWITCH );
    timeModeMenu->setStringValue("");
    timeModeMenu->setToggleswitchValue(ThisSettingsSerializer->getTimeModeIsEnabled());
    timeModeMenu->setLinkedSettingState(SETTING_INVALID);
    timeModeMenu->setToggleswitchType( SettingsMenu::TOGGLESWITCH_FOR_TIMEMODE_ENABLING );

    timeModeMenu->setUserRightsObtained( ( userRights == SettingsSerializer::LOGGEDIN_STOREOWNER) ||
                                         ( userRights == SettingsSerializer::LOGGEDIN_TECHNICIAN) );

    ThisMenuList.append(timeModeMenu);

    SettingsMenu* libraryModeMenu = new SettingsMenu( this );
    libraryModeMenu->setMenuName("Space Mode");
    libraryModeMenu->setMenuType( SettingsMenu::ENTRYTYPE_TOGGLESWITCH );
    libraryModeMenu->setStringValue("");
    libraryModeMenu->setToggleswitchValue(ThisSettingsSerializer->getLibraryModeIsEnabled());
    libraryModeMenu->setLinkedSettingState(SETTING_INVALID);
    libraryModeMenu->setToggleswitchType( SettingsMenu::TOGGLESWITCH_FOR_LIBRARYMODE_ENABLING );

    libraryModeMenu->setUserRightsObtained( ( userRights == SettingsSerializer::LOGGEDIN_STOREOWNER) ||
                                            ( userRights == SettingsSerializer::LOGGEDIN_TECHNICIAN) );

    ThisMenuList.append(libraryModeMenu);


    SettingsMenu* showDdd = new SettingsMenu( this );
    showDdd->setMenuName("Show ISS");
    showDdd->setMenuType( SettingsMenu::ENTRYTYPE_TOGGLESWITCH );
    showDdd->setStringValue("");
    showDdd->setToggleswitchValue(ThisSettingsSerializer->getShowDdd());
    showDdd->setLinkedSettingState(SETTING_INVALID);
    showDdd->setToggleswitchType( SettingsMenu::TOGGLESWITCH_FOR_SHOW_DDD );

    showDdd->setUserRightsObtained( ( userRights == SettingsSerializer::LOGGEDIN_NONE ) ||
                                    ( userRights == SettingsSerializer::LOGGEDIN_STOREOWNER) ||
                                    ( userRights == SettingsSerializer::LOGGEDIN_TECHNICIAN) );

    ThisMenuList.append(showDdd);


    SettingsMenu* showSpot = new SettingsMenu( this );
    showSpot->setMenuName("Show Spotlight");
    showSpot->setMenuType( SettingsMenu::ENTRYTYPE_TOGGLESWITCH );
    showSpot->setStringValue("");
    showSpot->setToggleswitchValue(ThisSettingsSerializer->getShowSpotlight());
    showSpot->setLinkedSettingState(SETTING_INVALID);
    showSpot->setToggleswitchType( SettingsMenu::TOGGLESWITCH_FOR_SHOWSPOT );

    showSpot->setUserRightsObtained( ( userRights == SettingsSerializer::LOGGEDIN_NONE ) ||
                                     ( userRights == SettingsSerializer::LOGGEDIN_STOREOWNER) ||
                                     ( userRights == SettingsSerializer::LOGGEDIN_TECHNICIAN) );


    ThisMenuList.append(showSpot);

    SettingsMenu* standbyTime = new SettingsMenu( this );
    standbyTime->setMenuName("Standby Time");
    standbyTime->setMenuType( SettingsMenu::ENTRYTYPE_STRING );
    standbyTime->setStringValue(ThisSettingsSerializer->stdbyTimeIndexToString(ThisSettingsSerializer->getStdbyTimeIndex()));
    standbyTime->setToggleswitchValue(false);
    standbyTime->setLinkedSettingState(SETTING_CUSTOMIZE_STDBY_ADJ);

    standbyTime->setUserRightsObtained( ( userRights == SettingsSerializer::LOGGEDIN_STOREOWNER) ||
                                        ( userRights == SettingsSerializer::LOGGEDIN_TECHNICIAN) );

    ThisMenuList.append(standbyTime);

    SettingsMenu* brightness = new SettingsMenu( this );
    brightness->setMenuName("Brightness");
    brightness->setMenuType( SettingsMenu::ENTRYTYPE_STRING );
    brightness->setStringValue(QString::number(ThisSettingsSerializer->getDisplayBrightnessPercent())+"%");
    brightness->setToggleswitchValue(false);
    brightness->setLinkedSettingState(SETTING_CUSTOMIZE_BRIGHTNESS);

    brightness->setUserRightsObtained( ( userRights == SettingsSerializer::LOGGEDIN_NONE ) ||
                                       ( userRights == SettingsSerializer::LOGGEDIN_STOREOWNER) ||
                                       ( userRights == SettingsSerializer::LOGGEDIN_TECHNICIAN) );

    ThisMenuList.append(brightness);

    SettingsMenu* temperature = new SettingsMenu( this );
    temperature->setMenuName("Temperature");
    temperature->setMenuType( SettingsMenu::ENTRYTYPE_NOVALUE );
    temperature->setStringValue("");
    temperature->setToggleswitchValue(false);
    temperature->setLinkedSettingState(SETTING_CUSTOMIZE_TEMP);

    temperature->setUserRightsObtained( ( userRights == SettingsSerializer::LOGGEDIN_STOREOWNER) ||
                                        ( userRights == SettingsSerializer::LOGGEDIN_TECHNICIAN) );

    ThisMenuList.append(temperature);


    setMenuTitle("Customize");

    emit qlpMenuListChanged();

}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsStatemachine::buildUpCustomizeTempSetting()
{
    setButtonPreviousVisible(true);
    setButtonOkVisible(false);
    emptyMenuList();

    SettingsSerializer::userRole userRights = ThisSettingsSerializer->getUserAutoLoginAs();

    SettingsMenu* showTemp = new SettingsMenu( this );
    showTemp->setMenuName("Show Temp");
    showTemp->setMenuType( SettingsMenu::ENTRYTYPE_TOGGLESWITCH );
    showTemp->setStringValue("");
    showTemp->setToggleswitchValue(ThisSettingsSerializer->getShowTemp());
    showTemp->setLinkedSettingState(SETTING_INVALID);
    showTemp->setToggleswitchType( SettingsMenu::TOGGLESWITCH_FOR_SHOWTEMP );

    showTemp->setUserRightsObtained( ( userRights == SettingsSerializer::LOGGEDIN_STOREOWNER) ||
                                     ( userRights == SettingsSerializer::LOGGEDIN_TECHNICIAN) );

    ThisMenuList.append(showTemp);


    SettingsMenu* tempWarning = new SettingsMenu( this );
    tempWarning->setMenuName("Temp. Warning");
    tempWarning->setMenuType( SettingsMenu::ENTRYTYPE_TOGGLESWITCH );
    tempWarning->setStringValue("");
    tempWarning->setToggleswitchValue(ThisSettingsSerializer->getShowTempWarning());
    tempWarning->setLinkedSettingState(SETTING_INVALID);
    tempWarning->setToggleswitchType( SettingsMenu::TOGGLESWITCH_FOR_WARNINGTEMP );

    tempWarning->setUserRightsObtained( ( userRights == SettingsSerializer::LOGGEDIN_STOREOWNER) ||
                                        ( userRights == SettingsSerializer::LOGGEDIN_TECHNICIAN) );

    ThisMenuList.append(tempWarning);


    SettingsMenu* warningtThresh = new SettingsMenu( this );
    warningtThresh->setMenuName("Warning Thold.");
    warningtThresh->setMenuType( SettingsMenu::ENTRYTYPE_STRING );
    warningtThresh->setStringValue( ThisSettingsSerializer->tempToString(ThisSettingsSerializer->getTempThresValueX100()) );
    warningtThresh->setToggleswitchValue(false);
    warningtThresh->setLinkedSettingState(SETTING_CUSTOMIZE_TEMP_THRESH);
    warningtThresh->setMenuChoosable(ThisSettingsSerializer->getShowTempWarning());

    warningtThresh->setUserRightsObtained( ( userRights == SettingsSerializer::LOGGEDIN_STOREOWNER) ||
                                           ( userRights == SettingsSerializer::LOGGEDIN_TECHNICIAN) );

    setMenuTitle("Temperature");


    ThisMenuList.append(warningtThresh);

    emit qlpMenuListChanged();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsStatemachine::buildUpSystemTempUnitSetting()
{
    setMenuTitle("Temperature Unit");
    setButtonPreviousVisible(true);
    setButtonOkVisible(true);

    emptyMenuList();
    SettingsMenu* tempUnit = new SettingsMenu( this );
    tempUnit->setMenuName("");
    tempUnit->setMenuType( SettingsMenu::ENTRYTYPE_ACTION );
    tempUnit->setStringValue("");
    tempUnit->setToggleswitchValue(false);
    tempUnit->setLinkedSettingState(SETTING_INVALID);
    tempUnit->setActionType(SettingsMenu::ACTIONTYPE_RADIOBUTTON);
    tempUnit->setActionValue( QString::number( ThisSettingsSerializer->getTempUnit() ));
    ThisMenuList.append(tempUnit);

    emit qlpMenuListChanged();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsStatemachine::buildUpCustomizeTempThreshSetting()
{
    setMenuTitle("Warning Threshold");
    setButtonPreviousVisible(true);
    setButtonOkVisible(true);

    emptyMenuList();
    SettingsMenu* threshold = new SettingsMenu( this );
    threshold->setMenuName("");
    threshold->setMenuType( SettingsMenu::ENTRYTYPE_ACTION );
    threshold->setStringValue("");
    threshold->setToggleswitchValue(false);
    threshold->setLinkedSettingState(SETTING_INVALID);
    threshold->setActionType(SettingsMenu::ACTIONTYPE_PROGRESS);
    threshold->setActionValue( QString::number( ThisSettingsSerializer->getTempThresValueX100() ));
    ThisMenuList.append(threshold);

    emit qlpMenuListChanged();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsStatemachine::buildUpCustomizeBrigSetting()
{
    setMenuTitle("Brightness");
    setButtonPreviousVisible(true);
    setButtonOkVisible(true);

    emptyMenuList();
    SettingsMenu* brightness = new SettingsMenu( this );
    brightness->setMenuName("");
    brightness->setMenuType( SettingsMenu::ENTRYTYPE_ACTION );
    brightness->setStringValue("");
    brightness->setToggleswitchValue(false);
    brightness->setLinkedSettingState(SETTING_INVALID);
    brightness->setActionType(SettingsMenu::ACTIONTYPE_PROGRESS);
    brightness->setActionValue(QString::number(ThisSettingsSerializer->getDisplayBrightnessPercent()));
    ThisMenuList.append(brightness);

    emit qlpMenuListChanged();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsStatemachine::buildUpDiscsCalibrationSetting()
{
    setMenuTitle("Calibration");
    emptyMenuList();

    SettingsMenu* calibration = new SettingsMenu( this );
    calibration->setMenuName("Calibration");
    calibration->setMenuType( SettingsMenu::ENTRYTYPE_ACTION );
    calibration->setActionType(SettingsMenu::ACTIONTYPE_CALIB);
    calibration->setLinkedSettingState(SETTING_INVALID);
    ThisMenuList.append(calibration);

    emit qlpMenuListChanged();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsStatemachine::buildUpDiscsUsageTimeSetting()
{
    setMenuTitle("Frog Usage Time");
    emptyMenuList();

    SettingsMenu* usageTimePage = new SettingsMenu( this );
    usageTimePage->setMenuName("Frog Usage Time");
    usageTimePage->setMenuType( SettingsMenu::ENTRYTYPE_ACTION );
    usageTimePage->setActionType( SettingsMenu::ACTIONTYPE_DISK_USAGE_TIME );
    usageTimePage->setLinkedSettingState(SETTING_INVALID);
    ThisMenuList.append(usageTimePage);

    emit qlpMenuListChanged();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsStatemachine::buildUpUserCpcSetting()
{
    emptyMenuList();

    setMenuTitle("Change Pin of");

    SettingsMenu* storeOwner = new SettingsMenu( this );
    storeOwner->setMenuName("Fast Driver");
    storeOwner->setMenuType( SettingsMenu::ENTRYTYPE_NOVALUE );
    storeOwner->setStringValue("");
    storeOwner->setToggleswitchValue(false);
    storeOwner->setLinkedSettingState(SETTING_USER_CPC_STORE);

    ThisMenuList.append(storeOwner);

    SettingsMenu* technician = new SettingsMenu( this );
    technician->setMenuName("Expert Driver");
    technician->setMenuType( SettingsMenu::ENTRYTYPE_NOVALUE );
    technician->setStringValue("");
    technician->setToggleswitchValue(false);
    technician->setLinkedSettingState(SETTING_USER_CPC_TECH);

    ThisMenuList.append(technician);

    emit qlpMenuListChanged();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsStatemachine::buildUpUserCpcStoreSetting()
{
    emptyMenuList();

    SettingsMenu* store = new SettingsMenu( this );
    store->setMenuName("CPC");
    store->setMenuType( SettingsMenu::ENTRYTYPE_ACTION );
    store->setStringValue("");
    store->setToggleswitchValue(false);
    store->setActionType(SettingsMenu::ACTIONTYPE_CPC);
    store->setLinkedSettingState(SETTING_INVALID);

    ThisMenuList.append(store);

    setMenuTitle("");

    emit qlpMenuListChanged();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsStatemachine::buildUpUserCpcTechSetting()
{
    emptyMenuList();

    SettingsMenu* technician = new SettingsMenu( this );
    technician->setMenuName("CPC");
    technician->setMenuType( SettingsMenu::ENTRYTYPE_ACTION );
    technician->setStringValue("");
    technician->setToggleswitchValue(false);
    technician->setActionType(SettingsMenu::ACTIONTYPE_CPC);
    technician->setLinkedSettingState(SETTING_INVALID);

    ThisMenuList.append(technician);

    setMenuTitle("");

    emit qlpMenuListChanged();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsStatemachine::buildUpSystemWifiSettings()
{
    emptyMenuList();

    SettingsSerializer::userRole userRights = ThisSettingsSerializer->getUserAutoLoginAs();

    SettingsMenu* wifi = new SettingsMenu( this );
    wifi->setMenuName("WiFi");
    wifi->setMenuType( SettingsMenu::ENTRYTYPE_TOGGLESWITCH );
    wifi->setStringValue("");
    wifi->setToggleswitchValue( m_pMainStatemachine->espDriver()->wifiDriver()->isActive() );
    wifi->setLinkedSettingState(SETTING_INVALID);
    wifi->setToggleswitchType(SettingsMenu::TOGGLESWITCH_FOR_WIFI);
    wifi->setUserRightsObtained( ( userRights == SettingsSerializer::LOGGEDIN_STOREOWNER) ||
                                 ( userRights == SettingsSerializer::LOGGEDIN_TECHNICIAN) );
    ThisMenuList.append(wifi);

    SettingsMenu* ssid = new SettingsMenu( this );
    ssid->setMenuName("SSID");
    ssid->setMenuType( SettingsMenu::ENTRYTYPE_NOVALUE );
    ssid->setStringValue("");
    ssid->setToggleswitchValue(false);
    ssid->setLinkedSettingState(SETTING_SYSTEM_WIFI_SETTING_SSID);
    ssid->setUserRightsObtained( ( userRights == SettingsSerializer::LOGGEDIN_STOREOWNER) ||
                                 ( userRights == SettingsSerializer::LOGGEDIN_TECHNICIAN) );
    ThisMenuList.append(ssid);

    SettingsMenu* password = new SettingsMenu( this );
    password->setMenuName("Password");
    password->setMenuType( SettingsMenu::ENTRYTYPE_NOVALUE );
    password->setStringValue("");
    password->setToggleswitchValue(false);
    password->setLinkedSettingState(SETTING_SYSTEM_WIFI_SETTING_PW);
    password->setUserRightsObtained( ( userRights == SettingsSerializer::LOGGEDIN_STOREOWNER) ||
                                     ( userRights == SettingsSerializer::LOGGEDIN_TECHNICIAN) );
    ThisMenuList.append(password);

    SettingsMenu* ip = new SettingsMenu( this );
    ip->setMenuName("IP");
    ip->setMenuType( SettingsMenu::ENTRYTYPE_NOVALUE );
    ip->setStringValue("");
    ip->setToggleswitchValue(false);
    ip->setLinkedSettingState(SETTING_SYSTEM_WIFI_SETTING_IP);
    ip->setMenuChoosable( m_pMainStatemachine->espDriver()->wifiDriver()->isActive() );
    ip->setUserRightsObtained( ( userRights == SettingsSerializer::LOGGEDIN_STOREOWNER) ||
                               ( userRights == SettingsSerializer::LOGGEDIN_TECHNICIAN) );
    ThisMenuList.append(ip);

    setMenuTitle("WIFI Settings");

    emit qlpMenuListChanged();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsStatemachine::buildUpSystemWifiIPSettings()
{
    QString eth0IpStr = "UNDEF";
    QString wifiIpStr = "UNDEF";

    // eth0:
    QNetworkInterface eth0Ip = QNetworkInterface::interfaceFromName("eth0");
    QList<QNetworkAddressEntry> entries = eth0Ip.addressEntries();
    if (!entries.isEmpty())
    {
        QNetworkAddressEntry entry = entries.first();
        eth0IpStr = "LAN: " + entry.ip().toString();
    }
    else
    {
        eth0IpStr = "";
    }

    // wifi:
    if( ThisSettingsSerializer->getWifiDriver() != nullptr )
    {
        //if(ThisSettingsSerializer->getWifiDriver()->getIsApConnection())
        //{
        //}
        wifiIpStr = "WiFi AP: 192.168.4.1";
    }
    else
    {
        wifiIpStr = "WiFi AP: 192.168.4.1";
    }

    // menu:
    setMenuTitle("IP");

    emptyMenuList();
    SettingsMenu* entryIpWifi = new SettingsMenu( this );
    entryIpWifi->setMenuName("");
    entryIpWifi->setMenuType( SettingsMenu::ENTRYTYPE_ACTION );
    entryIpWifi->setActionType( SettingsMenu::ACTIONTYPE_TEXTBOX );
    entryIpWifi->setActionValue((!eth0IpStr.isEmpty())? wifiIpStr + "\n" + eth0IpStr : wifiIpStr);
    entryIpWifi->setStringValue("");
    entryIpWifi->setToggleswitchType( SettingsMenu::TOGGLESWITCH_FOR_NONE );
    entryIpWifi->setToggleswitchValue(false);
    entryIpWifi->setLinkedSettingState(SETTING_INVALID);
    ThisMenuList.append(entryIpWifi);

    emit qlpMenuListChanged();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsStatemachine::buildUpSystemWifiPWSettings()
{
    emptyMenuList();

    setMenuTitle("WIFI Password");

    SettingsMenu* password = new SettingsMenu( this );
    password->setMenuName("");
    password->setMenuType( SettingsMenu::ENTRYTYPE_ACTION );
    password->setStringValue("");
    password->setToggleswitchValue(false);
    password->setLinkedSettingState(SETTING_INVALID);
    password->setActionType(SettingsMenu::ACTIONTYPE_PW);
    password->setActionValue(ThisSettingsSerializer->getWifiPw());

    ThisMenuList.append(password);

    emit qlpMenuListChanged();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsStatemachine::buildUpSystemWifiSSIDSettings()
{
    emptyMenuList();

    setMenuTitle("SSID");

    SettingsMenu* ssidEntry = new SettingsMenu(this);
    ssidEntry->setMenuType(SettingsMenu::ENTRYTYPE_ACTION);
    ssidEntry->setActionType(SettingsMenu::ACTIONTYPE_TEXTBOX);
    ssidEntry->setActionValue(ThisSettingsSerializer->getWifiApSsid());

    ThisMenuList.append(ssidEntry);

    emit qlpMenuListChanged();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsStatemachine::buildUpUserSettings()
{
    emptyMenuList();

    SettingsSerializer::userRole userRights = ThisSettingsSerializer->getUserAutoLoginAs();

    SettingsMenu* login = new SettingsMenu( this );
    login->setMenuName("Login");
    login->setMenuType( SettingsMenu::ENTRYTYPE_NOVALUE );
    login->setStringValue("");
    login->setToggleswitchValue(false);
    login->setLinkedSettingState(SETTING_USER_LOGIN);

    login->setUserRightsObtained( ( userRights == SettingsSerializer::LOGGEDIN_NONE ) ||
                                  ( userRights == SettingsSerializer::LOGGEDIN_STOREOWNER) ||
                                  ( userRights == SettingsSerializer::LOGGEDIN_TECHNICIAN) );
    ThisMenuList.append(login);

    SettingsMenu* logout = new SettingsMenu( this );
    logout->setMenuName("Logout");
    logout->setMenuType( SettingsMenu::ENTRYTYPE_NOVALUE );
    logout->setStringValue("");
    logout->setToggleswitchValue(false);
    logout->setMenuChoosable( ThisSettingsSerializer->getUserAutoLoginAs() != SettingsSerializer::LOGGEDIN_NONE );
    logout->setLinkedSettingState(SETTING_USER_LOGOUT);

    logout->setUserRightsObtained( ( userRights == SettingsSerializer::LOGGEDIN_NONE ) ||
                                   ( userRights == SettingsSerializer::LOGGEDIN_STOREOWNER) ||
                                   ( userRights == SettingsSerializer::LOGGEDIN_TECHNICIAN) );

    ThisMenuList.append(logout);

    SettingsMenu* cpc = new SettingsMenu( this );
    cpc->setMenuName("Change Pin Code");
    cpc->setMenuType( SettingsMenu::ENTRYTYPE_NOVALUE );
    cpc->setStringValue("");
    cpc->setToggleswitchValue(false);
    cpc->setLinkedSettingState(SETTING_USER_CPC);

    cpc->setUserRightsObtained( ( userRights == SettingsSerializer::LOGGEDIN_STOREOWNER) ||
                                ( userRights == SettingsSerializer::LOGGEDIN_TECHNICIAN) );

    ThisMenuList.append(cpc);

    SettingsMenu* autoLogout = new SettingsMenu( this );
    autoLogout->setMenuName("AutoLogout");
    autoLogout->setMenuType( SettingsMenu::ENTRYTYPE_TOGGLESWITCH );
    autoLogout->setStringValue("");
    autoLogout->setToggleswitchValue( ThisSettingsSerializer->getUserAutoLogoutEnable() );
    autoLogout->setLinkedSettingState(SETTING_INVALID);
    autoLogout->setToggleswitchType( SettingsMenu::TOGGLESWITCH_FOR_AUTOLOG );

    autoLogout->setUserRightsObtained( ( userRights == SettingsSerializer::LOGGEDIN_STOREOWNER) );

    ThisMenuList.append(autoLogout);

    setMenuTitle("User Management");

    emit qlpMenuListChanged();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsStatemachine::buildUpUserLoginSettings()
{
    emptyMenuList();

    SettingsMenu* storeOwner = new SettingsMenu( this );
    storeOwner->setMenuName("Fast Driver");
    storeOwner->setMenuType( SettingsMenu::ENTRYTYPE_RADIOBUTTON );
    storeOwner->setStringValue("");
    storeOwner->setToggleswitchValue( ThisSettingsSerializer->getUserAutoLoginAs() == SettingsSerializer::LOGGEDIN_STOREOWNER );
    storeOwner->setLinkedSettingState(SETTING_USER_STORE);

    ThisMenuList.append(storeOwner);

    SettingsMenu* technician = new SettingsMenu( this );
    technician->setMenuName("Expert Driver");
    technician->setMenuType( SettingsMenu::ENTRYTYPE_RADIOBUTTON );
    technician->setStringValue("");
    technician->setToggleswitchValue(ThisSettingsSerializer->getUserAutoLoginAs() == SettingsSerializer::LOGGEDIN_TECHNICIAN);
    technician->setLinkedSettingState(SETTING_USER_TECHNICIAN);

    ThisMenuList.append(technician);

    setMenuTitle("Login");

    emit qlpMenuListChanged();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsStatemachine::buildUpUserLogoutSettings()
{
    emptyMenuList();

    SettingsMenu* logout = new SettingsMenu( this );
    logout->setMenuName("");
    logout->setMenuType( SettingsMenu::ENTRYTYPE_ACTION );
    logout->setStringValue("");
    logout->setToggleswitchValue(false);
    logout->setLinkedSettingState(SETTING_INVALID);
    logout->setActionType(SettingsMenu::ACTIONTYPE_CONFIRM);
    logout->setActionValue("You have logged out \n as " + ThisSettingsSerializer->userRoleToString( ThisSettingsSerializer->getUserAutoLoginAs() ));

    ThisMenuList.append(logout);

    setMenuTitle("Logout");

    emit qlpMenuListChanged();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsStatemachine::buildUpUserStoreSettings()
{
    emptyMenuList();

    setMenuTitle("Enter Pin");

    SettingsMenu* store = new SettingsMenu( this );
    store->setMenuName("");
    store->setMenuType( SettingsMenu::ENTRYTYPE_ACTION );
    store->setStringValue("");
    store->setToggleswitchValue(false);
    store->setActionType(SettingsMenu::ACTIONTYPE_LOGIN);
    store->setLinkedSettingState(SETTING_INVALID);

    ThisMenuList.append(store);

    emit qlpMenuListChanged();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsStatemachine::buildUpUserTechSettings()
{
    emptyMenuList();

    setMenuTitle("Enter Pin");

    SettingsMenu* technician = new SettingsMenu( this );
    technician->setMenuName("");
    technician->setMenuType( SettingsMenu::ENTRYTYPE_ACTION );
    technician->setStringValue("");
    technician->setToggleswitchValue(false);
    technician->setActionType(SettingsMenu::ACTIONTYPE_LOGIN);
    technician->setLinkedSettingState(SETTING_INVALID);

    ThisMenuList.append(technician);

    emit qlpMenuListChanged();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsStatemachine::buildUpSystemSettings()
{
    setMenuTitle("System");
    setButtonPreviousVisible(true);
    setButtonOkVisible(false);

    emptyMenuList();

    SettingsSerializer::userRole userRights = ThisSettingsSerializer->getUserAutoLoginAs();

    SettingsMenu* wifiSetting = new SettingsMenu( this );
    wifiSetting->setMenuName("Wifi Settings");
    wifiSetting->setMenuType( SettingsMenu::ENTRYTYPE_NOVALUE );
    wifiSetting->setStringValue("");
    wifiSetting->setToggleswitchValue(false);
    wifiSetting->setLinkedSettingState(SETTING_SYSTEM_WIFI_SETTING);

    wifiSetting->setUserRightsObtained( ( userRights == SettingsSerializer::LOGGEDIN_STOREOWNER) ||
                                        ( userRights == SettingsSerializer::LOGGEDIN_TECHNICIAN) );

    ThisMenuList.append(wifiSetting);

    SettingsMenu* date = new SettingsMenu( this );
    date->setMenuName("Date");
    date->setMenuType( SettingsMenu::ENTRYTYPE_NOVALUE );
    date->setStringValue("");
    date->setToggleswitchValue(false);
    date->setLinkedSettingState(SETTING_SYSTEM_DATE);

    date->setUserRightsObtained( ( userRights == SettingsSerializer::LOGGEDIN_STOREOWNER) ||
                                 ( userRights == SettingsSerializer::LOGGEDIN_TECHNICIAN) );

    ThisMenuList.append(date);

    SettingsMenu* time = new SettingsMenu( this );
    time->setMenuName("Time");
    time->setMenuType( SettingsMenu::ENTRYTYPE_STRING );
    time->setStringValue("");
    time->setToggleswitchValue(false);
    time->setLinkedSettingState(SETTING_SYSTEM_TIME);

    time->setUserRightsObtained( ( userRights == SettingsSerializer::LOGGEDIN_STOREOWNER) ||
                                 ( userRights == SettingsSerializer::LOGGEDIN_TECHNICIAN) );

    ThisMenuList.append(time);

    SettingsMenu* startOfWeek = new SettingsMenu( this );
    startOfWeek->setMenuName("Start of Week");
    startOfWeek->setMenuType( SettingsMenu::ENTRYTYPE_STRING );
    startOfWeek->setStringValue(ThisSettingsSerializer->startOfWeekisSunNotMon()?"SUN":"MON");
    startOfWeek->setToggleswitchValue(false);
    startOfWeek->setLinkedSettingState(SETTING_SYSTEM_START_OF_WEEK);

    startOfWeek->setUserRightsObtained( ( userRights == SettingsSerializer::LOGGEDIN_STOREOWNER) ||
                                        ( userRights == SettingsSerializer::LOGGEDIN_TECHNICIAN) );

    ThisMenuList.append(startOfWeek);

    SettingsMenu* temperatureUnit = new SettingsMenu( this );
    temperatureUnit->setMenuName("Temp. Unit");
    temperatureUnit->setMenuType( SettingsMenu::ENTRYTYPE_STRING );
    temperatureUnit->setStringValue(ThisSettingsSerializer->tempUnitToString(ThisSettingsSerializer->getTempUnit()));
    temperatureUnit->setToggleswitchValue(false);
    temperatureUnit->setLinkedSettingState(SETTING_SYSTEM_TEMP_UNIT);

    temperatureUnit->setUserRightsObtained( ( userRights == SettingsSerializer::LOGGEDIN_STOREOWNER) ||
                                            ( userRights == SettingsSerializer::LOGGEDIN_TECHNICIAN) );

    ThisMenuList.append(temperatureUnit);

    SettingsMenu* grinderName = new SettingsMenu( this );
    grinderName->setMenuName("Smileyname");
    grinderName->setMenuType( SettingsMenu::ENTRYTYPE_NOVALUE );
    grinderName->setStringValue("");
    grinderName->setToggleswitchValue(false);
    grinderName->setLinkedSettingState(SETTING_SYSTEM_GRINDERNAME);

    grinderName->setUserRightsObtained( ( userRights == SettingsSerializer::LOGGEDIN_STOREOWNER) ||
                                        ( userRights == SettingsSerializer::LOGGEDIN_TECHNICIAN) );

    ThisMenuList.append(grinderName);

    SettingsMenu* factoryReset = new SettingsMenu( this );
    factoryReset->setMenuName("Factory Reset");
    factoryReset->setMenuType( SettingsMenu::ENTRYTYPE_NOVALUE );
    factoryReset->setStringValue("");
    factoryReset->setToggleswitchValue(false);
    factoryReset->setLinkedSettingState(SETTING_SYSTEM_FACTORY_RESET);

    factoryReset->setUserRightsObtained( ( userRights == SettingsSerializer::LOGGEDIN_STOREOWNER) ||
                                         ( userRights == SettingsSerializer::LOGGEDIN_TECHNICIAN) );

    ThisMenuList.append(factoryReset);

    SettingsMenu* colorTest = new SettingsMenu( this );
    colorTest->setMenuName("Display Test");
    colorTest->setMenuType( SettingsMenu::ENTRYTYPE_NOVALUE );
    colorTest->setStringValue("");
    colorTest->setToggleswitchValue(false);
    colorTest->setLinkedSettingState( SETTING_SYSTEM_DISPLAY_TEST );

    colorTest->setUserRightsObtained( ( userRights == SettingsSerializer::LOGGEDIN_STOREOWNER) ||
                                      ( userRights == SettingsSerializer::LOGGEDIN_TECHNICIAN) );

    ThisMenuList.append(colorTest);

    emit qlpMenuListChanged();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsStatemachine::buildUpSystemDateSettings()
{
    setMenuTitle("Date");
    setButtonPreviousVisible(true);
    setButtonOkVisible(true);
    emptyMenuList();

    QDate currentDate = QDate::currentDate();

    SettingsMenu* date = new SettingsMenu( this );
    date->setMenuName("");
    date->setMenuType( SettingsMenu::ENTRYTYPE_ACTION );
    date->setStringValue("");
    date->setToggleswitchValue(false);
    date->setLinkedSettingState(SETTING_INVALID);
    date->setActionType(SettingsMenu::ACTIONTYPE_DATE);
    date->setActionValue(currentDate.toString("d.M.yyyy"));
    ThisMenuList.append(date);

    emit qlpMenuListChanged();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsStatemachine::buildUpSystemTimeSettings()
{
    emptyMenuList();
    QTime currentTime = QTime::currentTime();

    SettingsMenu* time = new SettingsMenu( this );
    time->setMenuName("");
    time->setMenuType( SettingsMenu::ENTRYTYPE_ACTION );
    time->setStringValue("");
    time->setToggleswitchValue(false);
    time->setLinkedSettingState(SETTING_INVALID);
    time->setActionType(SettingsMenu::ACTIONTYPE_TIME);
    time->setActionValue(currentTime.toString("h.m"));
    ThisMenuList.append(time);

    setMenuTitle("Time");
    emit qlpMenuListChanged();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsStatemachine::buildUpSystemGrindernameSettings()
{
    emptyMenuList();

    SettingsMenu* grinderName = new SettingsMenu( this );
    grinderName->setMenuName("Ultra Smiley");
    grinderName->setMenuType( SettingsMenu::ENTRYTYPE_ACTION );
    grinderName->setStringValue("");
    grinderName->setToggleswitchValue(false);
    grinderName->setLinkedSettingState(SETTING_INVALID);
    grinderName->setActionType(SettingsMenu::ACTIONTYPE_PW);
    grinderName->setActionValue(ThisSettingsSerializer->getGrinderName());

    ThisMenuList.append(grinderName);

    setMenuTitle("Smiley Name");

    emit qlpMenuListChanged();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsStatemachine::buildUpSystemMachineInfoSettings()
{
    emptyMenuList();

    SettingsMenu* machineInfo = new SettingsMenu( this );
    machineInfo->setMenuName("");
    machineInfo->setMenuType( SettingsMenu::ENTRYTYPE_ACTION );
    machineInfo->setStringValue("");
    machineInfo->setToggleswitchValue(false);
    machineInfo->setActionType(SettingsMenu::ACTIONTYPE_MACHINFO);
    machineInfo->setLinkedSettingState(SETTING_INVALID);

    ThisMenuList.append(machineInfo);

    setMenuTitle("Machine Info");

    emit qlpMenuListChanged();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsStatemachine::buildUpSystemDisplayTestSettings()
{
    emptyMenuList();
    setMenuTitle( "Display Test" );

    SettingsMenu* displayTest = new SettingsMenu( this );
    displayTest->setMenuName("");
    displayTest->setMenuType( SettingsMenu::ENTRYTYPE_ACTION );
    displayTest->setStringValue("");
    displayTest->setToggleswitchValue(false);
    displayTest->setActionType(SettingsMenu::ACTIONTYPE_DISPLAY_TEST);
    displayTest->setLinkedSettingState(SETTING_INVALID);

    ThisMenuList.append(displayTest);

    emit qlpMenuListChanged();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsStatemachine::buildUpSystemFactoryResetSettings()
{
    setMenuTitle("Factory Reset");
    setButtonOkVisible(false);
    setButtonPreviousVisible(false);
    emptyMenuList();

    SettingsMenu* factoryResetMenu = new SettingsMenu( this );
    factoryResetMenu->setMenuName("Factory Reset");
    factoryResetMenu->setMenuType( SettingsMenu::ENTRYTYPE_ACTION );
    factoryResetMenu->setActionType(SettingsMenu::ACTIONTYPE_FACTORY_RESET);
    factoryResetMenu->setLinkedSettingState(SETTING_INVALID);
    ThisMenuList.append(factoryResetMenu);

    emit qlpMenuListChanged();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsStatemachine::buildUpSystemStartOfWeekSettings()
{
    setMenuTitle("Start of Week");
    setButtonPreviousVisible(true);
    setButtonOkVisible(true);

    emptyMenuList();
    SettingsMenu* startOfWeek = new SettingsMenu( this );
    startOfWeek->setMenuName("");
    startOfWeek->setMenuType( SettingsMenu::ENTRYTYPE_ACTION );
    startOfWeek->setStringValue("");
    startOfWeek->setToggleswitchValue(false);
    startOfWeek->setLinkedSettingState(SETTING_INVALID);
    startOfWeek->setActionType(SettingsMenu::ACTIONTYPE_RADIOBUTTON);
    startOfWeek->setActionValue( QString::number( ThisSettingsSerializer->startOfWeekisSunNotMon()?1:0 ) );
    ThisMenuList.append(startOfWeek);

    emit qlpMenuListChanged();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsStatemachine::buildUpCustomizeStandbyAdjSettings()
{
    emptyMenuList();

    SettingsMenu* stdbyAdj = new SettingsMenu( this );
    stdbyAdj->setMenuName("");
    stdbyAdj->setMenuType( SettingsMenu::ENTRYTYPE_ACTION );
    stdbyAdj->setStringValue("");
    stdbyAdj->setToggleswitchValue(false);
    stdbyAdj->setLinkedSettingState(SETTING_INVALID);
    stdbyAdj->setActionType(SettingsMenu::ACTIONTYPE_STDBY);
    stdbyAdj->setActionValue(QString::number(ThisSettingsSerializer->getStdbyTimeIndex()));

    ThisMenuList.append(stdbyAdj);

    setMenuTitle("Standby Time");

    emit qlpMenuListChanged();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsStatemachine::buildUpInfoSettings()
{

    emptyMenuList();

    SettingsSerializer::userRole userRights = ThisSettingsSerializer->getUserAutoLoginAs();

    SettingsMenu* statistics = new SettingsMenu( this );
    statistics->setMenuName("Statistics");
    statistics->setMenuType( SettingsMenu::ENTRYTYPE_NOVALUE );
    statistics->setStringValue("");
    statistics->setToggleswitchValue(false);
    statistics->setLinkedSettingState(SETTING_INFO_STATISTIC);

    statistics->setUserRightsObtained( ( userRights == SettingsSerializer::LOGGEDIN_STOREOWNER) ||
                                       ( userRights == SettingsSerializer::LOGGEDIN_TECHNICIAN) );

    ThisMenuList.append(statistics);

    SettingsMenu* notif = new SettingsMenu( this );
    notif->setMenuName("Notifications");
    notif->setMenuType( SettingsMenu::ENTRYTYPE_NOVALUE );
    notif->setStringValue("");
    notif->setToggleswitchValue(false);
    notif->setLinkedSettingState(SETTING_INFO_NOTIF);

    notif->setUserRightsObtained( ( userRights == SettingsSerializer::LOGGEDIN_STOREOWNER) ||
                                  ( userRights == SettingsSerializer::LOGGEDIN_TECHNICIAN) );

    ThisMenuList.append(notif);

    SettingsMenu* machineInfo = new SettingsMenu( this );
    machineInfo->setMenuName("Machine Info");
    machineInfo->setMenuType( SettingsMenu::ENTRYTYPE_NOVALUE );
    machineInfo->setStringValue("");
    machineInfo->setToggleswitchValue(false);
    machineInfo->setLinkedSettingState(SETTING_SYSTEM_MACHINEINFO);

    machineInfo->setUserRightsObtained( ( userRights == SettingsSerializer::LOGGEDIN_NONE )      ||
                                        ( userRights == SettingsSerializer::LOGGEDIN_STOREOWNER) ||
                                        ( userRights == SettingsSerializer::LOGGEDIN_TECHNICIAN) );

    ThisMenuList.append(machineInfo);

    setMenuTitle("Info");


    emit qlpMenuListChanged();

}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsStatemachine::buildUpInfoNotifSettings()
{
    emptyMenuList();

    SettingsMenu* notif = new SettingsMenu( this );
    notif->setMenuName("TODO");
    notif->setMenuType( SettingsMenu::ENTRYTYPE_NOVALUE );
    notif->setStringValue("");
    notif->setToggleswitchValue(false);
    notif->setLinkedSettingState(SETTING_INVALID);

    ThisMenuList.append(notif);

    setMenuTitle("Notifications");

    emit qlpMenuListChanged();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsStatemachine::buildUpInfoStatisticsSettings()
{
    emptyMenuList();
    setButtonPreviousVisible(true);

    SettingsMenu* statistics = new SettingsMenu( this );
    statistics->setMenuType( SettingsMenu::ENTRYTYPE_ACTION );
    statistics->setActionType( SettingsMenu::ACTIONTYPE_STATISTICS_SCREEN );
    statistics->setLinkedSettingState(SETTING_INVALID);
    statistics->setMenuName("ABC-DUMMY");
    ThisMenuList.append(statistics);

    setMenuTitle("Statistics");

    emit qlpMenuListChanged();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void  SettingsStatemachine::setWifiDriverInSettingsSerializer( WifiDriver* wd )
{
    if( ThisSettingsSerializer )
    {
        ThisSettingsSerializer->setWifiDriver( wd );
        ThisSettingsSerializer->applyWifiSettingsToDriver();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void  SettingsStatemachine::setDeviceInfoDriverInSettingsSerializer( DeviceInfoCollector* dic )
{
    if( ThisSettingsSerializer )
    {
        ThisSettingsSerializer->setDeviceInfoDriver(dic);
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

SettingsSerializer *SettingsStatemachine::settingsSerializer()
{
    return ThisSettingsSerializer;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsStatemachine::emptyMenuList()
{
    for ( int i = 0; i < ThisMenuList.count(); i++ )
    {
        delete ThisMenuList[ i ];
    }

    ThisMenuList.clear();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

QQmlListProperty<SettingsMenu> SettingsStatemachine::getQlpMenuList() const
{
    return ThisQlpMenuList;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

