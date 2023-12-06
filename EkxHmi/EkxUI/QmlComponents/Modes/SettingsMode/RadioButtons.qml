import QtQuick 2.0

import SettingsStateEnum 1.0
import MenuEnums 1.0
import EkxSqliteTypes 1.0

Item
{
    property var valueList:[];
    property int entryType;
    property int indexOfChoosenValue: 0
    width: 480
    height: 360

    function save()
    {
        // function must exist
    }

    function beforeExitOnClose()
    {

    }

    function beforeExitOnPrevious()
    {

    }

    function beforeExitOnOk()
    {
        if ( entryType === SettingsStateEnum.SETTING_SYSTEM_TEMP_UNIT )
        {
            settingsSerializer.tempUnit = indexOfChoosenValue;
            settingsSerializer.saveSettings();

            sqliteInterface.addEvent( EkxSqliteTypes.SqliteEventType_CHANGE_TEMP_UNIT,
                                      valueList[indexOfChoosenValue] );
        }
        else if ( entryType === SettingsStateEnum.SETTING_SYSTEM_START_OF_WEEK )
        {
            settingsSerializer.startOfWeekSun = indexOfChoosenValue;
            settingsSerializer.saveSettings();

            sqliteInterface.addEvent( EkxSqliteTypes.SqliteEventType_CHANGE_START_OF_WEEK,
                                      valueList[indexOfChoosenValue] );
        }
    }

    ListView
    {
        id: radioButtons

        clip: false
        anchors.top: parent.top
        anchors.topMargin: 80
        width: 480
        height: 360
        preferredHighlightBegin:     0
        preferredHighlightEnd:       0
        highlightRangeMode:          ListView.StrictlyEnforceRange
        highlightFollowsCurrentItem: true
        flickableDirection: Flickable.VerticalFlick
        orientation: ListView.Vertical
        interactive: valueList.length > 3 ? true : false
        model: valueList
        delegate: Item
        {
            width: 480;
            height: 96
            Text
            {
                anchors.centerIn: parent
                font.pixelSize: 58
                font.letterSpacing: 0.23
                font.family: "D-DIN Condensed HEMRO"
                color: ( indexOfChoosenValue == index ) ? idMain.colorEkxConfirmGreen : idMain.colorEkxTextWhite
                text: valueList[index];
            }

            Image
            {
                id: idSelectionImage

                anchors.verticalCenter: parent.verticalCenter
                anchors.right: parent.right
                anchors.rightMargin: 24

                width: 72
                height:72

                source: ( index == indexOfChoosenValue ) ? "qrc:/Icons/ICON_Indicator-72px_active.png" : "qrc:/Icons/ICON_Indicator-72px_deactive.png"
            }

            MouseArea
            {
                anchors.fill: parent
                onClicked:
                {
                    indexOfChoosenValue = index;
                }
            }
        }
    }
}

