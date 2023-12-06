import QtQuick 2.12

import "../../CommonItems" as Common

Common.OverlayPage
{
    /*required*/ property alias text: idLabel.text

    Common.Label {
        id: idLabel
        anchors.fill: parent
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        wrapMode: Text.WordWrap
        pixelSize: 43
        font.letterSpacing: 1.48
    }

}
