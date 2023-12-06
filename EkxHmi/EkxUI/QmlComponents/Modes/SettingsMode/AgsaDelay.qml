import QtQuick 2.12
import "../../CommonItems"
import EkxSqliteTypes 1.0

Rectangle
{
    id: idRoot

    width:  480
    height: 360
    color:  "black"

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // self defined properties
    ///////////////////////////////////////////////////////////////////////////////////////////////

    property int currentSecondIndex;
    property string currentSecondValue;

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // signal handlers
    ///////////////////////////////////////////////////////////////////////////////////////////////

    function setSecondStartValues( initSecondIndex )
    {
        currentSecondIndex = initSecondIndex;
        idSecondsWheel.setStartValues( currentSecondIndex );
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
        id: idSecondsWheel

        z: 0
        x: 0

        itemWidth: parent.width - idTimeUnit.x
        rightOversize: parent.width - itemWidth

        currentFontFamiliy: "D-DIN Condensed HEMRO _Mono_Numbers"

        model: dataList.length
        dataList: [0,1,2,3,4,5,6,7,8,9,10]
        useIndexData: true

        onCurrentValue:
        {
            currentSecondIndex = value;
            currentSecondValue = dataList[value] + "sec"
        }
    }

    // unit "sec" text
    Text
    {
        id: idTimeUnit

        text:  "sec"
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
            settingsSerializer.agsaStartDelaySec = currentSecondIndex;
            settingsSerializer.saveSettings();
            settingsStateMachine.back();

            sqliteInterface.addEvent( EkxSqliteTypes.SqliteEventType_CHANGE_AGSA_DELAY,
                                      currentSecondValue );
        }
    }
}
