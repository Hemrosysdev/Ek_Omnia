import QtQuick 2.12
import SettingsStateEnum 1.0
import EkxSqliteTypes 1.0


Rectangle
{
    id: idRoot;

    property string titleText: "Dummy Title"

    z: 3;
    x: 0;
    y: 0;
    width: 480;
    height: 72;
    color: "transparent";
    opacity: 1;

    Text
    {
        // # title:
        anchors.centerIn: parent;
        width: 480
        height: 72;
        opacity: 1;
        horizontalAlignment: Text.AlignHCenter;
        verticalAlignment:   Text.AlignVCenter;
        font.pixelSize: 40;
        font.letterSpacing: 2;
        font.bold: true;
        font.family: "D-DIN Condensed HEMRO";
        color: idMain.colorEkxTextGrey;
        text: idRoot.titleText;
    }
}


