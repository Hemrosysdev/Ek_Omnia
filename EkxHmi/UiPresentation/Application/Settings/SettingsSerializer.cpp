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

#include "SettingsSerializer.h"
#include "EkxGlobals.h"

#include <QtQml>

#include "DeviceInfoCollector.h"
#include "WifiDriver.h"
#include "MainStatemachine.h"
#include "SqliteInterface.h"

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#define SETTINGS_FILENAME          "/config/EkxSettings.json"
#define SETTINGS_CONFIG            "Version"
#define SETTINGS_CONFIG_VERSION    "01.07"

#define SETTING_JSON_GRINDER_NAME                "GrinderName"
#define SETTING_JSON_TIME_MODE_IS_ENABLED        "TimeModeIsEnabled"
#define SETTING_JSON_LIBRARY_MODE_IS_ENABLED     "LibraryModeIsEnabled"
#define SETTING_JSON_DISPLAY_BRIGHTNESS_PERCENT  "DisplayBrightnessPercent"
#define SETTING_JSON_STDBY_TIME_INDEX            "StandbyTimeIndex"
#define SETTING_JSON_DDD_CALIBRATION_ANGLE_DEG   "DddCalibrationAngleDeg"
#define SETTING_JSON_START_OF_WEEK_SUN_NOT_MON   "StartOfWeekIsSunNotMon"
#define SETTING_JSON_STATISTICS_PERIOD_INDEX     "StatisticsPeriodIndex"
#define SETTING_JSON_DISC_USAGE_MAX_HOURS        "DiscUsageMaxHours"
#define SETTING_JSON_SHOW_TEMP                   "ShowTemp"
#define SETTING_JSON_SHOW_TEMP_WARNING           "ShowTempWarning"
#define SETTING_JSON_SHOW_DDD                    "ShowDdd"
#define SETTING_JSON_SHOW_WARNINGS               "ShowWarnings"
#define SETTING_JSON_SHOW_SPOTLIGHT              "ShowSpotlight"
#define SETTING_JSON_TEMP_UNIT                   "TempUnit"
#define SETTING_JSON_TEMP_THRESHOLD_CELSIUS_X100 "TempThresholdCelsiusX100"
#define SETTING_JSON_USER_AUTOLOGIN_AS           "UserAutoLoginAs"
#define SETTING_JSON_USER_AUTOLOGOUT_ENABLE      "UserAutoLogoutEnable"
#define SETTING_JSON_USER_PIN_FOR_OWNER          "UserPinForOwner"
#define SETTING_JSON_USER_PIN_FOR_TECHNICIAN     "UserPinForTechnician"
#define SETTING_JSON_WIFI_ENABLE                 "WifiEnable"
#define SETTING_JSON_WIFI_ENC                    "WifiEnc"
#define SETTING_JSON_WIFI_MODE                   "WifiMode"
#define SETTING_JSON_WIFI_PW                     "WifiPw"
#define SETTING_JSON_WIFI_AP_SSID                "WifiApSSID"

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

//int SettingsSerializer::LoggedInUser_QTypeId    = qRegisterMetaType<SettingsSerializer::userRole>( "SettingsSerializer::userRole" );
int SettingsSerializer::QmlSettingsSerializerId = qmlRegisterUncreatableType<SettingsSerializer>( "SettingsSerializerEnum", 1, 0, "SettingsSerializerEnum", "something went wrong" );

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

