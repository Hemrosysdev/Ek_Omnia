import QtQuick 2.12
import QtQml 2.12
import QtGraphicalEffects 1.12
import SettingsStateEnum 1.0
import SettingsSerializerEnum 1.0
import EkxMainstateEnum 1.0

import ".." as Modes
import "../../CommonItems" as Common

Modes.ModePage
{
    id: idSettingsMode

    title: "Settings"
    stateId: EkxMainstateEnum.EKX_SETTINGS_MODE
    showDdd: false

    modeIsLocked: settingsStateMachine.settingsState !== SettingsStateEnum.SETTING_MAIN

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // items
    ///////////////////////////////////////////////////////////////////////////////////////////////

    mainContent: MainMenuPage
    {
        id: idSettingsMenu
        anchors.left: parent.left
        anchors.top: parent.top

        visible: settingsStateMachine.settingsState === SettingsStateEnum.SETTING_MAIN
    }

    Common.Label
    {
        anchors.left: parent.left
        anchors.top: parent.top
        width: parent.width
        height: 72

        horizontalAlignment: Text.AlignHCenter;
        verticalAlignment:   Text.AlignVCenter;
        pixelSize: 40
        font.letterSpacing: 2
        bold: true
        textColor: Common.Label.TextColor.Grey
        text: "Settings"
        font.capitalization: Font.AllUppercase
    }
}
