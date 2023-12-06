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

#ifndef SETTINGS_SERIALIZER_H
#define SETTINGS_SERIALIZER_H

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#include <QObject>
#include <QString>
#include <QQmlEngine>

class DeviceInfoCollector;
class WifiDriver;
class MainStatemachine;

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

class SettingsSerializer : public QObject
{
    Q_OBJECT

    Q_ENUMS(tempUnit)
    Q_ENUMS(wifiEncryption)
    Q_ENUMS(wifiMode)
    Q_ENUMS(userRole)

    // settings:
    Q_PROPERTY( QString                      grinderName              READ getGrinderName              WRITE setGrinderName               NOTIFY grinderNameChanged )
    Q_PROPERTY( bool                         timeModeIsEnabled        READ getTimeModeIsEnabled        WRITE setTimeModeIsEnabled         NOTIFY timeModeIsEnabledChanged )
    Q_PROPERTY( bool                         libraryModeIsEnabled     READ getLibraryModeIsEnabled     WRITE setLibraryModeIsEnabled      NOTIFY libraryModeIsEnabledChanged )
    Q_PROPERTY( int                          displayBrightnessPercent READ getDisplayBrightnessPercent WRITE setDisplayBrightnessPercent  NOTIFY displayBrightnessPercentChanged)
    Q_PROPERTY( int                          stdbyTimeIndex           READ getStdbyTimeIndex           WRITE setStdbyTimeIndex            NOTIFY stdbyTimeIndexChanged)
    Q_PROPERTY( bool                         startOfWeekisSunNotMon   READ startOfWeekisSunNotMon      WRITE setStartOfWeekisSunNotMon    NOTIFY startOfWeekisSunNotMonChanged)
    Q_PROPERTY( int                          statisticsPeriodIndex    READ statisticsPeriodIndex       WRITE setStatisticsPeriodIndex     NOTIFY statisticsPeriodIndexChanged)
    Q_PROPERTY( int                          discUsageMaxHours        READ getDiscUsageMaxHours                                           NOTIFY discUsageMaxHoursChanged )
    Q_PROPERTY( bool                         showTempEnable           READ getShowTemp                                                    NOTIFY showTempEnableChanged )
    Q_PROPERTY( bool                         showDddEnable            READ getShowDdd                                                     NOTIFY showDddEnableChanged )
    Q_PROPERTY( SettingsSerializer::tempUnit tempUnit                 READ getTempUnit                 WRITE setTempUnit                  NOTIFY tempUnitChanged )
    Q_PROPERTY( int                          tempThresValue           READ getTempThresValueX100       WRITE setTempThresValueX100        NOTIFY tempThresValueChanged)
    Q_PROPERTY( SettingsSerializer::userRole userAutoLoginAs          READ getUserAutoLoginAs          WRITE setUserAutoLoginAs           NOTIFY userAutoLoginAsChanged)
    Q_PROPERTY( bool                         autoLogoutEnable         READ getUserAutoLogoutEnable                                        NOTIFY userAutoLogoutEnableChanged )

    // other:
    Q_PROPERTY( int                          tempThresLowerLimit      READ getTempThresLowerLimit                                         NOTIFY tempThresLowerLimitChanged)
    Q_PROPERTY( int                          tempThresUpperLimit      READ getTempThresUpperLimit                                         NOTIFY tempThresUpperLimitChanged)

public:

    enum tempUnit
    {
        TEMPUNIT_CELSIUS       = 0,
        TEMPUNIT_FAHRENHEIT    = 1,
    };

    enum wifiEncryption
    {
        ENCRYPTION_NONE       = 0,
        ENCRYPTION_WPA        = 1,
        ENCRYPTION_WPA2       = 2
    };

    enum wifiMode
    {
        MODE_NONE       = 0,
        MODE_AP         = 1,
        MODE_STA        = 2
    };

    enum userRole
    {
        LOGGEDIN_STOREOWNER    = 0,
        LOGGEDIN_TECHNICIAN    = 1,
        LOGGEDIN_NONE          = 2
    };

    explicit SettingsSerializer( QQmlEngine* enigne,
                                 MainStatemachine * pMainStatemachine );
    virtual ~SettingsSerializer() override;

    // Settings getter/setter begin

    QString getGrinderName() const;
    void    setGrinderName( QString strGrinderName );

    // modes:
    bool getTimeModeIsEnabled( void ) const;
    void setTimeModeIsEnabled( bool enable );

    bool getLibraryModeIsEnabled( void ) const;
    void setLibraryModeIsEnabled( bool enable );

    // display:
    int  getDisplayBrightnessPercent( void ) const;
    void setDisplayBrightnessPercent( int displayBrightnessPercent );

    // standby:
    int  getStdbyTimeIndex( void ) const;
    void setStdbyTimeIndex( int newStdbyTimeIndex );

    // misc:
    int  getDddCalibrationAngleDeg( void ) const;
    void setDddCalibrationAngleDeg( const int dddCalibrationAngle );

    bool startOfWeekisSunNotMon( void ) const;
    void setStartOfWeekisSunNotMon( bool sundayNotMonday );

    int statisticsPeriodIndex() const;
    void setStatisticsPeriodIndex( const int nIndex );

    int  getDiscUsageMaxHours( void );
    void setDiscUsageMaxHours( int maxHours );

    // show:
    bool getShowTemp() const;
    void setShowTemp( bool newShowTemp );

    bool getShowTempWarning() const;
    void setShowTempWarning( bool newShowTempWarning );

    bool getShowDdd() const;
    void setShowDdd( bool newShowDdd );

