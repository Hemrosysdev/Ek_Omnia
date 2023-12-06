///////////////////////////////////////////////////////////////////////////////
///
/// @file SettingsSerializer.h
///
/// @brief Header file of class SettingsSerializer
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

#ifndef SettingsSerializer_h
#define SettingsSerializer_h

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#include <QObject>
#include <QString>

#include "AcyclicNotification.h"

class DeviceInfoCollector;
class WifiDriver;
class MainStatemachine;
class EspDcHallMotorDriver;
class AgsaControl;

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

class SettingsSerializer : public QObject
{
    Q_OBJECT

    Q_ENUMS( TempUnit )
    Q_ENUMS( WifiEncryption )
    Q_ENUMS( WifiMode )
    Q_ENUMS( UserRole )

    // settings:
    Q_PROPERTY( QString wifiApPassword READ wifiApPassword WRITE setWifiApPassword NOTIFY wifiApPasswordChanged )
    Q_PROPERTY( bool wifiEnabled READ isWifiEnabled NOTIFY wifiModeChanged )
    Q_PROPERTY( QString grinderName READ grinderName WRITE setGrinderName NOTIFY grinderNameChanged )
    Q_PROPERTY( bool timeModeIsEnabled READ timeModeIsEnabled WRITE setTimeModeIsEnabled NOTIFY timeModeIsEnabledChanged )
    Q_PROPERTY( bool libraryModeIsEnabled READ libraryModeIsEnabled WRITE setLibraryModeIsEnabled NOTIFY libraryModeIsEnabledChanged )
    Q_PROPERTY( bool agsaEnabled READ agsaEnabled WRITE setAgsaEnabled NOTIFY agsaEnabledChanged )
    Q_PROPERTY( int agsaStartDelaySec READ agsaStartDelaySec WRITE setAgsaStartDelaySec NOTIFY agsaStartDelaySecChanged )
    Q_PROPERTY( int displayBrightness READ displayBrightness WRITE setDisplayBrightness NOTIFY displayBrightnessChanged )
    Q_PROPERTY( int stdbyTimeIndex READ stdbyTimeIndex WRITE setStdbyTimeIndex NOTIFY stdbyTimeIndexChanged )
    Q_PROPERTY( int settingsModeExitTimeIndex READ settingsModeExitTimeIndex WRITE setSettingsModeExitTimeIndex NOTIFY settingsModeExitTimeIndexChanged )
    Q_PROPERTY( bool startOfWeekSun READ isStartOfWeekSun WRITE setStartOfWeekSunday NOTIFY startOfWeekSundayChanged )
    Q_PROPERTY( int statisticsPeriodIndex READ statisticsPeriodIndex WRITE setStatisticsPeriodIndex NOTIFY statisticsPeriodIndexChanged )
    Q_PROPERTY( int discUsageMaxHours READ discUsageMaxHours NOTIFY discUsageMaxHoursChanged )
    Q_PROPERTY( bool showTempEnable READ showTemp NOTIFY showTempEnableChanged )
    Q_PROPERTY( bool showDosingTime READ showDosingTime NOTIFY showDosingTimeChanged )
    Q_PROPERTY( SettingsSerializer::TempUnit tempUnit READ tempUnit WRITE setTempUnit NOTIFY tempUnitChanged )
    Q_PROPERTY( int tempThresValue READ getTempThresValueX100 WRITE setTempThresValueX100 NOTIFY tempThresValueChanged )
    Q_PROPERTY( SettingsSerializer::UserRole currentLoggedInUserRole READ currentLoggedInUserRole WRITE setCurrentLoggedInUserRole NOTIFY currentLoggedInUserRoleChanged )
    Q_PROPERTY( bool autoLogoutEnable READ isUserAutoLogoutEnabled NOTIFY userAutoLogoutEnableChanged )
    Q_PROPERTY( int mcuMaxMotorSpeed READ mcuMaxMotorSpeed WRITE setMcuMaxMotorSpeed NOTIFY mcuMaxMotorSpeedChanged )
    Q_PROPERTY( int mcuNominalMotorSpeed READ mcuNominalMotorSpeed WRITE setMcuNominalMotorSpeed NOTIFY mcuNominalMotorSpeedChanged )
    Q_PROPERTY( int mcuAccelerationTime READ mcuAccelerationTime WRITE setMcuAccelerationTime NOTIFY mcuAccelerationTimeChanged )
    Q_PROPERTY( int mcuDecelerationTime READ mcuDecelerationTime WRITE setMcuDecelerationTime NOTIFY mcuDecelerationTimeChanged )

