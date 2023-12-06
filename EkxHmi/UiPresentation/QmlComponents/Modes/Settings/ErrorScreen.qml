import QtQuick 2.0

Item
{
    width: 480
    height: 360

    property string errorText: "Invalid PIN Code"
    property string errorActionText: "Retype"

    property var errorFunction;   // (function() { console.log("Canceled this Function!!"); })


    Text
    {
        text: errorText;

        width: 480
        height: 90

        anchors.left: parent.left
        anchors.top: parent.top
        anchors.topMargin: 69

        font.pixelSize: 58
        font.letterSpacing: 2.9
        font.bold: true
        horizontalAlignment: Text.AlignHCenter
        font.family: "D-DIN Condensed HEMRO"

        color: idMain.colorEkxWarning

        Component.onCompleted:
        {
            console.log("Contentwidth: " + contentWidth);
        }

    }

    Image
    {
        width: 80
        height: 80
        anchors.left: parent.left
        anchors.leftMargin: 200
        anchors.top: parent.top
        anchors.topMargin: 159
        source:"qrc:/Icons/ICON-Strokes_Warning-80px_orange.png"
    }

    Item
    {
        width: 480
        height: 133

        anchors.left: parent.left
        anchors.bottom: parent.bottom

        Text
        {
            text: errorActionText;

            font.pixelSize: 43
            font.letterSpacing: 1.48
            font.family: "D-DIN Condensed HEMRO"

            color: idMain.colorEkxTextWhite

            anchors.centerIn: parent
        }

        MouseArea
        {
            anchors.fill: parent

            onClicked:
            {
                errorFunction();
            }
        }
    }

    Component.onCompleted:
    {
        if(visible)
        {
            settingsStateMachine.navAvailable = false;
        }
    }

    onVisibleChanged:
    {
        if(visible)
        {
            settingsStateMachine.navAvailable = false;
        }
    }
}
