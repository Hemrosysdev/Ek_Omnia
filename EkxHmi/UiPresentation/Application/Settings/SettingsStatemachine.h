///////////////////////////////////////////////////////////////////////////////
///
/// @file SettingsStatemachine.h
///
/// @brief Header file of class SettingsStatemachine.
///
/// @author Ultratronik GmbH
///         Dornierstr. 9
///         D-82205 Gilching
///         Germany
///         http://www.ultratronik.de
///
/// @author written by Gerd Esser, Forschung & Entwicklung, gesser@ultratronik.de
///
/// @date 21.03.2021
///
/// @copyright Copyright 2021 by Hemro International AG
///            Hemro International AG
///            Länggenstrasse 34
///            CH 8184 Bachenbülach
///            Switzerland
///            Homepage: www.hemrogroup.com
///
///////////////////////////////////////////////////////////////////////////////

#ifndef SETTINGS_STATEMACHINE_H
#define SETTINGS_STATEMACHINE_H

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#include <QObject>
#include <QQmlEngine>
#include <QVector>
#include "SettingsMenu.h"
#include "SettingsSerializer.h"
#include "WifiDriver.h"
#include "DeviceInfoCollector.h"

class MainStatemachine;
class StatisticsBoard;
class SqliteInterface;
class DiskUsageTime;

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

class SettingsStatemachine: public QObject
{
    Q_OBJECT

    Q_ENUMS( SettingStates )

    Q_PROPERTY( QQmlListProperty<SettingsMenu>      qlpMenuList READ getQlpMenuList                       NOTIFY qlpMenuListChanged )
    Q_PROPERTY( SettingsStatemachine::SettingStates settingState READ getSettingState                       NOTIFY settingStateChanged )
    Q_PROPERTY( QString                             menuTitle   READ getMenuTitle     WRITE setMenuTitle     NOTIFY menuTitleChanged )
    Q_PROPERTY( bool                                navAvailable READ getNavAvailable WRITE setNavAvailable NOTIFY navAvailableChanged )
    Q_PROPERTY( bool buttonPreviousVisible READ getButtonPreviousVisible WRITE setButtonPreviousVisible NOTIFY buttonPreviousVisibleChanged)
    Q_PROPERTY( bool buttonOkVisible       READ getButtonOkVisible       WRITE setButtonOkVisible       NOTIFY buttonOkVisibleChanged)

public:

    enum SettingStates
    {
        SETTING_MAIN,
        SETTING_DISCS,
        SETTING_DISCS_CALIB,
        SETTING_DISCS_USAGE_TIME,
        SETTING_SYSTEM,
        SETTING_SYSTEM_DATE,
        SETTING_SYSTEM_TIME,
        SETTING_SYSTEM_GRINDERNAME,
        SETTING_SYSTEM_MACHINEINFO,
        SETTING_SYSTEM_FACTORY_RESET,
        SETTING_SYSTEM_DISPLAY_TEST,
        SETTING_SYSTEM_TEMP_UNIT,
        SETTING_SYSTEM_WIFI_SETTING,
        SETTING_SYSTEM_WIFI_SETTING_IP,
        SETTING_SYSTEM_WIFI_SETTING_PW,
        SETTING_SYSTEM_WIFI_SETTING_SSID,
        SETTING_SYSTEM_START_OF_WEEK,
        SETTING_CUSTOMIZE,
        SETTING_CUSTOMIZE_TEMP,
        SETTING_CUSTOMIZE_TEMP_THRESH,
        SETTING_CUSTOMIZE_BRIGHTNESS,
        SETTING_CUSTOMIZE_STDBY_ADJ,
        SETTING_USER_CPC,
        SETTING_USER_CPC_STORE,
        SETTING_USER_CPC_TECH,
        SETTING_USER,
        SETTING_USER_LOGIN,
        SETTING_USER_LOGOUT,
        SETTING_USER_STORE,
        SETTING_USER_TECHNICIAN,
        SETTING_INFO,
        SETTING_INFO_NOTIF,
        SETTING_INFO_STATISTIC,
        SETTING_INVALID
    };

    explicit SettingsStatemachine( QQmlEngine* enigne,
                                   MainStatemachine * pMainStatemachine,
                                   SettingsSerializer* settingsSerializer,
                                   SqliteInterface* sqlInterface );
    virtual ~SettingsStatemachine() override;

    void create();

