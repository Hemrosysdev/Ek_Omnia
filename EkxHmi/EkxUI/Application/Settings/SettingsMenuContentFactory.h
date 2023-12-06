///////////////////////////////////////////////////////////////////////////////
///
/// @file SettingsMenuContentFactory.h
///
/// @brief Header file of class SettingsMenuContentFactory.
///
/// @author Ultratronik GmbH
///         Dornierstr. 9
///         D-82205 Gilching
///         Germany
///         http://www.ultratronik.de
///
/// @author written by Gerd Esser, Forschung & Entwicklung, gesser@ultratronik.de
///
/// @date 11.11.2022
///
/// @copyright Copyright 2022 by Hemro International AG
///            Hemro International AG
///            Länggenstrasse 34
///            CH 8184 Bachenbülach
///            Switzerland
///            Homepage: www.hemrogroup.com
///
///////////////////////////////////////////////////////////////////////////////

#ifndef SettingsMenuContentFactory_h
#define SettingsMenuContentFactory_h

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#include <QObject>
#include <QQmlListProperty>
#include <QList>

class MainStatemachine;
class SettingsStatemachine;
class SettingsMenu;
class SettingsSerializer;

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

class SettingsMenuContentFactory : public QObject
{
    Q_OBJECT

    Q_PROPERTY( QQmlListProperty<SettingsMenu>      qlpMenuList READ qlpMenuList NOTIFY qlpMenuListChanged )
    Q_PROPERTY( QString menuTitle READ menuTitle WRITE setMenuTitle NOTIFY menuTitleChanged )
    Q_PROPERTY( bool navAvailable READ isNavAvailable WRITE setNavAvailable NOTIFY navAvailableChanged )
    Q_PROPERTY( bool buttonPreviousVisible READ isButtonPreviousVisible WRITE setButtonPreviousVisible NOTIFY buttonPreviousVisibleChanged )
    Q_PROPERTY( bool buttonOkVisible READ isButtonOkVisible WRITE setButtonOkVisible NOTIFY buttonOkVisibleChanged )

public:

    explicit SettingsMenuContentFactory( QObject * pParent = nullptr );
    ~SettingsMenuContentFactory() override;

    void create( SettingsStatemachine * pSettingsStatemachine,
                 MainStatemachine *     pMainStatemachine,
                 SettingsSerializer *   pSettingsSerializer );

    Q_INVOKABLE void setMenuTitle( const QString & strMenuTitle );

    Q_INVOKABLE QString menuTitle();

    void setNavAvailable( const bool bNavAvailable );

    bool isNavAvailable() const;

    void setButtonPreviousVisible( const bool bVisible );

    bool isButtonPreviousVisible( void ) const;

    void setButtonOkVisible( const bool bVisible );

    bool isButtonOkVisible( void ) const;

    void buildUpMainState();

    void buildUpService();

    void buildUpServiceDiscCalibration();

    void buildUpServiceDiscUsageTime();

    void buildUpServiceMotorTest();

    void buildUpServiceMotorTestAgsa();

    void buildUpServiceMotorTestAgsaMoveToDdd();

    void buildUpServiceMotorTestAgsaNumSteps();

    void buildUpServiceMotorTestAgsaFrequency();

    void buildUpServiceMotorTestPdu();

    void buildUpServiceMotorTestAgsaLongDuration();

    void buildUpServiceDddTest();

    void buildUpServiceMcuConfig();

    void buildUpServiceMcuConfigMaxMotorSpeed();

    void buildUpServiceMcuConfigNominalMotorSpeed();

    void buildUpServiceMcuConfigAccelerationTime();

    void buildUpServiceMcuConfigDecelerationTime();

    void buildUpServiceSystemSetup();

    void buildUpSystem();

    void buildUpSystemWifi();

    void buildUpSystemWifiIp();

    void buildUpSystemWifiPassword();

    void buildUpSystemWifiSsid();

    void buildUpSystemDisplayTest();

    void buildUpSystemFactoryReset();

    void buildUpSystemGrindername();

    void buildUpSystemStartOfWeek();

    void buildUpSystemTempUnit();

    void buildUpSystemDate();

    void buildUpSystemTime();

    void buildUpCustomize();

    void buildUpCustomizeBrightness();

    void buildUpCustomizeStandbyAdj();

    void buildUpCustomizeSettingsModeExitTimeoutAdj();

    void buildUpCustomizeAgsaDelay();

    void buildUpCustomizeTemp();

    void buildUpCustomizeTempThresh();

    void buildUpUser();

    void buildUpUserCpc();

    void buildUpUserCpcStore();

    void buildUpUserCpcTech();

    void buildUpUserLogin();

    void buildUpUserLogout();

    void buildUpUserStore();

    void buildUpUserTech();

    void buildUpInfo();

    void buildUpInfoStatistics();

    void buildUpInfoNotificationCenter();

    void buildUpInfoMachineInfo();

    void buildUpInfoOwnersManual();

    void buildUpInfoGrinderTutorial();

    void buildUpInfoPduTutorial();

signals:

    void qlpMenuListChanged();

    void menuTitleChanged();

    void navAvailableChanged();

    void navigatedPrevious();

    void buttonPreviousVisibleChanged();

    void buttonOkVisibleChanged();

public slots:

private:

    void clearMenuList();

protected:

    QQmlListProperty<SettingsMenu> qlpMenuList() const;

private:

    SettingsStatemachine *         m_pSettingsStatemachine { nullptr };
    MainStatemachine *             m_pMainStatemachine { nullptr };
    SettingsSerializer *           m_pSettingsSerializer { nullptr };
    QString                        m_strMenuTitle;
    bool                           m_bNavAvailable { true };
    bool                           m_bButtonPreviousVisible { false };
    bool                           m_bButtonOkVisible { false };

    QList<SettingsMenu *>          m_theMenuList;
    QQmlListProperty<SettingsMenu> m_theQlpMenuList;

};

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#endif // SettingsMenuContentFactory_h
