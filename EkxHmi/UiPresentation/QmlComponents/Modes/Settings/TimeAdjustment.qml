import QtQuick 2.12
import "../../CommonItems"

Rectangle
{
    id: root

    property int entryType;

    function setTimeStartValues( initHour, initMinute )
    {
       currentHour = initHour;
       currentMinute = initMinute;
       hour.setStartValues(currentHour);
       minute.setStartValues(currentMinute);
    }

    property int currentMinute;
    property int currentHour;

    function acceptCurrentTime()
    {
        settingsSerializer.setMachineTime("'" + currentHour + ":" + currentMinute + "'");

        settingsStateMachine.back()
    }

    function save()
    {
    }


    width:  480
    height: 360
    color:  "black"

    ShadowGroup
    {
        z: 1;

        shadowTopSize: 135;
        shadowTopOuterOpacity: 0.6;

        shadowBottomSize: 145;
        shadowBottomOuterOpacity: 0.6;
    }

    SettingsNumberWheel
    {
        id: hour
        z:0

        anchors.right: point.left
        anchors.rightMargin: 12

        currentFontFamiliy: "D-DIN Condensed HEMRO _Mono_Numbers"

        itemWidth: 72

        model: 24

        onCurrentValue:
        {
            currentHour = value;
        }
    }

    Text
    {
        id: point
        width: 24
        height: 90
        text:  ":"
        color: idMain.colorEkxTextWhite

        font.pixelSize: 82
        font.family:    "D-DIN Condensed HEMRO _Mono_Numbers"

        horizontalAlignment: Text.AlignHCenter
//        verticalAlignment: Text.AlignBottom

        anchors.left: parent.left
        anchors.leftMargin: 228
        anchors.top: parent.top
        anchors.topMargin: 135

    }

    SettingsNumberWheel
    {
        id: minute
        z:0

        anchors.left: point.left
        anchors.leftMargin: 12

        informAboutChange: true

        currentFontFamiliy: "D-DIN Condensed HEMRO _Mono_Numbers"

        itemWidth: 72

        model: 60
        onCurrentValue:
        {
            currentMinute = value
        }

        onUpChange:
        {
            hour.incrementCurrentIndex();
        }

        onDownChange:
        {
           hour.decrementCurrentIndex();
        }
    }

    EkxButton
    {
        // id: idButtonOk;
        z:4
        buttonType: EkxButton.ButtonType.OK;
        onClicked:
        {
            acceptCurrentTime();
        }
    }
}
