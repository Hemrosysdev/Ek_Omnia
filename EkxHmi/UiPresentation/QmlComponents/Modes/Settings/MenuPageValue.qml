import QtQuick 2.0
import "../../CommonItems"
import MenuEnums 1.0

Rectangle
{
    width: 80
    height: 80

    color: "black"

    property int currentValueType; // valueType is defined via menuPage and QtEnum
    property bool currentActive: false;
    property string currentStringValue: "";

    property bool menuEntryChoosable: true


    visible: currentValueType !== MenuEnums.ENTRYTYPE_NOVALUE

    Text
    {
//        width: 40
//        height: 40

        anchors.fill: parent

        text: currentStringValue;

        visible: currentValueType === MenuEnums.ENTRYTYPE_STRING


        font.pixelSize: 40
        font.letterSpacing: 2
        font.family: "D-DIN Condensed HEMRO"
        font.bold: true

        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter

        color: idMain.colorEkxTextGrey

        anchors.centerIn: parent
    }

    ActivateToggle
    {
       anchors.centerIn: parent
       visible: currentValueType === MenuEnums.ENTRYTYPE_TOGGLESWITCH
       active:  currentActive

       opacity: menuEntryChoosable ? 1.0 : 0.5

       toggleEnabled: menuEntryChoosable
    }


    Image
    {
        width: 72
        height:72

        visible: currentValueType === MenuEnums.ENTRYTYPE_RADIOBUTTON

        anchors.centerIn: parent

        source: "qrc:/Icons/ICON_Indicator-72px_deactive.png"
    }
    Image
    {
        width: 72
        height:72

        visible: currentValueType === MenuEnums.ENTRYTYPE_RADIOBUTTON && currentActive

        source: "qrc:/Icons/ICON_Indicator-72px_active.png"
        anchors.centerIn: parent

    }

}


