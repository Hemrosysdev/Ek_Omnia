///////////////////////////////////////////////////////////////////////////////
///
/// @file SettingsSerializer.cpp
///
/// @brief Implementation file of class SettingsSerializer
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

#include "SettingsSerializer.h"
#include "EkxGlobals.h"

#include <QtQml>

#include "DeviceInfoCollector.h"
#include "WifiDriver.h"
#include "MainStatemachine.h"
#include "SqliteInterface.h"
#include "EspDcHallMotorDriver.h"
#include "AgsaControl.h"
#include "AgsaLongDurationTest.h"
#include "NotificationCenter.h"
#include "Notification.h"
#include "JsonHelper.h"

#ifdef TARGETBUILD
#include <unistd.h> // for sync(2)
#endif

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#define SETTINGS_FILENAME                          "/config/EkxSettings.json"
#define SETTINGS_CONFIG                            "version"
#define SETTINGS_CONFIG_VERSION                    "01.14"

#define SETTING_JSON_GRINDER_NAME                  "grinderName"
#define SETTING_JSON_TIME_MODE_IS_ENABLED          "timeModeIsEnabled"
#define SETTING_JSON_LIBRARY_MODE_IS_ENABLED       "libraryModeIsEnabled"
#define SETTING_JSON_AGSA_ENABLED                  "agsaEnabled"
#define SETTING_JSON_AGSA_START_DELAY_SEC          "agsaStartDelaySec"
#define SETTING_JSON_DISPLAY_BRIGHTNESS            "displayBrightness"
#define SETTING_JSON_STDBY_TIME_INDEX              "standbyTimeIndex"
#define SETTING_JSON_SETTINGS_MODE_EXIT_TIME_INDEX "settingsModeExitTimeIndex"
#define SETTING_JSON_DDD1_CALIBRATION_ANGLE_DEG    "ddd1CalibrationAngleDeg"
#define SETTING_JSON_DDD2_CALIBRATION_ANGLE_DEG    "ddd2CalibrationAngleDeg"
#define SETTING_JSON_START_OF_WEEK_SUN_NOT_MON     "startOfWeekIsSunNotMon"
#define SETTING_JSON_STATISTICS_PERIOD_INDEX       "statisticsPeriodIndex"
#define SETTING_JSON_DISC_USAGE_MAX_HOURS          "discUsageMaxHours"
#define SETTING_JSON_SHOW_TEMP                     "showTemp"
#define SETTING_JSON_SHOW_TEMP_WARNING             "showTempWarning"
#define SETTING_JSON_SHOW_DOSING_TIME              "showDosingTime"
#define SETTING_JSON_SHOW_SPOTLIGHT                "showSpotlight"
#define SETTING_JSON_TEMP_UNIT                     "tempUnit"
#define SETTING_JSON_TEMP_THRESHOLD_CELSIUS_X100   "tempThresholdCelsiusX100"
#define SETTING_JSON_USER_AUTOLOGIN_AS             "userAutoLoginAs"
#define SETTING_JSON_USER_AUTOLOGOUT_ENABLE        "userAutoLogoutEnable"
#define SETTING_JSON_USER_PIN_FOR_OWNER            "userPinForOwner"
#define SETTING_JSON_USER_PIN_FOR_TECHNICIAN       "userPinForTechnician"
#define SETTING_JSON_WIFI_MODE                     "wifiMode"
#define SETTING_JSON_WIFI_AP_PASSWORD              "wifiApPassword"
#define SETTING_JSON_WIFI_AP_SSID                  "wifiApSsid"

#define SETTING_JSON_PDU_INSTALLED                 "pduInstalled"
#define SETTING_JSON_SYSTEM_SETUP_DONE             "systemSetupDone"
#define SETTING_JSON_INITIAL_SETUP_DONE            "initialSetupDone"
#define SETTING_JSON_PDU_ACKNOWLEDGED              "pduAcknowledged"
#define SETTING_JSON_PDU_DC_HALL_STEPS             "pduDcHallSteps"
#define SETTING_JSON_PDU_DC_HALL_PWM_DUTY_10TH     "pduDcHallPwmDuty10th"
#define SETTING_JSON_PDU_DC_HALL_PWM_FREQ          "pduDcHallPwmFreq"

#define SETTING_JSON_AGSA_STEPPER_STEPS            "agsaStepperSteps"
#define SETTING_JSON_AGSA_STEPPER_RUN_FREQ         "agsaStepperRunFreq"
#define SETTING_JSON_AGSA_STEPPER_APPROACH_FREQ    "agsaStepperApproachFreq"
#define SETTING_JSON_AGSA_TEST_LIVE_MODE           "agsaTestLiveMode"
#define SETTING_JSON_AGSA_TEST_BLOCKAGE_DETECT     "agsaTestBlockageDetect"

#define SETTING_JSON_AGSA_LDT_START_DDD_VALUE      "agsaLdtStartDddValue"
#define SETTING_JSON_AGSA_LDT_STOP_DDD_VALUE       "agsaLdtStopDddValue"
#define SETTING_JSON_AGSA_LDT_CYCLES_NUM           "agsaLdtCyclesNum"

#define SETTING_JSON_AGSA_LDT_TEST_STEPS           "agsaLdtTestSteps"

#define SETTING_JSON_MCU_MAX_MOTOR_SPEED           "mcuMaxMotorSpeed"
#define SETTING_JSON_MCU_NOMINAL_MOTOR_SPEED       "mcuNominalMotorSpeed"
#define SETTING_JSON_MCU_ACCELERATION_TIME         "mcuAccelerationTime"
#define SETTING_JSON_MCU_DECELERATION_TIME         "mcuDecelerationTime"

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

const char * SettingsSerializer::c_pszGrinderName          = "";
const char * SettingsSerializer::c_pszUserPinForOwner      = "1924";
const char * SettingsSerializer::c_pszUserPinForTechnician = "1311";
const char * SettingsSerializer::c_pszWifiApPassword       = "";
const char * SettingsSerializer::c_pszWifiApSsid           = "";

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

