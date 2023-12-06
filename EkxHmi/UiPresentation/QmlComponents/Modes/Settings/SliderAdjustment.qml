import QtQuick 2.12
import SettingStatesEnum 1.0
import EkxSqliteTypes 1.0

Item
{
    id: progressBar

    width: 480
    height: 360

    property string progressText: "UNDEFINED"
    property string unit: "UNDEFINED"
    property int currentValueX100: -1
    property int entryType;
    property bool showRange: false
    property int minValue: 0
    property int maxValue: 100

    function calcCurrentValue( mousePos )
    {
        currentValueX100 = (minValue + ( ( maxValue - minValue ) * ( mousePos/idProgressRectOuter.width ) ))*100;
        if (currentValueX100<minValue*100) currentValueX100=minValue*100;
        if (currentValueX100>maxValue*100) currentValueX100=maxValue*100;
    }

    function div100RoundClip(valueX100)
    {
        let x=0;
        x = parseInt((valueX100+50)/100.0)
        if(x>maxValue)
            x=maxValue;
        else if(x<minValue)
            x=minValue;
        return(x);
    }

    function save()
    {
        // function must exist
    }

    function beforeExitOnClose()
    {
        if (entryType === SettingStatesEnum.SETTING_CUSTOMIZE_BRIGHTNESS)
            displayController.brightnessPercent = settingsSerializer.displayBrightnessPercent;
    }

    function beforeExitOnPrevious()
    {
        if (entryType === SettingStatesEnum.SETTING_CUSTOMIZE_BRIGHTNESS)
            displayController.brightnessPercent = settingsSerializer.displayBrightnessPercent;
    }

    function beforeExitOnOk()
    {
        var strContext;
        if (entryType === SettingStatesEnum.SETTING_CUSTOMIZE_BRIGHTNESS)
        {
            strContext = settingsSerializer.displayBrightnessPercent + "->" + div100RoundClip( currentValueX100 );
            settingsSerializer.displayBrightnessPercent = div100RoundClip( currentValueX100 );
            settingsSerializer.saveSettings();

            var a = EkxSqliteTypes.SqliteEventType_CHANGE_BRIGHTNESS;
            var b = EkxSqliteTypes.SqliteErrorType_UNKNOWN;
            sqliteInterface.addEvent( EkxSqliteTypes.SqliteEventType_CHANGE_BRIGHTNESS,
                                      strContext );
        }
        else if( entryType === SettingStatesEnum.SETTING_CUSTOMIZE_TEMP_THRESH)
        {
            strContext = settingsSerializer.tempThresValue + "->" + currentValueX100;
            settingsSerializer.tempThresValue = currentValueX100; // no rounding!
            settingsSerializer.saveSettings();

            sqliteInterface.addEvent( EkxSqliteTypes.SqliteEventType_CHANGE_TEMP_THRESHOLD,
                                      strContext );
        }
    }

    onCurrentValueX100Changed:
    {
        if ( entryType === SettingStatesEnum.SETTING_CUSTOMIZE_BRIGHTNESS )
        {
            displayController.brightnessPercent = div100RoundClip( currentValueX100 );
        }
    }


    Item {
        id: idProgressBarPrivate;
        property bool visualDebug: false; // DEBUG helper
        //property int pressAndHoldIntervalMs: 500;
        property int pressAndHoldIntervalMs: 250;
        anchors.left: parent.left
        anchors.right: parent.right;
        anchors.top: parent.top
        anchors.topMargin: 72
        height: 216
        Rectangle {
            id: idProgressRectOuter;
            width: parent.width
            height: parent.height
            color: idMain.colorEkxDarkGreen
            Rectangle {
                id: idProgressRectInner;
                anchors.left: parent.left
                width: idProgressRectOuter.width * ( (currentValueX100-minValue*100)/(maxValue*100-minValue*100) );
                height: parent.height
                color: idMain.colorEkxConfirmGreen
            }
            MouseArea { // Same drag behaviour like on idButtonIncrease/idButtonDecrease
                //anchors.fill: parent
                //onPressed: {
                //    calcCurrentValue( mouseX )
                //}
                //onPositionChanged: {
                //    if( ( mouseX >= 0 ) && ( mouseX <= idProgressRectOuter.width ) )
                //    {
                //        calcCurrentValue( mouseX )
                //    }
                //}
                property bool pressAndHoldWasTriggered: false;
                anchors.fill: parent;
                pressAndHoldInterval: idProgressBarPrivate.pressAndHoldIntervalMs;
                //onPressAndHold: calcCurrentValue( mouseX+parent.x );
                onPressAndHold: pressAndHoldWasTriggered = true;
                onReleased: pressAndHoldWasTriggered = false;
                onPositionChanged: {
                    if( pressAndHoldWasTriggered === true )
                        calcCurrentValue( mouseX+parent.x );
                }
            }
        }
        Text {
            id: progressbarText
            anchors.centerIn: parent
            font.pixelSize: 82
            font.letterSpacing: 0.33
            font.family: "D-DIN Condensed HEMRO"
            color: "black"
            text: {
                if( entryType === SettingStatesEnum.SETTING_CUSTOMIZE_TEMP_THRESH)
                    return(settingsSerializer.tempToString(currentValueX100))
                else
                    //return( parseInt((currentValueX100+50)/100) + unit);
                    return( div100RoundClip(currentValueX100) + unit );
            }
        }
        Rectangle {
            id: idButtonIncrease;
            anchors.right: parent.right;
            anchors.top: parent.top;
            anchors.bottom: parent.bottom;
            width: parent.width*0.4;
            color: "transparent";
            border.width: idProgressBarPrivate.visualDebug?1:0;
            border.color: "black";
            Rectangle{
                anchors.right: parent.right;
                anchors.verticalCenter: parent.verticalCenter;
                width: 80
                height: 80
                color: "transparent";
                border.width: idProgressBarPrivate.visualDebug?1:0;
                border.color: "black";
                Text {
                    anchors.fill: parent;
                    horizontalAlignment: Text.AlignHCenter;
                    verticalAlignment: Text.AlignVCenter;
                    font.pixelSize: 82
                    font.letterSpacing: 0.33
                    font.family: "D-DIN Condensed HEMRO"
                    color: "black"
                    text: "+"
                }
            }
            MouseArea {
                property bool pressAndHoldWasTriggered: false;
                anchors.fill: parent;
                onClicked:
                {
                    currentValueX100 = currentValueX100 + 100
                    if ( currentValueX100 > maxValue*100 )
                    {
                        currentValueX100 = maxValue*100;
                    }
                }
                // make sliding possible:
                pressAndHoldInterval: idProgressBarPrivate.pressAndHoldIntervalMs;
                //onPressAndHold: calcCurrentValue( mouseX+parent.x );
                onPressAndHold: pressAndHoldWasTriggered=true;
                onReleased: pressAndHoldWasTriggered=false;
                onPositionChanged: {
                    if(pressAndHoldWasTriggered===true)
                        calcCurrentValue( mouseX+parent.x );
                }
            }
        }
        Rectangle {
            id: idButtonDecrease;
            anchors.left: parent.left;
            anchors.top: parent.top;
            anchors.bottom: parent.bottom;
            width: parent.width*0.4;
            color: "transparent";
            border.width: idProgressBarPrivate.visualDebug?1:0;
            border.color: "black";
            Rectangle{
                anchors.left: parent.left;
                anchors.verticalCenter: parent.verticalCenter;
                width: 80
                height: 80
                color: "transparent";
                border.width: idProgressBarPrivate.visualDebug?1:0;
                border.color: "black";
                Text {
                    anchors.fill: parent;
                    horizontalAlignment: Text.AlignHCenter;
                    verticalAlignment: Text.AlignVCenter;
                    font.pixelSize: 82
                    font.letterSpacing: 0.33
                    font.family: "D-DIN Condensed HEMRO"
                    color: "black"
                    text: "-"
                }
            }
            MouseArea
            {
                property bool pressAndHoldWasTriggered: false;
                anchors.fill: parent;
                onClicked:
                {
                    mouse.accepted = true;
                    currentValueX100 = currentValueX100 - 100;
                    if ( currentValueX100 < minValue*100 )
                    {
                        currentValueX100 = minValue*100;
                    }
                }
                // make sliding possible:
                pressAndHoldInterval: idProgressBarPrivate.pressAndHoldIntervalMs;
                //onPressAndHold: calcCurrentValue( mouseX+parent.x );
                onPressAndHold: pressAndHoldWasTriggered = true;
                onReleased: pressAndHoldWasTriggered = false;
                onPositionChanged: {
                    if( pressAndHoldWasTriggered === true )
                        calcCurrentValue( mouseX+parent.x );
                }
            }
        }
    }
}
