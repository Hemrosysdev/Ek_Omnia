import QtQuick 2.12

import "../../CommonItems" as Common

import EkxSqliteTypes 1.0

Common.OverlayPage {
    id: idPromptPage
    controlsVisible: false

    property alias text: notification.text
    property alias cancelLabel: notification.cancelLabel
    property alias confirmLabel: notification.confirmLabel

    signal cancelClicked
    signal confirmClicked

    Common.Notification {
        id: notification
        classId: EkxSqliteTypes.Info
        // Match TextPage font.
        font.pixelSize: 43
        font.letterSpacing: 1.48
        font.bold: false

        onCancelClicked: idPromptPage.cancelClicked()
        onConfirmClicked: idPromptPage.confirmClicked()
    }
}
