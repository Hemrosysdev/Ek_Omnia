import QtQml 2.12
import QtQuick 2.12
import QtQuick.Controls 2.12

import "../../CommonItems"

import SettingStatesEnum 1.0
import MenuEnums 1.0

Item
{
    id: root
    width: 480
    height: 360
    property var menuList: settingsStateMachine.qlpMenuList  ; // interface to current radiobuttons data element in settings
    property bool mainMenu: false;
    property var menuEntryObject;
    property color pressColor: idMain.colorEkxTextWhite
    property  bool visualDebug: false;

    function createMenuObject( qmlFile )
    {
        var success = true
        var menuEntry = Qt.createComponent( qmlFile );
        menuEntryObject = menuEntry.createObject( entryPlaceholder );

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

    function buildUpDiskFactoryResetScreen()
    {
        if(settingsStateMachine.settingState === SettingStatesEnum.SETTING_SYSTEM_FACTORY_RESET )
        {
            if ( createMenuObject("ResetScreen.qml") )
            {
                // menuEntryObject.infoText = menuList[0].actionValue;
                // menuEntryObject.backFunction = settingsStateMachine.back;
                // menuEntryObject.entryType = settingsStateMachine.settingState;
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

    function buildUpProgressInput()
    {
        if ( createMenuObject("SliderAdjustment.qml") )
        {
            menuEntryObject.entryType = settingsStateMachine.settingState;
            if( settingsStateMachine.settingState === SettingStatesEnum.SETTING_CUSTOMIZE_BRIGHTNESS )
            {
                menuEntryObject.progressText = "Brightness"
                menuEntryObject.unit = "%"
                menuEntryObject.currentValueX100 = parseInt( menuList[0].actionValue )*100
                menuEntryObject.minValue = 0;
                menuEntryObject.maxValue = 100;
            }
            else if(settingsStateMachine.settingState === SettingStatesEnum.SETTING_CUSTOMIZE_TEMP_THRESH )
            {
                menuEntryObject.progressText = "Warning Thold"
                menuEntryObject.unit = settingsSerializer.isCelsius?"°C":"°F"
                menuEntryObject.currentValueX100 = settingsSerializer.tempThresValue;
                menuEntryObject.showRange = true
                menuEntryObject.minValue = settingsSerializer.tempThresLowerLimit;
                menuEntryObject.maxValue = settingsSerializer.tempThresUpperLimit;
            }
        }
    }

    function buildUpComboInput()
    {
        if ( createMenuObject("RadioButtons.qml") )
        {
            menuEntryObject.entryType = settingsStateMachine.settingState;
            if(settingsStateMachine.settingState === SettingStatesEnum.SETTING_SYSTEM_TEMP_UNIT )
            {
                menuEntryObject.valueList = ["Celsius","Fahrenheit"]
                menuEntryObject.indexOfChoosenValue = parseInt( menuList[0].actionValue )
            }
            else if(settingsStateMachine.settingState === SettingStatesEnum.SETTING_SYSTEM_START_OF_WEEK )
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
            menuEntryObject.entryType = settingsStateMachine.settingState;
        }
    }

    function buildUpCPCInput()
    {
        if ( createMenuObject( "CpcPinInput.qml") )
        {
            menuEntryObject.entryType = settingsStateMachine.settingState;
        }
    }

    function buildUpDisplayTest()
    {
        if ( createMenuObject( "ColorTestScreen.qml" ) )
        {
            menuEntryObject.entryType = settingsStateMachine.settingState;
        }
    }

    function buildUpLogin()
    {
        if ( createMenuObject("Login.qml") )
        {
            menuEntryObject.entryType = settingsStateMachine.settingState;
        }
    }

    function buildUpPWInput()
    {
        if( settingsStateMachine.settingState === SettingStatesEnum.SETTING_SYSTEM_WIFI_SETTING_PW )
        {
            if ( createMenuObject( "PasswordInput.qml" ) )
            {
                menuEntryObject.oldPassword = menuList[0].actionValue;
                menuEntryObject.entryType = settingsStateMachine.settingState;
            }
        }
        else if( settingsStateMachine.settingState === SettingStatesEnum.SETTING_SYSTEM_GRINDERNAME )
        {
            if ( createMenuObject("EditGrindername.qml") )
            {
                menuEntryObject.oldGrindername = menuList[0].actionValue;
                menuEntryObject.entryType = settingsStateMachine.settingState;
            }
        }
    }

    function buildUpDate()
    {
        if ( createMenuObject( "DateAdjustment.qml" ) )
        {
            menuEntryObject.entryType = settingsStateMachine.settingState;
            var date = menuList[0].actionValue.split(".");
            menuEntryObject.setDateStartValues(date[0],date[1],date[2])
        }
    }

    function buildUpTime()
    {
        if ( createMenuObject("TimeAdjustment.qml") )
        {
            menuEntryObject.entryType = settingsStateMachine.settingState;
            var time = menuList[0].actionValue.split(".");
            menuEntryObject.setTimeStartValues(time[0],time[1])
        }
    }

    function buildUpConfirm()
    {
        if(settingsStateMachine.settingState === SettingStatesEnum.SETTING_USER_LOGOUT )
        {
            if ( createMenuObject("ConfirmScreen.qml") )
            {
                menuEntryObject.infoText = menuList[0].actionValue;
                menuEntryObject.isInfoNotQuestion = true;
                menuEntryObject.okFunction = triggerLogout;
                menuEntryObject.entryType = settingsStateMachine.settingState;
            }
        }
    }

    function buildUpStandbyAdjust()
    {
        if ( createMenuObject("StandbyAdjustment.qml"))
        {
            menuEntryObject.entryType = settingsStateMachine.settingState;
            menuEntryObject.setMinuteStartValues(parseInt( menuList[0].actionValue ))
        }
    }

    function buildUpMachineInfo()
    {
        if ( createMenuObject("MachineInfo.qml") )
        {
            menuEntryObject.entryType = settingsStateMachine.settingState;
        }
    }

    function buildUpStatisticsScreen()
    {
        if ( createMenuObject( "StatisticsScreen.qml" ) )
        {
            statistics.queryResults()
            //menuEntryObject.entryType = settingsStateMachine.settingState;
        }
    }

    Connections
    {
        target: settingsStateMachine
        onNavigatedPrevious:
        {
            menuPage.contentY = settingsStateMachine.getSavedMenuPostion();
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
        anchors.fill: parent
        visible: ( ( menuList.length === 1 ) && ( menuList[0].menuType === MenuEnums.ENTRYTYPE_ACTION )) ? true : false
        onVisibleChanged: {
            if( visible )
            {
                if( menuList[0].actionType === MenuEnums.ACTIONTYPE_FACTORY_RESET )
                {
                    buildUpDiskFactoryResetScreen();
                }
                if( menuList[0].actionType === MenuEnums.ACTIONTYPE_DISK_USAGE_TIME )
                {
                    buildUpDiskUsageScreen();
                }
                if( menuList[0].actionType === MenuEnums.ACTIONTYPE_TEXTBOX )
                {
                    buildUpTextBox();
                }
                if( menuList[0].actionType === MenuEnums.ACTIONTYPE_PROGRESS )
                {
                    buildUpProgressInput();
                }
                if( menuList[0].actionType === MenuEnums.ACTIONTYPE_RADIOBUTTON )
                {
                    buildUpComboInput();
                }
                if( menuList[0].actionType === MenuEnums.ACTIONTYPE_CALIB)
                {
                    buildUpCalibInput();
                }
                if( menuList[0].actionType === MenuEnums.ACTIONTYPE_CPC)
                {
                    buildUpCPCInput();
                }
                if( menuList[0].actionType === MenuEnums.ACTIONTYPE_PW)
                {
                    buildUpPWInput();
                }
                if( menuList[0].actionType === MenuEnums.ACTIONTYPE_LOGIN)
                {
                    buildUpLogin();
                }
                if( menuList[0].actionType === MenuEnums.ACTIONTYPE_DATE)
                {
                    buildUpDate();
                }
                if( menuList[0].actionType === MenuEnums.ACTIONTYPE_TIME)
                {
                    buildUpTime();
                }
                if( menuList[0].actionType === MenuEnums.ACTIONTYPE_CONFIRM)
                {
                    buildUpConfirm();
                }
                if( menuList[0].actionType === MenuEnums.ACTIONTYPE_STDBY)
                {
                    buildUpStandbyAdjust();
                }
                if( menuList[0].actionType === MenuEnums.ACTIONTYPE_MACHINFO)
                {
                    buildUpMachineInfo();
                }
                if ( menuList[0].actionType === MenuEnums.ACTIONTYPE_DISPLAY_TEST )
                {
                    buildUpDisplayTest();
                }
                if ( menuList[0].actionType === MenuEnums.ACTIONTYPE_STATISTICS_SCREEN )
                {
                    buildUpStatisticsScreen();
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

    MainMenuPage
    {
        anchors.fill: parent
        visible: mainMenu
    }

    ListView
    {
        id: menuPage
        width: 480
        height: 226
        anchors.top: parent.top
        anchors.topMargin: mainMenu ? 0 : 80
        displayMarginBeginning: 500;
        displayMarginEnd: 500;
        preferredHighlightBegin:     0
        preferredHighlightEnd:       0
        //    highlightRangeMode:          ListView.StrictlyEnforceRange
        highlightFollowsCurrentItem: true

        flickableDirection: Flickable.VerticalFlick
        orientation: ListView.Vertical
        clip: false
        visible: ( ( ( menuList.length === 1 ) && ( menuList[0].menuType === MenuEnums.ENTRYTYPE_ACTION )) || mainMenu ) ? false : true
        interactive: ( menuList.length > 3 && !mainMenu) ? true : false
        //ScrollBar.vertical: ScrollBar {
        //    interactive: true;
        //    hoverEnabled: true
        //    active: hovered || pressed
        //    size: 0.2;
        //    width: 20;
        //    background: Rectangle {
        //        anchors.right: parent.right;
        //        radius: width/2;
        //        color: "grey";
        //        opacity: 0.5
        //    }
        //    contentItem: Rectangle {
        //        anchors.right: parent.right;
        //        radius: width/2;
        //        color: idMain.colorEkxTextWhite;
        //    }
        //}
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

            Rectangle {
                width: 15; height: parent.height
                anchors.right: mpv.left
                color: "black"
                opacity: 0.5
                visible: modelData.menuType === MenuEnums.ENTRYTYPE_STRING
                z:1
            }

            MenuPageValue {
                id: mpv
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.topMargin: 5
                menuEntryChoosable: modelData.menuChoosable && modelData.userRightsObtained
                currentValueType: modelData.menuType
                currentActive:    modelData.toggleswitchValue
                currentStringValue: modelData.stringValue
                visible:  modelData.menuType !== MenuEnums.ENTRYTYPE_MAINENTRY
            }

            MouseArea {
                id: menuPageMouse
                anchors.fill: parent
                enabled: modelData.menuChoosable && modelData.userRightsObtained
                onPressed: {
                    menuText.pressInAction = true;
                    pressTimer.start()
                }
                onCanceled: {
                    menuText.pressInAction = false;
                    pressTimer.stop()
                    pressColor = idMain.colorEkxTextWhite
                }
                onExited: {
                    menuText.pressInAction = false;
                    pressTimer.stop()
                    pressColor = idMain.colorEkxTextWhite
                }
                onClicked: {
                    menuText.pressInAction = false;
                    pressTimer.stop()
                    pressColor = idMain.colorEkxTextWhite
                    if( modelData.linkedMenu !== SettingStatesEnum.SETTING_INVALID )
                    {
                        settingsStateMachine.saveMenuPosition( menuPage.contentY )
                        settingsStateMachine.navigateToMenu( modelData.linkedMenu );
                    }
                    else
                    {
                        modelData.toggleswitchValue = !modelData.toggleswitchValue;
                        settingsStateMachine.menuToggleswitchChangeHandler( modelData.toggleswitchType, modelData.toggleswitchValue );
                    }
                }
            }
            Rectangle {
                visible: modelData.menuType===MenuEnums.ENTRYTYPE_TOGGLESWITCH
                anchors.fill: mpv;
                color: "transparent";
                //border.width: 2;     // visualDebug
                //border.color: "red"; // visualDebug
                MouseArea {
                    // drag and flick should not be possible fo toggleswitch
                    id: idMenuValueMouseArea;
                    anchors.fill: parent;
                    preventStealing: true;
                    propagateComposedEvents: false;
                    onClicked: {
                        menuPageMouse.clicked(mouse);
                    }
                }
            }
        }
    }

    Text {
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
        text: settingsStateMachine.menuTitle.toUpperCase();
    }

    EkxButton
    {
        // id: idButtonClose;
        z:2
        visible: settingsStateMachine.navAvailable
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
        // id: idButtonPrevious;
        z:2;
        visible: settingsStateMachine.buttonPreviousVisible || settingsStateMachine.navAvailable; // navAvailable to stay compatible to old menus
        buttonType: EkxButton.ButtonType.PREVIOUS;
        onClicked:
        {
            if( menuEntryObject !== undefined && menuEntryObject !== null)
            {
                if( typeof menuEntryObject.beforeExitOnPrevious === "function" )
                    menuEntryObject.beforeExitOnPrevious();
            }
            settingsStateMachine.back();
        }
    }

    EkxButton
    {
        // id: idButtonOk;
        z:2;
        visible: settingsStateMachine.buttonOkVisible;
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

