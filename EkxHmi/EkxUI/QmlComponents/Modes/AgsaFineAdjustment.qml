import QtQml 2.12
import QtQuick 2.12
import QtGraphicalEffects 1.12

import ".."
import "../CommonItems"
import EkxMainstateEnum 1.0
import SettingsStateEnum 1.0

Item
{
    id: idAgsaFineAdjustment

    visible: false
    objectName: "fineAdjustment"

    anchors.fill: parent

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // self defined properties
    ///////////////////////////////////////////////////////////////////////////////////////////////

    property int  currentEkxMainstate: mainStatemachine.ekxMainstate
    property bool doIncrement: false

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // functions
    ///////////////////////////////////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // signal handlers
    ///////////////////////////////////////////////////////////////////////////////////////////////

    onVisibleChanged:
    {
        if ( visible == true )
        {
            idAutoCloseTimer.restart()
            agsaControl.moveToDddValue( dddCouple.dddValue )
        }
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // items
    ///////////////////////////////////////////////////////////////////////////////////////////////

    Timer
    {
        id:  idAutoCloseTimer

        interval: 2000
        running: true
        repeat: false

        onTriggered:
        {
            idAgsaFineAdjustment.visible = false
        }
    }

    Timer
    {
        id:  idRepeatTimer

        interval: startIntervall
        running: false
        repeat: false

        property int startIntervall: 500

        onTriggered:
        {
            if ( doIncrement )
            {
                //agsaControl.targetDddValue++;
                agsaControl.incrementTargetDddValue()
            }
            else
            {
                //agsaControl.targetDddValue--;
                agsaControl.decrementTargetDddValue()
            }

            idAutoCloseTimer.restart()

            interval = interval * 0.7
            if ( interval < 50 )
            {
                interval = 20
            }
            restart()
        }

        onRunningChanged:
        {
            if ( !running )
            {
                interval = startIntervall
            }
        }
    }

    Rectangle
    {
        anchors.fill: parent

        color: "black"
        opacity: 0.65
    }

    DddValue
    {
        id: idDddValue

        z: 1
        centerWithUnits: false
        showUnit: false
        pixelSize: 162

        anchors.horizontalCenter: parent.horizontalCenter;
        anchors.verticalCenter: parent.verticalCenter;

        decimalValue: agsaControl.targetDddValue;
    }

    Image
    {
        id: idImageDecrement

        x: 63
        width: 40
        height: 40

        horizontalAlignment: Image.AlignHCenter
        verticalAlignment: Image.AlignVCenter

        anchors.verticalCenter: parent.verticalCenter;

        source: "qrc:/Icons/ICON_Minus-40px_white.png"
    }

    Image
    {
        id: idImageIncrement

        x: parent.width - 103
        width: 40
        height: 40

        horizontalAlignment: Image.AlignHCenter
        verticalAlignment: Image.AlignVCenter

        anchors.verticalCenter: parent.verticalCenter;

        source: "qrc:/Icons/ICON_Add-40px_white.png"
    }

    MouseArea
    {
        id: idDecrement

        height: parent.height
        width: parent.width / 3

        anchors.left: parent.left

        pressAndHoldInterval: 500

        onPressed:
        {
            idAutoCloseTimer.restart()
            idRepeatTimer.start()
            doIncrement = false
            agsaControl.decrementTargetDddValue()
        }

        onReleased:
        {
            idAutoCloseTimer.restart()
            idRepeatTimer.stop()
        }
    }

    MouseArea
    {
        id: idIncrement

        height: parent.height
        width: parent.width / 3

        anchors.right: parent.right

        pressAndHoldInterval: 500

        onPressed:
        {
            idAutoCloseTimer.restart()
            idRepeatTimer.start()
            doIncrement = true
            agsaControl.incrementTargetDddValue()
        }

        onReleased:
        {
            idAutoCloseTimer.restart()
            idRepeatTimer.stop()
        }
    }

    MouseArea
    {
        id: idDirectExit

        height: parent.height

        anchors.left: idDecrement.right
        anchors.right: idIncrement.left

        onClicked:
        {
            idAutoCloseTimer.stop()
            idRepeatTimer.stop()
            idAgsaFineAdjustment.visible = false
        }
    }
}
