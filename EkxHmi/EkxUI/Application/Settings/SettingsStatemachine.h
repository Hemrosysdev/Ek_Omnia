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
#include <QVector>

#include "SettingsMenu.h"
#include "SettingsSerializer.h"

class MainStatemachine;
class StatisticsBoard;
class SqliteInterface;
class DiskUsageTime;
class SettingsMenuContentFactory;
class WifiDriver;
class DeviceInfoCollector;

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

class SettingsStatemachine : public QObject
{
    Q_OBJECT

    Q_PROPERTY( SettingsStatemachine::SettingsState settingsState READ settingsState NOTIFY settingsStateChanged )

public:

    enum SettingsState
    {
        SETTING_MAIN,

        SETTING_SERVICE,
        SETTING_SERVICE_DISC_CALIB,
        SETTING_SERVICE_DISC_USAGE_TIME,
        SETTING_SERVICE_DISPLAY_TEST,
        SETTING_SERVICE_MOTOR_TEST,
        SETTING_SERVICE_MOTOR_TEST_PDU,
        SETTING_SERVICE_MOTOR_TEST_AGSA,
        SETTING_SERVICE_MOTOR_TEST_AGSA_MOVE_TO_DDD,
        SETTING_SERVICE_MOTOR_TEST_AGSA_NUM_STEPS,
        SETTING_SERVICE_MOTOR_TEST_AGSA_FREQUENCY,
        SETTING_SERVICE_MOTOR_TEST_AGSA_LONG_DURATION,
        SETTING_SERVICE_DDD_TEST,
        SETTING_SERVICE_MCU_CONFIG,
        SETTING_SERVICE_MCU_CONFIG_MAX_MOTOR_SPEED,
        SETTING_SERVICE_MCU_CONFIG_NOMINAL_MOTOR_SPEED,
        SETTING_SERVICE_MCU_CONFIG_ACCELERATION_TIME,
        SETTING_SERVICE_MCU_CONFIG_DECELERATION_TIME,
        SETTING_SERVICE_SYSTEM_SETUP,

        SETTING_SYSTEM,
        SETTING_SYSTEM_DATE,
        SETTING_SYSTEM_TIME,
        SETTING_SYSTEM_GRINDERNAME,
        SETTING_SYSTEM_FACTORY_RESET,
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
        SETTING_CUSTOMIZE_SETTINGS_MODE_EXIT_TIMEOUT_ADJ,
        SETTING_CUSTOMIZE_AGSA_DELAY,

        SETTING_USER_CPC,
        SETTING_USER_CPC_STORE,
        SETTING_USER_CPC_TECH,
        SETTING_USER,
        SETTING_USER_LOGIN,
        SETTING_USER_LOGOUT,
        SETTING_USER_STORE,
        SETTING_USER_TECHNICIAN,

        SETTING_INFO,
        SETTING_INFO_STATISTICS,
        SETTING_INFO_NOTIFICATION_CENTER,
        SETTING_INFO_MACHINE_INFO,
        SETTING_INFO_OWNERS_MANUAL,
        SETTING_INFO_GRINDER_TUTORIAL,
        SETTING_INFO_PDU_TUTORIAL,

        SETTING_INVALID
    };
    Q_ENUMS( SettingsState )

public:

    explicit SettingsStatemachine( QObject * pParent = nullptr );
    virtual ~SettingsStatemachine() override;

    void create( MainStatemachine *   pMainStatemachine,
                 SettingsSerializer * pSettingsSerializer,
                 SqliteInterface *    pSqliteInterface );

    bool setSettingsState( const SettingsStatemachine::SettingsState nSettingsState,
                           const bool                                bBackUsed = false );

    SettingsStatemachine::SettingsState settingsState() const;

    Q_INVOKABLE void navigateToMenu( const SettingsStatemachine::SettingsState nSettingsState );

    Q_INVOKABLE void back();

    Q_INVOKABLE void backToMainMenu();

    Q_INVOKABLE void saveMenuPosition( const double dPosition );

    Q_INVOKABLE double savedMenuPostion();

