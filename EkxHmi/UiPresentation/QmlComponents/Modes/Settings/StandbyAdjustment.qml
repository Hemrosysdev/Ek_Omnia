import QtQuick 2.12
import "../../CommonItems"
import EkxSqliteTypes 1.0

Rectangle
{
    id: idRoot

    property int entryType;
    property int currentMinuteIndex;
    property string currentMinuteValue;

    function setMinuteStartValues( initMinuteIndex )
    {
        currentMinuteIndex = initMinuteIndex;
        idMinutes.setStartValues( currentMinuteIndex );
    }

    function acceptCurrentMinute()
    {
        settingsSerializer.stdbyTimeIndex = currentMinuteIndex;
        settingsSerializer.saveSettings();
        settingsStateMachine.back();

        sqliteInterface.addEvent( EkxSqliteTypes.SqliteEventType_CHANGE_STANDBY_TIME,
                                  currentMinuteValue );

    }

    function save()
    {
        // do nothing
    }

    width:  480
    height: 360
    color:  "black"

    ShadowGroup
    {
        z: 1;
        shadowTopSize: ShadowGroup.ShadowTopSizes.StandardSize;
        shadowBottomSize: ShadowGroup.ShadowBottomSizes.StandardSize;
    }

    SettingsNumberWheel
    {
        id: idMinutes
        z: 0
        anchors.right: idTimeUnit.left
        anchors.rightMargin: 20
        //currentFontFamiliy: "D-DIN Condensed HEMRO"
        currentFontFamiliy: "D-DIN Condensed HEMRO _Mono_Numbers"

        model: dataList.length
        dataList: [3,5,10,20,30]
        useIndexData: true

        onCurrentValue:
        {
            currentMinuteIndex = value;
            currentMinuteValue = dataList[value] + "min"
        }
    }

    // unit "min" text
    Text
    {
        id: idTimeUnit
        anchors.left: parent.left;
        anchors.leftMargin: 252;
        anchors.verticalCenter: parent.verticalCenter;
        //font.pixelSize: 58
        font.pixelSize: 82
        font.family:    "D-DIN Condensed HEMRO"
        font.letterSpacing: 0.23
        color: idMain.colorEkxTextWhite
        text:  "min"
    }

    EkxButton
    {
        // id: idButtonOk;
        z:4
        buttonType: EkxButton.ButtonType.OK;
        onClicked:
        {
            acceptCurrentMinute();
        }
    }
}
