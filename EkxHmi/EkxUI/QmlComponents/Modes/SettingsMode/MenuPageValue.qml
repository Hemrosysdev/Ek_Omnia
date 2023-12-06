import QtQuick 2.0
import "../../CommonItems"
import MenuEnums 1.0

Rectangle
{
    width: 100
    height: 80

    color: "black"

    property int currentValueType;
    property bool currentActive: false;
    property string currentStringValue: "";

    property bool menuEntryChoosable: true


    visible: currentValueType !== MenuEnums.ENTRYTYPE_NOVALUE

    Text
    {
        visible: currentValueType === MenuEnums.ENTRYTYPE_STRING

        anchors.fill: parent
        anchors.centerIn: parent

        text: currentStringValue;
        color: idMain.colorEkxTextGrey

        font.pixelSize: 40
        font.letterSpacing: 2
        font.family: "D-DIN Condensed HEMRO"
        font.bold: true

        horizontalAlignment: Text.AlignRight
        verticalAlignment: Text.AlignVCenter
    }

    ActivateToggle
    {
       visible: currentValueType === MenuEnums.ENTRYTYPE_TOGGLE_SWITCH

       anchors.centerIn: parent

       active:  currentActive
       opacity: menuEntryChoosable ? 1.0 : 0.5
       toggleEnabled: menuEntryChoosable
    }

    Image
    {
        visible: currentValueType === MenuEnums.ENTRYTYPE_RADIOBUTTON

        anchors.centerIn: parent
        width: 72
        height:72

        source: "qrc:/Icons/ICON_Indicator-72px_deactive.png"
    }

    Image
    {
        visible: currentValueType === MenuEnums.ENTRYTYPE_RADIOBUTTON && currentActive

        anchors.centerIn: parent
        width: 72
        height:72

        source: "qrc:/Icons/ICON_Indicator-72px_active.png"
    }

    Item
    {
        visible: currentValueType === MenuEnums.ENTRYTYPE_IMAGE

        anchors.centerIn: parent
        width: 72
        height:72

        Image
        {
            anchors.centerIn: parent

            source: parent.visible ? currentStringValue : ""
        }
    }

}