    // other:
    Q_PROPERTY( bool pduInstalled READ isPduInstalled NOTIFY pduInstalledChanged )

    // This isn't writable, as only confirmSystemSetup() should change this.
    Q_PROPERTY( bool systemSetupDone READ isSystemSetupDone NOTIFY systemSetupDoneChanged )
    Q_PROPERTY( bool initialSetupDone READ isInitialSetupDone WRITE setInitialSetupDone NOTIFY initialSetupDoneChanged )
    Q_PROPERTY( bool pduAcknowledged READ isPduAcknowledged WRITE setPduAcknowledged NOTIFY pduAcknowledgedChanged )

    Q_PROPERTY( int tempThresLowerLimit READ tempThresLowerLimit NOTIFY tempThresLowerLimitChanged )
    Q_PROPERTY( int tempThresUpperLimit READ tempThresUpperLimit NOTIFY tempThresUpperLimitChanged )

    Q_PROPERTY( int minMcuMaxMotorSpeed READ minMcuMaxMotorSpeed CONSTANT )
    Q_PROPERTY( int maxMcuMaxMotorSpeed READ maxMcuMaxMotorSpeed CONSTANT )

    Q_PROPERTY( int minMcuNominalMotorSpeed READ minMcuNominalMotorSpeed CONSTANT )
    Q_PROPERTY( int maxMcuNominalMotorSpeed READ maxMcuNominalMotorSpeed NOTIFY mcuMaxMotorSpeedChanged )

    Q_PROPERTY( int minMcuAccelerationTime READ minMcuAccelerationTime CONSTANT )
    Q_PROPERTY( int maxMcuAccelerationTime READ maxMcuAccelerationTime NOTIFY mcuMaxMotorSpeedChanged )

    Q_PROPERTY( int minMcuDecelerationTime READ minMcuDecelerationTime CONSTANT )
    Q_PROPERTY( int maxMcuDecelerationTime READ maxMcuDecelerationTime NOTIFY mcuMaxMotorSpeedChanged )

public:

    enum class TempUnit
    {
        TEMPUNIT_CELSIUS = 0,
        TEMPUNIT_FAHRENHEIT = 1,
    };
    Q_ENUM( TempUnit );

    enum class WifiMode
    {
        MODE_OFF = 0,
        MODE_AP,
        MODE_AP_STA,
        MODE_STA
    };
    Q_ENUM( WifiMode );

    enum class UserRole
    {
        NoRole = 0,
        StoreOwner = 1,
        Technician = 2
    };
    Q_ENUM( UserRole );

private:

    static const char *   c_pszGrinderName;
    static const bool     c_bTimeModeIsEnabled         = true;
    static const bool     c_bLibraryModeIsEnabled      = true;
    static const bool     c_bAgsaEnabled               = true;
    static const int      c_nAgsaStartDelaySec         = 2;
    static const int      c_nDisplayBrightness         = 7;
    static const int      c_nStdbyTimeIndex            = 2;
    static const int      c_nSettingsModeExitTimeIndex = 0;
    static const int      c_nDdd1CalibrationAngleDeg   = 0;
    static const int      c_nDdd2CalibrationAngleDeg   = 0;
    static const bool     c_bStartOfWeekSun            = false;
    static const int      c_nStatisticsPeriodIndex     = 0;
    static const int      c_nDiscUsageMaxHours         = 120;
    static const bool     c_bShowTemp                  = true;
    static const bool     c_bShowTempWarning           = true;
    static const bool     c_bShowDosingTime            = false;
    static const bool     c_bShowSpotLight             = true;
    static const TempUnit c_nTempUnit                  = TempUnit::TEMPUNIT_CELSIUS;
    static const int      c_nTempThresholdValueX100    = 45 * 100;
#ifdef DEV_BUILD
    static const UserRole c_nCurrentLoggedInUserRole = UserRole::Technician;
    static const bool     c_bUserAutoLogoutEnable    = false;
#else
    static const UserRole c_nCurrentLoggedInUserRole = UserRole::NoRole;
    static const bool     c_bUserAutoLogoutEnable    = true;
#endif
    static const char *   c_pszUserPinForOwner;
    static const char *   c_pszUserPinForTechnician;

    static const WifiMode c_nWifiMode = WifiMode::MODE_OFF;
    static const char *   c_pszWifiApPassword;
    static const char *   c_pszWifiApSsid;

