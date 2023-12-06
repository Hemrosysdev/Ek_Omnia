import QtQuick 2.12
import QtQml 2.12
import QtGraphicalEffects 1.12
import SettingStatesEnum 1.0
import SettingsSerializerEnum 1.0
import EkxMainstateEnum 1.0

Item {
    id: settings
    width: 480
    height: 360
    property var currentSettingState: settingsStateMachine.settingState
    property int currentEkxMainstate: mainStatemachine.ekxMainstate
    property var currentUser: settingsSerializer.loggedInUser
    property bool autoLogout: settingsSerializer.autoLogoutEnable;
    signal settingsStatesChanged();

    function getTypeOfItem()
    {
        return 3;
    }

    function resetModeInfo()
    {
        blendOutblur.opacity = 1;
        blendOutblur.radius = 55;
        blendoutText.opacity = 1;
        settingsStateMachine.menuTitle.opacity = 0;
        blendOutTimer.stop();
        blendOut.stop();
    }

    function startBlendoutInfo()
    {
        blendOutTimer.start();
    }

    onCurrentSettingStateChanged: {
        settingsStatesChanged();
    }

    onAutoLogoutChanged: {
        if( !autoLogout && autoLogoutTimer.running )
        {
            autoLogoutTimer.stop()
        }
        if( autoLogout )
        {
            if( currentUser === SettingsSerializerEnum.LOGGEDIN_STOREOWNER )
            {
                 autoLogoutTimer.start()
            }
        }
    }

    onCurrentUserChanged: {
        if( currentUser === SettingsSerializerEnum.LOGGEDIN_TECHNICIAN )
        {
            autoLogoutTimer.start()
        }
        else if( currentUser === SettingsSerializerEnum.LOGGEDIN_STOREOWNER )
        {
            if( settingsSerializer.autoLogoutEnable )
                autoLogoutTimer.start()
        }
        else if( currentUser === SettingsSerializerEnum.LOGGEDIN_NONE )
        {
            autoLogoutTimer.stop();
        }
    }

    onCurrentEkxMainstateChanged: {
        if( currentEkxMainstate == EkxMainstateEnum.EKX_MENU )
        {
            startBlendoutInfo();
        }
        else
        {
            resetModeInfo();
        }
    }

    Timer {
        id: autoLogoutTimer
        repeat: false
        interval: 240000
        onTriggered: {
            settingsStateMachine.logout();
        }
    }

    MenuPage {
        id: currentMenu
        anchors.left: parent.left
        anchors.top: parent.top
        mainMenu: currentSettingState === SettingStatesEnum.SETTING_MAIN
    }

    FastBlur {
        id: blendOutblur
        z:1
        visible: true
        anchors.centerIn: parent
        width: 480
        height: 360
        opacity: 1.0
        source: currentMenu
        radius: 55
    }

    Text
    {
        id: blendoutText
        z:3
        anchors.centerIn: parent
        opacity: 1
        font.pixelSize: 58
        font.family:    "D-DIN Condensed HEMRO"
        font.bold:      true
        font.letterSpacing: 0.23
        color:  idMain.colorEkxTextWhite
        text: "Settings"
    }

    ParallelAnimation {
        id: blendOut
        running: false
        NumberAnimation { target: blendOutblur; property: "radius"; to: 0; duration: 150 }
        NumberAnimation { target: blendOutblur; property: "opacity"; to: 0; duration: 150 }
        NumberAnimation { target: blendoutText; property: "opacity"; to: 0; duration: 150 }
        NumberAnimation { target: currentMenu; property: "opacity"; to: 1; duration: 150 }
    }

    Timer {
        id: blendOutTimer
        interval: 260
        running: false
        repeat:  false
        onTriggered: {
            blendOut.start();
        }
    }
}
