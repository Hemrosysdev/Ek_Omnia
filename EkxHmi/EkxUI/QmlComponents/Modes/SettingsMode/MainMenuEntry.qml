import QtQuick 2.12

import SettingsStateEnum 1.0
import MenuEnums 1.0

Rectangle
{
    width:  160
    height: 360

    property string iconImage;
    property string notificationImage;
    property string menuText;

    property var linkedState;
    color: "black"

    Image
    {
        id: icon
        anchors.centerIn: parent
        source: iconImage

        width: 112
        height: 112
    }

    Image
    {
        id: notificationIcon

        z: 1
        anchors.left: icon.left
        anchors.leftMargin: 67
        anchors.bottom: icon.bottom
        anchors.bottomMargin: 57

        source: notificationImage

        width: 56
        height: 56
    }

    Item
    {
        width: 160
        height: 85

        anchors.top: icon.bottom
        anchors.topMargin: 14

        Text
        {
            width: 160

            text: menuText;
            font.pixelSize: 40
            font.letterSpacing: 2
            font.family: "D-DIN Condensed HEMRO"
            font.bold:true

            wrapMode: Text.WordWrap

            maximumLineCount: 2

            horizontalAlignment:  Text.AlignHCenter

            color: idMain.colorEkxTextGrey
        }
    }

    MouseArea
    {
        anchors.fill: parent

        onClicked:
        {
            settingsStateMachine.navigateToMenu( linkedState );
        }
    }
}
