///////////////////////////////////////////////////////////////////////////////
///
/// @file SettingsMenu.h
///
/// @brief Header file of class SettingsMenu.
///
/// @author Ultratronik GmbH
///         Dornierstr. 9
///         D-82205 Gilching
///         Germany
///         http://www.ultratronik.de
///
/// @author written by Gerd Esser, Forschung & Entwicklung, gesser@ultratronik.de
///
/// @date 25.03.2021
///
/// @copyright Copyright 2021 by Hemro International AG
///            Hemro International AG
///            Länggenstrasse 34
///            CH 8184 Bachenbülach
///            Switzerland
///            Homepage: www.hemrogroup.com
///
///////////////////////////////////////////////////////////////////////////////

#ifndef SettingsMenu_h
#define SettingsMenu_h

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#include <QObject>
#include <QString>

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

class SettingsMenu : public QObject
{
    Q_OBJECT

    Q_PROPERTY( QString menuName READ menuName NOTIFY menuNameChanged )
    Q_PROPERTY( SettingsMenu::SettingEntryTypes menuType READ menuType NOTIFY menuTypeChanged )
    Q_PROPERTY( bool toggleSwitchValue READ toggleSwitchValue WRITE setToggleSwitchValue NOTIFY toggleSwitchValueChanged )
    Q_PROPERTY( SettingsMenu::ToggleSwitchTypes toggleSwitchType READ toggleSwitchType NOTIFY toggleSwitchTypeChanged )
    Q_PROPERTY( QString stringValue READ stringValue NOTIFY stringValueChanged )
    Q_PROPERTY( int linkedSettingsState READ linkedSettingsState NOTIFY linkedSettingsStateChanged )
    Q_PROPERTY( SettingsMenu::ActionEntryTypes actionType READ actionType NOTIFY actionTypeChanged )
    Q_PROPERTY( QString actionValue READ actionValue WRITE setActionValue NOTIFY actionValueChanged )
    Q_PROPERTY( bool menuChoosable READ menuChoosable NOTIFY menuChooseableChanged )
    Q_PROPERTY( bool userRightsObtained READ userRightsObtained NOTIFY userRightsObtainedChanged )

public:

    enum SettingEntryTypes
    {
        ENTRYTYPE_UNDEFINED = 0,
        ENTRYTYPE_STRING,
        ENTRYTYPE_IMAGE,
        ENTRYTYPE_NOVALUE,
        ENTRYTYPE_MAINENTRY,
        ENTRYTYPE_RADIOBUTTON,
        ENTRYTYPE_ACTION,
        ENTRYTYPE_TOGGLE_SWITCH,

        ENTRYTYPE_LAST,
    };
    Q_ENUMS( SettingEntryTypes )

    enum ActionEntryTypes
    {
        ACTIONTYPE_NONE = 0,
        ACTIONTYPE_TIME,
        ACTIONTYPE_DATE,
        ACTIONTYPE_STDBY,
        ACTIONTYPE_SETTINGS_MODE_EXIT_TIMEOUT,
        ACTIONTYPE_AGSA_DELAY,
        ACTIONTYPE_SUCCESS,
        ACTIONTYPE_ERROR,
        ACTIONTYPE_CONFIRM,
        ACTIONTYPE_FACTORY_RESET,
        ACTIONTYPE_RADIOBUTTON,
        ACTIONTYPE_PIN,
        ACTIONTYPE_CALIB,
        ACTIONTYPE_DISK_USAGE_TIME,
        ACTIONTYPE_CPC,
        ACTIONTYPE_PW,
        ACTIONTYPE_LOGIN,
        ACTIONTYPE_SLIDER_ADJUSTMENT,
        ACTIONTYPE_TEXTBOX,
        ACTIONTYPE_DISPLAY_TEST,

        ACTIONTYPE_MOTOR_TEST_AGSA_MOVE_STEPS,
        ACTIONTYPE_MOTOR_TEST_AGSA_STOP,
        ACTIONTYPE_MOTOR_TEST_AGSA_LONG_DURATION_START,
        ACTIONTYPE_MOTOR_TEST_AGSA_LONG_DURATION_STOP,
        ACTIONTYPE_MOTOR_TEST,
        ACTIONTYPE_MOTOR_TEST_PDU,

