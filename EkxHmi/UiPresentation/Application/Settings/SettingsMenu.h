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

#ifndef SETTINGS_MENU_H
#define SETTINGS_MENU_H

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

    Q_ENUMS( SettingEntryTypes )
    Q_ENUMS( ActionEntryTypes )
    Q_ENUMS( ToggleswitchTypes )

    Q_PROPERTY( QString                         menuName          READ getMenuName                                     NOTIFY menuNameChanged )
    Q_PROPERTY( SettingsMenu::SettingEntryTypes menuType          READ getMenuType                                     NOTIFY menuTypeChanged )
    Q_PROPERTY( bool                            toggleswitchValue  READ getToggleswitchValue  WRITE setToggleswitchValue NOTIFY toggleswitchValueChanged )
    Q_PROPERTY( SettingsMenu::ToggleswitchTypes toggleswitchType   READ getToggleswitchType                              NOTIFY toggleswitchTypeChanged )
    Q_PROPERTY( QString                         stringValue        READ getStringValue                                   NOTIFY stringValueChanged )
    Q_PROPERTY( int                             linkedMenu        READ getLinkedSettingState                            NOTIFY linkedSettingStateChanged )
    Q_PROPERTY( SettingsMenu::ActionEntryTypes  actionType         READ getActionType                                    NOTIFY actionTypeChanged )
    Q_PROPERTY( QString                         actionValue        READ getActionValue        WRITE setActionValue       NOTIFY acionValueChanged )
    Q_PROPERTY( bool                            menuChoosable     READ getMenuChoosable                                NOTIFY menuChooseableChanged )
    Q_PROPERTY( bool                            userRightsObtained READ getUserRightsObtained                            NOTIFY userRightsObtainedChanged )

public:

    enum SettingEntryTypes
    {
        ENTRYTYPE_UNDEFINED = 0,
        ENTRYTYPE_STRING,
        ENTRYTYPE_NOVALUE,
        ENTRYTYPE_MAINENTRY,
        ENTRYTYPE_RADIOBUTTON,
        ENTRYTYPE_ACTION,
        ENTRYTYPE_TOGGLESWITCH,
        ENTRYTYPE_LAST,
    };

    enum ActionEntryTypes
    {
        ACTIONTYPE_NONE = 0,
        ACTIONTYPE_TIME,
        ACTIONTYPE_DATE,
        ACTIONTYPE_STDBY,
        ACTIONTYPE_SUCCESS,
        ACTIONTYPE_ERROR,
        ACTIONTYPE_CONFIRM,
        ACTIONTYPE_FACTORY_RESET,
        ACTIONTYPE_RADIOBUTTON,
        ACTIONTYPE_PIN,
        ACTIONTYPE_CALIB,
        ACTIONTYPE_DISK_USAGE_TIME,
        ACTIONTYPE_CPC,
        ACTIONTYPE_PW ,
        ACTIONTYPE_LOGIN,
        ACTIONTYPE_PROGRESS,
        ACTIONTYPE_MACHINFO,
        ACTIONTYPE_TEXTBOX,
        ACTIONTYPE_DISPLAY_TEST,
        ACTIONTYPE_STATISTICS_SCREEN,
        ACTIONTYPE_LAST
    };

    enum ToggleswitchTypes
    {
        TOGGLESWITCH_FOR_NONE = 0,
        TOGGLESWITCH_FOR_SHOWTEMP,
        TOGGLESWITCH_FOR_WARNINGTEMP,
        TOGGLESWITCH_FOR_LIBRARYMODE_ENABLING,
        TOGGLESWITCH_FOR_TIMEMODE_ENABLING,
        TOGGLESWITCH_FOR_SHOW_DDD,
        TOGGLESWITCH_FOR_SHOWSPOT,
        TOGGLESWITCH_FOR_WIFI,
        TOGGLESWITCH_FOR_AUTOLOG,
        TOGGLESWITCH_FOR_WARNINGS,
        TOGGLESWITCH_FOR_LAST
    };

    explicit SettingsMenu(QObject *parent = nullptr);
    virtual ~SettingsMenu();

    // QML property setters and getters:
    QString                         getMenuName() const;
    void                            setMenuName( QString newMenuName );
    SettingsMenu::SettingEntryTypes getMenuType() const;
    void                            setMenuType( SettingsMenu::SettingEntryTypes newMenuType );
    int                             getToggleswitchValue() const;
    void                            setToggleswitchValue( int newActiveValue );
    SettingsMenu::ToggleswitchTypes getToggleswitchType() const;
    void                            setToggleswitchType( SettingsMenu::ToggleswitchTypes newToggleswitchType  );
    QString                         getStringValue() const ;
    void                            setStringValue( QString newMenuValue );
    int                             getLinkedSettingState() const;
    void                            setLinkedSettingState( int newLinkedMenu );
    SettingsMenu::ActionEntryTypes  getActionType() const;
    void                            setActionType( SettingsMenu::ActionEntryTypes newActionType );
    QString                         getActionValue() const;
    void                            setActionValue( QString newActionValue );
    bool                            getMenuChoosable() const;
    void                            setMenuChoosable( bool newMenuChoosable  );
    bool                            getUserRightsObtained() const;
    void                            setUserRightsObtained( bool newUserRightsObtained  );

signals:

    void menuNameChanged();
    void menuTypeChanged();
    void toggleswitchValueChanged();
    void toggleswitchTypeChanged();
    void stringValueChanged();
    void linkedSettingStateChanged();
    void actionTypeChanged();
    void acionValueChanged();
    void menuChooseableChanged();
    void userRightsObtainedChanged();

private:

    QString                             m_menuName;
    SettingsMenu::SettingEntryTypes     m_menuType;
    bool                                m_toggleswitchValue;
    SettingsMenu::ToggleswitchTypes     m_toggleswitchType;
    QString                             m_stringValue;
    int                                 m_linkedSettingState;
    SettingsMenu::ActionEntryTypes      m_actionType;
    QString                             m_actionValue;
    bool                                m_menuChoosable;
    bool                                m_userRightsObtained;

    static int m_settingEntryTypes_QTypeId;
    static int m_actionEntryTypes_QTypeId;
    static int m_toggleswitchTypes_QTypeId;
    static int m_qmlTypeId;
};

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#endif // SETTINGS_MENU_H
