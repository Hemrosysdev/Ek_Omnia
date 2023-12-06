import QtQuick 2.12
import "../../CommonItems"
import SettingsStateEnum 1.0

Rectangle
{
    id: root

    width:  480
    height: 360
    color:  "black"

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // self defined properties
    ///////////////////////////////////////////////////////////////////////////////////////////////

    property int entryType;
    property int currentDay;
    property int currentMonth;
    property int currentYear;

    function setDateStartValues( initDay, initMonth, initYear )
    {
        currentDay = initDay;
        currentMonth = initMonth;
        currentYear = initYear;

        day.setStartValues(currentDay);
        month.setStartValues(currentMonth);
        year.setStartValues(currentYear);
    }

    function save()
    {
        // do nothing
    }

    function beforeExitOnClose()
    {

    }

    function beforeExitOnPrevious()
    {

    }

    function beforeExitOnOk()
    {
        if (entryType === SettingsStateEnum.SETTING_SYSTEM_DATE)
        {
            var stringMonth = currentMonth;
            var stringDay = currentDay;

            if( currentMonth < 10 )
            {
                stringMonth = "0"+currentMonth;
            }
            if( currentDay < 10)
            {
                stringDay = "0"+currentDay
            }

            settingsSerializer.setMachineDate( currentYear +"-"+ stringMonth + "-" + stringDay )
        }
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // items
    ///////////////////////////////////////////////////////////////////////////////////////////////

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
        id: day
        z:0

        anchors.left: parent.left
        anchors.leftMargin: 72

        informAboutChange: true

        currentFontFamiliy: "D-DIN Condensed HEMRO _Mono_Numbers"

        model: 31
        zeroLeadDigits: 2

        itemWidth: 72

        conversionNumber: 1

        onCurrentValue:
        {
            currentDay = value;
        }

        onUpChange:
        {
            month.incrementCurrentIndex();
        }

        onDownChange:
        {
            month.decrementCurrentIndex();
        }
    }

    Text
    {
        id: point
        text:  "."
        color: idMain.colorEkxTextWhite

        height: 90
        width:  16

        font.pixelSize: 82
        font.family:    "D-DIN Condensed HEMRO _Mono_Numbers"

        horizontalAlignment: Text.AlignHCenter

        anchors.left: parent.left
        anchors.leftMargin: 144
        anchors.top: parent.top
        anchors.topMargin: 135

    }

    SettingsNumberWheel
    {
        id: month
        z:0

        anchors.right: pointTwo.left
        anchors.rightMargin: 5

        currentFontFamiliy: "D-DIN Condensed HEMRO _Mono_Numbers"

        model: 12
        zeroLeadDigits: 2

        conversionNumber: 1

        informAboutChange: true

        itemWidth: 72

        onCurrentValue:
        {
            currentMonth = value
        }

        onUpChange:
        {
            year.incrementCurrentIndex();
        }

        onDownChange:
        {
            year.decrementCurrentIndex();
        }
    }

    Text
    {
        id: pointTwo
        text:  "."
        color: idMain.colorEkxTextWhite

        height: 90
        width:  16

        font.pixelSize: 82
        font.family:    "D-DIN Condensed HEMRO _Mono_Numbers"

        horizontalAlignment: Text.AlignHCenter

        anchors.left: parent.left
        anchors.leftMargin: 240
        anchors.top: parent.top
        anchors.topMargin: 135

    }

    SettingsNumberWheel
    {
        id: year
        z:0

        anchors.left: pointTwo.right
        anchors.leftMargin: 7

        currentFontFamiliy: "D-DIN Condensed HEMRO _Mono_Numbers"

        itemWidth: 144

        informAboutChange: true

        model: 100

        conversionNumber: 1970

        onCurrentValue:
        {
            currentYear = value
        }
    }
}