        ACTIONTYPE_STATISTICS,
        ACTIONTYPE_NOTIFICATION_CENTER,
        ACTIONTYPE_MACHINE_INFO,
        ACTIONTYPE_OWNERS_MANUAL,
        ACTIONTYPE_TUTORIAL,

        ACTIONTYPE_DDD_TEST,

        ACTIONTYPE_SYSTEM_SETUP,

        ACTIONTYPE_LAST
    };
    Q_ENUMS( ActionEntryTypes )

    enum ToggleSwitchTypes
    {
        TOGGLE_SWITCH_FOR_NONE = 0,
        TOGGLE_SWITCH_FOR_SHOWTEMP,
        TOGGLE_SWITCH_FOR_WARNINGTEMP,
        TOGGLE_SWITCH_FOR_LIBRARYMODE_ENABLING,
        TOGGLE_SWITCH_FOR_AGSA_ENABLE,
        TOGGLE_SWITCH_FOR_TIMEMODE_ENABLING,
        TOGGLE_SWITCH_FOR_DOSING_TIME,
        TOGGLE_SWITCH_FOR_SHOWSPOT,
        TOGGLE_SWITCH_FOR_WIFI,
        TOGGLE_SWITCH_FOR_AUTOLOG,
        TOGGLE_SWITCH_FOR_AGSA_BLOCKAGE_DETECT,
        TOGGLE_SWITCH_FOR_AGSA_LIVE_MODE,

        TOGGLE_SWITCH_FOR_LAST
    };
    Q_ENUMS( ToggleSwitchTypes )

public:

    explicit SettingsMenu( QObject * parent = nullptr );
    ~SettingsMenu() override;

    // QML property setters and getters:
    const QString & menuName() const;

    void setMenuName( const QString & strMenuName );

    SettingsMenu::SettingEntryTypes menuType() const;

    void setMenuType( const SettingEntryTypes nMenuType );

    bool toggleSwitchValue() const;

    void setToggleSwitchValue( const bool bToggleSwitchValue );

    SettingsMenu::ToggleSwitchTypes toggleSwitchType() const;

    void setToggleSwitchType( const ToggleSwitchTypes nToggleSwitchType );

    const QString & stringValue() const;

    void setStringValue( const QString & strValue );

    int linkedSettingsState() const;

    void setLinkedSettingsState( const int nLinkedState );

    SettingsMenu::ActionEntryTypes actionType() const;

    void setActionType( const SettingsMenu::ActionEntryTypes nActionType );

    const QString & actionValue() const;

    void setActionValue( const QString & strActionValue );

    bool menuChoosable() const;

    void setMenuChoosable( const bool bMenuChoosable );

    bool userRightsObtained() const;

    void setUserRightsObtained( const bool bUserRightsObtained );

signals:

    void menuNameChanged();

    void menuTypeChanged();

    void toggleSwitchValueChanged();

    void toggleSwitchTypeChanged();

    void stringValueChanged();

    void linkedSettingsStateChanged();

    void actionTypeChanged();

    void actionValueChanged();

    void menuChooseableChanged();

    void userRightsObtainedChanged();

private:

    QString                         m_strMenuName;
    SettingsMenu::SettingEntryTypes m_nMenuType { SettingEntryTypes::ENTRYTYPE_MAINENTRY };
    bool                            m_bToggleSwitchValue { false };
    SettingsMenu::ToggleSwitchTypes m_nToggleSwitchType { ToggleSwitchTypes::TOGGLE_SWITCH_FOR_NONE };
    QString                         m_strValue;
    int                             m_nLinkedSettingsState { 0 };
    SettingsMenu::ActionEntryTypes  m_nActionType { ActionEntryTypes::ACTIONTYPE_NONE };
    QString                         m_strActionValue;
    bool                            m_bMenuChoosable { true };
    bool                            m_bUserRightsObtained { true };

    //    static int m_settingEntryTypes_QTypeId;
    //    static int m_actionEntryTypes_QTypeId;
    //    static int m_toggleSwitchTypes_QTypeId;
    //    static int m_qmlTypeId;
};

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#endif // SettingsMenu_h