QString SettingsSerializer::tempToString(int tempX100) const
{
    int temp;
    tempX100 = ((tempX100+(int)(0.5*100))/100)*100; // round to 1 Unit
    temp = tempX100/100;
    if(m_settings.tempUnit == TEMPUNIT_CELSIUS)
        return( QString::asprintf("%2d°C", temp) );
    else if(m_settings.tempUnit == TEMPUNIT_FAHRENHEIT)
        return( QString::asprintf("%2d°F", temp) );
    else
        return( QString::asprintf("%2d ?", temp) );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

int SettingsSerializer::fahrenheitToCelsius(int tempF)
{
    return( (tempF-32)*(5.0/9.0) );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

int SettingsSerializer::celsiusToFahrenheit(int tempC)
{
    return( (tempC * (9.0/5.0))+32 );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

int SettingsSerializer::fahrenheitX100ToCelsiusX100(int tempFahrenheitX100)
{
    return( (tempFahrenheitX100-(32*100)) * (5.0/9.0) );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

int SettingsSerializer::celsiusX100ToFahrenheitX100(int tempCelsiusX100)
{
    return( (tempCelsiusX100*(9.0/5.0)) + (32*100) );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

int SettingsSerializer::getTempThresLowerLimit(void)
{
    int lowerLimit;

    if(m_settings.tempUnit == TEMPUNIT_CELSIUS)
        lowerLimit = m_nTempThresLowerLimitCelsiusX100/100;
    else if( m_settings.tempUnit == TEMPUNIT_FAHRENHEIT )
        lowerLimit =  celsiusX100ToFahrenheitX100(m_nTempThresLowerLimitCelsiusX100)/100;
    else
    {
        qCritical() << "SettingsSerializer::getTempThresLowerLimit(): UNKNOWN TEMP UNIT "<<m_settings.tempUnit<<"\n  Assuming Celsius";
        lowerLimit = m_nTempThresLowerLimitCelsiusX100/100;
    }

    return(lowerLimit);
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

int SettingsSerializer::getTempThresUpperLimit(void)
{
    int upperLimit;

    if(m_settings.tempUnit == TEMPUNIT_CELSIUS)
        upperLimit = m_nTempThresUpperLimitCelsiusX100/100;
    else if( m_settings.tempUnit == TEMPUNIT_FAHRENHEIT )
        upperLimit = celsiusX100ToFahrenheitX100(m_nTempThresUpperLimitCelsiusX100)/100;
    else
    {
        qWarning() << "SettingsSerializer::getTempThresUpperLimit(): UNKNOWN TEMP UNIT "<<m_settings.tempUnit<<"\n  Assuming Celsius";
        upperLimit = m_nTempThresUpperLimitCelsiusX100/100;
    }

    return(upperLimit);
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

SettingsSerializer::SettingsSerializer(QQmlEngine* enigne,
                                       MainStatemachine *pMainStatemachine)
    : QObject(pMainStatemachine)
    , m_qmlEngine( enigne )
    , m_pMainStatemachine( pMainStatemachine )
{
    MainStatemachine::ensureExistingDirectory( MainStatemachine::rootPath() + SETTINGS_FILENAME );

    m_qmlEngine->rootContext()->setContextProperty("settingsSerializer" , this);
    initSettings();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

SettingsSerializer::~SettingsSerializer()
{
    m_qmlEngine        = nullptr;
    m_wifiDriver       = nullptr;
    m_deviceInfoDriver = nullptr;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsSerializer::initSettings()
{
    if(!loadSettings())
    {
        loadFactorySettings();
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
    if( m_wifiDriver )
    {
        m_wifiDriver->setApPw( getWifiPw() );
        m_wifiDriver->setApSsid( getWifiApSsid() );
        m_wifiDriver->setIsAccesspoint(getWifiMode() == wifiMode::MODE_AP);
        m_wifiDriver->setStaPw( "" );
        m_wifiDriver->setStaSsid("");
        //ThisWifiDriver->setActive( getWifi() );
        m_wifiDriver->setWifiValid(true);
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsSerializer::loadFactorySettings()
{
    qDebug() << "SettingsSerializer::loadFactorySettings() called!";

    setGrinderName( "" );       // will be set after reading serial# info
    setTimeModeIsEnabled( true );
    setLibraryModeIsEnabled( true );
    setDisplayBrightnessPercent( 92 );
    setStdbyTimeIndex( 2 );
    setDddCalibrationAngleDeg( 0 );
    setStartOfWeekisSunNotMon( false );
    setStatisticsPeriodIndex( 0 );
    setDiscUsageMaxHours( 120 );

    setShowTemp( true );
    setShowTempWarning( true );
    setShowDdd( true );
    setShowWarnings( false );
    setShowSpotlight( false );

    setTempUnit( SettingsSerializer::tempUnit::TEMPUNIT_CELSIUS );
    setTempThresValueX100( 45*100 );

    setUserAutoLoginAs( SettingsSerializer::userRole::LOGGEDIN_STOREOWNER );
    setUserAutoLogoutEnable( false );
    setUserPinForOwner( "1234" );
    setUserPinForTechnician( "5678" );

    setWifiEnable( false );
    setWifiEnc( SettingsSerializer::ENCRYPTION_WPA2 );
    setWifiMode( SettingsSerializer::MODE_AP );
    setWifiPw( "" );            // will be set after reading serial# info
    setWifiApSsid( "" );        // will be set after reading serial# info

    qDebug() << "factory settings loaded!";
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

QString SettingsSerializer::tempUnitToString( SettingsSerializer::tempUnit tempUnit )
{
    QString tempUnitString;

    switch ( tempUnit )
    {
        case SettingsSerializer::TEMPUNIT_CELSIUS:
            tempUnitString = "°C";
        break;
        case SettingsSerializer::TEMPUNIT_FAHRENHEIT:
            tempUnitString = "°F";
        break;
        default:
            tempUnitString = "?";
    }
    return tempUnitString;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

QString SettingsSerializer::userRoleToString( SettingsSerializer::userRole loggedIn )
{
    QString loggedInString;
    switch ( loggedIn )
    {
        case SettingsSerializer::LOGGEDIN_STOREOWNER:
            loggedInString = "Fast Driver";
        break;
        case SettingsSerializer::LOGGEDIN_TECHNICIAN:
            loggedInString = "Expert Driver";
        break;
        case SettingsSerializer::LOGGEDIN_NONE:
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

QString SettingsSerializer::stdbyTimeIndexToString( int stdbyTimeIndex )
{
    QString stdbyString;
    switch ( stdbyTimeIndex )
    {
        case 0:
            stdbyString = "3m";
        break;
        case 1:
            stdbyString = "5m";
        break;
        case 2:
            stdbyString = "10m";
        break;
        case 3:
            stdbyString = "20m";
        break;
        default:
            stdbyString = "";
    }
    return stdbyString;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

int SettingsSerializer::stdbyTimeIndexToStdbyTimeMinutes( int stdbyTimeIndex )
{
    int nMinutes = 5;

    switch ( stdbyTimeIndex )
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

bool SettingsSerializer::getShowTemp() const
{
    return(m_settings.showTemp);
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsSerializer::setShowTemp( bool newShowTemp )
{
    if( m_settings.showTemp != newShowTemp )
    {
        m_settings.showTemp = newShowTemp;

        emit showTempEnableChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

SettingsSerializer::tempUnit SettingsSerializer::getTempUnit() const
{
    return( m_settings.tempUnit );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsSerializer::setTempUnit( SettingsSerializer::tempUnit tempUnit )
{
    if(m_settings.tempUnit != tempUnit)
    {
        m_settings.tempUnit = tempUnit;
        emit tempUnitChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SettingsSerializer::getShowTempWarning() const
{
    return(m_settings.showTempWarning);
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsSerializer::setShowTempWarning( bool newShowTempWarning )
{
    if( m_settings.showTempWarning != newShowTempWarning )
    {
        m_settings.showTempWarning = newShowTempWarning;

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

    if( m_settings.tempUnit==TEMPUNIT_CELSIUS )
        tempX100 = m_settings.tempThreshCelsiusX100;
    else if(m_settings.tempUnit==TEMPUNIT_FAHRENHEIT)
        tempX100 = celsiusX100ToFahrenheitX100( m_settings.tempThreshCelsiusX100 );
    else
    {
        qCritical() << "SettingsSerializer::getTempThresValue(): NOT IMPLEMENTED FOR THIS TEMP UNIT"<<m_settings.tempUnit<<"\n  Assuming Celsius";
        tempX100 = m_settings.tempThreshCelsiusX100;
    }

    return( tempX100 );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsSerializer::setTempThresValueX100( int newTemp )
{
    int newTempCelsiusX100 = m_settings.tempThreshCelsiusX100;

    if( m_settings.tempUnit==TEMPUNIT_CELSIUS )
    {
        newTemp = ((newTemp+(int)(0.5*100))/100)*100; // round to 1 Celsius
        newTempCelsiusX100 = newTemp;
    }
    else if(m_settings.tempUnit==TEMPUNIT_FAHRENHEIT)
    {
        newTemp = ((newTemp+(int)(0.5*100))/100)*100; // round to 1 Fahrenheit
        newTempCelsiusX100 = fahrenheitX100ToCelsiusX100(newTemp);
    }
    else
    {
        qCritical() << "SettingsSerializer::setTempThresValue(): NOT IMPLEMENTED FOR THIS TEMP UNIT"<<m_settings.tempUnit;
    }

    if( m_settings.tempThreshCelsiusX100!=newTempCelsiusX100 )
    {
        m_settings.tempThreshCelsiusX100 = newTempCelsiusX100;
        emit tempThresValueChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SettingsSerializer::getShowWarnings() const
{
    return m_settings.showWarnings;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsSerializer::setShowWarnings( bool newShowWarnings )
{
    if( m_settings.showWarnings != newShowWarnings )
    {
        m_settings.showWarnings = newShowWarnings;
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SettingsSerializer::getLibraryModeIsEnabled() const
{
    return m_settings.libraryModeIsEnabled;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsSerializer::setLibraryModeIsEnabled( bool enable )
{
    if( m_settings.libraryModeIsEnabled != enable )
    {
        m_settings.libraryModeIsEnabled = enable;
        emit libraryModeIsEnabledChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SettingsSerializer::getUserAutoLogoutEnable() const
{
    return m_settings.userAutoLogoutEnable;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsSerializer::setUserAutoLogoutEnable( bool autologout )
{
    if( m_settings.userAutoLogoutEnable != autologout )
    {
        m_settings.userAutoLogoutEnable = autologout;

        emit userAutoLogoutEnableChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SettingsSerializer::getTimeModeIsEnabled() const
{
    return m_settings.timeModeIsEnabled;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsSerializer::setTimeModeIsEnabled( bool enable )
{
    if( m_settings.timeModeIsEnabled != enable )
    {
        m_settings.timeModeIsEnabled = enable;

        emit timeModeIsEnabledChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

int SettingsSerializer::getDisplayBrightnessPercent() const
{
    return m_settings.displayBrightnessPercent;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsSerializer::setDisplayBrightnessPercent( int displayBrightnessPercent )
{
    if( m_settings.displayBrightnessPercent != displayBrightnessPercent )
    {
        m_settings.displayBrightnessPercent = displayBrightnessPercent;
        emit displayBrightnessPercentChanged(displayBrightnessPercent);
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SettingsSerializer::getShowDdd() const
{
    return m_settings.showDdd;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsSerializer::setShowDdd( bool newShowDdd )
{
    if( m_settings.showDdd != newShowDdd )
    {
        m_settings.showDdd = newShowDdd;

        emit showDddEnableChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

int SettingsSerializer::getDddCalibrationAngleDeg() const
{
    return m_settings.dddCalibrationAngleDeg;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsSerializer::setDddCalibrationAngleDeg(const int dddCalibrationAngle)
{
    if ( dddCalibrationAngle != m_settings.dddCalibrationAngleDeg )
    {
        m_settings.dddCalibrationAngleDeg = dddCalibrationAngle;
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SettingsSerializer::getShowSpotlight() const
{
    return m_settings.showSpotlight;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsSerializer::setShowSpotlight( bool newShowSpot )
{
    if( m_settings.showSpotlight != newShowSpot )
    {
        m_settings.showSpotlight = newShowSpot;
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

QString SettingsSerializer::getUserPinForOwner() const
{
    return m_settings.userPinForOwner;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsSerializer::setUserPinForOwner( QString newStoreOwnerPin )
{
    if( m_settings.userPinForOwner != newStoreOwnerPin )
    {
        m_settings.userPinForOwner = newStoreOwnerPin;

    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

QString SettingsSerializer::getUserPinForTechnician() const
{
    return m_settings.userPinForTechnician;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsSerializer::setUserPinForTechnician( QString newTechnicianPin )
{
    if( m_settings.userPinForTechnician != newTechnicianPin )
    {
        m_settings.userPinForTechnician = newTechnicianPin;
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

SettingsSerializer::userRole SettingsSerializer::getUserAutoLoginAs() const
{
    return m_settings.userAutoLoginAs;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsSerializer::setUserAutoLoginAs( SettingsSerializer::userRole userRole )
{
    if( m_settings.userAutoLoginAs != userRole )
    {
        switch ( m_settings.userAutoLoginAs )
        {
            case LOGGEDIN_NONE:
            {
                switch ( userRole )
                {
                    case LOGGEDIN_NONE:
                    break;

                    case LOGGEDIN_STOREOWNER:
                        m_pMainStatemachine->sqliteInterface()->addEvent( EkxSqliteTypes::SqliteEventType_OWNER_LOGIN,
                                                                          "",
                                                                          EkxSqliteTypes::SqliteErrorType_UNKNOWN,
                                                                          "" );

                    break;

                    case LOGGEDIN_TECHNICIAN:
                        m_pMainStatemachine->sqliteInterface()->addEvent( EkxSqliteTypes::SqliteEventType_TECHNICIAN_LOGIN,
                                                                          "",
                                                                          EkxSqliteTypes::SqliteErrorType_UNKNOWN,
                                                                          "" );

                    break;
                }
            }
            break;

            case LOGGEDIN_STOREOWNER:
            {
                switch ( userRole )
                {
                    case LOGGEDIN_NONE:
                        m_pMainStatemachine->sqliteInterface()->addEvent( EkxSqliteTypes::SqliteEventType_OWNER_LOGOUT,
                                                                          "",
                                                                          EkxSqliteTypes::SqliteErrorType_UNKNOWN,
                                                                          "" );

                    break;

                    case LOGGEDIN_STOREOWNER:
                    break;

                    case LOGGEDIN_TECHNICIAN:
                        m_pMainStatemachine->sqliteInterface()->addEvent( EkxSqliteTypes::SqliteEventType_OWNER_LOGOUT,
                                                                          "",
                                                                          EkxSqliteTypes::SqliteErrorType_UNKNOWN,
                                                                          "" );

                        m_pMainStatemachine->sqliteInterface()->addEvent( EkxSqliteTypes::SqliteEventType_TECHNICIAN_LOGIN,
                                                                          "",
                                                                          EkxSqliteTypes::SqliteErrorType_UNKNOWN,
                                                                          "" );

                    break;
                }
            }
            break;

            case LOGGEDIN_TECHNICIAN:
            {
                switch ( userRole )
                {
                    case LOGGEDIN_NONE:
                        m_pMainStatemachine->sqliteInterface()->addEvent( EkxSqliteTypes::SqliteEventType_TECHNICIAN_LOGOUT,
                                                                          "",
                                                                          EkxSqliteTypes::SqliteErrorType_UNKNOWN,
                                                                          "" );

                    break;

                    case LOGGEDIN_STOREOWNER:
                        m_pMainStatemachine->sqliteInterface()->addEvent( EkxSqliteTypes::SqliteEventType_TECHNICIAN_LOGOUT,
                                                                          "",
                                                                          EkxSqliteTypes::SqliteErrorType_UNKNOWN,
                                                                          "" );
                        m_pMainStatemachine->sqliteInterface()->addEvent( EkxSqliteTypes::SqliteEventType_OWNER_LOGIN,
                                                                          "",
                                                                          EkxSqliteTypes::SqliteErrorType_UNKNOWN,
                                                                          "" );

                    break;

                    case LOGGEDIN_TECHNICIAN:
                    break;
                }
            }
            break;
        }

        m_settings.userAutoLoginAs = userRole;

        emit userAutoLoginAsChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

QString SettingsSerializer::getGrinderName() const
{
    return m_settings.grinderName;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsSerializer::setGrinderName( QString strGrinderName )
{
    if( m_settings.grinderName != strGrinderName )
    {
        m_settings.grinderName = strGrinderName;

        setWifiApSsid(QString(strGrinderName));
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

int SettingsSerializer::getStdbyTimeIndex() const
{
    return m_settings.stdbyTimeIndex;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsSerializer::setStdbyTimeIndex( int newStdbyTimeIndex )
{
    if( m_settings.stdbyTimeIndex != newStdbyTimeIndex )
    {
        m_settings.stdbyTimeIndex = newStdbyTimeIndex;
        emit stdbyTimeIndexChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SettingsSerializer::getWifiEnable() const
{
    return m_settings.wifiEnable;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsSerializer::setWifiEnable( bool newWifi )
{
    if( m_settings.wifiEnable != newWifi )
    {
        m_settings.wifiEnable = newWifi;

        if( m_wifiDriver )
        {
            //ThisWifiDriver->setActive( newWifi );
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

SettingsSerializer::wifiEncryption SettingsSerializer::getWifiEnc() const
{
    return m_settings.wifiEnc;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsSerializer::setWifiEnc( SettingsSerializer::wifiEncryption newWifiEnc )
{
    if( m_settings.wifiEnc != newWifiEnc )
    {
        m_settings.wifiEnc = static_cast<SettingsSerializer::wifiEncryption>(newWifiEnc);
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

SettingsSerializer::wifiMode SettingsSerializer::getWifiMode() const
{
    return m_settings.wifiMode;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsSerializer::setWifiMode( int newMode )
{
    if( m_settings.wifiMode != newMode )
    {
        m_settings.wifiMode = static_cast<SettingsSerializer::wifiMode>(newMode);

        if( m_wifiDriver )
        {
            m_wifiDriver->setIsAccesspoint( m_settings.wifiMode == wifiMode::MODE_AP );
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

QString SettingsSerializer::getWifiPw() const
{
    return m_settings.wifiPw;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsSerializer::setWifiPw( QString newWifiPw )
{
    if( m_settings.wifiPw != newWifiPw )
    {
        m_settings.wifiPw = newWifiPw;

        if( m_wifiDriver )
        {
            m_wifiDriver->setApPw( newWifiPw );
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

QString SettingsSerializer::getWifiApSsid() const
{
    return m_settings.wifiApSsid;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsSerializer::setWifiApSsid( QString newWifiApSSID )
{
    if( m_settings.wifiApSsid != newWifiApSSID )
    {
        m_settings.wifiApSsid = newWifiApSSID;

        if( m_wifiDriver )
        {
            m_wifiDriver->setApSsid( newWifiApSSID );
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
    bool success = false;

    QFile file( MainStatemachine::rootPath() + SETTINGS_FILENAME );
    if ( file.open(QIODevice::ReadWrite) )
    {
        file.resize(0);

        QJsonObject settings;

        settings[SETTINGS_CONFIG]                              = SETTINGS_CONFIG_VERSION;

        settings[SETTING_JSON_GRINDER_NAME]                    = getGrinderName();
        settings[SETTING_JSON_TIME_MODE_IS_ENABLED]            = getTimeModeIsEnabled();
        settings[SETTING_JSON_LIBRARY_MODE_IS_ENABLED]         = getLibraryModeIsEnabled();
        settings[SETTING_JSON_DISPLAY_BRIGHTNESS_PERCENT]      = getDisplayBrightnessPercent();
        settings[SETTING_JSON_STDBY_TIME_INDEX]                = getStdbyTimeIndex();
        settings[SETTING_JSON_DDD_CALIBRATION_ANGLE_DEG]       = getDddCalibrationAngleDeg();
        settings[SETTING_JSON_START_OF_WEEK_SUN_NOT_MON]       = startOfWeekisSunNotMon();
        settings[SETTING_JSON_STATISTICS_PERIOD_INDEX]         = statisticsPeriodIndex();
        settings[SETTING_JSON_DISC_USAGE_MAX_HOURS]            = getDiscUsageMaxHours();

        settings[SETTING_JSON_SHOW_TEMP]                       = getShowTemp();
        settings[SETTING_JSON_SHOW_TEMP_WARNING]               = getShowTempWarning();
        settings[SETTING_JSON_SHOW_DDD]                        = getShowDdd();
        settings[SETTING_JSON_SHOW_WARNINGS]                   = getShowWarnings();
        settings[SETTING_JSON_SHOW_SPOTLIGHT]                  = getShowSpotlight();

        settings[SETTING_JSON_TEMP_UNIT]                       = static_cast<int>(getTempUnit());
        settings[SETTING_JSON_TEMP_THRESHOLD_CELSIUS_X100]     = m_settings.tempThreshCelsiusX100; // always in celsiusX100

        settings[SETTING_JSON_USER_AUTOLOGIN_AS]               = static_cast<int>(getUserAutoLoginAs());
        settings[SETTING_JSON_USER_AUTOLOGOUT_ENABLE]          = getUserAutoLogoutEnable();
        settings[SETTING_JSON_USER_PIN_FOR_OWNER]              = getUserPinForOwner();
        settings[SETTING_JSON_USER_PIN_FOR_TECHNICIAN]         = getUserPinForTechnician();

        settings[SETTING_JSON_WIFI_ENABLE]                     = getWifiEnable();
        settings[SETTING_JSON_WIFI_ENC]                        = getWifiEnc();
        settings[SETTING_JSON_WIFI_MODE]                       = getWifiMode();
        settings[SETTING_JSON_WIFI_PW]                         = getWifiPw();
        settings[SETTING_JSON_WIFI_AP_SSID]                    = getWifiApSsid();

        QJsonDocument jsonDoc(settings);
        file.write( jsonDoc.toJson() );
        file.flush();
        file.close();

        success = true;
    }
    else
    {
        qCritical() << "saveSettings(): could not write to settings file";
    }

    return success;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SettingsSerializer::loadSettings()
{
    //qDebug() << "SettingsSerializer::loadSettings() called!";
    bool success = false;

    QFile file( MainStatemachine::rootPath() + SETTINGS_FILENAME );
    if ( file.open(QIODevice::ReadOnly) )
    {
        QByteArray ba = file.readAll();
        file.close();

        QJsonDocument doc = QJsonDocument::fromJson(ba);

        if( doc.isObject() )
        {
            QJsonObject settingsObj = doc.object();

            if(settingsObj[SETTINGS_CONFIG] != QJsonValue::Undefined)
            {
                if(settingsObj[SETTINGS_CONFIG].toString() == SETTINGS_CONFIG_VERSION)
                {
                    if(settingsObj[SETTING_JSON_GRINDER_NAME] != QJsonValue::Undefined)
                        setGrinderName( settingsObj[SETTING_JSON_GRINDER_NAME].toString() );

                    if(settingsObj[SETTING_JSON_TIME_MODE_IS_ENABLED] != QJsonValue::Undefined)
                        setTimeModeIsEnabled( settingsObj[SETTING_JSON_TIME_MODE_IS_ENABLED].toBool() );

                    if(settingsObj[SETTING_JSON_LIBRARY_MODE_IS_ENABLED] != QJsonValue::Undefined)
                        setLibraryModeIsEnabled( settingsObj[SETTING_JSON_LIBRARY_MODE_IS_ENABLED].toBool() );

                    if(settingsObj[SETTING_JSON_DISPLAY_BRIGHTNESS_PERCENT] != QJsonValue::Undefined)
                        setDisplayBrightnessPercent( settingsObj[SETTING_JSON_DISPLAY_BRIGHTNESS_PERCENT].toInt() );

                    if(settingsObj[SETTING_JSON_STDBY_TIME_INDEX] != QJsonValue::Undefined)
                        setStdbyTimeIndex( settingsObj[SETTING_JSON_STDBY_TIME_INDEX].toInt() );

                    if(settingsObj[SETTING_JSON_DDD_CALIBRATION_ANGLE_DEG] != QJsonValue::Undefined)
                        setDddCalibrationAngleDeg( settingsObj[SETTING_JSON_DDD_CALIBRATION_ANGLE_DEG].toInt() );

                    if(settingsObj[SETTING_JSON_START_OF_WEEK_SUN_NOT_MON] != QJsonValue::Undefined)
                        setStartOfWeekisSunNotMon( settingsObj[SETTING_JSON_START_OF_WEEK_SUN_NOT_MON].toBool() );

                    if(settingsObj[SETTING_JSON_STATISTICS_PERIOD_INDEX] != QJsonValue::Undefined)
                        setStatisticsPeriodIndex( settingsObj[SETTING_JSON_STATISTICS_PERIOD_INDEX].toInt() );

                    if(settingsObj[SETTING_JSON_DISC_USAGE_MAX_HOURS] != QJsonValue::Undefined)
                        setDiscUsageMaxHours( settingsObj[SETTING_JSON_DISC_USAGE_MAX_HOURS].toInt() );

                    if(settingsObj[SETTING_JSON_SHOW_TEMP] != QJsonValue::Undefined)
                        setShowTemp( settingsObj[SETTING_JSON_SHOW_TEMP].toBool() );

                    if(settingsObj[SETTING_JSON_SHOW_TEMP_WARNING] != QJsonValue::Undefined)
                        setShowTempWarning( settingsObj[SETTING_JSON_SHOW_TEMP_WARNING].toBool() );

                    if(settingsObj[SETTING_JSON_SHOW_DDD] != QJsonValue::Undefined)
                        setShowDdd( settingsObj[SETTING_JSON_SHOW_DDD].toBool() );

                    if(settingsObj[SETTING_JSON_SHOW_WARNINGS] != QJsonValue::Undefined)
                        setShowWarnings( settingsObj[SETTING_JSON_SHOW_WARNINGS].toBool() );

                    if(settingsObj[SETTING_JSON_SHOW_SPOTLIGHT] != QJsonValue::Undefined)
                        setShowSpotlight( settingsObj[SETTING_JSON_SHOW_SPOTLIGHT].toBool() );

                    if(settingsObj[SETTING_JSON_TEMP_UNIT] != QJsonValue::Undefined)
                        setTempUnit( static_cast<SettingsSerializer::tempUnit>(settingsObj[SETTING_JSON_TEMP_UNIT].toInt()) );

                    if(settingsObj[SETTING_JSON_TEMP_THRESHOLD_CELSIUS_X100] != QJsonValue::Undefined)
                        setTempThresValueX100( settingsObj[SETTING_JSON_TEMP_THRESHOLD_CELSIUS_X100].toInt() );

                    if(settingsObj[SETTING_JSON_USER_AUTOLOGIN_AS] != QJsonValue::Undefined)
                        setUserAutoLoginAs( static_cast<SettingsSerializer::userRole>(settingsObj[SETTING_JSON_USER_AUTOLOGIN_AS].toInt()) );

                    if(settingsObj[SETTING_JSON_USER_AUTOLOGOUT_ENABLE] != QJsonValue::Undefined)
                        setUserAutoLogoutEnable( settingsObj[SETTING_JSON_USER_AUTOLOGOUT_ENABLE].toBool() );

                    if(settingsObj[SETTING_JSON_USER_PIN_FOR_OWNER] != QJsonValue::Undefined)
                        setUserPinForOwner( settingsObj[SETTING_JSON_USER_PIN_FOR_OWNER].toString() );

                    if(settingsObj[SETTING_JSON_USER_PIN_FOR_TECHNICIAN] != QJsonValue::Undefined)
                        setUserPinForTechnician( settingsObj[SETTING_JSON_USER_PIN_FOR_TECHNICIAN].toString() );

                    if(settingsObj[SETTING_JSON_WIFI_ENABLE] != QJsonValue::Undefined)
                        setWifiEnable( settingsObj[SETTING_JSON_WIFI_ENABLE].toBool() );

                    if(settingsObj[SETTING_JSON_WIFI_ENC] != QJsonValue::Undefined)
                        setWifiEnc( static_cast<SettingsSerializer::wifiEncryption>(settingsObj[SETTING_JSON_WIFI_ENC].toInt()) );

                    if(settingsObj[SETTING_JSON_WIFI_MODE] != QJsonValue::Undefined)
                        setWifiMode( static_cast<SettingsSerializer::wifiMode>(settingsObj[SETTING_JSON_WIFI_MODE].toInt()) );

                    if(settingsObj[SETTING_JSON_WIFI_PW] != QJsonValue::Undefined)
                        setWifiPw( settingsObj[SETTING_JSON_WIFI_PW].toString() );

                    if(settingsObj[SETTING_JSON_WIFI_AP_SSID] != QJsonValue::Undefined)
                        setWifiApSsid( settingsObj[SETTING_JSON_WIFI_AP_SSID].toString() );

                    success = true;
                    qDebug() << "SettingsSerializer::loadSettings() successful";
                }
                else
                {
                    qWarning().noquote() << QString("")
                                          + "Could not load Settings: settings version value is wrong!"
                                          + "\n    expected: \"" + SETTINGS_CONFIG_VERSION + "\""
                                          + "\n    found:    \"" + settingsObj[SETTINGS_CONFIG].toString() + "\"";
                }
            }
            else
            {
                qWarning() << "Could not load Settings: settings version field is missing!";
            }
        }
        else
        {
            qWarning() << "Could not parse " << file.fileName();
        }
    }
    else
    {
        qWarning() << "Could not open " << file.fileName();
    }

    return success;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsSerializer::restoreToFactorySettings()
{
    QString ekxSerialNumber = m_deviceInfoDriver->ekxUiSerialNo();

    loadFactorySettings();
    saveSettings();

    setGrinderName(QString("Ultra-" + ekxSerialNumber));
    setWifiApSsid(QString( getGrinderName() ));
    setWifiPw((QString("king" + ekxSerialNumber.right(4))));
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
    if ( file.open(QIODevice::ReadOnly) )
    {
        QByteArray ba = file.readAll();
        file.close();

        QJsonDocument doc = QJsonDocument::fromJson(ba);

        if( doc.isObject() )
        {
            QJsonObject settingsObj = doc.object();

            currentSettingConfigVersion = settingsObj[SETTINGS_CONFIG].toString();
        }
    }

    if( currentSettingConfigVersion != SETTINGS_CONFIG )
    {
        restoreToFactorySettings();
    }

}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsSerializer::setWifiDriver( WifiDriver* wifiDriver )
{
    if( m_wifiDriver != wifiDriver )
    {
        m_wifiDriver = wifiDriver;
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

WifiDriver* SettingsSerializer::getWifiDriver( ) const
{
    return( m_wifiDriver );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsSerializer::setDeviceInfoDriver( DeviceInfoCollector* dic  )
{
    if( m_deviceInfoDriver == nullptr )
    {
        m_deviceInfoDriver = dic;

        if( getGrinderName().isEmpty() )
        {
            setGrinderName(QString("Ultra-" + m_deviceInfoDriver->ekxUiSerialNo()));
            setWifiApSsid(QString( getGrinderName() ));
            saveSettings();
        }
        if( getWifiPw().isEmpty() )
        {
            setWifiPw((QString("king" + m_deviceInfoDriver->ekxUiSerialNo().right(4))));
        }

    }
    if( dic == nullptr )
    {
        m_deviceInfoDriver = dic;
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsSerializer::setMachineDate( QString newDate )
{
    QString strCurrentDate = QDate::currentDate().toString( "dd.MM.yyyy" );
    QString dateTimeString ("date -s ");
    dateTimeString.append(newDate);
    dateTimeString.append(" $(date +%H:%M:%S)\"");

#ifdef TARGETBUILD
    QString hwClockString("hwclock --systohc");

    int systemDateTimeStatus = system(dateTimeString.toStdString().c_str());

    if (systemDateTimeStatus == -1)
    {
        qCritical() << "Failed to change date time";
    }
    else
    {
        systemDateTimeStatus = system(hwClockString.toStdString().c_str());
    }
#endif

    m_pMainStatemachine->sqliteInterface()->addEvent( EkxSqliteTypes::SqliteEventType_CHANGE_DATE,
                                                      "",
                                                      EkxSqliteTypes::SqliteErrorType_UNKNOWN,
                                                      strCurrentDate + "->" + newDate );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsSerializer::setMachineTime( QString newTime )
{
    QString strCurrentTime = QTime::currentTime().toString( "HH:mm" );
    QString dateTimeString ("date +%T -s ");
    dateTimeString.append( newTime );

#ifdef TARGETBUILD
    QString hwClockString("hwclock --systohc");

    int systemDateTimeStatus= system(dateTimeString.toStdString().c_str());

    if (systemDateTimeStatus == -1)
    {
        qCritical() << "Failed to change date time";
    }
    else
    {
        systemDateTimeStatus = system(hwClockString.toStdString().c_str());
    }
#endif

    m_pMainStatemachine->sqliteInterface()->addEvent( EkxSqliteTypes::SqliteEventType_CHANGE_TIME,
                                                      "",
                                                      EkxSqliteTypes::SqliteErrorType_UNKNOWN,
                                                      strCurrentTime + "->" + newTime );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SettingsSerializer::startOfWeekisSunNotMon() const
{
    return(m_settings.startOfWeekisSunNotMon);
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsSerializer::setStartOfWeekisSunNotMon(bool sundayNotMonday)
{
    if(m_settings.startOfWeekisSunNotMon != sundayNotMonday)
    {
        m_settings.startOfWeekisSunNotMon = sundayNotMonday;

        emit startOfWeekisSunNotMonChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

int SettingsSerializer::statisticsPeriodIndex() const
{
    return m_settings.statisticsPeriodIndex;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsSerializer::setStatisticsPeriodIndex(const int nIndex)
{
    if ( m_settings.statisticsPeriodIndex != nIndex )
    {
        m_settings.statisticsPeriodIndex = nIndex;
        emit statisticsPeriodIndexChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

int SettingsSerializer::getDiscUsageMaxHours()
{
    return( m_settings.discUsageMaxHours );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsSerializer::setDiscUsageMaxHours(int maxHours)
{
    if(maxHours>0)
    {
        if( m_settings.discUsageMaxHours != maxHours )
        {
            m_settings.discUsageMaxHours = maxHours;

            emit discUsageMaxHoursChanged(m_settings.discUsageMaxHours);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsSerializer::userAutoLogout()
{
    if( getUserAutoLogoutEnable() )
    {
        setUserAutoLoginAs( SettingsSerializer::LOGGEDIN_NONE );
        saveSettings();
    }
    else
    {
        if( getUserAutoLoginAs() == SettingsSerializer::LOGGEDIN_TECHNICIAN )
        {
            setUserAutoLoginAs( SettingsSerializer::LOGGEDIN_NONE );
            saveSettings();
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////