    static const bool     c_bPduInstalled = false;

    static const bool     c_bSystemSetupDone  = false;
    static const bool     c_bInitialSetupDone = false;
    static const bool     c_bPduAcknowledged  = false;

    static const uint32_t c_u32MinMcuMaxMotorSpeed { 500 };
    static const uint32_t c_u32MaxMcuMaxMotorSpeed { 1500 };
    static const uint32_t c_u32DefMcuMaxMotorSpeed { 1500 };

    static const uint32_t c_u32MinMcuNominalMotorSpeed { 0 };
    //static const     uint32_t        c_u32MaxMcuNominalMotorSpeed { };       // is dynamic: MaxMotorSpeed
    static const uint32_t c_u32DefMcuNominalMotorSpeed { 1500 };

    static const uint32_t c_u32MinMcuAccelerationTime { 100 };
    //static const     uint32_t        c_u32MaxMcuAccelerationTime { };        // is dynamic: MaxMotorSpeed
    static const uint32_t c_u32DefMcuAccelerationTime { 200 };

    static const uint32_t c_u32MinMcuDecelerationTime { 100 };
    //static const     uint32_t        c_u32MaxMcuDecelerationTime { };        // is dynamic: MaxMotorSpeed
    static const uint32_t c_u32DefMcuDecelerationTime { 200 };

    static const int      c_nTempThresLowerLimitCelsiusX100 { 30 * 100 };
    static const int      c_nTempThresUpperLimitCelsiusX100 { 60 * 100 };

public:

    explicit SettingsSerializer( QObject * pParent = nullptr );
    ~SettingsSerializer() override;

    void create( MainStatemachine *     pMainStatemachine,
                 EspDcHallMotorDriver * pPduDcHallMotorDriver,
                 AgsaControl *          pAgsaControl,
                 DeviceInfoCollector *  pDeviceInfoCollector,
                 WifiDriver *           pWifiDriver );

    WifiDriver * wifiDriver() const;

    // Settings getter/setter begin

    QString grinderName() const;

    void setGrinderName( const QString & strGrinderName );

    // modes:
    bool timeModeIsEnabled( void ) const;

    void setTimeModeIsEnabled( const bool bEnable );

    bool libraryModeIsEnabled( void ) const;

    void setLibraryModeIsEnabled( const bool bEnable );

    bool agsaEnabled() const;

    void setAgsaEnabled( const bool bEnabled );

    int agsaStartDelaySec() const;

    void setAgsaStartDelaySec( const int nDelaySec );

    // display:
    int displayBrightness( void ) const;

    void setDisplayBrightness( const int nDisplayBrightness );

    // standby:
    int stdbyTimeIndex( void ) const;

    void setStdbyTimeIndex( const int nStdbyTimeIndex );

    int settingsModeExitTimeIndex( void ) const;

    void setSettingsModeExitTimeIndex( const int nIndex );

    // misc:
    int ddd1CalibrationAngle10thDeg( void ) const;

    void setDdd1CalibrationAngle10thDeg( const int nDddCalibrationAngle10thDeg );

    int ddd2CalibrationAngleq10thDeg( void ) const;

    void setDdd2CalibrationAngle10thDeg( const int nDddCalibrationAngle10thDeg );

    bool isStartOfWeekSun( void ) const;

    void setStartOfWeekSunday( const bool bSunday );

    int statisticsPeriodIndex() const;

    void setStatisticsPeriodIndex( const int nIndex );

    int discUsageMaxHours( void );

    void setDiscUsageMaxHours( const int nMaxHours );

    // show:
    bool showTemp() const;

    void setShowTemp( const bool bShowTemp );

    bool showTempWarning() const;

    void setShowTempWarning( const bool bShowTempWarning );

    bool showDosingTime() const;

    void setShowDosingTime( bool bShow );

    bool showSpotLight() const;

    void setShowSpotLight( const bool bShow );

    // NTC temperature (at grinding gearbox):
    SettingsSerializer::TempUnit tempUnit() const;

    void setTempUnit( const SettingsSerializer::TempUnit tempUnit );

    int getTempThresValueX100() const;

    void setTempThresValueX100( const int nTempX100 );

    // user:
    SettingsSerializer::UserRole currentLoggedInUserRole() const;

    void setCurrentLoggedInUserRole( SettingsSerializer::UserRole userRole );

    bool isUserAutoLogoutEnabled( void ) const;

    void setUserAutoLogoutEnable( const bool bEnable );

