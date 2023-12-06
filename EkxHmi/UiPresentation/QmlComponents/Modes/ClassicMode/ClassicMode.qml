import QtQml 2.12
import QtQuick 2.12
import QtGraphicalEffects 1.12

import ".."
import "../../CommonItems"
import EkxMainstateEnum 1.0
import SettingStatesEnum 1.0

Item
{
    id: idClassicMode

    objectName: "classic"

    width: 480
    height: 360

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // self defined properties
    ///////////////////////////////////////////////////////////////////////////////////////////////

    property bool visualDebug: false;
    property bool modeIsLocked: mainStatemachine.isCoffeeRunning;
    property int  currentEkxMainstate: mainStatemachine.ekxMainstate

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // functions
    ///////////////////////////////////////////////////////////////////////////////////////////////

    function getTypeOfItem()
    {
        return 0;
    }

    function resetModeInfo()
    {
        idBlendOutBlur.opacity = 1;
        idBlendOutBlur.radius = 55;
        idBlendOutText.opacity = 1;
        idTupleValue.opacity = 0;
        idTemperatureView.opacity = 0;
        idBlendOutTimer.stop();
        idBlendOutAnimation.stop();
    }

    function startBlendoutInfo()
    {
        idBlendOutTimer.start();
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // signal handlers
    ///////////////////////////////////////////////////////////////////////////////////////////////

    onVisibleChanged:
    {
        console.log("ClassicMode : " + visible);
    }

    onCurrentEkxMainstateChanged:
    {
        if( currentEkxMainstate == EkxMainstateEnum.EKX_CLASSIC_MODE )
        {
            startBlendoutInfo();
        }
        else
        {
            resetModeInfo();
        }
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // items
    ///////////////////////////////////////////////////////////////////////////////////////////////

    Timer
    {
        id: idBlendOutTimer

        interval: 250
        running: false
        repeat:  false

        onTriggered:
        {
            idBlendOutAnimation.start();
        }
    }

    TupleValue
    {
        id: idTupleValue

        centerXByHallbach: false;
        centerXByMass: true;
        //anchors.left: parent.left
        //anchors.leftMargin: 80
        anchors.horizontalCenter: parent.horizontalCenter;
        anchors.verticalCenter: parent.verticalCenter;
        opacity: 0.0

        preDecimalValue: dddDriver.dddBigValue;
        postDecimalValue: dddDriver.dddCommaValue;
        DebugFrame{visible: visualDebug}
    }

    TemperatureView
    {
        id: idTemperatureView

        opacity: 0

        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        DebugFrame{visible: visualDebug}
    }

    FastBlur
    {
        id: idBlendOutBlur
        anchors.fill: idTupleValue
        width: 320
        height: 360
        z:1
        visible: true

        opacity: 1.0

        source: idTupleValue
        radius: 55
    }

    Text
    {
        id: idBlendOutText
        text: "Pet Mode"

        anchors.centerIn: parent

        opacity: 1

        color:  idMain.colorEkxTextWhite

        font.pixelSize: 58
        font.family:    "D-DIN Condensed HEMRO"
        font.bold:      true
        font.letterSpacing: 0.23
    }

    ParallelAnimation
    {
        id: idBlendOutAnimation

        running: false
        NumberAnimation { target: idBlendOutBlur; property: "radius"; to: 0; duration: 150 }
        NumberAnimation { target: idBlendOutBlur; property: "opacity"; to: 0; duration: 150 }
        NumberAnimation { target: idBlendOutText; property: "opacity"; to: 0; duration: 150 }
        NumberAnimation { target: idTupleValue; property: "opacity"; to: 1; duration: 150 }
        NumberAnimation { target: idTemperatureView; property: "opacity"; to: 1; duration: 150 }
    }
}
