///////////////////////////////////////////////////////////////////////////////
///
/// @file QmlTypes.h
///
/// @brief Implementation file of namespace QmlTypes.
///
/// @author Ultratronik GmbH
///         Dornierstr. 9
///         D-82205 Gilching
///         Germany
///         http://www.ultratronik.de
///
/// @author written by Kai Uwe Broulik, kai.uwe.broulik@basyskom.com
///
/// @date 2023-02-24
///
/// @copyright Copyright 2021 by Hemro International AG
///            Hemro International AG
///            Länggenstrasse 34
///            CH 8184 Bachenbülach
///            Switzerland
///            Homepage: www.hemrogroup.com
///
///////////////////////////////////////////////////////////////////////////////

#include "QmlTypes.h"

#include <QQmlEngine>

#include "EkxSqliteTypes.h"
#include "ElapsedTimer.h"
#include "SpotLightDriver.h"
#include "StartStopLightDriver.h"
#include "LibraryRecipe.h"
#include "LibraryGrammage.h"
#include "MainStatemachine.h"
#include "Notification.h"
#include "TimeRecipe.h"
#include "RadioButtonMenu.h"
#include "RadioButtonMenuItem.h"
#include "StatisticsBoard.h"
#include "StatisticsBoardResult.h"
#include "SwUpdateController.h"
#include "SettingsStatemachine.h"
#include "SettingsMenu.h"

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void QmlTypes::registerTypes()
{

    // EkxSqliteTypes
    const char sqliteTypesUri[] = "EkxSqliteTypes";
    qmlRegisterUncreatableMetaObject( EkxSqliteTypes::staticMetaObject,
                                      sqliteTypesUri, 1, 0,
                                      "EkxSqliteTypes",
                                      "cannot register EkxSqliteTypes" );
    qmlProtectModule( sqliteTypesUri, 1 );

    // EkxMainstateEnum
    const char mainstateEnumUri[] = "EkxMainstateEnum";
    qmlRegisterUncreatableType<MainStatemachine>( mainstateEnumUri, 1, 0,
                                                  "EkxMainstateEnum",
                                                  "cannot register EkxMainstateEnum" );
    qmlProtectModule( mainstateEnumUri, 1 );

    // Recipes
    const char recipesUri[] = "com.hemrogroup.recipes";
    const auto recipesError = QStringLiteral( "Cannot create instances of %1 " );
    qmlRegisterUncreatableType<LibraryRecipe>( recipesUri, 1, 0, "LibraryRecipe", recipesError.arg( QStringLiteral( "LibraryRecipe" ) ) );
    qmlRegisterUncreatableType<LibraryGrammage>( recipesUri, 1, 0, "LibraryGrammage", recipesError.arg( QStringLiteral( "LibraryGrammage" ) ) );
    qmlRegisterUncreatableType<TimeRecipe>( recipesUri, 1, 0, "TimeRecipe", recipesError.arg( QStringLiteral( "TimeRecipe" ) ) );
    qmlProtectModule( recipesUri, 1 );

    // Drivers
    const char spotLightDriverUri[] = "com.hemrogroup.drivers";
    const auto spotLightDriverError = QStringLiteral( "Cannot create instances of %1 " );
    qmlRegisterUncreatableType<SpotLightDriver>( spotLightDriverUri, 1, 0,
                                                 "SpotLightDriver",
                                                 spotLightDriverError.arg( QStringLiteral( "SpotLightDriver" ) ) );
    const auto startStopLightDriverError = QStringLiteral( "Cannot create instances of %1 " );
    qmlRegisterUncreatableType<StartStopLightDriver>( spotLightDriverUri, 1, 0,
                                                      "StartStopLightDriver",
                                                      startStopLightDriverError.arg( QStringLiteral( "StartStopLightDriver" ) ) );
    qmlProtectModule( spotLightDriverUri, 1 );

    // Utils
    const char utilsUri[] = "com.hemrogroup.utils";
    qmlRegisterType<ElapsedTimer>( utilsUri, 1, 0, "ElapsedTimer" );
    qmlProtectModule( utilsUri, 1 );

    // EkxSwUpdateStates
    const char ekxSwUpdateStatesUri[] = "EkxSwUpdateStates";
    qmlRegisterUncreatableType<SwUpdateController>( ekxSwUpdateStatesUri, 1, 0,
                                                    "SwUpdateStatesEnum",
                                                    "cannot register SwUpdateStatesEnum" );
    qmlProtectModule( ekxSwUpdateStatesUri, 1 );

    // StatisticsBoardResult
    const char statisticsUri[] = "Statistics";
    qmlRegisterUncreatableType<StatisticsBoardResult>( statisticsUri, 1, 0,
                                                       "StatisticsBoardResult",
                                                       "cannot register StatisticsBoardResult" );

    // RadioButtonMenuItem
    qmlRegisterUncreatableType<RadioButtonMenuItem>( statisticsUri, 1, 0,
                                                     "RadioButtonMenuItem",
                                                     "cannot register RadioButtonMenuItem" );
    qmlProtectModule( statisticsUri, 1 );

    // SettingsStateEnum
    const char settingsStateEnumUri[] = "SettingsStateEnum";
    qmlRegisterUncreatableType<SettingsStatemachine>( settingsStateEnumUri, 1, 0,
                                                      "SettingsStateEnum",
                                                      "cannot register SettingsStateEnum" );
    qmlProtectModule( settingsStateEnumUri, 1 );

    qRegisterMetaType<SettingsStatemachine::SettingsState>( "SettingsStatemachine::SettingsState" );

    // SettingsStateEnum
    const char settingsSerializerEnumUri[] = "SettingsSerializerEnum";
    qmlRegisterUncreatableType<SettingsSerializer>( settingsSerializerEnumUri, 1, 0,
                                                    "SettingsSerializerEnum",
                                                    "cannot register SettingsSerializerEnum" );
    qmlProtectModule( settingsSerializerEnumUri, 1 );

    // SettingsStateEnum
    const char menuEnumsUri[] = "MenuEnums";
    qmlRegisterUncreatableType<SettingsMenu>( menuEnumsUri, 1, 0,
                                              "MenuEnums",
                                              "cannot register MenuEnums" );
    qmlProtectModule( menuEnumsUri, 1 );

    qRegisterMetaType<SettingsMenu::SettingEntryTypes>( "SettingsMenu::SettingEntryTypes" );
    qRegisterMetaType<SettingsMenu::ActionEntryTypes>( "SettingsMenu::ActionEntryTypes" );
    qRegisterMetaType<SettingsMenu::ToggleSwitchTypes>( "SettingsMenu::ToggleSwitchTypes" );

    // Anonymous types
    // Types that are created by C++ and passed to QML but never
    // created or referenced by name (no enums) on QML side.
    // TODO Qt 5.15 use qmlRegisterAnonymousType
    qmlRegisterAnonymousType<Notification>( "App", 1 );
    qmlRegisterAnonymousType<RadioButtonMenuItem>( "App", 1 );
    qmlRegisterAnonymousType<RadioButtonMenu>( "App", 1 );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////