    const QString & userPinForOwner() const;

    void setUserPinForOwner( const QString & strStoreOwnerPin );

    const QString & userPinForTechnician() const;

    void setUserPinForTechnician( const QString & strTechnicianPin );

    // wifi:
    bool isWifiEnabled() const;

    SettingsSerializer::WifiMode wifiMode() const;

    void setWifiMode( const WifiMode newMode );

    const QString & wifiApPassword() const;

    void setWifiApPassword( const QString & strApPassword );

    const QString & wifiApSsid() const;

    void setWifiApSsid( const QString & strSsid );

    // Settings getter/setter end

    // serializer related begin

    Q_INVOKABLE bool saveSettings();

    void applyWifiSettingsToDriver();

    void checkAndConvertSettingConfig();

    void restoreToFactorySettings();

    // serializer related end

    // functions to be moved elsewhere (should not be part of SettingsSerielizer):
    static QString stdbyTimeIndexToString( const int nIndex );

    static int stdbyTimeIndexToStdbyTimeMinutes( const int nIndex );

    static QString settingsModeExitTimeIndexToString( const int nIndex );

    static int settingsModeExitTimeIndexToSeconds( const int nIndex );

    Q_INVOKABLE void setMachineDate( const QString & strDate );

    Q_INVOKABLE void setMachineTime( const QString & strTime );

    Q_INVOKABLE void confirmSystemSetup();

    // --- To be tidied up... ---

    // temperature: (move to ntcTempDriver)
    Q_INVOKABLE QString tempToString( const int nTemp ) const;

    static int fahrenheitToCelsius( const int nTempF );

    static int celsiusToFahrenheit( const int nTempC );

    static int fahrenheitX100ToCelsiusX100( const int nTempFahrenheitX100 );

    static int celsiusX100ToFahrenheitX100( const int tempC );

    static QString tempUnitToString( const SettingsSerializer::TempUnit nTempUnit );

    int tempThresLowerLimit( void );

    int tempThresUpperLimit( void );

    // user account:
    static QString userRoleToString( SettingsSerializer::UserRole loggedIn );

    static int checkRanges( const int nCheckValue,
                            const int nDefaultValue,
                            const int nMinValue,
                            const int nMaxValue );

    Q_INVOKABLE void raiseMcuConfigChanged();

    void setMcuMaxMotorSpeed( const uint32_t u32Value );

    uint32_t mcuMaxMotorSpeed() const;

    uint32_t minMcuMaxMotorSpeed() const;

    uint32_t maxMcuMaxMotorSpeed() const;

    void setMcuNominalMotorSpeed( const uint32_t u32Value );

    uint32_t mcuNominalMotorSpeed() const;

    uint32_t minMcuNominalMotorSpeed() const;

    uint32_t maxMcuNominalMotorSpeed() const;

    void setMcuAccelerationTime( const uint32_t u32Value );

    uint32_t mcuAccelerationTime() const;

    uint32_t minMcuAccelerationTime() const;

    uint32_t maxMcuAccelerationTime() const;

    void setMcuDecelerationTime( const uint32_t u32Value );

    uint32_t mcuDecelerationTime() const;

    uint32_t minMcuDecelerationTime() const;

    uint32_t maxMcuDecelerationTime() const;

    void setPduInstalled( const bool bInstalled );

    bool isPduInstalled() const;

    void setSystemSetupDone( bool bDone );

    bool isSystemSetupDone() const;

    void setInitialSetupDone( bool bDone );

    bool isInitialSetupDone() const;

    void setPduAcknowledged( bool bAcknowledged );

    bool isPduAcknowledged() const;

signals:

    // settings changed:
    void grinderNameChanged();

    void timeModeIsEnabledChanged();

    void libraryModeIsEnabledChanged();

    void agsaEnabledChanged();

    void agsaStartDelaySecChanged();

    void displayBrightnessChanged( const int displayBrightness );

    void stdbyTimeIndexChanged();

    void settingsModeExitTimeIndexChanged();

    void startOfWeekSundayChanged();

    void statisticsPeriodIndexChanged();

    void discUsageMaxHoursChanged( int newValue );

    void showTempWarningChanged();

    void showTempEnableChanged();

    void tempUnitChanged();

    void tempThresValueChanged();

    void currentLoggedInUserRoleChanged();

    void userAutoLogoutEnableChanged();

    void showDosingTimeChanged( bool bShow );

    void showSpotLightChanged( const bool bShow );