    Q_INVOKABLE void menuToggleSwitchChangeHandler( const SettingsMenu::ToggleSwitchTypes nToggleSwitchType,
                                                    const bool                            bCurrentValue );

    bool checkPinCode( const QString &                    strPin,
                       const SettingsSerializer::UserRole nUserRole,
                       const bool                         bLogIn );

    Q_INVOKABLE bool checkPinCode( QString pin,
                                   int     user,
                                   bool    logIn );

    bool savePinCode( const QString &                    strPin,
                      const SettingsSerializer::UserRole nUserRole );

    Q_INVOKABLE bool savePinCode( QString pin,
                                  int     user );

    Q_INVOKABLE void logout();

    //SettingsSerializer & settingsSerializer( void );
    DiskUsageTime & diskUsageTime();

    SettingsMenuContentFactory & contentFactory();

    StatisticsBoard & statisticsBoard();

signals:

    void settingsStateChanged();

    void navigatedPrevious();

    void popNextRecoverActionStep();

public slots:

    void processMainstateChanged();

private:

    bool setMainMenuState();

    bool setServiceState();

    bool setServiceDiscCalibrationState();

    bool setServiceDiscUsageTimeState();

    bool setServiceDisplayTestState();

    bool setServiceMotorTestState();

    bool setServiceMotorTestAgsaState();

    bool setServiceMotorTestAgsaMoveToDddState();

    bool setServiceMotorTestAgsaNumStepsState();

    bool setServiceMotorTestAgsaFrequencyState();

    bool setServiceMotorTestPduState();

    bool setServiceMotorTestAgsaLongDurationState();

    bool setServiceDddTestState();

    bool setServiceMcuConfigState();

    bool setServiceMcuConfigMaxMotorSpeedState();

    bool setServiceMcuConfigNominalMotorSpeedState();

    bool setServiceMcuConfigAccelerationTimeState();

    bool setServiceMcuConfigDecelerationTimeState();

    bool setServiceSystemSetupState();

    bool setSystemState();

    bool setSystemDateState();

    bool setSystemFactoryResState();

    bool setSystemGrinderNameState();

    bool setSystemStartOfWeekState();

    bool setSystemTempUnitState();

    bool setSystemTimeState();

    bool setSystemWifiIpState();

    bool setSystemWifiPasswordState();

    bool setSystemWifiSsidState();

    bool setSystemWifiState();

    bool setCustomizeState();

    bool setCustomizeBrightnessState();

    bool setCustomizeStdbyAdjState();

    bool setCustomizeSettingsModeExitTimeoutAdjState();

    bool setCustomizeAgsaDelayState();

    bool setCustomizeTempState();

    bool setCustomizeTempThreshState();

    bool setUserState();

    bool setUserCPCState();

    bool setUserCPCStoreState();

    bool setUserCPCTechState();

    bool setUserLoginState();

    bool setUserLogoutState();

    bool setUserStoreState();

    bool setUserTechState();

    bool setInfoState();

    bool setInfoStatisticsState();

    bool setInfoNotificationCenterState();

    bool setInfoMachineInfoState();

    bool setInfoOwnersManualState();

    bool setInfoGrinderTutorialState();

    bool setInfoPduTutorialState();

private:

    MainStatemachine *                           m_pMainStatemachine { nullptr };
    StatisticsBoard *                            m_pStatisticsBoard { nullptr };
    DiskUsageTime *                              m_pDiskUsageTime { nullptr };
    SettingsSerializer *                         m_pSettingsSerializer { nullptr };
    SettingsStatemachine::SettingsState          m_nCurrentState { SettingsState::SETTING_MAIN };
    SettingsStatemachine::SettingsState          m_nLastSettingsState { SettingsState::SETTING_INVALID };

    QVector<SettingsStatemachine::SettingsState> m_theOldStates;
    QVector<double>                              m_theOldPositions;

    SettingsMenuContentFactory *                 m_pContentFactory { nullptr };

    //    static int                                      SettingsState_QTypeId;
    //    static int                                      QmlSettingsStateId;
};

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#endif // SETTINGS_STATEMACHINE_H
