import QtQml 2.12
import QtQuick 2.12
import QtQuick.Controls 2.12

import "../../CommonItems"

import SettingsStateEnum 1.0
import MenuEnums 1.0

// we need a black background here, because otherwise we see flickering when
// enabling/disabling mode pages in swipe area
Rectangle
{
    id: idMenuPage

    width: 480
    height: 360
    color: "black"

    property var menuList: settingsMenuContentFactory.qlpMenuList  ; // interface to current radiobuttons data element in settings
    property bool mainMenu: false;
    property var menuEntryObject;
    property color pressColor: idMain.colorEkxTextWhite
    property  bool visualDebug: false;

    function createMenuObject( qmlFile, props )
    {
        let initialProperties = props || {};
        var success = true
        var menuEntry = Qt.createComponent( qmlFile );
        if (menuEntry.status === Component.Error) {
            console.error("Failed to create", qmlFile, menuEntry.errorString());
        }

        menuEntryObject = menuEntry.createObject( entryPlaceholder, initialProperties );

        if ( menuEntryObject === null )
        {
            menuEntry = Qt.createComponent("TextBox.qml");
            menuEntryObject = menuEntry.createObject(entryPlaceholder);

            menuEntryObject.textColor = "red"
            menuEntryObject.text = "Can't create " + qmlFile

            success = false
        }

        return success;
    }

    function triggerLogout( )
    {
        settingsStateMachine.logout();
        settingsStateMachine.back()
    }

    function cancelLogout()
    {
        settingsStateMachine.back();
    }

    function buildUpSystemMotorTestScreen()
    {
        if ( createMenuObject("MotorTestScreen.qml") )
        {
        }
    }

    function buildUpDiskFactoryResetScreen()
    {
        if(settingsStateMachine.settingsState === SettingsStateEnum.SETTING_SYSTEM_FACTORY_RESET )
        {
            if ( createMenuObject("ResetScreen.qml") )
            {
                // menuEntryObject.infoText = menuList[0].actionValue;
                // menuEntryObject.backFunction = settingsStateMachine.back;
                // menuEntryObject.entryType = settingsStateMachine.settingsState;
            }
        }
    }

    function buildUpDiskUsageScreen()
    {
        if ( createMenuObject("DiskUsageScreen.qml") )
        {
            //menuEntryObject.text = menuList[0].actionValue;
        }
    }

    function buildUpTextBox()
    {
        if ( createMenuObject("TextBox.qml") )
        {
            menuEntryObject.text = menuList[0].actionValue;
        }
    }

    function buildUpSliderAdjustment()
    {
        if ( createMenuObject("SliderAdjustment.qml") )
        {
            menuEntryObject.entryType = settingsStateMachine.settingsState;
            switch ( settingsStateMachine.settingsState )
            {
            case SettingsStateEnum.SETTING_CUSTOMIZE_BRIGHTNESS:
                menuEntryObject.progressText = "Brightness"
                menuEntryObject.unit = ""
                menuEntryObject.currentValueX100 = parseInt( menuList[0].actionValue ) * 100;
                menuEntryObject.minValue = displayController.minBrightness;
                menuEntryObject.maxValue = displayController.maxBrightness;
                menuEntryObject.step = 1;
                break;

            case SettingsStateEnum.SETTING_CUSTOMIZE_TEMP_THRESH:
                menuEntryObject.progressText = "Warning Thold"
                menuEntryObject.unit = settingsSerializer.isCelsius?"°C":"°F"
                menuEntryObject.currentValueX100 = settingsSerializer.tempThresValue;
                menuEntryObject.showRange = true
                menuEntryObject.minValue = settingsSerializer.tempThresLowerLimit;
                menuEntryObject.maxValue = settingsSerializer.tempThresUpperLimit;
                menuEntryObject.step = 1;
                break;

            case SettingsStateEnum.SETTING_SERVICE_MCU_CONFIG_MAX_MOTOR_SPEED:
                menuEntryObject.progressText = "Max. Motor Speed"
                menuEntryObject.unit = "rpm"
                menuEntryObject.currentValueX100 = settingsSerializer.mcuMaxMotorSpeed * 100;
                menuEntryObject.showRange = true
                menuEntryObject.minValue = settingsSerializer.minMcuMaxMotorSpeed;
                menuEntryObject.maxValue = settingsSerializer.maxMcuMaxMotorSpeed;
                menuEntryObject.step = 50;
                break;

            case SettingsStateEnum.SETTING_SERVICE_MCU_CONFIG_NOMINAL_MOTOR_SPEED:
                menuEntryObject.progressText = "Nom. Motor Speed"
                menuEntryObject.unit = "rpm"
                menuEntryObject.currentValueX100 = settingsSerializer.mcuNominalMotorSpeed * 100;
                menuEntryObject.showRange = true
                menuEntryObject.minValue = settingsSerializer.minMcuNominalMotorSpeed;
                menuEntryObject.maxValue = settingsSerializer.maxMcuNominalMotorSpeed;
                menuEntryObject.step = 50;
                break;

            case SettingsStateEnum.SETTING_SERVICE_MCU_CONFIG_ACCELERATION_TIME:
                menuEntryObject.progressText = "Acceleration Time"
                menuEntryObject.unit = "ms"
                menuEntryObject.currentValueX100 = settingsSerializer.mcuAccelerationTime * 100;
                menuEntryObject.showRange = true
                menuEntryObject.minValue = settingsSerializer.minMcuAccelerationTime;
                menuEntryObject.maxValue = settingsSerializer.maxMcuAccelerationTime;
                menuEntryObject.step = 50;
                break;

            case SettingsStateEnum.SETTING_SERVICE_MCU_CONFIG_DECELERATION_TIME:
                menuEntryObject.progressText = "Deceleration Time"
                menuEntryObject.unit = "ms"
                menuEntryObject.currentValueX100 = settingsSerializer.mcuDecelerationTime * 100;
                menuEntryObject.showRange = true
                menuEntryObject.minValue = settingsSerializer.minMcuDecelerationTime;
                menuEntryObject.maxValue = settingsSerializer.maxMcuDecelerationTime;
                menuEntryObject.step = 50;
                break;
            default:
                console.log("MenuPage.qml;buildUpProgressInput; implementation missing");
                break;
            }
        }
    }

    function buildUpComboInput()
    {
        if ( createMenuObject("RadioButtons.qml") )
        {
            menuEntryObject.entryType = settingsStateMachine.settingsState;
            if(settingsStateMachine.settingsState === SettingsStateEnum.SETTING_SYSTEM_TEMP_UNIT )
            {
                menuEntryObject.valueList = ["Celsius","Fahrenheit"]
                menuEntryObject.indexOfChoosenValue = parseInt( menuList[0].actionValue )
            }
            else if(settingsStateMachine.settingsState === SettingsStateEnum.SETTING_SYSTEM_START_OF_WEEK )
            {
                menuEntryObject.valueList = ["Monday","Sunday"]
                menuEntryObject.indexOfChoosenValue = parseInt( menuList[0].actionValue )
            }
        }
    }

    function buildUpCalibInput()
    {
        if ( createMenuObject( "DddCalibration.qml" ) )
        {
            menuEntryObject.entryType = settingsStateMachine.settingsState;
        }
    }

    function buildUpChangePinCodeInput()
    {
        if ( createMenuObject( "CpcPinInput.qml") )
        {
            menuEntryObject.entryType = settingsStateMachine.settingsState;
        }
    }

    function buildUpDisplayTest()
    {
        if ( createMenuObject( "ColorTestScreen.qml" ) )
        {
        }
    }

    function buildUpLogin()
    {
        if ( createMenuObject("Login.qml") )
        {
            menuEntryObject.entryType = settingsStateMachine.settingsState;
        }
    }

    function buildUpPasswordInput()
    {
        if( settingsStateMachine.settingsState === SettingsStateEnum.SETTING_SYSTEM_WIFI_SETTING_PW )
        {
            createMenuObject("PasswordInput.qml", {
                oldPassword: menuList[0].actionValue
            });
        }
        else if( settingsStateMachine.settingsState === SettingsStateEnum.SETTING_SYSTEM_GRINDERNAME )
        {
            createMenuObject("EditGrindername.qml", {
                oldGrindername: menuList[0].actionValue
            });
        }
    }

    function buildUpDate()
    {
        if ( createMenuObject( "DateAdjustment.qml" ) )
        {
            menuEntryObject.entryType = settingsStateMachine.settingsState;
            var date = menuList[0].actionValue.split(".");
            menuEntryObject.setDateStartValues(date[0],date[1],date[2])
        }
    }

    function buildUpTime()
    {
        if ( createMenuObject("TimeAdjustment.qml") )
        {
            menuEntryObject.entryType = settingsStateMachine.settingsState;
            var time = menuList[0].actionValue.split(".");
            menuEntryObject.setTimeStartValues(time[0],time[1])
        }
    }

    function buildUpConfirm()
    {
        if(settingsStateMachine.settingsState === SettingsStateEnum.SETTING_USER_LOGOUT )
        {
            if ( createMenuObject("ConfirmScreen.qml") )
            {
                menuEntryObject.infoText = menuList[0].actionValue;
                menuEntryObject.confirmFunction = triggerLogout;
                menuEntryObject.cancelFunction = cancelLogout;
                menuEntryObject.entryType = settingsStateMachine.settingsState;
            }
        }
    }

    function buildUpStandbyAdjust()
    {
        if ( createMenuObject("StandbyAdjustment.qml"))
        {
            menuEntryObject.setMinutesStartValues( MenuEnums.ACTIONTYPE_STDBY, parseInt( menuList[0].actionValue ), [3,5,10,20,30] )
        }
    }

    function buildUpSettingsModeExitTimeoutAdjust()
    {
        if ( createMenuObject( "StandbyAdjustment.qml" ) )
        {
            menuEntryObject.setMinutesStartValues( MenuEnums.ACTIONTYPE_SETTINGS_MODE_EXIT_TIMEOUT, parseInt( menuList[0].actionValue ), [.5,1,3,5,10,20,30] )
        }
    }

    function buildUpAgsaDelay()
    {
        if ( createMenuObject("AgsaDelay.qml"))
        {
            menuEntryObject.setSecondStartValues( parseInt( menuList[0].actionValue ) )
        }
    }

    function buildUpMachineInfo()
    {
        if ( createMenuObject("MachineInfo.qml") )
        {
        }
    }

    function buildUpOwnersManual()
    {
        createMenuObject("OwnersManual.qml");
    }

    function buildUpTutorial()
    {
        const stage = menuList[0].actionValue;
        idWelcomeHandler.start({
            stage,
            returnToSettings: true
        });
    }

    function buildUpStatisticsScreen()
    {
        if ( createMenuObject( "StatisticsScreen.qml" ) )
        {
            statisticsBoard.queryResults()
        }
    }

    function buildUpMotorTestScreen()
    {
        if ( createMenuObject( "MotorTestScreen.qml" ) )
        {
        }
    }

    function buildUpDddTestScreen()
    {
        if ( createMenuObject( "DddTestScreen.qml" ) )
        {
        }
    }

    function buildUpSystemSetupScreen()
    {
        createMenuObject("SystemSetupScreen.qml");
    }

    function buildUpNotificationCenter()
    {
        if ( createMenuObject( "NotificationCenter.qml" ) )
        {
        }
    }

    function onPreviousClicked()
    {
    }

    Connections
    {
        target: settingsStateMachine
        function onNavigatedPrevious()
        {
            menuPage.contentY = settingsStateMachine.savedMenuPostion();
        }
    }

    ShadowGroup
    {
        visible:  ( menuList.length > 3 && !mainMenu) ? true : false
        z:1;

        shadowTopSize: ShadowGroup.ShadowTopSizes.StandardSize;
        shadowTopThirdGradientStopEnable:   true;
        shadowTopThirdGradientStopPosition: 0.5;
        shadowTopThirdGradientStopOpacity:  0.85;

        shadowBottomSize: 75;

        //visualDebug: true;
    }

    Item
    {
        id: entryPlaceholder

        visible: ( ( menuList.length === 1 ) && ( menuList[0].menuType === MenuEnums.ENTRYTYPE_ACTION )) ? true : false

        anchors.fill: parent

        onVisibleChanged:
        {
            if( visible )
            {
                switch ( menuList[0].actionType )
                {
                case MenuEnums.ACTIONTYPE_MOTOR_TEST_SCREEN:
                    buildUpSystemMotorTestScreen();
                    break;
                case MenuEnums.ACTIONTYPE_FACTORY_RESET:
                    buildUpDiskFactoryResetScreen();
                    break;
                case MenuEnums.ACTIONTYPE_DISK_USAGE_TIME:
                    buildUpDiskUsageScreen();
                    break;
                case MenuEnums.ACTIONTYPE_TEXTBOX:
                    buildUpTextBox();
                    break;
                case MenuEnums.ACTIONTYPE_SLIDER_ADJUSTMENT:
                    buildUpSliderAdjustment();
                    break;
                case MenuEnums.ACTIONTYPE_RADIOBUTTON:
                    buildUpComboInput();
                    break;
                case MenuEnums.ACTIONTYPE_CALIB:
                    buildUpCalibInput();
                    break;
                case MenuEnums.ACTIONTYPE_CPC:
                    buildUpChangePinCodeInput();
                    break;
                case MenuEnums.ACTIONTYPE_PW:
                    buildUpPasswordInput();
                    break;
                case MenuEnums.ACTIONTYPE_LOGIN:
                    buildUpLogin();
                    break;
                case MenuEnums.ACTIONTYPE_DATE:
                    buildUpDate();
                    break;
                case MenuEnums.ACTIONTYPE_TIME:
                    buildUpTime();
                    break;
                case MenuEnums.ACTIONTYPE_CONFIRM:
                    buildUpConfirm();
                    break;
                case MenuEnums.ACTIONTYPE_STDBY:
                    buildUpStandbyAdjust();
                    break;
                case MenuEnums.ACTIONTYPE_SETTINGS_MODE_EXIT_TIMEOUT:
                    buildUpSettingsModeExitTimeoutAdjust();
                    break;
                case MenuEnums.ACTIONTYPE_AGSA_DELAY:
                    buildUpAgsaDelay();
                    break;
                case MenuEnums.ACTIONTYPE_DISPLAY_TEST:
                    buildUpDisplayTest();
                    break;
                case MenuEnums.ACTIONTYPE_MOTOR_TEST:
                    buildUpMotorTestScreen();
                    break;
                case MenuEnums.ACTIONTYPE_DDD_TEST:
                    buildUpDddTestScreen();
                    break;
                case MenuEnums.ACTIONTYPE_STATISTICS:
                    buildUpStatisticsScreen();
                    break;
                case MenuEnums.ACTIONTYPE_NOTIFICATION_CENTER:
                    buildUpNotificationCenter();
                    break;
                case MenuEnums.ACTIONTYPE_MACHINE_INFO:
                    buildUpMachineInfo();
                    break;
                case MenuEnums.ACTIONTYPE_OWNERS_MANUAL:
                    buildUpOwnersManual();
                    break;
                case MenuEnums.ACTIONTYPE_TUTORIAL:
                    buildUpTutorial();
                    break;
                case MenuEnums.ACTIONTYPE_SYSTEM_SETUP:
                    buildUpSystemSetupScreen();
                    break;
                default:
                    console.log( "Fatal, enum not mentioned" );
                    break;
                }
            }
            else
            {
                if( menuEntryObject !== undefined )
                {
                    menuEntryObject.destroy();
                }
            }
        }
    }

    Timer
    {
        id: pressTimer
        interval: 20; running: false; repeat: true
        onTriggered:
        {
            pressColor = idMain.colorEkxConfirmGreen
        }
    }

    ListView
    {
        id: menuPage

        visible: ( ( ( menuList.length === 1 ) && ( menuList[0].menuType === MenuEnums.ENTRYTYPE_ACTION )) || mainMenu ) ? false : true

        anchors.fill: parent
        // For a display height of 226.
        topMargin: mainMenu ? 0 : 80
        bottomMargin: mainMenu ? 134 : 54

        preferredHighlightBegin:     0
        preferredHighlightEnd:       0
        //    highlightRangeMode:          ListView.StrictlyEnforceRange
        highlightFollowsCurrentItem: true

        flickableDirection: Flickable.VerticalFlick
        orientation: ListView.Vertical
        interactive: ( menuList.length > 3 && !mainMenu) ? true : false

        model: menuList
        delegate: Item
        {
            width: 480;
            height: 96
            Text
            {
                id: menuText
                z:2
                property bool pressInAction: false
                text: modelData.menuName;

                font.pixelSize: 58
                font.letterSpacing: 0.23
                font.bold: modelData.menuType === MenuEnums.ENTRYTYPE_MAINENTRY ? true : false
                font.family: "D-DIN Condensed HEMRO"

                color: pressInAction ? pressColor : ( modelData.menuChoosable && modelData.userRightsObtained ) ? idMain.colorEkxTextWhite : idMain.colorEkxTextGrey
                anchors.centerIn: parent
            }

            Rectangle
            {
                width: 15; height: parent.height
                anchors.right: mpv.left
                color: "black"
                opacity: 0.5
                visible: modelData.menuType === MenuEnums.ENTRYTYPE_STRING
                z:1
            }

            MenuPageValue
            {
                id: mpv
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.topMargin: 5

                menuEntryChoosable: modelData.menuChoosable && modelData.userRightsObtained

                currentValueType: modelData.menuType
                currentActive:    modelData.toggleSwitchValue
                currentStringValue: modelData.stringValue

                visible:  modelData.menuType !== MenuEnums.ENTRYTYPE_MAINENTRY
            }

            MouseArea
            {
                id: menuPageMouse
                anchors.fill: parent
                enabled: modelData.menuChoosable && modelData.userRightsObtained
                onPressed:
                {
                    menuText.pressInAction = true;
                    pressTimer.start()
                }
                onCanceled:
                {
                    menuText.pressInAction = false;
                    pressTimer.stop()
                    pressColor = idMain.colorEkxTextWhite
                }
                onExited:
                {
                    menuText.pressInAction = false;
                    pressTimer.stop()
                    pressColor = idMain.colorEkxTextWhite
                }
                onClicked:
                {
                    menuText.pressInAction = false;
                    pressTimer.stop()
                    pressColor = idMain.colorEkxTextWhite
                    if( modelData.linkedSettingsState !== SettingsStateEnum.SETTING_INVALID )
                    {
                        settingsStateMachine.saveMenuPosition( menuPage.contentY )
                        settingsStateMachine.navigateToMenu( modelData.linkedSettingsState );
                    }
                    else if ( enabled )
                    {
                        modelData.toggleSwitchValue = !modelData.toggleSwitchValue;
                        settingsStateMachine.menuToggleSwitchChangeHandler( modelData.toggleSwitchType, modelData.toggleSwitchValue );
                    }
                }
            }
            Rectangle
            {
                visible: modelData.menuType===MenuEnums.ENTRYTYPE_TOGGLE_SWITCH
                anchors.fill: mpv;
                color: "transparent";
                //border.width: 2;     // visualDebug
                //border.color: "red"; // visualDebug
                MouseArea
                {
                    // drag and flick should not be possible fo toggleSwitch
                    id: idMenuValueMouseArea;
                    anchors.fill: parent;
                    preventStealing: true;
                    propagateComposedEvents: false;
                    onClicked:
                    {
                        menuPageMouse.clicked(mouse);
                    }
                }
            }
        }
    }

    Text
    {
        id: idTitle;
        z:2
        anchors.left: parent.left
        anchors.top: parent.top
        width: 480
        height: 72
        horizontalAlignment: Text.AlignHCenter;
        verticalAlignment:   Text.AlignVCenter;
        font.pixelSize: 40
        font.letterSpacing: 2
        font.bold: true
        font.family: "D-DIN Condensed HEMRO"
        color: idMain.colorEkxTextGrey
        text: settingsMenuContentFactory.menuTitle.toUpperCase();
        visible: opacity > 0
        Behavior on opacity {
            NumberAnimation {
                duration: 200
            }
        }
    }

    EkxButton
    {
        id: idButtonClose;

        z:2
        visible: settingsMenuContentFactory.navAvailable
        buttonType: EkxButton.ButtonType.CLOSE;
        onClicked:
        {
            if( menuEntryObject !== undefined && menuEntryObject !== null)
            {
                if(typeof menuEntryObject.beforeExitOnClose === "function")
                    menuEntryObject.beforeExitOnClose();
            }
            settingsStateMachine.backToMainMenu();
        }
    }

    EkxButton
    {
        id: idButtonPrevious;

        z:2;
        visible: settingsMenuContentFactory.buttonPreviousVisible || settingsMenuContentFactory.navAvailable; // navAvailable to stay compatible to old menus
        buttonType: EkxButton.ButtonType.PREVIOUS;

        property var clickFunc

        onClicked:
        {
            if ( clickFunc )
            {
                clickFunc()
            }
            else
            {
                if( menuEntryObject !== undefined && menuEntryObject !== null)
                {
                    if ( typeof menuEntryObject.beforeExitOnPrevious === "function" )
                        menuEntryObject.beforeExitOnPrevious();
                }
                settingsStateMachine.back();
            }
        }
    }

    EkxButton
    {
        id: idButtonOk;

        z:2;
        visible: settingsMenuContentFactory.buttonOkVisible;
        buttonType: EkxButton.ButtonType.OK;
        onClicked:
        {
            if( menuEntryObject !== undefined && menuEntryObject !== null)
            {
                if( typeof menuEntryObject.beforeExitOnOk === "function" )
                    menuEntryObject.beforeExitOnOk();
                if(typeof menuEntryObject.save === "function")
                    menuEntryObject.save();
            }
            settingsStateMachine.back();
        }
    }
}

