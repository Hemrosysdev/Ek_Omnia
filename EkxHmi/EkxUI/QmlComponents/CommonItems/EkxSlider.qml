import QtQuick 2.12
import SettingsStateEnum 1.0
import EkxSqliteTypes 1.0

Item
{
    id: idRoot

    width: 480
    height: 360

    property int currentValueX100: -1 // must be monitored by onCurrentValueX100Changed handler
    property bool showLimits: false
    property int minValue: 0
    property int maxValue: 100
    property real increment: 1;
    property string currentValueText: "UNDEFINED" // must be updated by onCurrentValueX100Changed handler

    signal released();

    function currentValueX100Rounded()
    {
        //# This must be a functions since onCurentValueX100Changed
        //# is to be overwritten by user function
        //# (workarounds using a private property would result in delayed values).
        let rounder = increment * 100.0;
        let rounded = ( Math.round( currentValueX100/rounder ) * rounder );

        return rounded;
    }

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

    Item
    {
        id: idProgressBarPrivate;
        property bool visualDebug: false; // DEBUG helper
        //property int pressAndHoldIntervalMs: 500;
        property int pressAndHoldIntervalMs: 250;
        anchors.left: parent.left
        anchors.right: parent.right;
        anchors.top: parent.top
        anchors.topMargin: 72
        height: 216
        Rectangle
        {
            id: idProgressRectOuter;
            width: parent.width
            height: parent.height
            color: idMain.colorEkxDarkGreen

            Rectangle
            {
                id: idProgressRectInner;
                anchors.left: parent.left
                width: idProgressRectOuter.width * ( (currentValueX100-minValue*100)/(maxValue*100-minValue*100) );
                height: parent.height
                color: idMain.colorEkxConfirmGreen
            }

            MouseArea
            {
                property bool pressAndHoldWasTriggered: false;

                anchors.fill: parent;

                pressAndHoldInterval: idProgressBarPrivate.pressAndHoldIntervalMs;

                //onPressAndHold: calcCurrentValue( mouseX+parent.x );

                onPressAndHold:
                {
                    pressAndHoldWasTriggered = true;
                }

                onReleased:
                {
                    if( pressAndHoldWasTriggered === true )
                    {
                        calcCurrentValue( mouseX+parent.x );
                    }
                    pressAndHoldWasTriggered=false;
                    idRoot.released();
                }

                onPositionChanged:
                {
                    if( pressAndHoldWasTriggered === true )
                        calcCurrentValue( mouseX+parent.x );
                }
            }
        }

        Text
        {
            id: idTextCurrentValue
            anchors.centerIn: parent
            font.pixelSize: 82
            font.letterSpacing: 0.33
            font.family: "D-DIN Condensed HEMRO"
            color: "black"
            text: idRoot.currentValueText;
        }

        Rectangle
        {
            id: idButtonIncrease;
            anchors.right: parent.right;
            anchors.top: parent.top;
            anchors.bottom: parent.bottom;
            width: parent.width*0.4;
            color: "transparent";
            border.width: idProgressBarPrivate.visualDebug?1:0;
            border.color: "black";
            Rectangle
            {
                anchors.right: parent.right;
                anchors.verticalCenter: parent.verticalCenter;
                width: 80
                height: 80
                color: "transparent";
                border.width: idProgressBarPrivate.visualDebug?1:0;
                border.color: "black";
                Text
                {
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
            MouseArea
            {
                property bool pressAndHoldWasTriggered: false;
                anchors.fill: parent;

                onClicked:
                {
                    currentValueX100 = currentValueX100 + (100*idRoot.increment);
                    let rounder = idRoot.increment * 100.0;
                    currentValueX100 = Math.round( currentValueX100/rounder ) * rounder;

                    //currentValueX100 = currentValueX100 + (100*increment);
                    if ( currentValueX100 > maxValue*100 )
                    {
                        currentValueX100 = maxValue*100;
                    }
                    idRoot.released();
                }

                // make sliding possible:
                pressAndHoldInterval:
                {
                    idProgressBarPrivate.pressAndHoldIntervalMs;
                }

                //onPressAndHold: calcCurrentValue( mouseX+parent.x );

                onPressAndHold:
                {
                    pressAndHoldWasTriggered=true;
                }

                onReleased:
                {
                    if( pressAndHoldWasTriggered === true )
                    {
                        calcCurrentValue( mouseX+parent.x );
                    }
                    pressAndHoldWasTriggered=false;
                    idRoot.released();
                }

                onPositionChanged:
                {
                    if(pressAndHoldWasTriggered===true)
                        calcCurrentValue( mouseX+parent.x );
                }
            }
        }
        Rectangle
        {
            id: idButtonDecrease;
            anchors.left: parent.left;
            anchors.top: parent.top;
            anchors.bottom: parent.bottom;
            width: parent.width*0.4;
            color: "transparent";
            border.width: idProgressBarPrivate.visualDebug?1:0;
            border.color: "black";
            Rectangle
            {
                anchors.left: parent.left;
                anchors.verticalCenter: parent.verticalCenter;
                width: 80
                height: 80
                color: "transparent";
                border.width: idProgressBarPrivate.visualDebug?1:0;
                border.color: "black";
                Text
                {
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

                    currentValueX100 = currentValueX100 - (100*idRoot.increment);
                    let rounder = idRoot.increment * 100.0;
                    currentValueX100 = Math.round( currentValueX100/rounder ) * rounder;

                    //currentValueX100 = currentValueX100 - (100);
                    if ( currentValueX100 < minValue*100 )
                    {
                        currentValueX100 = minValue*100;
                    }
                    idRoot.released();
                }

                // make sliding possible:
                pressAndHoldInterval:
                    idProgressBarPrivate.pressAndHoldIntervalMs;

                //onPressAndHold: calcCurrentValue( mouseX+parent.x );
                onPressAndHold:
                    pressAndHoldWasTriggered = true;

                onReleased:
                {
                    if( pressAndHoldWasTriggered === true )
                    {
                        calcCurrentValue( mouseX+parent.x );
                    }
                    pressAndHoldWasTriggered=false;
                    idRoot.released();
                }

                onPositionChanged:
                {
                    if( pressAndHoldWasTriggered === true )
                        calcCurrentValue( mouseX+parent.x );
                }
            }
        }
    }
}
