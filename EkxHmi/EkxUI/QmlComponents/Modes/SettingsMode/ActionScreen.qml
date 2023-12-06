import QtQuick 2.0

Item
{
    width: 480
    height: 360

    property string actionInfoText: "Zero the wheel + \n confirm calibration"
    property string actionText: "Calibration"

    property var actionFunction;   // (function() { console.log("Canceled this Function!!"); })


    Text
    {
        text: actionInfoText;

        width: 480
        height: 125

        anchors.left: parent.left
        anchors.top: parent.top
        anchors.topMargin: 90

        font.pixelSize: 58
        font.letterSpacing: 2.1
        font.bold: true
        horizontalAlignment: Text.AlignHCenter
        font.family: "D-DIN Condensed HEMRO"

        color: idMain.colorEkxTextWhite

        wrapMode: Text.Wrap

    }

    Item
    {
        width: 480
        height: 150

        anchors.left: parent.left
        anchors.bottom: parent.bottom

        Text
        {
            text: actionText;

            font.pixelSize: 43
            font.letterSpacing: 1.48
            font.family: "D-DIN Condensed HEMRO"

            color: idMain.colorEkxConfirmGreen

            anchors.centerIn: parent
        }

        MouseArea
        {
            anchors.fill: parent

            onClicked:
            {
                actionFunction();
            }
        }
    }

    Component.onCompleted:
    {
        if(visible)
        {
            settingsMenuContentFactory.navAvailable = false;
        }
    }

    onVisibleChanged:
    {
        if(visible)
        {
            settingsMenuContentFactory.navAvailable = false;
        }
    }
}