SettingsSerializer::SettingsSerializer( QObject * pParent )
    : QObject( pParent )
{
    MainStatemachine::ensureExistingDirectory( MainStatemachine::rootPath() + SETTINGS_FILENAME );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

SettingsSerializer::~SettingsSerializer()
{
    m_pMainStatemachine     = nullptr;
    m_pPduDcHallMotorDriver = nullptr;
    m_pAgsaControl          = nullptr;
    m_pDeviceInfoCollector  = nullptr;
    m_pWifiDriver           = nullptr;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////


void SettingsSerializer::create( MainStatemachine *     pMainStatemachine,
                                 EspDcHallMotorDriver * pPduDcHallMotorDriver,
                                 AgsaControl *          pAgsaControl,
                                 DeviceInfoCollector *  pDeviceInfoCollector,
                                 WifiDriver *           pWifiDriver )
{
    m_pMainStatemachine     = pMainStatemachine;
    m_pPduDcHallMotorDriver = pPduDcHallMotorDriver;
    m_pAgsaControl          = pAgsaControl;
    m_pDeviceInfoCollector  = pDeviceInfoCollector;
    m_pWifiDriver           = pWifiDriver;

    m_acycNotificationSaveFailed.create( m_pMainStatemachine->notificationCenter(),
                                         EkxSqliteTypes::SqliteNotificationType_SAVE_SETTINGS_FAILED );
    m_acycNotificationLoadFailed.create( m_pMainStatemachine->notificationCenter(),
                                         EkxSqliteTypes::SqliteNotificationType_LOAD_SETTINGS_FAILED );

    if ( !loadSettings() )
    {
        setDefaultSettings();
    }

    if ( updateStandardGrinderData() )
    {
        saveSettings();
    }

    userAutoLogout();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsSerializer::applyWifiSettingsToDriver()
{
    if ( m_pWifiDriver )
    {
        m_pWifiDriver->setConfig( wifiMode(),
                                  wifiApSsid(),
                                  wifiApPassword(),
                                  "192.168.4.1",
                                  "",
                                  "",
                                  true,
                                  "",
                                  "",
                                  "",
                                  "" );
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsSerializer::setDefaultSettings()
{
    qInfo() << "SettingsSerializer::setDefaultSettings()";

    setGrinderName( c_pszGrinderName );       // will be set after reading serial# info
    setTimeModeIsEnabled( c_bTimeModeIsEnabled );
    setLibraryModeIsEnabled( c_bLibraryModeIsEnabled );
    setAgsaEnabled( c_bAgsaEnabled );
    setAgsaStartDelaySec( c_nAgsaStartDelaySec );
    setDisplayBrightness( c_nDisplayBrightness );
    setStdbyTimeIndex( c_nStdbyTimeIndex );
    setSettingsModeExitTimeIndex( c_nSettingsModeExitTimeIndex );
    setDdd1CalibrationAngle10thDeg( c_nDdd1CalibrationAngleDeg );
    setDdd2CalibrationAngle10thDeg( c_nDdd2CalibrationAngleDeg );
    setStartOfWeekSunday( c_bStartOfWeekSun );
    setStatisticsPeriodIndex( c_nStatisticsPeriodIndex );
    setDiscUsageMaxHours( c_nDiscUsageMaxHours );

    setShowTemp( c_bShowTemp );
    setShowTempWarning( c_bShowTempWarning );
    setShowDosingTime( c_bShowDosingTime );
    setShowSpotLight( c_bShowSpotLight );

    setTempUnit( c_nTempUnit );
    setTempThresValueX100( c_nTempThresholdValueX100 );

    setCurrentLoggedInUserRole( c_nCurrentLoggedInUserRole );
    setUserAutoLogoutEnable( c_bUserAutoLogoutEnable );
    setUserPinForOwner( c_pszUserPinForOwner );
    setUserPinForTechnician( c_pszUserPinForTechnician );

    setWifiMode( c_nWifiMode );
    setWifiApSsid( c_pszWifiApSsid );         // will be set after reading serial# info
    setWifiApPassword( c_pszWifiApPassword ); // will be set after reading serial# info

    setMcuMaxMotorSpeed( c_u32DefMcuMaxMotorSpeed );
    setMcuNominalMotorSpeed( c_u32DefMcuNominalMotorSpeed );
    setMcuAccelerationTime( c_u32DefMcuAccelerationTime );
    setMcuDecelerationTime( c_u32DefMcuDecelerationTime );

    // DO NOT reset systemSetupDone!

    setInitialSetupDone( c_bInitialSetupDone );
    setPduAcknowledged( c_bPduAcknowledged );

    updateStandardGrinderData();
    applyWifiSettingsToDriver();
    emit mcuConfigChanged();

    saveSettings();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

QString SettingsSerializer::tempUnitToString( const SettingsSerializer::TempUnit nTempUnit )
{
    QString strTempUnitString;

    switch ( nTempUnit )
    {
        case TempUnit::TEMPUNIT_CELSIUS:
            strTempUnitString = "°C";
            break;
        case TempUnit::TEMPUNIT_FAHRENHEIT:
            strTempUnitString = "°F";
            break;
        default:
            strTempUnitString = "?";
    }
    return strTempUnitString;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

QString SettingsSerializer::userRoleToString( SettingsSerializer::UserRole loggedIn )
{
    QString loggedInString;
    switch ( loggedIn )
    {
        case SettingsSerializer::UserRole::StoreOwner:
            loggedInString = "Store Owner";
            break;
        case SettingsSerializer::UserRole::Technician:
            loggedInString = "Technician";
            break;
        case SettingsSerializer::UserRole::NoRole:
            loggedInString = "None";
            break;
        default:
            loggedInString = "?";
    }
    return loggedInString;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

int SettingsSerializer::checkRanges( const int nCheckValue,
                                     const int nDefaultValue,
                                     const int nMinValue,
                                     const int nMaxValue )
{
    int nTempValue = nCheckValue;

    if ( ( nTempValue < nMinValue )
         || ( nTempValue > nMaxValue ) )
    {
        nTempValue = nDefaultValue;
    }

    return nTempValue;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsSerializer::raiseMcuConfigChanged()
{
    emit mcuConfigChanged();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsSerializer::setMcuMaxMotorSpeed( const uint32_t u32Value )
{
    //uint32_t u32TempValue = checkRanges( u32Value,
    //                                     u32McuMaxMotorSpeed,
    //                                     c_u32MinMcuMaxMotorSpeed,
    //                                     c_u32MaxMcuMaxMotorSpeed );

    if ( m_u32McuMaxMotorSpeed != u32Value )
    {
        m_u32McuMaxMotorSpeed = u32Value;
        emit mcuMaxMotorSpeedChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

uint32_t SettingsSerializer::mcuMaxMotorSpeed() const
{
    return m_u32McuMaxMotorSpeed;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

uint32_t SettingsSerializer::minMcuMaxMotorSpeed() const
{
    return c_u32MinMcuMaxMotorSpeed;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

uint32_t SettingsSerializer::maxMcuMaxMotorSpeed() const
{
    return c_u32MaxMcuMaxMotorSpeed;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsSerializer::setMcuNominalMotorSpeed( const uint32_t u32Value )
{
    //uint32_t u32TempValue = checkRanges( u32Value,
    //                                     u32McuNominalMotorSpeed,
    //                                     c_u32MinMcuNominalMotorSpeed,
    //                                     u32McuMaxMotorSpeed );

    if ( m_u32McuNominalMotorSpeed != u32Value )
    {
        m_u32McuNominalMotorSpeed = u32Value;
        emit mcuNominalMotorSpeedChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

uint32_t SettingsSerializer::mcuNominalMotorSpeed() const
{
    return m_u32McuNominalMotorSpeed;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

uint32_t SettingsSerializer::minMcuNominalMotorSpeed() const
{
    return c_u32MinMcuNominalMotorSpeed;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

uint32_t SettingsSerializer::maxMcuNominalMotorSpeed() const
{
    return mcuMaxMotorSpeed();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsSerializer::setMcuAccelerationTime( const uint32_t u32Value )
{
    if ( m_u32McuAccelerationTime != u32Value )
    {
        m_u32McuAccelerationTime = u32Value;
        emit mcuAccelerationTimeChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

uint32_t SettingsSerializer::mcuAccelerationTime() const
{
    return m_u32McuAccelerationTime;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

uint32_t SettingsSerializer::minMcuAccelerationTime() const
{
    return c_u32MinMcuAccelerationTime;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

uint32_t SettingsSerializer::maxMcuAccelerationTime() const
{
    return mcuMaxMotorSpeed();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsSerializer::setMcuDecelerationTime( const uint32_t u32Value )
{
    if ( m_u32McuDecelerationTime != u32Value )
    {
        m_u32McuDecelerationTime = u32Value;
        emit mcuDecelerationTimeChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

uint32_t SettingsSerializer::mcuDecelerationTime() const
{
    return m_u32McuDecelerationTime;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

uint32_t SettingsSerializer::minMcuDecelerationTime() const
{
    return c_u32MinMcuDecelerationTime;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

uint32_t SettingsSerializer::maxMcuDecelerationTime() const
{
    return mcuMaxMotorSpeed();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsSerializer::setPduInstalled( const bool bInstalled )
{
    if ( m_bPduInstalled != bInstalled )
    {
        m_bPduInstalled = bInstalled;
        emit pduInstalledChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SettingsSerializer::isPduInstalled() const
{
    return m_bPduInstalled;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsSerializer::setSystemSetupDone( bool bDone )
{
    if ( m_bSystemSetupDone != bDone )
    {
        m_bSystemSetupDone = bDone;
        Q_EMIT systemSetupDoneChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SettingsSerializer::isSystemSetupDone() const
{
    return m_bSystemSetupDone;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsSerializer::setInitialSetupDone( bool bDone )
{
    if ( m_bInitialSetupDone != bDone )
    {
        m_bInitialSetupDone = bDone;
        Q_EMIT initialSetupDoneChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SettingsSerializer::isInitialSetupDone() const
{
    return m_bInitialSetupDone;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsSerializer::setPduAcknowledged( bool bAcknowledged )
{
    if ( m_bPduAcknowledged != bAcknowledged )
    {
        m_bPduAcknowledged = bAcknowledged;
        Q_EMIT pduAcknowledgedChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SettingsSerializer::isPduAcknowledged() const
{
    return m_bPduAcknowledged;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

QString SettingsSerializer::stdbyTimeIndexToString( const int nIndex )
{
    QString str;

    switch ( nIndex )
    {
        case 0:
            str = "3m";
            break;
        case 1:
            str = "5m";
            break;
        case 2:
            str = "10m";
            break;
        case 3:
            str = "20m";
            break;
        case 4:
            str = "30m";
            break;
        default:
            str = "??m";
    }
    return str;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

int SettingsSerializer::stdbyTimeIndexToStdbyTimeMinutes( const int nIndex )
{
    int nMinutes = 5;

    switch ( nIndex )
    {
        case 0:
            nMinutes = 3;
            break;
        case 1:
            nMinutes = 5;
            break;
        case 2:
            nMinutes = 10;
            break;
        case 3:
            nMinutes = 20;
            break;
        case 4:
            nMinutes = 30;
            break;
        default:
            nMinutes = 5;
            break;
    }

    return nMinutes;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

QString SettingsSerializer::settingsModeExitTimeIndexToString( const int nIndex )
{
    QString str;

    switch ( nIndex )
    {
        case 0:
            str = "30s";
            break;
        case 1:
            str = "1m";
            break;
        case 2:
            str = "3m";
            break;
        case 3:
            str = "5m";
            break;
        case 4:
            str = "10m";
            break;
        case 5:
            str = "20m";
            break;
        case 6:
            str = "30m";
            break;
        default:
            str = "??m";
    }
    return str;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

int SettingsSerializer::settingsModeExitTimeIndexToSeconds( const int nIndex )
{
    int nSeconds = 5;

    switch ( nIndex )
    {
        case 0:
            nSeconds = 30;
            break;
        case 1:
            nSeconds = 60;
            break;
        case 2:
            nSeconds = 180;
            break;
        case 3:
            nSeconds = 300;
            break;
        case 4:
            nSeconds = 600;
            break;
        case 5:
            nSeconds = 1200;
            break;
        case 6:
            nSeconds = 1800;
            break;
        default:
            nSeconds = 30;
            break;
    }

    return nSeconds;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SettingsSerializer::showTemp() const
{
    return m_bShowTemp;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsSerializer::setShowTemp( const bool bShowTemp )
{
    if ( m_bShowTemp != bShowTemp )
    {
        m_bShowTemp = bShowTemp;

        emit showTempEnableChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

SettingsSerializer::TempUnit SettingsSerializer::tempUnit() const
{
    return m_nTempUnit;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsSerializer::setTempUnit( const TempUnit nTempUnit )
{
    if ( m_nTempUnit != nTempUnit )
    {
        m_nTempUnit = nTempUnit;
        emit tempUnitChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SettingsSerializer::showTempWarning() const
{
    return m_bShowTempWarning;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsSerializer::setShowTempWarning( const bool bShowTempWarning )
{
    if ( m_bShowTempWarning != bShowTempWarning )
    {
        m_bShowTempWarning = bShowTempWarning;

        emit showTempWarningChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

int SettingsSerializer::getTempThresValueX100() const
{
    int tempX100;

    if ( m_nTempUnit == TempUnit::TEMPUNIT_CELSIUS )
    {
        tempX100 = m_nTempThreshCelsiusX100;
    }
    else if ( m_nTempUnit == TempUnit::TEMPUNIT_FAHRENHEIT )
    {
        tempX100 = celsiusX100ToFahrenheitX100( m_nTempThreshCelsiusX100 );
    }
    else
    {
        qCritical() << "SettingsSerializer::getTempThresValue(): NOT IMPLEMENTED FOR THIS TEMP UNIT" << static_cast<int>( m_nTempUnit ) << "\n  Assuming Celsius";
        tempX100 = m_nTempThreshCelsiusX100;
    }

    return tempX100;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsSerializer::setTempThresValueX100( const int nTempX100 )
{
    int nNewTempCelsiusX100 = m_nTempThreshCelsiusX100;

    if ( m_nTempUnit == TempUnit::TEMPUNIT_CELSIUS )
    {
        nNewTempCelsiusX100 = ( ( nTempX100 + (int)( 0.5 * 100 ) ) / 100 ) * 100; // round to 1 Celsius
    }
    else if ( m_nTempUnit == TempUnit::TEMPUNIT_FAHRENHEIT )
    {
        nNewTempCelsiusX100 = ( ( nTempX100 + (int)( 0.5 * 100 ) ) / 100 ) * 100; // round to 1 Fahrenheit
        nNewTempCelsiusX100 = fahrenheitX100ToCelsiusX100( nNewTempCelsiusX100 );
    }
    else
    {
        qCritical() << "SettingsSerializer::setTempThresValue(): NOT IMPLEMENTED FOR THIS TEMP UNIT" << static_cast<int>( m_nTempUnit );
    }

    if ( m_nTempThreshCelsiusX100 != nNewTempCelsiusX100 )
    {
        m_nTempThreshCelsiusX100 = nNewTempCelsiusX100;
        emit tempThresValueChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SettingsSerializer::libraryModeIsEnabled() const
{
    return m_bLibraryModeIsEnabled;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsSerializer::setLibraryModeIsEnabled( const bool bEnable )
{
    if ( m_bLibraryModeIsEnabled != bEnable )
    {
        m_bLibraryModeIsEnabled = bEnable;
        emit libraryModeIsEnabledChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SettingsSerializer::agsaEnabled() const
{
    return m_bAgsaEnabled;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsSerializer::setAgsaEnabled( const bool bEnabled )
{
    if ( m_bAgsaEnabled != bEnabled )
    {
        m_bAgsaEnabled = bEnabled;
        emit agsaEnabledChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

int SettingsSerializer::agsaStartDelaySec() const
{
    return m_nAgsaStartDelaySec;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsSerializer::setAgsaStartDelaySec( const int nDelaySec )
{
    if ( m_nAgsaStartDelaySec != nDelaySec )
    {
        m_nAgsaStartDelaySec = nDelaySec;
        emit agsaStartDelaySecChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SettingsSerializer::isUserAutoLogoutEnabled() const
{
    return m_bUserAutoLogoutEnable;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsSerializer::setUserAutoLogoutEnable( const bool bEnable )
{
    if ( m_bUserAutoLogoutEnable != bEnable )
    {
        m_bUserAutoLogoutEnable = bEnable;

        emit userAutoLogoutEnableChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SettingsSerializer::timeModeIsEnabled() const
{
    return m_bTimeModeIsEnabled;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsSerializer::setTimeModeIsEnabled( const bool bEnable )
{
    if ( m_bTimeModeIsEnabled != bEnable )
    {
        m_bTimeModeIsEnabled = bEnable;

        emit timeModeIsEnabledChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

int SettingsSerializer::displayBrightness() const
{
    return m_nDisplayBrightness;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsSerializer::setDisplayBrightness( const int nDisplayBrightness )
{
    if ( m_nDisplayBrightness != nDisplayBrightness )
    {
        m_nDisplayBrightness = nDisplayBrightness;
        emit displayBrightnessChanged( nDisplayBrightness );
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

int SettingsSerializer::ddd1CalibrationAngle10thDeg() const
{
    return m_nDdd1CalibrationAngle10thDeg;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsSerializer::setDdd1CalibrationAngle10thDeg( const int nDddCalibrationAngle10thDeg )
{
    if ( nDddCalibrationAngle10thDeg != m_nDdd1CalibrationAngle10thDeg )
    {
        m_nDdd1CalibrationAngle10thDeg = nDddCalibrationAngle10thDeg;
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

int SettingsSerializer::ddd2CalibrationAngleq10thDeg() const
{
    return m_nDdd2CalibrationAngle10thDeg;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsSerializer::setDdd2CalibrationAngle10thDeg( const int nDddCalibrationAngle10thDeg )
{
    if ( nDddCalibrationAngle10thDeg != m_nDdd2CalibrationAngle10thDeg )
    {
        m_nDdd2CalibrationAngle10thDeg = nDddCalibrationAngle10thDeg;
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SettingsSerializer::showDosingTime() const
{
    return m_bShowDosingTime;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsSerializer::setShowDosingTime( const bool bShow )
{
    if ( m_bShowDosingTime != bShow )
    {
        m_bShowDosingTime = bShow;
        Q_EMIT showDosingTimeChanged( bShow );
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SettingsSerializer::showSpotLight() const
{
    return m_bShowSpotlight;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsSerializer::setShowSpotLight( const bool bShow )
{
    if ( m_bShowSpotlight != bShow )
    {
        m_bShowSpotlight = bShow;
        emit showSpotLightChanged( bShow );
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

const QString & SettingsSerializer::userPinForOwner() const
{
    return m_strUserPinForOwner;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsSerializer::setUserPinForOwner( const QString & strStoreOwnerPin )
{
    if ( m_strUserPinForOwner != strStoreOwnerPin )
    {
        m_strUserPinForOwner = strStoreOwnerPin;
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

const QString & SettingsSerializer::userPinForTechnician() const
{
    return m_strUserPinForTechnician;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsSerializer::setUserPinForTechnician( const QString & strTechnicianPin )
{
    if ( m_strUserPinForTechnician != strTechnicianPin )
    {
        m_strUserPinForTechnician = strTechnicianPin;
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

SettingsSerializer::UserRole SettingsSerializer::currentLoggedInUserRole() const
{
    return m_nCurrentLoggedInUserRole;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsSerializer::setCurrentLoggedInUserRole( SettingsSerializer::UserRole userRole )
{
    if ( m_nCurrentLoggedInUserRole != userRole )
    {
        switch ( m_nCurrentLoggedInUserRole )
        {
            case UserRole::NoRole:
                break;

            case UserRole::StoreOwner:
                m_pMainStatemachine->sqliteInterface()->addEvent( EkxSqliteTypes::SqliteEventType_OWNER_LOGOUT, "" );
                break;

            case UserRole::Technician:
                m_pMainStatemachine->sqliteInterface()->addEvent( EkxSqliteTypes::SqliteEventType_TECHNICIAN_LOGOUT, "" );
                break;
        }

        switch ( userRole )
        {
            case UserRole::NoRole:
                break;

            case UserRole::StoreOwner:
                m_pMainStatemachine->sqliteInterface()->addEvent( EkxSqliteTypes::SqliteEventType_OWNER_LOGIN, "" );

                break;

            case UserRole::Technician:
                m_pMainStatemachine->sqliteInterface()->addEvent( EkxSqliteTypes::SqliteEventType_TECHNICIAN_LOGIN, "" );

                break;
        }

        m_nCurrentLoggedInUserRole = userRole;

        emit currentLoggedInUserRoleChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

QString SettingsSerializer::grinderName() const
{
    return m_strGrinderName;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsSerializer::setGrinderName( const QString & strGrinderName )
{
    if ( m_strGrinderName != strGrinderName )
    {
        m_strGrinderName = strGrinderName;
        emit grinderNameChanged();

        setWifiApSsid( strGrinderName );
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

int SettingsSerializer::stdbyTimeIndex() const
{
    return m_nStdbyTimeIndex;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsSerializer::setStdbyTimeIndex( const int nStdbyTimeIndex )
{
    if ( m_nStdbyTimeIndex != nStdbyTimeIndex )
    {
        m_nStdbyTimeIndex = nStdbyTimeIndex;
        emit stdbyTimeIndexChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

int SettingsSerializer::settingsModeExitTimeIndex() const
{
    return m_nSettingsModeExitTimeIndex;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsSerializer::setSettingsModeExitTimeIndex( const int nIndex )
{
    int nTempIndex = checkRanges( nIndex, 0, 0, 6 );

    if ( m_nSettingsModeExitTimeIndex != nTempIndex )
    {
        m_nSettingsModeExitTimeIndex = nTempIndex;
        emit settingsModeExitTimeIndexChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SettingsSerializer::isWifiEnabled() const
{
    return m_nWifiMode != WifiMode::MODE_OFF;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

SettingsSerializer::WifiMode SettingsSerializer::wifiMode() const
{
    return m_nWifiMode;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsSerializer::setWifiMode( const WifiMode nMode )
{
    if ( m_nWifiMode != nMode )
    {
        m_nWifiMode = nMode;
        emit wifiModeChanged( m_nWifiMode );
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

const QString & SettingsSerializer::wifiApPassword() const
{
    return m_strWifiApPassword;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsSerializer::setWifiApPassword( const QString & strApPassword )
{
    if ( m_strWifiApPassword != strApPassword )
    {
        m_strWifiApPassword = strApPassword;
        emit wifiApPasswordChanged();

        if ( m_pWifiDriver )
        {
            m_pWifiDriver->setApPassword( strApPassword );
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

const QString & SettingsSerializer::wifiApSsid() const
{
    return m_strWifiApSsid;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsSerializer::setWifiApSsid( const QString & strSsid )
{
    if ( m_strWifiApSsid != strSsid )
    {
        m_strWifiApSsid = strSsid;
        emit wifiApSsidChanged();

        if ( m_pWifiDriver )
        {
            m_pWifiDriver->setApSsid( strSsid );
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SettingsSerializer::saveSettings()
{
    qDebug() << "SettingsSerializer::saveSettings() called!";
    bool bSuccess = false;

    QJsonObject settings;

    settings[SETTINGS_CONFIG] = SETTINGS_CONFIG_VERSION;

    settings[SETTING_JSON_GRINDER_NAME]                  = grinderName();
    settings[SETTING_JSON_TIME_MODE_IS_ENABLED]          = timeModeIsEnabled();
    settings[SETTING_JSON_LIBRARY_MODE_IS_ENABLED]       = libraryModeIsEnabled();
    settings[SETTING_JSON_AGSA_ENABLED]                  = agsaEnabled();
    settings[SETTING_JSON_AGSA_START_DELAY_SEC]          = agsaStartDelaySec();
    settings[SETTING_JSON_DISPLAY_BRIGHTNESS]            = displayBrightness();
    settings[SETTING_JSON_STDBY_TIME_INDEX]              = stdbyTimeIndex();
    settings[SETTING_JSON_SETTINGS_MODE_EXIT_TIME_INDEX] = settingsModeExitTimeIndex();
    settings[SETTING_JSON_DDD1_CALIBRATION_ANGLE_DEG]    = ddd1CalibrationAngle10thDeg();
    settings[SETTING_JSON_DDD2_CALIBRATION_ANGLE_DEG]    = ddd2CalibrationAngleq10thDeg();
    settings[SETTING_JSON_START_OF_WEEK_SUN_NOT_MON]     = isStartOfWeekSun();
    settings[SETTING_JSON_STATISTICS_PERIOD_INDEX]       = statisticsPeriodIndex();
    settings[SETTING_JSON_DISC_USAGE_MAX_HOURS]          = discUsageMaxHours();

    settings[SETTING_JSON_SHOW_TEMP]         = showTemp();
    settings[SETTING_JSON_SHOW_TEMP_WARNING] = showTempWarning();
    settings[SETTING_JSON_SHOW_DOSING_TIME]  = showDosingTime();
    settings[SETTING_JSON_SHOW_SPOTLIGHT]    = showSpotLight();

    settings[SETTING_JSON_TEMP_UNIT]                   = static_cast<int>( tempUnit() );
    settings[SETTING_JSON_TEMP_THRESHOLD_CELSIUS_X100] = m_nTempThreshCelsiusX100;     // always in celsiusX100

    settings[SETTING_JSON_USER_AUTOLOGIN_AS]       = static_cast<int>( currentLoggedInUserRole() );
    settings[SETTING_JSON_USER_AUTOLOGOUT_ENABLE]  = isUserAutoLogoutEnabled();
    settings[SETTING_JSON_USER_PIN_FOR_OWNER]      = userPinForOwner();
    settings[SETTING_JSON_USER_PIN_FOR_TECHNICIAN] = userPinForTechnician();

    settings[SETTING_JSON_WIFI_MODE]        = static_cast<int>( wifiMode() );
    settings[SETTING_JSON_WIFI_AP_PASSWORD] = wifiApPassword();
    settings[SETTING_JSON_WIFI_AP_SSID]     = wifiApSsid();

    settings[SETTING_JSON_MCU_MAX_MOTOR_SPEED]     = static_cast<int>( mcuMaxMotorSpeed() );
    settings[SETTING_JSON_MCU_NOMINAL_MOTOR_SPEED] = static_cast<int>( mcuNominalMotorSpeed() );
    settings[SETTING_JSON_MCU_ACCELERATION_TIME]   = static_cast<int>( mcuAccelerationTime() );
    settings[SETTING_JSON_MCU_DECELERATION_TIME]   = static_cast<int>( mcuDecelerationTime() );

    settings[SETTING_JSON_PDU_INSTALLED] = isPduInstalled();

    settings[SETTING_JSON_SYSTEM_SETUP_DONE]  = isSystemSetupDone();
    settings[SETTING_JSON_INITIAL_SETUP_DONE] = isInitialSetupDone();
    settings[SETTING_JSON_PDU_ACKNOWLEDGED]   = isPduAcknowledged();

    if ( m_pPduDcHallMotorDriver )
    {
        settings[SETTING_JSON_PDU_DC_HALL_STEPS]         = m_pPduDcHallMotorDriver->steps();
        settings[SETTING_JSON_PDU_DC_HALL_PWM_DUTY_10TH] = m_pPduDcHallMotorDriver->pwmDuty10th();
        settings[SETTING_JSON_PDU_DC_HALL_PWM_FREQ]      = m_pPduDcHallMotorDriver->pwmFrequency();
    }

    if ( m_pAgsaControl )
    {
        settings[SETTING_JSON_AGSA_STEPPER_STEPS]         = m_pAgsaControl->steps();
        settings[SETTING_JSON_AGSA_STEPPER_RUN_FREQ]      = m_pAgsaControl->runFrequency();
        settings[SETTING_JSON_AGSA_STEPPER_APPROACH_FREQ] = m_pAgsaControl->approachFrequency();
        settings[SETTING_JSON_AGSA_TEST_LIVE_MODE]        = m_pAgsaControl->testLiveMode();
        settings[SETTING_JSON_AGSA_TEST_BLOCKAGE_DETECT]  = m_pAgsaControl->testBlockageDetect();

        settings[SETTING_JSON_AGSA_LDT_START_DDD_VALUE] = m_pAgsaControl->longDurationTest()->manualStartDddValue();
        settings[SETTING_JSON_AGSA_LDT_STOP_DDD_VALUE]  = m_pAgsaControl->longDurationTest()->manualStopDddValue();
        settings[SETTING_JSON_AGSA_LDT_CYCLES_NUM]      = m_pAgsaControl->longDurationTest()->manualCyclesNum();

        settings[SETTING_JSON_AGSA_LDT_TEST_STEPS] = m_pAgsaControl->longDurationTest()->testSteps();
    }

    if ( !JsonHelper::writeJsonFile( MainStatemachine::rootPath() + SETTINGS_FILENAME, settings ) )
    {
        qCritical() << "saveSettings(): could not write to settings file";
    }
    else
    {
        bSuccess = true;
    }

    m_acycNotificationSaveFailed.setActive( !bSuccess );

    return bSuccess;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SettingsSerializer::loadSettings()
{
    bool bSuccess = false;

    QJsonObject jsonFile;
    if ( !JsonHelper::readJsonFile( MainStatemachine::rootPath() + SETTINGS_FILENAME, jsonFile ) )
    {
        qWarning() << "SettingsSerializer::loadSettings(): Could not open " << SETTINGS_FILENAME;
    }
    else
    {
        QString strVersion = JsonHelper::read( jsonFile, SETTINGS_CONFIG, QString() );

        if ( strVersion.isEmpty() )
        {
            qWarning().noquote() << QString( "" )
                + "Could not load Settings: settings version value is wrong!"
                + "\n    expected: \"" + SETTINGS_CONFIG_VERSION + "\""
                + "\n    found:    \"" + jsonFile[SETTINGS_CONFIG].toString() + "\"";
        }
        else
        {
            setGrinderName( JsonHelper::read( jsonFile, SETTING_JSON_GRINDER_NAME, c_pszGrinderName ) );
            setTimeModeIsEnabled( JsonHelper::read( jsonFile, SETTING_JSON_TIME_MODE_IS_ENABLED, c_bTimeModeIsEnabled ) );
            setLibraryModeIsEnabled( JsonHelper::read( jsonFile, SETTING_JSON_LIBRARY_MODE_IS_ENABLED, c_bLibraryModeIsEnabled ) );
            setAgsaEnabled( JsonHelper::read( jsonFile, SETTING_JSON_AGSA_ENABLED, c_bAgsaEnabled ) );
            setAgsaStartDelaySec( JsonHelper::read( jsonFile, SETTING_JSON_AGSA_START_DELAY_SEC, c_nAgsaStartDelaySec ) );
            setDisplayBrightness( JsonHelper::read( jsonFile, SETTING_JSON_DISPLAY_BRIGHTNESS, c_nDisplayBrightness ) );
            setStdbyTimeIndex( JsonHelper::read( jsonFile, SETTING_JSON_STDBY_TIME_INDEX, c_nStdbyTimeIndex ) );
            setSettingsModeExitTimeIndex( JsonHelper::read( jsonFile, SETTING_JSON_SETTINGS_MODE_EXIT_TIME_INDEX, c_nSettingsModeExitTimeIndex ) );
            setDdd1CalibrationAngle10thDeg( JsonHelper::read( jsonFile, SETTING_JSON_DDD1_CALIBRATION_ANGLE_DEG, c_nDdd1CalibrationAngleDeg ) );
            setDdd2CalibrationAngle10thDeg( JsonHelper::read( jsonFile, SETTING_JSON_DDD2_CALIBRATION_ANGLE_DEG, c_nDdd2CalibrationAngleDeg ) );
            setStartOfWeekSunday( JsonHelper::read( jsonFile, SETTING_JSON_START_OF_WEEK_SUN_NOT_MON, c_bStartOfWeekSun ) );
            setStatisticsPeriodIndex( JsonHelper::read( jsonFile, SETTING_JSON_STATISTICS_PERIOD_INDEX, c_nStatisticsPeriodIndex ) );
            setDiscUsageMaxHours( JsonHelper::read( jsonFile, SETTING_JSON_DISC_USAGE_MAX_HOURS, c_nDiscUsageMaxHours ) );
            setShowTemp( JsonHelper::read( jsonFile, SETTING_JSON_SHOW_TEMP, c_bShowTemp ) );
            setShowTempWarning( JsonHelper::read( jsonFile, SETTING_JSON_SHOW_TEMP_WARNING, c_bShowTempWarning ) );
            setShowDosingTime( JsonHelper::read( jsonFile, SETTING_JSON_SHOW_DOSING_TIME, c_bShowDosingTime ) );
            setShowSpotLight( JsonHelper::read( jsonFile, SETTING_JSON_SHOW_SPOTLIGHT, c_bShowSpotLight ) );
            setTempUnit( static_cast<SettingsSerializer::TempUnit>( JsonHelper::read( jsonFile, SETTING_JSON_TEMP_UNIT, static_cast<int>( c_nTempUnit ) ) ) );
            setTempThresValueX100( JsonHelper::read( jsonFile, SETTING_JSON_TEMP_THRESHOLD_CELSIUS_X100, c_nTempThresholdValueX100 ) );

#ifdef DEV_BUILD
            QByteArray array = qgetenv( "EKX_USER_LOGIN" );
#else
            QByteArray array;
#endif

            if ( array.isEmpty() )
            {
                setCurrentLoggedInUserRole( static_cast<UserRole>( JsonHelper::read( jsonFile, SETTING_JSON_USER_AUTOLOGIN_AS, static_cast<int>( c_nCurrentLoggedInUserRole ) ) ) );
                setUserAutoLogoutEnable( JsonHelper::read( jsonFile, SETTING_JSON_USER_AUTOLOGOUT_ENABLE, c_bUserAutoLogoutEnable ) );
            }
            else
            {
                qCritical() << "Read EKX_USER_LOGIN" << array.toInt();
                setCurrentLoggedInUserRole( static_cast<UserRole>( array.toInt() ) );
                setUserAutoLogoutEnable( false );
            }

            setUserPinForOwner( JsonHelper::read( jsonFile, SETTING_JSON_USER_PIN_FOR_OWNER, c_pszUserPinForOwner ) );
            setUserPinForTechnician( JsonHelper::read( jsonFile, SETTING_JSON_USER_PIN_FOR_TECHNICIAN, c_pszUserPinForTechnician ) );

            setWifiMode( static_cast<WifiMode>( JsonHelper::read( jsonFile, SETTING_JSON_WIFI_MODE, static_cast<int>( c_nWifiMode ) ) ) );
            setWifiApPassword( JsonHelper::read( jsonFile, SETTING_JSON_WIFI_AP_PASSWORD, c_pszWifiApPassword ) );
            setWifiApSsid( JsonHelper::read( jsonFile, SETTING_JSON_WIFI_AP_SSID, c_pszWifiApSsid ) );
            applyWifiSettingsToDriver();

            setMcuMaxMotorSpeed( JsonHelper::read( jsonFile, SETTING_JSON_MCU_MAX_MOTOR_SPEED, static_cast<int>( c_u32DefMcuMaxMotorSpeed ) ) );
            setMcuNominalMotorSpeed( JsonHelper::read( jsonFile, SETTING_JSON_MCU_NOMINAL_MOTOR_SPEED, static_cast<int>( c_u32DefMcuNominalMotorSpeed ) ) );
            setMcuAccelerationTime( JsonHelper::read( jsonFile, SETTING_JSON_MCU_ACCELERATION_TIME, static_cast<int>( c_u32DefMcuAccelerationTime ) ) );
            setMcuDecelerationTime( JsonHelper::read( jsonFile, SETTING_JSON_MCU_DECELERATION_TIME, static_cast<int>( c_u32DefMcuDecelerationTime ) ) );
            emit mcuConfigChanged();

            setPduInstalled( JsonHelper::read( jsonFile, SETTING_JSON_PDU_INSTALLED, c_bPduInstalled ) );

            setSystemSetupDone( JsonHelper::read( jsonFile, SETTING_JSON_SYSTEM_SETUP_DONE, c_bSystemSetupDone ) );
            setInitialSetupDone( JsonHelper::read( jsonFile, SETTING_JSON_INITIAL_SETUP_DONE, c_bInitialSetupDone ) );
            setPduAcknowledged( JsonHelper::read( jsonFile, SETTING_JSON_PDU_ACKNOWLEDGED, c_bPduAcknowledged ) );

            if ( m_pPduDcHallMotorDriver )
            {
                m_pPduDcHallMotorDriver->setSteps( JsonHelper::read( jsonFile, SETTING_JSON_PDU_DC_HALL_STEPS, 4000 ) );
                m_pPduDcHallMotorDriver->setPwmDuty10th( JsonHelper::read( jsonFile, SETTING_JSON_PDU_DC_HALL_PWM_DUTY_10TH, 500 ) );
                m_pPduDcHallMotorDriver->setPwmFrequency( JsonHelper::read( jsonFile, SETTING_JSON_PDU_DC_HALL_PWM_FREQ, 5000 ) );
            }

            if ( m_pAgsaControl )
            {
                m_pAgsaControl->setSteps( JsonHelper::read( jsonFile, SETTING_JSON_AGSA_STEPPER_STEPS, 1000 ) );
                m_pAgsaControl->setRunFrequency( JsonHelper::read( jsonFile, SETTING_JSON_AGSA_STEPPER_RUN_FREQ, 10000 ) );
                m_pAgsaControl->setApproachFrequency( JsonHelper::read( jsonFile, SETTING_JSON_AGSA_STEPPER_APPROACH_FREQ, 300 ) );
                m_pAgsaControl->setTestLiveMode( JsonHelper::read( jsonFile, SETTING_JSON_AGSA_TEST_LIVE_MODE, true ) );
                m_pAgsaControl->setTestBlockageDetect( JsonHelper::read( jsonFile, SETTING_JSON_AGSA_TEST_BLOCKAGE_DETECT, true ) );

                m_pAgsaControl->longDurationTest()->setManualStartDddValue( JsonHelper::read( jsonFile, SETTING_JSON_AGSA_LDT_START_DDD_VALUE, 5 ) );
                m_pAgsaControl->longDurationTest()->setManualStopDddValue( JsonHelper::read( jsonFile, SETTING_JSON_AGSA_LDT_STOP_DDD_VALUE, 795 ) );
                m_pAgsaControl->longDurationTest()->setManualCyclesNum( JsonHelper::read( jsonFile, SETTING_JSON_AGSA_LDT_CYCLES_NUM, 100 ) );

                m_pAgsaControl->longDurationTest()->setTestSteps( JsonHelper::read( jsonFile, SETTING_JSON_AGSA_LDT_TEST_STEPS, 1000 ) );
            }

            bSuccess = true;
            qInfo() << "SettingsSerializer::loadSettings() successful";
        }
    }

    m_acycNotificationLoadFailed.setActive( !bSuccess );

    return bSuccess;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsSerializer::restoreToFactorySettings()
{
    setDefaultSettings();
    saveSettings();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsSerializer::checkAndConvertSettingConfig()
{
    QString currentSettingConfigVersion = "";

    QFile file( MainStatemachine::rootPath() + SETTINGS_FILENAME );
    if ( file.open( QIODevice::ReadOnly ) )
    {
        QByteArray ba = file.readAll();
        file.close();

        QJsonDocument doc = QJsonDocument::fromJson( ba );

        if ( doc.isObject() )
        {
            QJsonObject settingsObj = doc.object();

            currentSettingConfigVersion = settingsObj[SETTINGS_CONFIG].toString();
        }
    }

    if ( currentSettingConfigVersion != SETTINGS_CONFIG )
    {
        restoreToFactorySettings();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

WifiDriver * SettingsSerializer::wifiDriver() const
{
    return m_pWifiDriver;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SettingsSerializer::updateStandardGrinderData()
{
    bool bSave = false;
    qInfo() << "updateStandardGrinderData()";

    if ( m_pDeviceInfoCollector )
    {
        if ( grinderName().isEmpty() )
        {
            // sets also WiFi AP SSID
            setGrinderName( QString( "EK-Omnia-%1" ).arg( m_pDeviceInfoCollector->ekxUiSerialNo() ) );
            bSave = true;
        }

        if ( wifiApPassword().isEmpty() )
        {
            setWifiApPassword( QString( "king%1" ).arg( m_pDeviceInfoCollector->ekxUiSerialNo().right( 4 ) ) );
            bSave = true;
        }
    }

    return bSave;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsSerializer::setMachineDate( const QString & strDate )
{
    QString strCurrentDate = QDate::currentDate().toString( "dd.MM.yyyy" );

#ifdef TARGETBUILD
    QString dateTimeString = QString( "date -s \"%1 $(date +%H:%M:%S)\"" ).arg( strDate );

    int systemDateTimeStatus = system( dateTimeString.toStdString().c_str() );

    if ( systemDateTimeStatus == -1 )
    {
        qCritical() << "SettingsSerializer::setMachineDate() Failed to change date time";
    }
    else
    {
        systemDateTimeStatus = system( "hwclock --systohc" );
    }
#endif

    m_pMainStatemachine->sqliteInterface()->addEvent( EkxSqliteTypes::SqliteEventType_CHANGE_DATE,
                                                      "",
                                                      EkxSqliteTypes::SqliteNotificationType_UNKNOWN,
                                                      QString( "from %1 to %2" ).arg( strCurrentDate, strDate ) );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsSerializer::setMachineTime( const QString & strTime )
{
    QString strCurrentTime = QTime::currentTime().toString( "HH:mm" );

#ifdef TARGETBUILD
    QString dateTimeString = QString( "date +%T -s '%1'" ).arg( strTime );

    int systemDateTimeStatus= system( dateTimeString.toStdString().c_str() );

    if ( systemDateTimeStatus == -1 )
    {
        qCritical() << "SettingsSerializer::setMachineTime() Failed to change date time";
    }
    else
    {
        systemDateTimeStatus = system( "hwclock --systohc" );
    }
#endif

    m_pMainStatemachine->sqliteInterface()->addEvent( EkxSqliteTypes::SqliteEventType_CHANGE_TIME,
                                                      "",
                                                      EkxSqliteTypes::SqliteNotificationType_UNKNOWN,
                                                      QString( "from %1 to %2" ).arg( strCurrentTime, strTime ) );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsSerializer::confirmSystemSetup()
{
    Q_ASSERT( !m_bSystemSetupDone );

    m_pMainStatemachine->sqliteInterface()->addEvent( EkxSqliteTypes::SqliteEventType_SYSTEM_SETUP_CONFIRMED );

    setSystemSetupDone( true );
    saveSettings();

#ifdef TARGETBUILD
    sync();
#endif
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SettingsSerializer::isStartOfWeekSun() const
{
    return m_bStartOfWeekSunday;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsSerializer::setStartOfWeekSunday( const bool bSunday )
{
    if ( m_bStartOfWeekSunday != bSunday )
    {
        m_bStartOfWeekSunday = bSunday;

        emit startOfWeekSundayChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

int SettingsSerializer::statisticsPeriodIndex() const
{
    return m_nStatisticsPeriodIndex;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsSerializer::setStatisticsPeriodIndex( const int nIndex )
{
    if ( m_nStatisticsPeriodIndex != nIndex )
    {
        m_nStatisticsPeriodIndex = nIndex;
        emit statisticsPeriodIndexChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

int SettingsSerializer::discUsageMaxHours()
{
    return m_nDiscUsageMaxHours;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsSerializer::setDiscUsageMaxHours( const int nMaxHours )
{
    if ( nMaxHours > 0 )
    {
        if ( m_nDiscUsageMaxHours != nMaxHours )
        {
            m_nDiscUsageMaxHours = nMaxHours;

            emit discUsageMaxHoursChanged( m_nDiscUsageMaxHours );
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsSerializer::userAutoLogout()
{
    if ( isUserAutoLogoutEnabled() )
    {
        setCurrentLoggedInUserRole( SettingsSerializer::UserRole::NoRole );
        saveSettings();
    }
    else
    {
#ifndef DEV_BUILD
        if ( currentLoggedInUserRole() == SettingsSerializer::UserRole::Technician )
        {
            setCurrentLoggedInUserRole( SettingsSerializer::UserRole::NoRole );
            saveSettings();
        }
#endif
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

QString SettingsSerializer::tempToString( int tempX100 ) const
{
    int temp;
    tempX100 = ( ( tempX100 + (int)( 0.5 * 100 ) ) / 100 ) * 100; // round to 1 Unit
    temp     = tempX100 / 100;
    if ( m_nTempUnit == TempUnit::TEMPUNIT_CELSIUS )
    {
        return QString::asprintf( "%2d°C", temp );
    }
    else if ( m_nTempUnit == TempUnit::TEMPUNIT_FAHRENHEIT )
    {
        return QString::asprintf( "%2d°F", temp );
    }
    else
    {
        return QString::asprintf( "%2d ?", temp );
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

int SettingsSerializer::fahrenheitToCelsius( const int nTempF )
{
    return ( nTempF - 32 ) * ( 5.0 / 9.0 );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

int SettingsSerializer::celsiusToFahrenheit( const int nTempC )
{
    return ( nTempC * ( 9.0 / 5.0 ) ) + 32;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

int SettingsSerializer::fahrenheitX100ToCelsiusX100( const int nTempFahrenheitX100 )
{
    return ( nTempFahrenheitX100 - ( 32 * 100 ) ) * ( 5.0 / 9.0 );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

int SettingsSerializer::celsiusX100ToFahrenheitX100( int tempCelsiusX100 )
{
    return ( tempCelsiusX100 * ( 9.0 / 5.0 ) ) + ( 32 * 100 );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

int SettingsSerializer::tempThresLowerLimit( void )
{
    int lowerLimit;

    if ( m_nTempUnit == TempUnit::TEMPUNIT_CELSIUS )
    {
        lowerLimit = c_nTempThresLowerLimitCelsiusX100 / 100;
    }
    else if ( m_nTempUnit == TempUnit::TEMPUNIT_FAHRENHEIT )
    {
        lowerLimit =  celsiusX100ToFahrenheitX100( c_nTempThresLowerLimitCelsiusX100 ) / 100;
    }
    else
    {
        qCritical() << "SettingsSerializer::tempThresLowerLimit(): UNKNOWN TEMP UNIT " << static_cast<int>( m_nTempUnit ) << "\n  Assuming Celsius";
        lowerLimit = c_nTempThresLowerLimitCelsiusX100 / 100;
    }

    return lowerLimit;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

int SettingsSerializer::tempThresUpperLimit( void )
{
    int upperLimit;

    if ( m_nTempUnit == TempUnit::TEMPUNIT_CELSIUS )
    {
        upperLimit = c_nTempThresUpperLimitCelsiusX100 / 100;
    }
    else if ( m_nTempUnit == TempUnit::TEMPUNIT_FAHRENHEIT )
    {
        upperLimit = celsiusX100ToFahrenheitX100( c_nTempThresUpperLimitCelsiusX100 ) / 100;
    }
    else
    {
        qWarning() << "SettingsSerializer::tempThresUpperLimit(): UNKNOWN TEMP UNIT " << static_cast<int>( m_nTempUnit ) << "\n  Assuming Celsius";
        upperLimit = c_nTempThresUpperLimitCelsiusX100 / 100;
    }

    return upperLimit;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