    void wifiModeChanged( const SettingsSerializer::WifiMode nWifiMode );

    void wifiApSsidChanged();

    void wifiApPasswordChanged();

    // other:
    void tempThresLowerLimitChanged(); // depending on tempUnit changes

    void tempThresUpperLimitChanged(); // depending on tempUnit changes

    void mcuMaxMotorSpeedChanged();

    void mcuNominalMotorSpeedChanged();

    void mcuAccelerationTimeChanged();

    void mcuDecelerationTimeChanged();

    void mcuConfigChanged();

    void pduInstalledChanged();

    void systemSetupDoneChanged();

    void initialSetupDoneChanged();

    void pduAcknowledgedChanged();

public slots:

    bool updateStandardGrinderData();

private:

    bool loadSettings();

    void setDefaultSettings();

    void userAutoLogout();

private:

    QString                      m_strGrinderName = c_pszGrinderName;

    bool                         m_bTimeModeIsEnabled           = c_bTimeModeIsEnabled;
    bool                         m_bLibraryModeIsEnabled        = c_bLibraryModeIsEnabled;
    bool                         m_bAgsaEnabled                 = c_bAgsaEnabled;
    int                          m_nAgsaStartDelaySec           = c_nAgsaStartDelaySec;
    int                          m_nDisplayBrightness           = c_nDisplayBrightness;
    int                          m_nStdbyTimeIndex              = c_nStdbyTimeIndex;
    int                          m_nSettingsModeExitTimeIndex   = c_nSettingsModeExitTimeIndex;
    int                          m_nDdd1CalibrationAngle10thDeg = c_nDdd1CalibrationAngleDeg;
    int                          m_nDdd2CalibrationAngle10thDeg = c_nDdd2CalibrationAngleDeg;
    bool                         m_bStartOfWeekSunday           = c_bStartOfWeekSun; // bool here, but for enums: DIN 1355,ISO 8601:  Monday:=1;
    int                          m_nStatisticsPeriodIndex       = c_nStatisticsPeriodIndex;
    int                          m_nDiscUsageMaxHours           = c_nDiscUsageMaxHours;

    bool                         m_bShowTemp        = c_bShowTemp;
    bool                         m_bShowTempWarning = c_bShowTempWarning;
    bool                         m_bShowDosingTime  = c_bShowDosingTime;
    bool                         m_bShowSpotlight   = c_bShowSpotLight;

    SettingsSerializer::TempUnit m_nTempUnit              = c_nTempUnit;
    int                          m_nTempThreshCelsiusX100 = c_nTempThresholdValueX100;

    SettingsSerializer::UserRole m_nCurrentLoggedInUserRole = c_nCurrentLoggedInUserRole;
    bool                         m_bUserAutoLogoutEnable    = c_bUserAutoLogoutEnable;
    QString                      m_strUserPinForOwner       = c_pszUserPinForOwner;
    QString                      m_strUserPinForTechnician  = c_pszUserPinForTechnician;

    SettingsSerializer::WifiMode m_nWifiMode         = c_nWifiMode;
    QString                      m_strWifiApPassword = c_pszWifiApPassword;
    QString                      m_strWifiApSsid     = c_pszWifiApSsid;

    uint32_t                     m_u32McuMaxMotorSpeed     = c_u32DefMcuMaxMotorSpeed;
    uint32_t                     m_u32McuNominalMotorSpeed = c_u32DefMcuNominalMotorSpeed;
    uint32_t                     m_u32McuAccelerationTime  = c_u32DefMcuAccelerationTime;
    uint32_t                     m_u32McuDecelerationTime  = c_u32DefMcuDecelerationTime;

    bool                         m_bPduInstalled = c_bPduInstalled;

    bool                         m_bSystemSetupDone  = c_bSystemSetupDone;
    bool                         m_bInitialSetupDone = c_bInitialSetupDone;
    bool                         m_bPduAcknowledged  = c_bPduAcknowledged;

    MainStatemachine *           m_pMainStatemachine { nullptr };
    WifiDriver *                 m_pWifiDriver { nullptr };
    DeviceInfoCollector *        m_pDeviceInfoCollector { nullptr };
    EspDcHallMotorDriver *       m_pPduDcHallMotorDriver { nullptr };
    AgsaControl *                m_pAgsaControl { nullptr };

    AcyclicNotification          m_acycNotificationSaveFailed;
    AcyclicNotification          m_acycNotificationLoadFailed;

};

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#endif // SettingsSerializer_h
