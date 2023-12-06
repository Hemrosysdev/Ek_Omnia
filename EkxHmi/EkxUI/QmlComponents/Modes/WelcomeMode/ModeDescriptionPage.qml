import QtQml 2.12
import QtQuick 2.12
import QtQuick.Layouts 1.1
import QtGraphicalEffects 1.12

import ".." as Modes
import "../../CommonItems" as Common

import EkxMainstateEnum 1.0

Common.OverlayPage {
    id: idModeDescriptionPage

    required property Modes.ModeSelector modeSelector

    property int mode
    property bool modeIndicatorVisible: mode > 0
    // title does a OverlayView title which we don't want here
    property alias titleText: idModeDescriptionTitleLabel.text
    property alias text: idModeDescriptionLabel.text
    property bool skipButton: false
    property bool showMeButton: false

    property int textVerticalCenterOffset: 0

    signal skipClicked

    backdrop: false
    leftAction: skipButton ? 0 : Common.EkxButton.ButtonType.PREVIOUS
    rightAction: showMeButton ? 0 : Common.EkxButton.ButtonType.NEXT

    onActivating: {
        modeSelector.opacity = 0;
        modeSelector.blendOutEnabled = false;

        if (mode > 0) {
            modeSelector.allModesEnabled = true;

            mainStatemachine.ekxMainstate = mode;
        }
    }

    FastBlur {
        id: idModeDemoBlur

        anchors.fill: parent
        source: idWorkflow.modeSelector
        radius: 55
    }

    MouseArea {
        anchors {
            left: parent.left
            bottom: parent.bottom
        }
        width: idSkipLabel.width + 20
        height: idSkipLabel.height + 20
        visible: idModeDescriptionPage.skipButton

        onClicked: idModeDescriptionPage.skipClicked()

        Common.Label {
            id: idSkipLabel
            anchors.centerIn: parent
            // Align it vertically centered with the back button.
            height: 72
            verticalAlignment: Text.AlignVCenter
            text: "Skip"
            pixelSize: 43
            font.letterSpacing: 1.48
        }
    }

    MouseArea {
        anchors {
            right: parent.right
            bottom: parent.bottom
        }
        width: idShowMeLabel.width + 20
        height: idShowMeLabel.height + 20
        visible: idModeDescriptionPage.showMeButton

        onClicked: idModeDescriptionPage.goNext()

        Common.Label {
            id: idShowMeLabel
            anchors.centerIn: parent
            // Align it vertically centered with the back button.
            height: 72
            verticalAlignment: Text.AlignVCenter
            textColor: Common.Label.TextColor.ConfirmGreen
            text: "Preview"
            pixelSize: 43
            font.letterSpacing: 1.48
        }
    }

    Modes.ModeIndicator {
        anchors.verticalCenter: parent.verticalCenter
        currentMode: idModeDescriptionPage.mode
        highlightColor: idModeDescriptionPage.showMeButton ? idMain.colorEkxConfirmGreen : idMain.colorEkxTextWhite
        visible: idModeDescriptionPage.modeIndicatorVisible
    }

    ColumnLayout {
        anchors {
            verticalCenter: parent.verticalCenter
            verticalCenterOffset: idModeDescriptionPage.textVerticalCenterOffset
        }
        width: parent.width
        spacing: 0

        Common.Label {
            id: idModeDescriptionTitleLabel
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
            pixelSize: 43
            font.letterSpacing: 1.48
            bold: true
            wrapMode: Text.WordWrap
            visible: text !== ""
        }

        Common.Label {
            id: idModeDescriptionLabel
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
            pixelSize: 43
            font.letterSpacing: 1.48
            wrapMode: Text.WordWrap
            visible: text !== ""
        }
    }
}
