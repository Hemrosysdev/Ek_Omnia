import QtQuick 2.12
import "../../CommonItems"
import EkxSqliteTypes 1.0
import MenuEnums 1.0

Rectangle
{
    id: idRoot

    width:  480
    height: 360
    color:  "black"

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // self defined properties
    ///////////////////////////////////////////////////////////////////////////////////////////////

    property int actionType;
    property int currentMinuteIndex;
    property string currentMinuteValue;

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // signal handlers
    ///////////////////////////////////////////////////////////////////////////////////////////////

    function setMinutesStartValues( initActionType,
                                    initMinuteIndex,
                                    initDataList )
    {
        idMinutes.dataList = initDataList
        actionType = initActionType;
        currentMinuteIndex = initMinuteIndex;
        idMinutes.setStartValues( currentMinuteIndex );
    }

    function save()
    {
        // do nothing
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // items
    ///////////////////////////////////////////////////////////////////////////////////////////////

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
        x: 0

        itemWidth: parent.width - idTimeUnit.x
        rightOversize: parent.width - itemWidth

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

        text:  "min"
        color: idMain.colorEkxTextWhite

        anchors.left: parent.left;
        anchors.leftMargin: 252;
        anchors.verticalCenter: parent.verticalCenter;

        font.pixelSize: 82
        font.family:    "D-DIN Condensed HEMRO"
        font.letterSpacing: 0.23
    }

    EkxButton
    {
        z:4

        buttonType: EkxButton.ButtonType.OK;

        onClicked:
        {
            if ( actionType === MenuEnums.ACTIONTYPE_STDBY )
            {
                settingsSerializer.stdbyTimeIndex = currentMinuteIndex;
                settingsSerializer.saveSettings();
                settingsStateMachine.back();

                sqliteInterface.addEvent( EkxSqliteTypes.SqliteEventType_CHANGE_STANDBY_TIME,
                                          currentMinuteValue );
            }
            else if ( actionType === MenuEnums.ACTIONTYPE_SETTINGS_MODE_EXIT_TIMEOUT )
            {
                settingsSerializer.settingsModeExitTimeIndex = currentMinuteIndex;
                settingsSerializer.saveSettings();
                settingsStateMachine.back();

                sqliteInterface.addEvent( EkxSqliteTypes.SqliteEventType_CHANGE_SETTINGS_MODE_EXIT_TIME,
                                          currentMinuteValue );
            }
        }
    }
}