    bool getShowWarnings() const;
    void setShowWarnings( bool newShowWarnings );

    bool getShowSpotlight() const;
    void setShowSpotlight( bool newShowSpot );

    // NTC temperature (at grinding gearbox):
    SettingsSerializer::tempUnit getTempUnit() const;
    void setTempUnit( SettingsSerializer::tempUnit tempUnit );

    int  getTempThresValueX100() const;
    void setTempThresValueX100( int tempX100 );

    // user:
    SettingsSerializer::userRole getUserAutoLoginAs() const;
    void                         setUserAutoLoginAs( SettingsSerializer::userRole userRole );

    bool getUserAutoLogoutEnable( void ) const;
    void setUserAutoLogoutEnable( bool enable );

    QString getUserPinForOwner() const;
    void    setUserPinForOwner( QString newStoreOwnerPin );

    QString getUserPinForTechnician() const;
    void    setUserPinForTechnician( QString newTechnicianPin );

    // wifi:
    bool getWifiEnable() const;
    void setWifiEnable( bool enable );

    SettingsSerializer::wifiEncryption getWifiEnc() const;
    void                               setWifiEnc( SettingsSerializer::wifiEncryption newWifiEnc );

    SettingsSerializer::wifiMode getWifiMode() const;
    void                         setWifiMode( int newMode );

    QString          getWifiPw() const;
    Q_INVOKABLE void setWifiPw( QString newWifiPw );

    QString getWifiApSsid() const;
    void    setWifiApSsid( QString newWifiApSSID );

    // Settings getter/setter end

    // serializer related begin

    Q_INVOKABLE bool saveSettings();

    void        setWifiDriver( WifiDriver* wd );
    WifiDriver* getWifiDriver( ) const;
    void applyWifiSettingsToDriver();

    void setDeviceInfoDriver( DeviceInfoCollector* dic );
    void checkAndConvertSettingConfig();
    Q_INVOKABLE void restoreToFactorySettings();

    // serializer related end


    // functions to be moved elsewhere (should not be part of SettingsSerielizer):
    static QString stdbyTimeIndexToString( int stdbyTimeIndex );
    static int stdbyTimeIndexToStdbyTimeMinutes( int stdbyTimeIndex );

    Q_INVOKABLE void setMachineDate( QString newDate );
    Q_INVOKABLE void setMachineTime( QString newTime );

    // --- To be tidied up... ---

    // temperature: (move to ntcTempDriver)
    Q_INVOKABLE QString tempToString(int temp) const;
    static int fahrenheitToCelsius(int tempF);
    static int celsiusToFahrenheit(int tempC);
    static int fahrenheitX100ToCelsiusX100(int tempF);
    static int celsiusX100ToFahrenheitX100(int tempC);
    static QString tempUnitToString( SettingsSerializer::tempUnit tempUnit );
    int  getTempThresLowerLimit(void);
    int  getTempThresUpperLimit(void);

    // user account:
    static QString userRoleToString( SettingsSerializer::userRole loggedIn );

signals:

    // settings changed:
    void grinderNameChanged();
    void timeModeIsEnabledChanged();
    void libraryModeIsEnabledChanged();
    void displayBrightnessPercentChanged(int displayBrightnessPercent);
    void stdbyTimeIndexChanged();
    void startOfWeekisSunNotMonChanged();
    void statisticsPeriodIndexChanged();
    void discUsageMaxHoursChanged(int newValue);
    void showTempWarningChanged();
    void showTempEnableChanged();
    void showDddEnableChanged();
    void tempUnitChanged();
    void tempThresValueChanged();
    void userAutoLoginAsChanged();
    void userAutoLogoutEnableChanged();

    // other:
    void tempThresLowerLimitChanged(); // depending on tempUnit changes
    void tempThresUpperLimitChanged(); // depending on tempUnit changes

private:

    void initSettings();
    bool loadSettings();
    void loadFactorySettings();
    void userAutoLogout();

private:

    struct ekxSettings_t
    {
        QString grinderName;

        bool timeModeIsEnabled;
        bool libraryModeIsEnabled;
        int  displayBrightnessPercent;
        int  stdbyTimeIndex;
        int  dddCalibrationAngleDeg;
        bool startOfWeekisSunNotMon; // bool here, but for enums: DIN 1355,ISO 8601:  Monday:=1;
        int  statisticsPeriodIndex;
        int  discUsageMaxHours;

        bool showTemp;
        bool showTempWarning;
        bool showDdd;
        bool showWarnings;
        bool showSpotlight;

        SettingsSerializer::tempUnit       tempUnit;
        int                                tempThreshCelsiusX100;

        SettingsSerializer::userRole       userAutoLoginAs;
        bool                               userAutoLogoutEnable;
        QString                            userPinForOwner;
        QString                            userPinForTechnician;

        bool                               wifiEnable;
        SettingsSerializer::wifiEncryption wifiEnc;
        SettingsSerializer::wifiMode       wifiMode;
        QString                            wifiPw;
        QString                            wifiApSsid;
    } m_settings;

    QQmlEngine*              m_qmlEngine { nullptr };
    MainStatemachine *       m_pMainStatemachine { nullptr };
    WifiDriver*              m_wifiDriver { nullptr };
    DeviceInfoCollector*     m_deviceInfoDriver { nullptr };

    const int                m_nTempThresLowerLimitCelsiusX100 { 30*100 };
    const int                m_nTempThresUpperLimitCelsiusX100 { 60*100 };

    static int LoggedInUser_QTypeId;
    static int QmlSettingsSerializerId;
};

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#endif // SETTINGS_SERIALIZER_H
