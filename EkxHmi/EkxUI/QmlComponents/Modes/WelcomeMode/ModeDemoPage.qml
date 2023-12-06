import QtQuick 2.12

import ".." as Modes
import "../../CommonItems" as Common

Common.OverlayPage {
    id: idModeDemoPage

    required property Modes.ModeSelector modeSelector
    required property int mode
    property bool modeIndicatorVisible: mode > 0

    backdrop: false
    // Might overlap with DDDModeDisplay.
    closeButtonVisible: false

    rightActionPulseAnimationEnabled: true
    rightActionLongPressColor: idMain.colorEkxConfirmGreen

    onActivating: {
        modeSelector.allModesEnabled = true;
        mainStatemachine.ekxMainstate = mode;
        modeSelector.opacity = 1;
    }

    onModeChanged: {
        if (active) {
            mainStatemachine.ekxMainstate = mode;
        }
    }

    Modes.ModeIndicator {
        anchors.verticalCenter: parent.verticalCenter
        currentMode: idModeDemoPage.mode
        visible: idModeDemoPage.modeIndicatorVisible
    }
}
