import QtQuick 2.12

import SettingStatesEnum 1.0
import MenuEnums 1.0

Rectangle
{
    width:  160
    height: 360

    property string iconImage;
    property string menuText;

    property var linkedState;
    color: "black"

    MouseArea
    {
        anchors.fill: parent

        onClicked:
        {
            settingsStateMachine.navigateToMenu( linkedState );
        }
    }

    Image
    {
        id: icon
        anchors.centerIn: parent
        source: iconImage

        width: 112
        height: 112
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



}