    Q_INVOKABLE SettingsStatemachine::SettingStates  getSettingState();
    Q_INVOKABLE bool setSettingState( SettingsStatemachine::SettingStates newSettingState,
                                      bool backUsed = false  );
    Q_INVOKABLE QString  getMenuTitle();
    Q_INVOKABLE void setMenuTitle( QString newMenuTitle  );
    Q_INVOKABLE void navigateToMenu(SettingsStatemachine::SettingStates settingState);
    Q_INVOKABLE void back();
    Q_INVOKABLE void saveMenuPosition( double newPosition );
    Q_INVOKABLE double getSavedMenuPostion();
    Q_INVOKABLE void backToMainMenu();
    bool getNavAvailable();
    void setNavAvailable( bool newNavAvailable  );
    bool getButtonPreviousVisible(void);
    void setButtonPreviousVisible(bool buttonPreviousVisible);
    bool getButtonOkVisible(void);
    void setButtonOkVisible(bool buttonOkVisible);
    Q_INVOKABLE void menuToggleswitchChangeHandler( SettingsMenu::ToggleswitchTypes toggleswitchType,
                                                    bool currentValue  );
    bool  checkPinCode( QString pin,
                                    SettingsSerializer::userRole user,
                                    bool logIn);
    Q_INVOKABLE bool  checkPinCode( QString pin,
                                    int user,
                                    bool logIn);
    bool  savePinCode( QString pin,
                                   SettingsSerializer::userRole user);
    Q_INVOKABLE bool  savePinCode( QString pin,
                                   int user);
    Q_INVOKABLE void  logout();
    void  setWifiDriverInSettingsSerializer( WifiDriver* wd );
    void  setDeviceInfoDriverInSettingsSerializer( DeviceInfoCollector* dic );
    SettingsSerializer * settingsSerializer( void );

signals:

    void settingStateChanged();
    void qlpMenuListChanged();
    void menuTitleChanged();
    void navAvailableChanged();
    void navigatedPrevious();
    void buttonPreviousVisibleChanged();
    void buttonOkVisibleChanged();

private:

    bool setMainMenuState();
    bool setDiscsState();
    bool setDiscsCalibrationState();
    bool setDiscsUsageTimeState();
    bool setSystemDateState();
    bool setSystemDisplayTestState();
    bool setSystemFactoryResState();
    bool setSystemGrinderNameState();
    bool setSystemMachineInfoState();
    bool setSystemStartOfWeekState();
    bool setSystemState();
    bool setSystemTempUnitState();
    bool setSystemTimeState();
    bool setSystemWifiIPSettingState();
    bool setSystemWifiPWSettingState();
    bool setSystemWifiSSIDSettingState();
    bool setSystemWifiSettingState();
    bool setCustomizeBrigState();
    bool setCustomizeState();
    bool setCustomizeStdbyAdjState();
    bool setCustomizeTempState();
    bool setCustomizeTempThreshState();
    bool setUserCPCState();
    bool setUserCPCStoreState();
    bool setUserCPCTechState();
    bool setUserLoginState();
    bool setUserLogoutState();
    bool setUserState();
    bool setUserStoreState();
    bool setUserTechState();
    bool setInfoNotifState();
    bool setInfoState();
    bool setInfoStatisticsState();

    void buildUpMainState();
    void buildUpDiscsSetting();
    void buildUpDiscsCalibrationSetting();
    void buildUpDiscsUsageTimeSetting();
    void buildUpSystemDateSettings();
    void buildUpSystemDisplayTestSettings();
    void buildUpSystemFactoryResetSettings();
    void buildUpSystemGrindernameSettings();
    void buildUpSystemMachineInfoSettings();
    void buildUpSystemSettings();
    void buildUpSystemStartOfWeekSettings();
    void buildUpSystemTempUnitSetting();
    void buildUpSystemTimeSettings();
    void buildUpSystemWifiIPSettings();
    void buildUpSystemWifiPWSettings();
    void buildUpSystemWifiSSIDSettings();
    void buildUpSystemWifiSettings();
    void buildUpCustomizeBrigSetting();
    void buildUpCustomizeSetting();
    void buildUpCustomizeStandbyAdjSettings();
    void buildUpCustomizeTempSetting();
    void buildUpCustomizeTempThreshSetting();
    void buildUpUserCpcSetting();
    void buildUpUserCpcStoreSetting();
    void buildUpUserCpcTechSetting();
    void buildUpUserLoginSettings();
    void buildUpUserLogoutSettings();
    void buildUpUserSettings();
    void buildUpUserStoreSettings();
    void buildUpUserTechSettings();
    void buildUpInfoNotifSettings();
    void buildUpInfoSettings();
    void buildUpInfoStatisticsSettings();
    void emptyMenuList();

protected:

    QQmlListProperty<SettingsMenu> getQlpMenuList() const;

private:

    QQmlEngine*                         ThisQMLEngine;
    MainStatemachine *                  m_pMainStatemachine { nullptr };
    StatisticsBoard*                    m_statisticsBoard;
    DiskUsageTime*                      m_diskUsageTime;
    SettingsSerializer *                ThisSettingsSerializer;
    SettingsStatemachine::SettingStates ThisCurrentState;
    SettingsStatemachine::SettingStates ThisLastSettingState;
    QString                             ThisMenuTitle;
    bool                                ThisNavAvailable;
    bool                                m_buttonPreviousVisible;
    bool                                m_buttonOkVisible;

    QList<SettingsMenu*>           ThisMenuList;
    QQmlListProperty<SettingsMenu> ThisQlpMenuList;

    QVector<SettingsStatemachine::SettingStates> oldStates;
    QVector<double> oldPosition;

    static int SettingStates_QTypeId;
    static int QmlSettingStatesId;
};

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#endif // SETTINGS_STATEMACHINE_H
