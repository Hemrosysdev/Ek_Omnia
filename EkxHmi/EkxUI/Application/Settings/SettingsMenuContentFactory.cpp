///////////////////////////////////////////////////////////////////////////////
///
/// @file SettingsMenuContentFactory.cpp
///
/// @brief Implementation file of class SettingsMenuContentFactory
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

#include "SettingsMenuContentFactory.h"

#include <QtDebug>
#include <QDateTime>
#include <QHostAddress>
#include <QNetworkInterface>

#include "SettingsMenu.h"
#include "MainStatemachine.h"
#include "AgsaControl.h"
#include "NotificationCenter.h"
#include "Notification.h"
#include "SettingsStatemachine.h"
#include "DddCouple.h"
#include "WifiDriver.h"

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

SettingsMenuContentFactory::SettingsMenuContentFactory( QObject * pParent )
    : QObject( pParent )
    , m_theQlpMenuList( this, &m_theMenuList )
{
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

SettingsMenuContentFactory::~SettingsMenuContentFactory()
{
    clearMenuList();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsMenuContentFactory::create( SettingsStatemachine * pSettingsStatemachine,
                                         MainStatemachine *     pMainStatemachine,
                                         SettingsSerializer *   pSettingsSerializer )
{
    m_pSettingsStatemachine = pSettingsStatemachine;
    m_pMainStatemachine     = pMainStatemachine;
    m_pSettingsSerializer   = pSettingsSerializer;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SettingsMenuContentFactory::isNavAvailable() const
{
    return m_bNavAvailable;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsMenuContentFactory::setNavAvailable( const bool bNavAvailable )
{
    if ( bNavAvailable != m_bNavAvailable )
    {
        m_bNavAvailable = bNavAvailable;
        emit navAvailableChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SettingsMenuContentFactory::isButtonPreviousVisible( void ) const
{
    return m_bButtonPreviousVisible;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsMenuContentFactory::setButtonPreviousVisible( const bool bVisible )
{
    if ( m_bButtonPreviousVisible != bVisible )
    {
        m_bButtonPreviousVisible = bVisible;

        emit buttonPreviousVisibleChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool SettingsMenuContentFactory::isButtonOkVisible( void ) const
{
    return m_bButtonOkVisible;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsMenuContentFactory::setButtonOkVisible( const bool bVisible )
{
    if ( m_bButtonOkVisible != bVisible )
    {
        m_bButtonOkVisible = bVisible;

        emit buttonOkVisibleChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

QString SettingsMenuContentFactory::menuTitle()
{
    return m_strMenuTitle;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsMenuContentFactory::setMenuTitle( const QString & strMenuTitle )
{
    if ( strMenuTitle != m_strMenuTitle )
    {
        m_strMenuTitle = strMenuTitle;

        emit menuTitleChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsMenuContentFactory::buildUpMainState()
{
    setMenuTitle( "Settings" );
    setNavAvailable( false );
    setButtonPreviousVisible( false );
    setButtonOkVisible( false );
    clearMenuList();

    SettingsMenu * pMenu = new SettingsMenu( this );
    pMenu->setMenuName( "Service Settings" );
    pMenu->setMenuType( SettingsMenu::ENTRYTYPE_MAINENTRY );
    pMenu->setStringValue( "" );
    pMenu->setToggleSwitchValue( false );
    pMenu->setLinkedSettingsState( SettingsStatemachine::SETTING_SERVICE );
    m_theMenuList.append( pMenu );

    pMenu = new SettingsMenu( this );
    pMenu->setMenuName( "User Management" );
    pMenu->setMenuType( SettingsMenu::ENTRYTYPE_MAINENTRY );
    pMenu->setStringValue( "" );
    pMenu->setToggleSwitchValue( false );
    pMenu->setLinkedSettingsState( SettingsStatemachine::SETTING_USER );
    m_theMenuList.append( pMenu );

    pMenu = new SettingsMenu( this );
    pMenu->setMenuName( "General Settings" );
    pMenu->setMenuType( SettingsMenu::ENTRYTYPE_MAINENTRY );
    pMenu->setStringValue( "" );
    pMenu->setToggleSwitchValue( false );
    pMenu->setLinkedSettingsState( SettingsStatemachine::SETTING_SYSTEM );
    m_theMenuList.append( pMenu );

    emit qlpMenuListChanged();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsMenuContentFactory::buildUpService()
{
    setMenuTitle( "Service" );
    clearMenuList();

    SettingsSerializer::UserRole nUserRole = m_pSettingsSerializer->currentLoggedInUserRole();

    SettingsMenu * pMenu = new SettingsMenu( this );
    pMenu->setMenuName( "Disc Calibration" );
    pMenu->setMenuType( SettingsMenu::ENTRYTYPE_NOVALUE );
    pMenu->setLinkedSettingsState( SettingsStatemachine::SETTING_SERVICE_DISC_CALIB );
    pMenu->setUserRightsObtained( ( nUserRole == SettingsSerializer::UserRole::StoreOwner )
                                  || ( nUserRole == SettingsSerializer::UserRole::Technician ) );
    m_theMenuList.append( pMenu );

    pMenu = new SettingsMenu( this );
    pMenu->setMenuName( "Disc Usage Time" );
    pMenu->setMenuType( SettingsMenu::ENTRYTYPE_NOVALUE );
    pMenu->setLinkedSettingsState( SettingsStatemachine::SETTING_SERVICE_DISC_USAGE_TIME );
    pMenu->setUserRightsObtained( true );
    m_theMenuList.append( pMenu );

    pMenu = new SettingsMenu( this );
    pMenu->setMenuName( "Display Test" );
    pMenu->setMenuType( SettingsMenu::ENTRYTYPE_NOVALUE );
    pMenu->setStringValue( "" );
    pMenu->setToggleSwitchValue( false );
    pMenu->setLinkedSettingsState( SettingsStatemachine::SETTING_SERVICE_DISPLAY_TEST );
    pMenu->setUserRightsObtained( ( nUserRole == SettingsSerializer::UserRole::StoreOwner )
                                  || ( nUserRole == SettingsSerializer::UserRole::Technician ) );
    m_theMenuList.append( pMenu );

    pMenu = new SettingsMenu( this );
    pMenu->setMenuName( "Motor Test" );
    pMenu->setMenuType( SettingsMenu::ENTRYTYPE_NOVALUE );
    pMenu->setStringValue( "" );
    pMenu->setToggleSwitchValue( false );
    pMenu->setLinkedSettingsState( SettingsStatemachine::SETTING_SERVICE_MOTOR_TEST );
    pMenu->setUserRightsObtained( nUserRole == SettingsSerializer::UserRole::Technician );
    m_theMenuList.append( pMenu );


    pMenu = new SettingsMenu( this );
    pMenu->setMenuName( "DDD Test" );
    pMenu->setMenuType( SettingsMenu::ENTRYTYPE_NOVALUE );
    pMenu->setStringValue( "" );
    pMenu->setToggleSwitchValue( false );
    pMenu->setLinkedSettingsState( SettingsStatemachine::SETTING_SERVICE_DDD_TEST );
    pMenu->setUserRightsObtained( ( nUserRole == SettingsSerializer::UserRole::StoreOwner )
                                  || ( nUserRole == SettingsSerializer::UserRole::Technician ) );
    m_theMenuList.append( pMenu );

    pMenu = new SettingsMenu( this );
    pMenu->setMenuName( "MCU Config" );
    pMenu->setMenuType( SettingsMenu::ENTRYTYPE_NOVALUE );
    pMenu->setStringValue( "" );
    pMenu->setToggleSwitchValue( false );
    pMenu->setLinkedSettingsState( SettingsStatemachine::SETTING_SERVICE_MCU_CONFIG );
    pMenu->setUserRightsObtained( nUserRole == SettingsSerializer::UserRole::Technician );
    m_theMenuList.append( pMenu );

    pMenu = new SettingsMenu( this );
    pMenu->setMenuName( "System Setup" );
    pMenu->setMenuType( SettingsMenu::ENTRYTYPE_NOVALUE );
    pMenu->setStringValue( "" );
    pMenu->setToggleSwitchValue( false );
    pMenu->setLinkedSettingsState( SettingsStatemachine::SETTING_SERVICE_SYSTEM_SETUP );
    pMenu->setUserRightsObtained( nUserRole == SettingsSerializer::UserRole::Technician );
    m_theMenuList.append( pMenu );

    emit qlpMenuListChanged();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsMenuContentFactory::buildUpCustomize()
{
    setMenuTitle( "Customize" );
    setButtonPreviousVisible( true );
    setButtonOkVisible( false );
    clearMenuList();

    SettingsSerializer::UserRole nUserRole = m_pSettingsSerializer->currentLoggedInUserRole();

    SettingsMenu * pMenu = new SettingsMenu( this );
    pMenu->setMenuName( "Time Mode" );
    pMenu->setMenuType( SettingsMenu::ENTRYTYPE_TOGGLE_SWITCH );
    pMenu->setStringValue( "" );
    pMenu->setToggleSwitchValue( m_pSettingsSerializer->timeModeIsEnabled() );
    pMenu->setLinkedSettingsState( SettingsStatemachine::SETTING_INVALID );
    pMenu->setToggleSwitchType( SettingsMenu::TOGGLE_SWITCH_FOR_TIMEMODE_ENABLING );
    pMenu->setUserRightsObtained( ( nUserRole == SettingsSerializer::UserRole::StoreOwner )
                                  || ( nUserRole == SettingsSerializer::UserRole::Technician ) );
    m_theMenuList.append( pMenu );

    pMenu = new SettingsMenu( this );
    pMenu->setMenuName( "Library Mode" );
    pMenu->setMenuType( SettingsMenu::ENTRYTYPE_TOGGLE_SWITCH );
    pMenu->setStringValue( "" );
    pMenu->setToggleSwitchValue( m_pSettingsSerializer->libraryModeIsEnabled() );
    pMenu->setLinkedSettingsState( SettingsStatemachine::SETTING_INVALID );
    pMenu->setToggleSwitchType( SettingsMenu::TOGGLE_SWITCH_FOR_LIBRARYMODE_ENABLING );
    pMenu->setUserRightsObtained( ( nUserRole == SettingsSerializer::UserRole::StoreOwner )
                                  || ( nUserRole == SettingsSerializer::UserRole::Technician ) );
    m_theMenuList.append( pMenu );

    pMenu = new SettingsMenu( this );
    pMenu->setMenuName( "Enable AGSA" );
    pMenu->setMenuType( SettingsMenu::ENTRYTYPE_TOGGLE_SWITCH );
    pMenu->setStringValue( "" );
    pMenu->setToggleSwitchValue( m_pSettingsSerializer->agsaEnabled() );
    pMenu->setLinkedSettingsState( SettingsStatemachine::SETTING_INVALID );
    pMenu->setToggleSwitchType( SettingsMenu::TOGGLE_SWITCH_FOR_AGSA_ENABLE );
    pMenu->setUserRightsObtained( ( nUserRole == SettingsSerializer::UserRole::StoreOwner )
                                  || ( nUserRole == SettingsSerializer::UserRole::Technician ) );
    m_theMenuList.append( pMenu );

    pMenu = new SettingsMenu( this );
    pMenu->setMenuName( "AGSA Delay" );
    pMenu->setMenuType( SettingsMenu::ENTRYTYPE_STRING );
    pMenu->setStringValue( QString( "%1s" ).arg( m_pSettingsSerializer->agsaStartDelaySec() ) );
    pMenu->setToggleSwitchValue( false );
    pMenu->setLinkedSettingsState( SettingsStatemachine::SETTING_CUSTOMIZE_AGSA_DELAY );
    pMenu->setUserRightsObtained( ( nUserRole == SettingsSerializer::UserRole::StoreOwner )
                                  || ( nUserRole == SettingsSerializer::UserRole::Technician ) );
    m_theMenuList.append( pMenu );

    pMenu = new SettingsMenu( this );
    pMenu->setMenuName( "Show PDU Time" );
    pMenu->setMenuType( SettingsMenu::ENTRYTYPE_TOGGLE_SWITCH );
    pMenu->setStringValue( "" );
    pMenu->setToggleSwitchValue( m_pSettingsSerializer->showDosingTime() );
    pMenu->setLinkedSettingsState( SettingsStatemachine::SETTING_INVALID );
    pMenu->setToggleSwitchType( SettingsMenu::TOGGLE_SWITCH_FOR_DOSING_TIME );
    pMenu->setUserRightsObtained( ( nUserRole == SettingsSerializer::UserRole::StoreOwner )
                                  || ( nUserRole == SettingsSerializer::UserRole::Technician ) );
    m_theMenuList.append( pMenu );

    pMenu = new SettingsMenu( this );
    pMenu->setMenuName( "Show Spotlight" );
    pMenu->setMenuType( SettingsMenu::ENTRYTYPE_TOGGLE_SWITCH );
    pMenu->setStringValue( "" );
    pMenu->setToggleSwitchValue( m_pSettingsSerializer->showSpotLight() );
    pMenu->setLinkedSettingsState( SettingsStatemachine::SETTING_INVALID );
    pMenu->setToggleSwitchType( SettingsMenu::TOGGLE_SWITCH_FOR_SHOWSPOT );
    pMenu->setUserRightsObtained( ( nUserRole == SettingsSerializer::UserRole::NoRole )
                                  || ( nUserRole == SettingsSerializer::UserRole::StoreOwner )
                                  || ( nUserRole == SettingsSerializer::UserRole::Technician ) );
    m_theMenuList.append( pMenu );

    pMenu = new SettingsMenu( this );
    pMenu->setMenuName( "Standby Time" );
    pMenu->setMenuType( SettingsMenu::ENTRYTYPE_STRING );
    pMenu->setStringValue( m_pSettingsSerializer->stdbyTimeIndexToString( m_pSettingsSerializer->stdbyTimeIndex() ) );
    pMenu->setToggleSwitchValue( false );
    pMenu->setLinkedSettingsState( SettingsStatemachine::SETTING_CUSTOMIZE_STDBY_ADJ );
    pMenu->setUserRightsObtained( ( nUserRole == SettingsSerializer::UserRole::StoreOwner )
                                  || ( nUserRole == SettingsSerializer::UserRole::Technician ) );
    m_theMenuList.append( pMenu );

    pMenu = new SettingsMenu( this );
    pMenu->setMenuName( "Settings Exit Time" );
    pMenu->setMenuType( SettingsMenu::ENTRYTYPE_STRING );
    pMenu->setStringValue( m_pSettingsSerializer->settingsModeExitTimeIndexToString( m_pSettingsSerializer->settingsModeExitTimeIndex() ) );
    pMenu->setToggleSwitchValue( false );
    pMenu->setLinkedSettingsState( SettingsStatemachine::SETTING_CUSTOMIZE_SETTINGS_MODE_EXIT_TIMEOUT_ADJ );
    pMenu->setUserRightsObtained( ( nUserRole == SettingsSerializer::UserRole::StoreOwner )
                                  || ( nUserRole == SettingsSerializer::UserRole::Technician ) );
    m_theMenuList.append( pMenu );

    pMenu = new SettingsMenu( this );
    pMenu->setMenuName( "Brightness" );
    pMenu->setMenuType( SettingsMenu::ENTRYTYPE_STRING );
    pMenu->setStringValue( QString::number( m_pSettingsSerializer->displayBrightness() ) );
    pMenu->setToggleSwitchValue( false );
    pMenu->setLinkedSettingsState( SettingsStatemachine::SETTING_CUSTOMIZE_BRIGHTNESS );
    pMenu->setUserRightsObtained( ( nUserRole == SettingsSerializer::UserRole::NoRole )
                                  || ( nUserRole == SettingsSerializer::UserRole::StoreOwner )
                                  || ( nUserRole == SettingsSerializer::UserRole::Technician ) );
    m_theMenuList.append( pMenu );

    pMenu = new SettingsMenu( this );
    pMenu->setMenuName( "Temperature" );
    pMenu->setMenuType( SettingsMenu::ENTRYTYPE_NOVALUE );
    pMenu->setStringValue( "" );
    pMenu->setToggleSwitchValue( false );
    pMenu->setLinkedSettingsState( SettingsStatemachine::SETTING_CUSTOMIZE_TEMP );
    pMenu->setUserRightsObtained( ( nUserRole == SettingsSerializer::UserRole::StoreOwner )
                                  || ( nUserRole == SettingsSerializer::UserRole::Technician ) );
    m_theMenuList.append( pMenu );

    emit qlpMenuListChanged();

}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsMenuContentFactory::buildUpCustomizeTemp()
{
    setMenuTitle( "Temperature" );
    setButtonPreviousVisible( true );
    setButtonOkVisible( false );
    clearMenuList();

    SettingsSerializer::UserRole nUserRole = m_pSettingsSerializer->currentLoggedInUserRole();

    SettingsMenu * pMenu = new SettingsMenu( this );
    pMenu->setMenuName( "Show Temp" );
    pMenu->setMenuType( SettingsMenu::ENTRYTYPE_TOGGLE_SWITCH );
    pMenu->setStringValue( "" );
    pMenu->setToggleSwitchValue( m_pSettingsSerializer->showTemp() );
    pMenu->setLinkedSettingsState( SettingsStatemachine::SETTING_INVALID );
    pMenu->setToggleSwitchType( SettingsMenu::TOGGLE_SWITCH_FOR_SHOWTEMP );
    pMenu->setUserRightsObtained( ( nUserRole == SettingsSerializer::UserRole::StoreOwner )
                                  || ( nUserRole == SettingsSerializer::UserRole::Technician ) );
    m_theMenuList.append( pMenu );


    pMenu = new SettingsMenu( this );
    pMenu->setMenuName( "Temp. Warning" );
    pMenu->setMenuType( SettingsMenu::ENTRYTYPE_TOGGLE_SWITCH );
    pMenu->setStringValue( "" );
    pMenu->setToggleSwitchValue( m_pSettingsSerializer->showTempWarning() );
    pMenu->setLinkedSettingsState( SettingsStatemachine::SETTING_INVALID );
    pMenu->setToggleSwitchType( SettingsMenu::TOGGLE_SWITCH_FOR_WARNINGTEMP );
    pMenu->setUserRightsObtained( ( nUserRole == SettingsSerializer::UserRole::StoreOwner )
                                  || ( nUserRole == SettingsSerializer::UserRole::Technician ) );
    m_theMenuList.append( pMenu );

    pMenu = new SettingsMenu( this );
    pMenu->setMenuName( "Warning Thold." );
    pMenu->setMenuType( SettingsMenu::ENTRYTYPE_STRING );
    pMenu->setStringValue( m_pSettingsSerializer->tempToString( m_pSettingsSerializer->getTempThresValueX100() ) );
    pMenu->setToggleSwitchValue( false );
    pMenu->setLinkedSettingsState( SettingsStatemachine::SETTING_CUSTOMIZE_TEMP_THRESH );
    pMenu->setMenuChoosable( m_pSettingsSerializer->showTempWarning() );
    pMenu->setUserRightsObtained( ( nUserRole == SettingsSerializer::UserRole::StoreOwner )
                                  || ( nUserRole == SettingsSerializer::UserRole::Technician ) );
    m_theMenuList.append( pMenu );

    emit qlpMenuListChanged();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsMenuContentFactory::buildUpSystemTempUnit()
{
    setMenuTitle( "Temperature Unit" );
    setButtonPreviousVisible( true );
    setButtonOkVisible( true );

    clearMenuList();
    SettingsMenu * pMenu = new SettingsMenu( this );
    pMenu->setMenuName( "" );
    pMenu->setMenuType( SettingsMenu::ENTRYTYPE_ACTION );
    pMenu->setStringValue( "" );
    pMenu->setToggleSwitchValue( false );
    pMenu->setLinkedSettingsState( SettingsStatemachine::SETTING_INVALID );
    pMenu->setActionType( SettingsMenu::ACTIONTYPE_RADIOBUTTON );
    pMenu->setActionValue( QString::number( static_cast<int>( m_pSettingsSerializer->tempUnit() ) ) );
    m_theMenuList.append( pMenu );

    emit qlpMenuListChanged();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsMenuContentFactory::buildUpCustomizeTempThresh()
{
    setMenuTitle( "Warning Threshold" );
    setButtonPreviousVisible( true );
    setButtonOkVisible( true );
    clearMenuList();

    SettingsMenu * pMenu = new SettingsMenu( this );
    pMenu->setMenuName( "" );
    pMenu->setMenuType( SettingsMenu::ENTRYTYPE_ACTION );
    pMenu->setStringValue( "" );
    pMenu->setToggleSwitchValue( false );
    pMenu->setLinkedSettingsState( SettingsStatemachine::SETTING_INVALID );
    pMenu->setActionType( SettingsMenu::ACTIONTYPE_SLIDER_ADJUSTMENT );
    pMenu->setActionValue( QString::number( m_pSettingsSerializer->getTempThresValueX100() ) );
    m_theMenuList.append( pMenu );

    emit qlpMenuListChanged();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsMenuContentFactory::buildUpCustomizeBrightness()
{
    setMenuTitle( "Brightness" );
    setButtonPreviousVisible( true );
    setButtonOkVisible( true );

    clearMenuList();
    SettingsMenu * pMenu = new SettingsMenu( this );
    pMenu->setMenuName( "" );
    pMenu->setMenuType( SettingsMenu::ENTRYTYPE_ACTION );
    pMenu->setStringValue( "" );
    pMenu->setToggleSwitchValue( false );
    pMenu->setLinkedSettingsState( SettingsStatemachine::SETTING_INVALID );
    pMenu->setActionType( SettingsMenu::ACTIONTYPE_SLIDER_ADJUSTMENT );
    pMenu->setActionValue( QString::number( m_pSettingsSerializer->displayBrightness() ) );
    m_theMenuList.append( pMenu );

    emit qlpMenuListChanged();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsMenuContentFactory::buildUpServiceDiscCalibration()
{
    setMenuTitle( "Disc Calibration" );
    clearMenuList();

    SettingsMenu * pMenu = new SettingsMenu( this );
    pMenu->setMenuName( "Disc Calibration" );
    pMenu->setMenuType( SettingsMenu::ENTRYTYPE_ACTION );
    pMenu->setActionType( SettingsMenu::ACTIONTYPE_CALIB );
    pMenu->setLinkedSettingsState( SettingsStatemachine::SETTING_INVALID );
    m_theMenuList.append( pMenu );

    emit qlpMenuListChanged();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsMenuContentFactory::buildUpServiceDiscUsageTime()
{
    setMenuTitle( "Disc Usage Time" );
    clearMenuList();

    SettingsMenu * pMenu = new SettingsMenu( this );
    pMenu->setMenuName( "Disc Usage Time" );
    pMenu->setMenuType( SettingsMenu::ENTRYTYPE_ACTION );
    pMenu->setActionType( SettingsMenu::ACTIONTYPE_DISK_USAGE_TIME );
    pMenu->setLinkedSettingsState( SettingsStatemachine::SETTING_INVALID );
    m_theMenuList.append( pMenu );

    emit qlpMenuListChanged();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsMenuContentFactory::buildUpServiceMotorTest()
{
    setMenuTitle( "Motor Test" );
    clearMenuList();
    setButtonOkVisible( false );

    SettingsMenu * pMenu = new SettingsMenu( this );
    pMenu->setMenuName( "AGSA Motor Test" );
    pMenu->setMenuType( SettingsMenu::ENTRYTYPE_ACTION );
    pMenu->setLinkedSettingsState( SettingsStatemachine::SETTING_INVALID );
    pMenu->setActionType( SettingsMenu::ACTIONTYPE_MOTOR_TEST );

    m_theMenuList.append( pMenu );

    emit qlpMenuListChanged();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsMenuContentFactory::buildUpServiceMotorTestAgsa()
{
    setMenuTitle( "AGSA Motor Test" );
    clearMenuList();
    setButtonOkVisible( false );

    SettingsMenu * pMenu = nullptr;

    pMenu = new SettingsMenu( this );
    pMenu->setMenuName( "DDD Value" );
    pMenu->setMenuType( SettingsMenu::ENTRYTYPE_STRING );
    pMenu->setStringValue( QString( "%1" ).arg( m_pMainStatemachine->dddCouple()->dddValue() ) );
    pMenu->setToggleSwitchValue( false );
    pMenu->setActionType( SettingsMenu::ACTIONTYPE_NONE );
    pMenu->setLinkedSettingsState( SettingsStatemachine::SETTING_INVALID );
    m_theMenuList.append( pMenu );

    pMenu = new SettingsMenu( this );
    pMenu->setMenuName( "Status" );
    pMenu->setMenuType( SettingsMenu::ENTRYTYPE_STRING );
    pMenu->setStringValue( m_pMainStatemachine->agsaControl()->isFailed() ? "FAILED" : "OK" );
    pMenu->setToggleSwitchValue( false );
    pMenu->setActionType( SettingsMenu::ACTIONTYPE_NONE );
    pMenu->setLinkedSettingsState( SettingsStatemachine::SETTING_INVALID );
    m_theMenuList.append( pMenu );

    pMenu = new SettingsMenu( this );
    pMenu->setMenuName( "Move to DDD" );
    pMenu->setMenuType( SettingsMenu::ENTRYTYPE_STRING );
    pMenu->setStringValue( QString::number( m_pMainStatemachine->agsaControl()->targetDddValue() ) );
    pMenu->setToggleSwitchValue( false );
    pMenu->setLinkedSettingsState( SettingsStatemachine::SETTING_SERVICE_MOTOR_TEST_AGSA_MOVE_TO_DDD );
    m_theMenuList.append( pMenu );

    pMenu = new SettingsMenu( this );
    pMenu->setMenuName( "Move Steps" );
    pMenu->setMenuType( SettingsMenu::ENTRYTYPE_ACTION );
    pMenu->setStringValue( "" );
    pMenu->setToggleSwitchValue( false );
    pMenu->setActionType( SettingsMenu::ACTIONTYPE_MOTOR_TEST_AGSA_MOVE_STEPS );
    pMenu->setLinkedSettingsState( SettingsStatemachine::SETTING_INVALID );
    m_theMenuList.append( pMenu );

    pMenu = new SettingsMenu( this );
    pMenu->setMenuName( "Stop" );
    pMenu->setMenuType( SettingsMenu::ENTRYTYPE_ACTION );
    pMenu->setStringValue( "" );
    pMenu->setToggleSwitchValue( false );
    pMenu->setActionType( SettingsMenu::ACTIONTYPE_MOTOR_TEST_AGSA_STOP );
    pMenu->setLinkedSettingsState( SettingsStatemachine::SETTING_INVALID );
    m_theMenuList.append( pMenu );

    pMenu = new SettingsMenu( this );
    pMenu->setMenuName( "Num. Steps" );
    pMenu->setMenuType( SettingsMenu::ENTRYTYPE_STRING );
    pMenu->setStringValue( QString::number( m_pMainStatemachine->agsaControl()->steps() ) );
    pMenu->setToggleSwitchValue( false );
    pMenu->setLinkedSettingsState( SettingsStatemachine::SETTING_SERVICE_MOTOR_TEST_AGSA_NUM_STEPS );
    m_theMenuList.append( pMenu );

    pMenu = new SettingsMenu( this );
    pMenu->setMenuName( "Live Mode" );
    pMenu->setMenuType( SettingsMenu::ENTRYTYPE_TOGGLE_SWITCH );
    pMenu->setStringValue( "" );
    pMenu->setToggleSwitchValue( m_pMainStatemachine->agsaControl()->testLiveMode() );
    pMenu->setLinkedSettingsState( SettingsStatemachine::SETTING_INVALID );
    pMenu->setToggleSwitchType( SettingsMenu::TOGGLE_SWITCH_FOR_AGSA_LIVE_MODE );
    m_theMenuList.append( pMenu );

    pMenu = new SettingsMenu( this );
    pMenu->setMenuName( "Blockage Detect" );
    pMenu->setMenuType( SettingsMenu::ENTRYTYPE_TOGGLE_SWITCH );
    pMenu->setStringValue( "" );
    pMenu->setToggleSwitchValue( m_pMainStatemachine->agsaControl()->testBlockageDetect() );
    pMenu->setLinkedSettingsState( SettingsStatemachine::SETTING_INVALID );
    pMenu->setToggleSwitchType( SettingsMenu::TOGGLE_SWITCH_FOR_AGSA_BLOCKAGE_DETECT );
    m_theMenuList.append( pMenu );

    pMenu = new SettingsMenu( this );
    pMenu->setMenuName( "Frequency" );
    pMenu->setMenuType( SettingsMenu::ENTRYTYPE_STRING );
    pMenu->setStringValue( QString( "%1kHz" ).arg( m_pMainStatemachine->agsaControl()->runFrequency() / 1000.0, 4, 'f', 2, QLatin1Char( '0' ) ) );
    pMenu->setToggleSwitchValue( false );
    pMenu->setLinkedSettingsState( SettingsStatemachine::SETTING_SERVICE_MOTOR_TEST_AGSA_FREQUENCY );
    m_theMenuList.append( pMenu );

    emit qlpMenuListChanged();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsMenuContentFactory::buildUpServiceMotorTestAgsaMoveToDdd()
{
    setMenuTitle( "Move to DDD" );
    setButtonPreviousVisible( true );
    setButtonOkVisible( true );

    clearMenuList();

    SettingsMenu * pMenu = new SettingsMenu( this );
    pMenu->setMenuName( "" );
    pMenu->setMenuType( SettingsMenu::ENTRYTYPE_ACTION );
    pMenu->setStringValue( "" );
    pMenu->setToggleSwitchValue( false );
    pMenu->setLinkedSettingsState( SettingsStatemachine::SETTING_INVALID );
    pMenu->setActionType( SettingsMenu::ACTIONTYPE_SLIDER_ADJUSTMENT );
    pMenu->setActionValue( QString::number( m_pMainStatemachine->agsaControl()->targetDddValue() ) );
    m_theMenuList.append( pMenu );

    emit qlpMenuListChanged();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsMenuContentFactory::buildUpServiceMotorTestAgsaNumSteps()
{
    setMenuTitle( "AGSA Num. Steps" );
    setButtonPreviousVisible( true );
    setButtonOkVisible( true );

    clearMenuList();

    SettingsMenu * pMenu = new SettingsMenu( this );
    pMenu->setMenuName( "" );
    pMenu->setMenuType( SettingsMenu::ENTRYTYPE_ACTION );
    pMenu->setStringValue( "" );
    pMenu->setToggleSwitchValue( false );
    pMenu->setLinkedSettingsState( SettingsStatemachine::SETTING_INVALID );
    pMenu->setActionType( SettingsMenu::ACTIONTYPE_SLIDER_ADJUSTMENT );
    pMenu->setActionValue( QString::number( m_pMainStatemachine->agsaControl()->steps() ) );
    m_theMenuList.append( pMenu );

    emit qlpMenuListChanged();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsMenuContentFactory::buildUpServiceMotorTestAgsaFrequency()
{
    setMenuTitle( "AGSA Frequency" );
    setButtonPreviousVisible( true );
    setButtonOkVisible( true );

    clearMenuList();

    SettingsMenu * pMenu = new SettingsMenu( this );

    pMenu->setMenuName( "" );
    pMenu->setMenuType( SettingsMenu::ENTRYTYPE_ACTION );
    pMenu->setStringValue( "" );
    pMenu->setToggleSwitchValue( false );
    pMenu->setLinkedSettingsState( SettingsStatemachine::SETTING_INVALID );
    pMenu->setActionType( SettingsMenu::ACTIONTYPE_SLIDER_ADJUSTMENT );
    pMenu->setActionValue( QString::number( m_pMainStatemachine->agsaControl()->runFrequency() ) );

    m_theMenuList.append( pMenu );

    emit qlpMenuListChanged();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsMenuContentFactory::buildUpServiceMotorTestPdu()
{
    setMenuTitle( "PDU Motor Test" );
    clearMenuList();

    emit qlpMenuListChanged();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsMenuContentFactory::buildUpServiceMotorTestAgsaLongDuration()
{
    setMenuTitle( "AGSA Long Duration" );
    clearMenuList();

    SettingsMenu * pMenu = new SettingsMenu( this );
    pMenu->setMenuName( "Start" );
    pMenu->setMenuType( SettingsMenu::ENTRYTYPE_ACTION );
    pMenu->setStringValue( "" );
    pMenu->setToggleSwitchValue( false );
    pMenu->setActionType( SettingsMenu::ACTIONTYPE_MOTOR_TEST_AGSA_LONG_DURATION_START );
    pMenu->setLinkedSettingsState( SettingsStatemachine::SETTING_INVALID );
    m_theMenuList.append( pMenu );

    pMenu = new SettingsMenu( this );
    pMenu->setMenuName( "Stop" );
    pMenu->setMenuType( SettingsMenu::ENTRYTYPE_ACTION );
    pMenu->setStringValue( "" );
    pMenu->setToggleSwitchValue( false );
    pMenu->setActionType( SettingsMenu::ACTIONTYPE_MOTOR_TEST_AGSA_LONG_DURATION_STOP );
    pMenu->setLinkedSettingsState( SettingsStatemachine::SETTING_INVALID );
    m_theMenuList.append( pMenu );

    emit qlpMenuListChanged();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsMenuContentFactory::buildUpServiceDddTest()
{
    setMenuTitle( "DDD Sensors Test" );
    clearMenuList();
    setButtonOkVisible( false );

    SettingsMenu * pMenu = new SettingsMenu( this );
    pMenu->setMenuName( "DDD Sensors Test" );
    pMenu->setMenuType( SettingsMenu::ENTRYTYPE_ACTION );
    pMenu->setLinkedSettingsState( SettingsStatemachine::SETTING_INVALID );
    pMenu->setActionType( SettingsMenu::ACTIONTYPE_DDD_TEST );
    m_theMenuList.append( pMenu );

    emit qlpMenuListChanged();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsMenuContentFactory::buildUpServiceMcuConfig()
{
    setMenuTitle( "MCU Configuration" );
    clearMenuList();
    setButtonOkVisible( true );

    SettingsSerializer::UserRole nUserRole = m_pSettingsSerializer->currentLoggedInUserRole();

    SettingsMenu * pMenu = new SettingsMenu( this );
    pMenu->setMenuName( "Max.Mot.Sp." );
    pMenu->setMenuType( SettingsMenu::ENTRYTYPE_STRING );
    pMenu->setStringValue( QString( "%1" ).arg( m_pSettingsSerializer->mcuMaxMotorSpeed() ) );
    pMenu->setToggleSwitchValue( false );
    pMenu->setLinkedSettingsState( SettingsStatemachine::SETTING_SERVICE_MCU_CONFIG_MAX_MOTOR_SPEED );
    pMenu->setMenuChoosable( m_pSettingsSerializer->mcuMaxMotorSpeed() );
    pMenu->setUserRightsObtained( nUserRole == SettingsSerializer::UserRole::Technician );
    m_theMenuList.append( pMenu );

    pMenu = new SettingsMenu( this );
    pMenu->setMenuName( "Nom.Mot.Sp." );
    pMenu->setMenuType( SettingsMenu::ENTRYTYPE_STRING );
    pMenu->setStringValue( QString( "%1" ).arg( m_pSettingsSerializer->mcuNominalMotorSpeed() ) );
    pMenu->setToggleSwitchValue( false );
    pMenu->setLinkedSettingsState( SettingsStatemachine::SETTING_SERVICE_MCU_CONFIG_NOMINAL_MOTOR_SPEED );
    pMenu->setMenuChoosable( m_pSettingsSerializer->mcuNominalMotorSpeed() );
    pMenu->setUserRightsObtained( nUserRole == SettingsSerializer::UserRole::Technician );
    m_theMenuList.append( pMenu );

    pMenu = new SettingsMenu( this );
    pMenu->setMenuName( "Accel. Time" );
    pMenu->setMenuType( SettingsMenu::ENTRYTYPE_STRING );
    pMenu->setStringValue( QString( "%1ms" ).arg( m_pSettingsSerializer->mcuAccelerationTime() ) );
    pMenu->setToggleSwitchValue( false );
    pMenu->setLinkedSettingsState( SettingsStatemachine::SETTING_SERVICE_MCU_CONFIG_ACCELERATION_TIME );
    pMenu->setMenuChoosable( m_pSettingsSerializer->mcuAccelerationTime() );
    pMenu->setUserRightsObtained( nUserRole == SettingsSerializer::UserRole::Technician );
    m_theMenuList.append( pMenu );

    pMenu = new SettingsMenu( this );
    pMenu->setMenuName( "Decel. Time" );
    pMenu->setMenuType( SettingsMenu::ENTRYTYPE_STRING );
    pMenu->setStringValue( QString( "%1ms" ).arg( m_pSettingsSerializer->mcuDecelerationTime() ) );
    pMenu->setToggleSwitchValue( false );
    pMenu->setLinkedSettingsState( SettingsStatemachine::SETTING_SERVICE_MCU_CONFIG_DECELERATION_TIME );
    pMenu->setMenuChoosable( m_pSettingsSerializer->mcuDecelerationTime() );
    pMenu->setUserRightsObtained( nUserRole == SettingsSerializer::UserRole::Technician );
    m_theMenuList.append( pMenu );

    emit qlpMenuListChanged();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsMenuContentFactory::buildUpServiceMcuConfigMaxMotorSpeed()
{
    setMenuTitle( "Max. Motor Speed" );
    clearMenuList();
    setButtonPreviousVisible( true );
    setButtonOkVisible( true );

    SettingsMenu * pMenu = new SettingsMenu( this );
    pMenu->setMenuName( "" );
    pMenu->setMenuType( SettingsMenu::ENTRYTYPE_ACTION );
    pMenu->setStringValue( "" );
    pMenu->setToggleSwitchValue( false );
    pMenu->setLinkedSettingsState( SettingsStatemachine::SETTING_INVALID );
    pMenu->setActionType( SettingsMenu::ACTIONTYPE_SLIDER_ADJUSTMENT );
    pMenu->setActionValue( QString::number( m_pSettingsSerializer->mcuMaxMotorSpeed() ) );
    m_theMenuList.append( pMenu );

    emit qlpMenuListChanged();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsMenuContentFactory::buildUpServiceMcuConfigNominalMotorSpeed()
{
    setMenuTitle( "Nom. Motor Speed" );
    clearMenuList();
    setButtonPreviousVisible( true );
    setButtonOkVisible( true );

    SettingsMenu * pMenu = new SettingsMenu( this );
    pMenu->setMenuName( "" );
    pMenu->setMenuType( SettingsMenu::ENTRYTYPE_ACTION );
    pMenu->setStringValue( "" );
    pMenu->setToggleSwitchValue( false );
    pMenu->setLinkedSettingsState( SettingsStatemachine::SETTING_INVALID );
    pMenu->setActionType( SettingsMenu::ACTIONTYPE_SLIDER_ADJUSTMENT );
    pMenu->setActionValue( QString::number( m_pSettingsSerializer->mcuNominalMotorSpeed() ) );
    m_theMenuList.append( pMenu );

    emit qlpMenuListChanged();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsMenuContentFactory::buildUpServiceMcuConfigAccelerationTime()
{
    setMenuTitle( "Acceleration Time" );
    clearMenuList();
    setButtonPreviousVisible( true );
    setButtonOkVisible( true );

    SettingsMenu * pMenu = new SettingsMenu( this );
    pMenu->setMenuName( "" );
    pMenu->setMenuType( SettingsMenu::ENTRYTYPE_ACTION );
    pMenu->setStringValue( "" );
    pMenu->setToggleSwitchValue( false );
    pMenu->setLinkedSettingsState( SettingsStatemachine::SETTING_INVALID );
    pMenu->setActionType( SettingsMenu::ACTIONTYPE_SLIDER_ADJUSTMENT );
    pMenu->setActionValue( QString::number( m_pSettingsSerializer->mcuAccelerationTime() ) );
    m_theMenuList.append( pMenu );

    emit qlpMenuListChanged();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsMenuContentFactory::buildUpServiceMcuConfigDecelerationTime()
{
    setMenuTitle( "Deceleration Time" );
    clearMenuList();
    setButtonPreviousVisible( true );
    setButtonOkVisible( true );

    SettingsMenu * pMenu = new SettingsMenu( this );
    pMenu->setMenuName( "" );
    pMenu->setMenuType( SettingsMenu::ENTRYTYPE_ACTION );
    pMenu->setStringValue( "" );
    pMenu->setToggleSwitchValue( false );
    pMenu->setLinkedSettingsState( SettingsStatemachine::SETTING_INVALID );
    pMenu->setActionType( SettingsMenu::ACTIONTYPE_SLIDER_ADJUSTMENT );
    pMenu->setActionValue( QString::number( m_pSettingsSerializer->mcuDecelerationTime() ) );
    m_theMenuList.append( pMenu );

    emit qlpMenuListChanged();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsMenuContentFactory::buildUpServiceSystemSetup()
{
    setMenuTitle( "System Setup" );
    clearMenuList();

    SettingsMenu * pMenu = new SettingsMenu( this );
    pMenu->setMenuName( "" );
    pMenu->setMenuType( SettingsMenu::ENTRYTYPE_ACTION );
    pMenu->setStringValue( "" );
    pMenu->setToggleSwitchValue( false );
    pMenu->setActionType( SettingsMenu::ACTIONTYPE_SYSTEM_SETUP );
    pMenu->setLinkedSettingsState( SettingsStatemachine::SETTING_INVALID );
    m_theMenuList.append( pMenu );

    emit qlpMenuListChanged();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsMenuContentFactory::buildUpUserCpc()
{
    setMenuTitle( "Change Pin of" );
    clearMenuList();

    SettingsMenu * pMenu = new SettingsMenu( this );
    pMenu->setMenuName( "Store Owner" );
    pMenu->setMenuType( SettingsMenu::ENTRYTYPE_NOVALUE );
    pMenu->setStringValue( "" );
    pMenu->setToggleSwitchValue( false );
    pMenu->setLinkedSettingsState( SettingsStatemachine::SETTING_USER_CPC_STORE );
    m_theMenuList.append( pMenu );

    pMenu = new SettingsMenu( this );
    pMenu->setMenuName( "Technician" );
    pMenu->setMenuType( SettingsMenu::ENTRYTYPE_NOVALUE );
    pMenu->setStringValue( "" );
    pMenu->setToggleSwitchValue( false );
    pMenu->setLinkedSettingsState( SettingsStatemachine::SETTING_USER_CPC_TECH );
    m_theMenuList.append( pMenu );

    emit qlpMenuListChanged();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsMenuContentFactory::buildUpUserCpcStore()
{
    setMenuTitle( "" );
    clearMenuList();

    SettingsMenu * store = new SettingsMenu( this );
    store->setMenuName( "CPC" );
    store->setMenuType( SettingsMenu::ENTRYTYPE_ACTION );
    store->setStringValue( "" );
    store->setToggleSwitchValue( false );
    store->setActionType( SettingsMenu::ACTIONTYPE_CPC );
    store->setLinkedSettingsState( SettingsStatemachine::SETTING_INVALID );
    m_theMenuList.append( store );

    emit qlpMenuListChanged();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsMenuContentFactory::buildUpUserCpcTech()
{
    setMenuTitle( "" );
    clearMenuList();

    SettingsMenu * technician = new SettingsMenu( this );
    technician->setMenuName( "CPC" );
    technician->setMenuType( SettingsMenu::ENTRYTYPE_ACTION );
    technician->setStringValue( "" );
    technician->setToggleSwitchValue( false );
    technician->setActionType( SettingsMenu::ACTIONTYPE_CPC );
    technician->setLinkedSettingsState( SettingsStatemachine::SETTING_INVALID );
    m_theMenuList.append( technician );

    emit qlpMenuListChanged();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsMenuContentFactory::buildUpSystemWifi()
{
    setMenuTitle( "WIFI Settings" );
    clearMenuList();

    SettingsSerializer::UserRole nUserRole = m_pSettingsSerializer->currentLoggedInUserRole();

    SettingsMenu * pMenu = new SettingsMenu( this );
    pMenu->setMenuName( "WiFi" );
    pMenu->setMenuType( SettingsMenu::ENTRYTYPE_TOGGLE_SWITCH );
    pMenu->setStringValue( "" );
    pMenu->setToggleSwitchValue( m_pSettingsSerializer->isWifiEnabled() );
    pMenu->setLinkedSettingsState( SettingsStatemachine::SETTING_INVALID );
    pMenu->setToggleSwitchType( SettingsMenu::TOGGLE_SWITCH_FOR_WIFI );
    pMenu->setUserRightsObtained( ( nUserRole == SettingsSerializer::UserRole::StoreOwner )
                                  || ( nUserRole == SettingsSerializer::UserRole::Technician ) );
    m_theMenuList.append( pMenu );

    pMenu = new SettingsMenu( this );
    pMenu->setMenuName( "SSID" );
    pMenu->setMenuType( SettingsMenu::ENTRYTYPE_NOVALUE );
    pMenu->setStringValue( "" );
    pMenu->setToggleSwitchValue( false );
    pMenu->setLinkedSettingsState( SettingsStatemachine::SETTING_SYSTEM_WIFI_SETTING_SSID );
    pMenu->setUserRightsObtained( ( nUserRole == SettingsSerializer::UserRole::StoreOwner )
                                  || ( nUserRole == SettingsSerializer::UserRole::Technician ) );
    m_theMenuList.append( pMenu );

    pMenu = new SettingsMenu( this );
    pMenu->setMenuName( "Password" );
    pMenu->setMenuType( SettingsMenu::ENTRYTYPE_NOVALUE );
    pMenu->setStringValue( "" );
    pMenu->setToggleSwitchValue( false );
    pMenu->setLinkedSettingsState( SettingsStatemachine::SETTING_SYSTEM_WIFI_SETTING_PW );
    pMenu->setUserRightsObtained( ( nUserRole == SettingsSerializer::UserRole::StoreOwner )
                                  || ( nUserRole == SettingsSerializer::UserRole::Technician ) );
    m_theMenuList.append( pMenu );

    pMenu = new SettingsMenu( this );
    pMenu->setMenuName( "IP" );
    pMenu->setMenuType( SettingsMenu::ENTRYTYPE_NOVALUE );
    pMenu->setStringValue( "" );
    pMenu->setToggleSwitchValue( false );
    pMenu->setLinkedSettingsState( SettingsStatemachine::SETTING_SYSTEM_WIFI_SETTING_IP );
    pMenu->setMenuChoosable( true );
    pMenu->setUserRightsObtained( true );
    m_theMenuList.append( pMenu );

    emit qlpMenuListChanged();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsMenuContentFactory::buildUpSystemWifiIp()
{
    setMenuTitle( "IP" );
    clearMenuList();

    QString eth0IpStr = "UNDEF";
    QString wifiIpStr = "UNDEF";

    // eth0:
    QNetworkInterface           eth0Ip  = QNetworkInterface::interfaceFromName( "eth0" );
    QList<QNetworkAddressEntry> entries = eth0Ip.addressEntries();
    if ( !entries.isEmpty() )
    {
        QNetworkAddressEntry entry = entries.first();
        eth0IpStr = "LAN: " + entry.ip().toString();
    }
    else
    {
        eth0IpStr = "";
    }

    // wifi:
    if ( m_pSettingsSerializer->wifiDriver() )
    {
        switch ( m_pSettingsSerializer->wifiMode() )
        {
            case SettingsSerializer::WifiMode::MODE_OFF:
                wifiIpStr = "WiFi OFF";
                break;
            case SettingsSerializer::WifiMode::MODE_AP:
                wifiIpStr = QString( "WiFi AP: %1" ).arg( m_pSettingsSerializer->wifiDriver()->statusApIp() );
                break;
            case SettingsSerializer::WifiMode::MODE_STA:
                wifiIpStr = QString( "WiFi STA: %1" ).arg( m_pSettingsSerializer->wifiDriver()->statusStaIp() );
                break;
            case SettingsSerializer::WifiMode::MODE_AP_STA:
                wifiIpStr = QString( "WiFi AP: %1\nWiFi STA: %2" ).arg( m_pSettingsSerializer->wifiDriver()->statusApIp(), m_pSettingsSerializer->wifiDriver()->statusStaIp() );
                break;
        }
    }

    SettingsMenu * pMenu = new SettingsMenu( this );
    pMenu->setMenuName( "" );
    pMenu->setMenuType( SettingsMenu::ENTRYTYPE_ACTION );
    pMenu->setActionType( SettingsMenu::ACTIONTYPE_TEXTBOX );
    pMenu->setActionValue( ( !eth0IpStr.isEmpty() ) ? wifiIpStr + "\n" + eth0IpStr : wifiIpStr );
    pMenu->setStringValue( "" );
    pMenu->setToggleSwitchType( SettingsMenu::TOGGLE_SWITCH_FOR_NONE );
    pMenu->setToggleSwitchValue( false );
    pMenu->setLinkedSettingsState( SettingsStatemachine::SETTING_INVALID );
    m_theMenuList.append( pMenu );

    emit qlpMenuListChanged();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsMenuContentFactory::buildUpSystemWifiPassword()
{
    setMenuTitle( "WIFI Password" );
    clearMenuList();

    SettingsMenu * pMenu = new SettingsMenu( this );
    pMenu->setMenuName( "" );
    pMenu->setMenuType( SettingsMenu::ENTRYTYPE_ACTION );
    pMenu->setStringValue( "" );
    pMenu->setToggleSwitchValue( false );
    pMenu->setLinkedSettingsState( SettingsStatemachine::SETTING_INVALID );
    pMenu->setActionType( SettingsMenu::ACTIONTYPE_PW );
    pMenu->setActionValue( m_pSettingsSerializer->wifiApPassword() );
    m_theMenuList.append( pMenu );

    emit qlpMenuListChanged();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsMenuContentFactory::buildUpSystemWifiSsid()
{
    setMenuTitle( "SSID" );
    clearMenuList();

    SettingsMenu * pMenu = new SettingsMenu( this );
    pMenu->setMenuType( SettingsMenu::ENTRYTYPE_ACTION );
    pMenu->setActionType( SettingsMenu::ACTIONTYPE_TEXTBOX );
    pMenu->setActionValue( m_pSettingsSerializer->wifiApSsid() );
    m_theMenuList.append( pMenu );

    emit qlpMenuListChanged();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsMenuContentFactory::buildUpUser()
{
    setMenuTitle( "User Management" );
    clearMenuList();

    SettingsSerializer::UserRole nUserRole = m_pSettingsSerializer->currentLoggedInUserRole();

    SettingsMenu * pMenu = new SettingsMenu( this );
    pMenu->setMenuName( "Login" );
    pMenu->setMenuType( SettingsMenu::ENTRYTYPE_NOVALUE );
    pMenu->setStringValue( "" );
    pMenu->setToggleSwitchValue( false );
    pMenu->setLinkedSettingsState( SettingsStatemachine::SETTING_USER_LOGIN );
    pMenu->setUserRightsObtained( ( nUserRole == SettingsSerializer::UserRole::NoRole )
                                  || ( nUserRole == SettingsSerializer::UserRole::StoreOwner )
                                  || ( nUserRole == SettingsSerializer::UserRole::Technician ) );
    m_theMenuList.append( pMenu );

    pMenu = new SettingsMenu( this );
    pMenu->setMenuName( "Logout" );
    pMenu->setMenuType( SettingsMenu::ENTRYTYPE_NOVALUE );
    pMenu->setStringValue( "" );
    pMenu->setToggleSwitchValue( false );
    pMenu->setMenuChoosable( m_pSettingsSerializer->currentLoggedInUserRole() != SettingsSerializer::UserRole::NoRole );
    pMenu->setLinkedSettingsState( SettingsStatemachine::SETTING_USER_LOGOUT );
    pMenu->setUserRightsObtained( ( nUserRole == SettingsSerializer::UserRole::NoRole )
                                  || ( nUserRole == SettingsSerializer::UserRole::StoreOwner )
                                  || ( nUserRole == SettingsSerializer::UserRole::Technician ) );
    m_theMenuList.append( pMenu );

    pMenu = new SettingsMenu( this );
    pMenu->setMenuName( "Change Pin Code" );
    pMenu->setMenuType( SettingsMenu::ENTRYTYPE_NOVALUE );
    pMenu->setStringValue( "" );
    pMenu->setToggleSwitchValue( false );
    pMenu->setLinkedSettingsState( SettingsStatemachine::SETTING_USER_CPC );
    pMenu->setUserRightsObtained( ( nUserRole == SettingsSerializer::UserRole::StoreOwner )
                                  || ( nUserRole == SettingsSerializer::UserRole::Technician ) );
    m_theMenuList.append( pMenu );

    pMenu = new SettingsMenu( this );
    pMenu->setMenuName( "AutoLogout" );
    pMenu->setMenuType( SettingsMenu::ENTRYTYPE_TOGGLE_SWITCH );
    pMenu->setStringValue( "" );
    pMenu->setToggleSwitchValue( m_pSettingsSerializer->isUserAutoLogoutEnabled() );
    pMenu->setLinkedSettingsState( SettingsStatemachine::SETTING_INVALID );
    pMenu->setToggleSwitchType( SettingsMenu::TOGGLE_SWITCH_FOR_AUTOLOG );
    pMenu->setUserRightsObtained( ( nUserRole == SettingsSerializer::UserRole::StoreOwner ) );
    m_theMenuList.append( pMenu );

    emit qlpMenuListChanged();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsMenuContentFactory::buildUpUserLogin()
{
    setMenuTitle( "Login" );
    clearMenuList();

    SettingsMenu * pMenu = new SettingsMenu( this );
    pMenu->setMenuName( "Store Owner" );
    pMenu->setMenuType( SettingsMenu::ENTRYTYPE_RADIOBUTTON );
    pMenu->setStringValue( "" );
    pMenu->setToggleSwitchValue( m_pSettingsSerializer->currentLoggedInUserRole() == SettingsSerializer::UserRole::StoreOwner );
    pMenu->setLinkedSettingsState( SettingsStatemachine::SETTING_USER_STORE );
    m_theMenuList.append( pMenu );

    pMenu = new SettingsMenu( this );
    pMenu->setMenuName( "Technician" );
    pMenu->setMenuType( SettingsMenu::ENTRYTYPE_RADIOBUTTON );
    pMenu->setStringValue( "" );
    pMenu->setToggleSwitchValue( m_pSettingsSerializer->currentLoggedInUserRole() == SettingsSerializer::UserRole::Technician );
    pMenu->setLinkedSettingsState( SettingsStatemachine::SETTING_USER_TECHNICIAN );
    m_theMenuList.append( pMenu );

    emit qlpMenuListChanged();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsMenuContentFactory::buildUpUserLogout()
{
    setMenuTitle( "Logout" );
    clearMenuList();

    SettingsMenu * pMenu = new SettingsMenu( this );
    pMenu->setMenuName( "" );
    pMenu->setMenuType( SettingsMenu::ENTRYTYPE_ACTION );
    pMenu->setStringValue( "" );
    pMenu->setToggleSwitchValue( false );
    pMenu->setLinkedSettingsState( SettingsStatemachine::SETTING_INVALID );
    pMenu->setActionType( SettingsMenu::ACTIONTYPE_CONFIRM );
    pMenu->setActionValue( "You have logged out \n as " + m_pSettingsSerializer->userRoleToString( m_pSettingsSerializer->currentLoggedInUserRole() ) );
    m_theMenuList.append( pMenu );

    emit qlpMenuListChanged();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsMenuContentFactory::buildUpUserStore()
{
    setMenuTitle( "Enter Pin" );
    clearMenuList();

    SettingsMenu * pMenu = new SettingsMenu( this );
    pMenu->setMenuName( "" );
    pMenu->setMenuType( SettingsMenu::ENTRYTYPE_ACTION );
    pMenu->setStringValue( "" );
    pMenu->setToggleSwitchValue( false );
    pMenu->setActionType( SettingsMenu::ACTIONTYPE_LOGIN );
    pMenu->setLinkedSettingsState( SettingsStatemachine::SETTING_INVALID );
    m_theMenuList.append( pMenu );

    emit qlpMenuListChanged();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsMenuContentFactory::buildUpUserTech()
{
    setMenuTitle( "Enter Pin" );
    clearMenuList();

    SettingsMenu * pMenu = new SettingsMenu( this );
    pMenu->setMenuName( "" );
    pMenu->setMenuType( SettingsMenu::ENTRYTYPE_ACTION );
    pMenu->setStringValue( "" );
    pMenu->setToggleSwitchValue( false );
    pMenu->setActionType( SettingsMenu::ACTIONTYPE_LOGIN );
    pMenu->setLinkedSettingsState( SettingsStatemachine::SETTING_INVALID );
    m_theMenuList.append( pMenu );

    emit qlpMenuListChanged();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsMenuContentFactory::buildUpSystem()
{
    setMenuTitle( "System" );
    setButtonPreviousVisible( true );
    setButtonOkVisible( false );
    clearMenuList();

    SettingsSerializer::UserRole nUserRole = m_pSettingsSerializer->currentLoggedInUserRole();

    SettingsMenu * pMenu = new SettingsMenu( this );
    pMenu->setMenuName( "Wifi Settings" );
    pMenu->setMenuType( SettingsMenu::ENTRYTYPE_NOVALUE );
    pMenu->setStringValue( "" );
    pMenu->setToggleSwitchValue( false );
    pMenu->setLinkedSettingsState( SettingsStatemachine::SETTING_SYSTEM_WIFI_SETTING );
    pMenu->setUserRightsObtained( true );
    m_theMenuList.append( pMenu );

    pMenu = new SettingsMenu( this );
    pMenu->setMenuName( "Date" );
    pMenu->setMenuType( SettingsMenu::ENTRYTYPE_NOVALUE );
    pMenu->setStringValue( "" );
    pMenu->setToggleSwitchValue( false );
    pMenu->setLinkedSettingsState( SettingsStatemachine::SETTING_SYSTEM_DATE );
    pMenu->setUserRightsObtained( ( nUserRole == SettingsSerializer::UserRole::StoreOwner )
                                  || ( nUserRole == SettingsSerializer::UserRole::Technician ) );
    m_theMenuList.append( pMenu );

    pMenu = new SettingsMenu( this );
    pMenu->setMenuName( "Time" );
    pMenu->setMenuType( SettingsMenu::ENTRYTYPE_STRING );
    pMenu->setStringValue( "" );
    pMenu->setToggleSwitchValue( false );
    pMenu->setLinkedSettingsState( SettingsStatemachine::SETTING_SYSTEM_TIME );
    pMenu->setUserRightsObtained( ( nUserRole == SettingsSerializer::UserRole::StoreOwner )
                                  || ( nUserRole == SettingsSerializer::UserRole::Technician ) );
    m_theMenuList.append( pMenu );

    pMenu = new SettingsMenu( this );
    pMenu->setMenuName( "Start of Week" );
    pMenu->setMenuType( SettingsMenu::ENTRYTYPE_STRING );
    pMenu->setStringValue( m_pSettingsSerializer->isStartOfWeekSun() ? "SUN" : "MON" );
    pMenu->setToggleSwitchValue( false );
    pMenu->setLinkedSettingsState( SettingsStatemachine::SETTING_SYSTEM_START_OF_WEEK );
    pMenu->setUserRightsObtained( ( nUserRole == SettingsSerializer::UserRole::StoreOwner )
                                  || ( nUserRole == SettingsSerializer::UserRole::Technician ) );
    m_theMenuList.append( pMenu );

    pMenu = new SettingsMenu( this );
    pMenu->setMenuName( "Temp. Unit" );
    pMenu->setMenuType( SettingsMenu::ENTRYTYPE_STRING );
    pMenu->setStringValue( m_pSettingsSerializer->tempUnitToString( m_pSettingsSerializer->tempUnit() ) );
    pMenu->setToggleSwitchValue( false );
    pMenu->setLinkedSettingsState( SettingsStatemachine::SETTING_SYSTEM_TEMP_UNIT );
    pMenu->setUserRightsObtained( ( nUserRole == SettingsSerializer::UserRole::StoreOwner )
                                  || ( nUserRole == SettingsSerializer::UserRole::Technician ) );
    m_theMenuList.append( pMenu );

    pMenu = new SettingsMenu( this );
    pMenu->setMenuName( "Grindername" );
    pMenu->setMenuType( SettingsMenu::ENTRYTYPE_NOVALUE );
    pMenu->setStringValue( "" );
    pMenu->setToggleSwitchValue( false );
    pMenu->setLinkedSettingsState( SettingsStatemachine::SETTING_SYSTEM_GRINDERNAME );
    pMenu->setUserRightsObtained( ( nUserRole == SettingsSerializer::UserRole::StoreOwner )
                                  || ( nUserRole == SettingsSerializer::UserRole::Technician ) );
    m_theMenuList.append( pMenu );

    pMenu = new SettingsMenu( this );
    pMenu->setMenuName( "Factory Reset" );
    pMenu->setMenuType( SettingsMenu::ENTRYTYPE_NOVALUE );
    pMenu->setStringValue( "" );
    pMenu->setToggleSwitchValue( false );
    pMenu->setLinkedSettingsState( SettingsStatemachine::SETTING_SYSTEM_FACTORY_RESET );
    pMenu->setUserRightsObtained( ( nUserRole == SettingsSerializer::UserRole::StoreOwner )
                                  || ( nUserRole == SettingsSerializer::UserRole::Technician ) );
    m_theMenuList.append( pMenu );

    emit qlpMenuListChanged();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsMenuContentFactory::buildUpSystemDate()
{
    setMenuTitle( "Date" );
    setButtonPreviousVisible( true );
    setButtonOkVisible( true );
    clearMenuList();

    QDate currentDate = QDate::currentDate();

    SettingsMenu * pMenu = new SettingsMenu( this );
    pMenu->setMenuName( "" );
    pMenu->setMenuType( SettingsMenu::ENTRYTYPE_ACTION );
    pMenu->setStringValue( "" );
    pMenu->setToggleSwitchValue( false );
    pMenu->setLinkedSettingsState( SettingsStatemachine::SETTING_INVALID );
    pMenu->setActionType( SettingsMenu::ACTIONTYPE_DATE );
    pMenu->setActionValue( currentDate.toString( "d.M.yyyy" ) );
    m_theMenuList.append( pMenu );

    emit qlpMenuListChanged();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsMenuContentFactory::buildUpSystemTime()
{
    setMenuTitle( "Time" );
    setButtonPreviousVisible( true );
    setButtonOkVisible( true );
    clearMenuList();

    QTime currentTime = QTime::currentTime();

    SettingsMenu * pMenu = new SettingsMenu( this );
    pMenu->setMenuName( "" );
    pMenu->setMenuType( SettingsMenu::ENTRYTYPE_ACTION );
    pMenu->setStringValue( "" );
    pMenu->setToggleSwitchValue( false );
    pMenu->setLinkedSettingsState( SettingsStatemachine::SETTING_INVALID );
    pMenu->setActionType( SettingsMenu::ACTIONTYPE_TIME );
    pMenu->setActionValue( currentTime.toString( "h.m" ) );
    m_theMenuList.append( pMenu );

    emit qlpMenuListChanged();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsMenuContentFactory::buildUpSystemGrindername()
{
    setMenuTitle( "Grindername" );
    clearMenuList();

    SettingsMenu * pMenu = new SettingsMenu( this );
    pMenu->setMenuName( "Grindername" );
    pMenu->setMenuType( SettingsMenu::ENTRYTYPE_ACTION );
    pMenu->setStringValue( "" );
    pMenu->setToggleSwitchValue( false );
    pMenu->setLinkedSettingsState( SettingsStatemachine::SETTING_INVALID );
    pMenu->setActionType( SettingsMenu::ACTIONTYPE_PW );
    pMenu->setActionValue( m_pSettingsSerializer->grinderName() );
    m_theMenuList.append( pMenu );

    emit qlpMenuListChanged();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsMenuContentFactory::buildUpSystemDisplayTest()
{
    setMenuTitle( "Display Test" );
    clearMenuList();

    SettingsMenu * pMenu = new SettingsMenu( this );
    pMenu->setMenuName( "" );
    pMenu->setMenuType( SettingsMenu::ENTRYTYPE_ACTION );
    pMenu->setStringValue( "" );
    pMenu->setToggleSwitchValue( false );
    pMenu->setActionType( SettingsMenu::ACTIONTYPE_DISPLAY_TEST );
    pMenu->setLinkedSettingsState( SettingsStatemachine::SETTING_INVALID );
    m_theMenuList.append( pMenu );

    emit qlpMenuListChanged();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsMenuContentFactory::buildUpSystemFactoryReset()
{
    setMenuTitle( "Factory Reset" );
    setButtonOkVisible( false );
    setButtonPreviousVisible( false );
    clearMenuList();

    SettingsMenu * pMenu = new SettingsMenu( this );
    pMenu->setMenuName( "Factory Reset" );
    pMenu->setMenuType( SettingsMenu::ENTRYTYPE_ACTION );
    pMenu->setActionType( SettingsMenu::ACTIONTYPE_FACTORY_RESET );
    pMenu->setLinkedSettingsState( SettingsStatemachine::SETTING_INVALID );
    m_theMenuList.append( pMenu );

    emit qlpMenuListChanged();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsMenuContentFactory::buildUpSystemStartOfWeek()
{
    setMenuTitle( "Start of Week" );
    setButtonPreviousVisible( true );
    setButtonOkVisible( true );
    clearMenuList();

    SettingsMenu * pMenu = new SettingsMenu( this );
    pMenu->setMenuName( "" );
    pMenu->setMenuType( SettingsMenu::ENTRYTYPE_ACTION );
    pMenu->setStringValue( "" );
    pMenu->setToggleSwitchValue( false );
    pMenu->setLinkedSettingsState( SettingsStatemachine::SETTING_INVALID );
    pMenu->setActionType( SettingsMenu::ACTIONTYPE_RADIOBUTTON );
    pMenu->setActionValue( QString::number( m_pSettingsSerializer->isStartOfWeekSun() ? 1 : 0 ) );
    m_theMenuList.append( pMenu );

    emit qlpMenuListChanged();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsMenuContentFactory::buildUpCustomizeStandbyAdj()
{
    setMenuTitle( "Standby Time" );
    clearMenuList();

    SettingsMenu * pMenu = new SettingsMenu( this );
    pMenu->setMenuName( "" );
    pMenu->setMenuType( SettingsMenu::ENTRYTYPE_ACTION );
    pMenu->setStringValue( "" );
    pMenu->setToggleSwitchValue( false );
    pMenu->setLinkedSettingsState( SettingsStatemachine::SETTING_INVALID );
    pMenu->setActionType( SettingsMenu::ACTIONTYPE_STDBY );
    pMenu->setActionValue( QString::number( m_pSettingsSerializer->stdbyTimeIndex() ) );
    m_theMenuList.append( pMenu );


    emit qlpMenuListChanged();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsMenuContentFactory::buildUpCustomizeSettingsModeExitTimeoutAdj()
{
    setMenuTitle( "Settings Exit Time" );
    clearMenuList();

    SettingsMenu * pMenu = new SettingsMenu( this );
    pMenu->setMenuName( "" );
    pMenu->setMenuType( SettingsMenu::ENTRYTYPE_ACTION );
    pMenu->setStringValue( "" );
    pMenu->setToggleSwitchValue( false );
    pMenu->setLinkedSettingsState( SettingsStatemachine::SETTING_INVALID );
    pMenu->setActionType( SettingsMenu::ACTIONTYPE_SETTINGS_MODE_EXIT_TIMEOUT );
    pMenu->setActionValue( QString::number( m_pSettingsSerializer->settingsModeExitTimeIndex() ) );
    m_theMenuList.append( pMenu );

    emit qlpMenuListChanged();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsMenuContentFactory::buildUpCustomizeAgsaDelay()
{
    setMenuTitle( "AGSA Delay" );
    clearMenuList();

    SettingsMenu * pMenu = new SettingsMenu( this );
    pMenu->setMenuName( "" );
    pMenu->setMenuType( SettingsMenu::ENTRYTYPE_ACTION );
    pMenu->setStringValue( "" );
    pMenu->setToggleSwitchValue( false );
    pMenu->setLinkedSettingsState( SettingsStatemachine::SETTING_INVALID );
    pMenu->setActionType( SettingsMenu::ACTIONTYPE_AGSA_DELAY );
    pMenu->setActionValue( QString::number( m_pSettingsSerializer->agsaStartDelaySec() ) );
    m_theMenuList.append( pMenu );

    emit qlpMenuListChanged();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsMenuContentFactory::buildUpInfo()
{
    setMenuTitle( "Info" );
    clearMenuList();

    SettingsSerializer::UserRole nUserRole = m_pSettingsSerializer->currentLoggedInUserRole();

    SettingsMenu * pMenu = nullptr;

    pMenu = new SettingsMenu( this );
    pMenu->setMenuName( "Statistics" );
    pMenu->setMenuType( SettingsMenu::ENTRYTYPE_NOVALUE );
    pMenu->setStringValue( "" );
    pMenu->setToggleSwitchValue( false );
    pMenu->setLinkedSettingsState( SettingsStatemachine::SETTING_INFO_STATISTICS );
    pMenu->setUserRightsObtained( ( nUserRole == SettingsSerializer::UserRole::StoreOwner )
                                  || ( nUserRole == SettingsSerializer::UserRole::Technician ) );
    m_theMenuList.append( pMenu );

    pMenu = new SettingsMenu( this );
    pMenu->setMenuName( "Notifications" );
    pMenu->setMenuType( SettingsMenu::ENTRYTYPE_IMAGE );
    pMenu->setStringValue( m_pMainStatemachine->notificationCenter()->notAcknowledgedNotification() ? m_pMainStatemachine->notificationCenter()->notAcknowledgedNotification()->menuImage() : "" );
    pMenu->setToggleSwitchValue( false );
    pMenu->setLinkedSettingsState( SettingsStatemachine::SETTING_INFO_NOTIFICATION_CENTER );
    pMenu->setMenuChoosable( !m_pMainStatemachine->notificationCenter()->isEmpty() );
    pMenu->setUserRightsObtained( true );
    m_theMenuList.append( pMenu );

    pMenu = new SettingsMenu( this );
    pMenu->setMenuName( "Machine Info" );
    pMenu->setMenuType( SettingsMenu::ENTRYTYPE_NOVALUE );
    pMenu->setStringValue( "" );
    pMenu->setToggleSwitchValue( false );
    pMenu->setLinkedSettingsState( SettingsStatemachine::SETTING_INFO_MACHINE_INFO );
    pMenu->setUserRightsObtained( true );
    m_theMenuList.append( pMenu );

    pMenu = new SettingsMenu( this );
    pMenu->setMenuName( "Manual" );
    pMenu->setMenuType( SettingsMenu::ENTRYTYPE_NOVALUE );
    pMenu->setStringValue( "" );
    pMenu->setToggleSwitchValue( false );
    pMenu->setLinkedSettingsState( SettingsStatemachine::SETTING_INFO_OWNERS_MANUAL );
    pMenu->setUserRightsObtained( true );
    m_theMenuList.append( pMenu );

    pMenu = new SettingsMenu( this );
    pMenu->setMenuName( "Grinder Tutorial" );
    pMenu->setMenuType( SettingsMenu::ENTRYTYPE_NOVALUE );
    pMenu->setStringValue( "" );
    pMenu->setToggleSwitchValue( false );
    pMenu->setLinkedSettingsState( SettingsStatemachine::SETTING_INFO_GRINDER_TUTORIAL );
    pMenu->setUserRightsObtained( true );
    m_theMenuList.append( pMenu );

    pMenu = new SettingsMenu( this );
    pMenu->setMenuName( "PDU Tutorial" );
    pMenu->setMenuType( SettingsMenu::ENTRYTYPE_NOVALUE );
    pMenu->setStringValue( "" );
    pMenu->setToggleSwitchValue( false );
    pMenu->setLinkedSettingsState( SettingsStatemachine::SETTING_INFO_PDU_TUTORIAL );
    pMenu->setUserRightsObtained( true );
    m_theMenuList.append( pMenu );

    emit qlpMenuListChanged();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsMenuContentFactory::buildUpInfoStatistics()
{
    setMenuTitle( "Statistics" );
    clearMenuList();
    setButtonPreviousVisible( true );

    SettingsMenu * pMenu = new SettingsMenu( this );
    pMenu->setMenuType( SettingsMenu::ENTRYTYPE_ACTION );
    pMenu->setActionType( SettingsMenu::ACTIONTYPE_STATISTICS );
    pMenu->setLinkedSettingsState( SettingsStatemachine::SETTING_INVALID );
    pMenu->setMenuName( "Fake" );
    m_theMenuList.append( pMenu );

    emit qlpMenuListChanged();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsMenuContentFactory::buildUpInfoNotificationCenter()
{
    setMenuTitle( "Notification Center" );
    clearMenuList();
    setButtonPreviousVisible( true );

    SettingsMenu * pMenu = new SettingsMenu( this );
    pMenu->setMenuType( SettingsMenu::ENTRYTYPE_ACTION );
    pMenu->setActionType( SettingsMenu::ACTIONTYPE_NOTIFICATION_CENTER );
    pMenu->setLinkedSettingsState( SettingsStatemachine::SETTING_INVALID );
    pMenu->setMenuName( "Fake" );
    m_theMenuList.append( pMenu );

    emit qlpMenuListChanged();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsMenuContentFactory::buildUpInfoMachineInfo()
{
    setMenuTitle( "Machine Info" );
    clearMenuList();

    SettingsMenu * pMenu = new SettingsMenu( this );
    pMenu->setMenuName( "" );
    pMenu->setMenuType( SettingsMenu::ENTRYTYPE_ACTION );
    pMenu->setStringValue( "" );
    pMenu->setToggleSwitchValue( false );
    pMenu->setActionType( SettingsMenu::ACTIONTYPE_MACHINE_INFO );
    pMenu->setLinkedSettingsState( SettingsStatemachine::SETTING_INVALID );
    m_theMenuList.append( pMenu );

    emit qlpMenuListChanged();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsMenuContentFactory::buildUpInfoOwnersManual()
{
    setMenuTitle( "Manual" );
    clearMenuList();

    SettingsMenu * pMenu = new SettingsMenu( this );
    pMenu->setMenuName( "" );
    pMenu->setMenuType( SettingsMenu::ENTRYTYPE_ACTION );
    pMenu->setStringValue( "" );
    pMenu->setToggleSwitchValue( false );
    pMenu->setActionType( SettingsMenu::ACTIONTYPE_OWNERS_MANUAL );
    pMenu->setLinkedSettingsState( SettingsStatemachine::SETTING_INVALID );
    m_theMenuList.append( pMenu );

    emit qlpMenuListChanged();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsMenuContentFactory::buildUpInfoGrinderTutorial()
{
    setMenuTitle( "Tutorial" );
    clearMenuList();

    SettingsMenu * pMenu = new SettingsMenu( this );
    pMenu->setMenuName( "" );
    pMenu->setMenuType( SettingsMenu::ENTRYTYPE_ACTION );
    pMenu->setStringValue( "" );
    pMenu->setToggleSwitchValue( false );
    pMenu->setActionType( SettingsMenu::ACTIONTYPE_TUTORIAL );
    pMenu->setLinkedSettingsState( SettingsStatemachine::SETTING_INVALID );
    pMenu->setActionValue( "modesTutorial" );
    m_theMenuList.append( pMenu );

    emit qlpMenuListChanged();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsMenuContentFactory::buildUpInfoPduTutorial()
{
    setMenuTitle( "Tutorial" );
    clearMenuList();

    SettingsMenu * pMenu = new SettingsMenu( this );
    pMenu->setMenuName( "" );
    pMenu->setMenuType( SettingsMenu::ENTRYTYPE_ACTION );
    pMenu->setStringValue( "" );
    pMenu->setToggleSwitchValue( false );
    pMenu->setActionType( SettingsMenu::ACTIONTYPE_TUTORIAL );
    pMenu->setLinkedSettingsState( SettingsStatemachine::SETTING_INVALID );
    pMenu->setActionValue( "pduTutorial" );
    m_theMenuList.append( pMenu );

    emit qlpMenuListChanged();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SettingsMenuContentFactory::clearMenuList()
{
    for ( int i = 0; i < m_theMenuList.count(); i++ )
    {
        delete m_theMenuList[ i ];
    }

    m_theMenuList.clear();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

QQmlListProperty<SettingsMenu> SettingsMenuContentFactory::qlpMenuList() const
{
    return m_theQlpMenuList;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

