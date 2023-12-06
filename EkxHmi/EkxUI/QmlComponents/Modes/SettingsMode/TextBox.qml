import QtQuick 2.0

Item
{
    id: idTextBox;

    width: 480
    height: 360
    property alias text: idText.text;
    property color textColor: idMain.colorEkxTextWhite

    function save()
    {
        // This function must exist for "back" button to work!
    }

    Text {
        id: idText;
        anchors.fill: parent;
        color: idTextBox.textColor;
        font.pixelSize: 58
        font.letterSpacing: 0.23
        font.bold: false
        font.family: "D-DIN Condensed HEMRO"
        horizontalAlignment: Text.AlignHCenter;
        verticalAlignment: Text.AlignVCenter;
        text: "UNDEFINED";
        wrapMode: Text.WordWrap
    }
}
