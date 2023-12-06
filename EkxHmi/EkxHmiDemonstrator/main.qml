import QtQml 2.12
import QtQuick 2.12
import QtQuick.Shapes 1.12

import "QmlComponents"

Item
{
    id: idEkxHmiDemonstratorMain

    visible: true
    width: 1689
    height: 1047

    property int centerScaleX: 844
    property int centerScaleY: 524

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // self defined properties
    ///////////////////////////////////////////////////////////////////////////////////////////////

    property bool showHelp: true

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // signal handlers
    ///////////////////////////////////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // items
    ///////////////////////////////////////////////////////////////////////////////////////////////

    Timer
    {
        interval: 3000
        repeat: false
        running: true
        onTriggered: showHelp = false
    }

    Image
    {
        id: idGrinderImage

        anchors.fill: parent

        source: "qrc:/Images/HEM-EKX-HMI-REd_02.png"
    }

    EkxUi
    {
        id: idEkxUi

        z: 3

        x: centerScaleX - width / 2
        y: centerScaleY - height / 2

        width: 480
        height: 360
        clip: true
    }

    Item
    {
        id: idDrehring

        z: 2
        transformOrigin: Item.Center

        x: centerScaleX - width / 2
        y: centerScaleY - height / 2

        width: idDrehringImage.sourceSize.width + 200
        height: idDrehringImage.sourceSize.height + 200

        property int centerX: width / 2
        property int centerY: height / 2

        Image
        {
            id: idDrehringImage

            anchors.centerIn: parent

            source: "qrc:/Images/HEM-EKX-HMI-Drehring.png"
            antialiasing: true

            transform: Rotation
            {
                origin.x: idDrehringImage.width / 2
                origin.y: idDrehringImage.height / 2
                angle: -dddCouple.simulatedScaleRotation * 1.005
            }
        }

        MouseArea
        {
            anchors.fill: parent;

            property double scaleRotationOffset: 0

            function calcDeg( mouseX, mouseY)
            {
                var point =  mapToItem( idDrehring, mouseX, mouseY);
                var diffX = (point.x - idDrehring.centerX);
                var diffY = -1 * (point.y - idDrehring.centerY);
                var rad   = Math.atan (diffY / diffX);
                var deg   = (rad * 180 / Math.PI);

                if ( diffX >= 0 && diffY >= 0)
                {
                    deg = 90 - Math.abs (deg);
                }
                else if ( diffX >= 0 && diffY < 0 )
                {
                    deg = 90 + Math.abs (deg);
                }
                else if ( diffX < 0 && diffY >= 0 )
                {
                    deg = 270 + Math.abs (deg);
                }
                else if ( diffX < 0 && diffY < 0 )
                {
                    deg = 270 - Math.abs (deg);
                }

                deg = ( -deg + 90 + 360 ) % 360;

                return deg;
            }

            onPressed:
            {
                scaleRotationOffset = calcDeg( mouse.x, mouse.y ) - dddCouple.simulatedScaleRotation;
            }

            onPositionChanged:
            {
                var deg = calcDeg( mouse.x, mouse.y ) - scaleRotationOffset;

                if ( deg >= 0 && deg <= 288 )
                {
                    dddCouple.simulatedScaleRotation = deg;
                }
            }
        }
    }

    Text
    {
        z: 5
        id: idDebug
        text: ""
        color: "red"
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        opacity: 1.0

        font.family: "D-DIN Condensed HEMRO"
        font.pixelSize: 40
    }

    Shape
    {
        id: idHelpTab

        z: 4

        width: idHelpText.width + idVerticalTab.width
        height: idHelpText.height

        x: parent.x + parent.width - width
        anchors.top: parent.top
        anchors.topMargin: 20

        opacity: 0.5

        ShapePath
        {
            strokeWidth: 3
            strokeColor: "white"
            fillColor: "black"

            startX: 0
            startY: 0
            PathLine { x: idHelpTab.width; y: 0 }
            PathLine { x: idHelpTab.width; y: idHelpTab.height }
            PathLine { x: idVerticalTab.width; y: idHelpTab.height }
            PathLine { x: idVerticalTab.width; y: idVerticalTab.height + 20 }
            PathLine { x: 0; y: idVerticalTab.height }
            PathLine { x: 0; y: 0 }
        }

        states: State
        {
            name: "hidden"; when: !showHelp
            PropertyChanges { target: idHelpTab; x: parent.x + parent.width - idVerticalTab.width }
        }

        transitions: Transition
        {
            NumberAnimation { properties: "x"; easing.type: Easing.InOutQuad; duration: 1000 }
        }

        Text
        {
            id: idHelpText

            anchors.right: parent.right

            padding: 20

            text: "F10 start/stop grinding\n\n"
                  + "+/- inc./dec. scale\n\n"
                  + "move scale with mouse\n\n"
                  + "P to toggle PDU"
            color: "white"

            opacity: 1.0

            font.family: "D-DIN Condensed HEMRO"
            font.pixelSize: 40
        }

        Item
        {
            id: idVerticalTab

            anchors.top: parent.top
            anchors.right: idHelpText.left

            width: idTabText.height + 6
            height: idTabText.width + 10

            Text
            {
                id: idTabText

                anchors.centerIn: parent

                text: "Help"
                color: "white"

                opacity: 1.0

                font.family: "D-DIN Condensed HEMRO"
                font.pixelSize: 40

                rotation: 270
            }
        }

        MouseArea
        {
            anchors.fill: parent

            onClicked:
            {
                showHelp = !showHelp;
            }
        }
    }

//    Text
//    {
//        id: idVersionText

//        anchors.right: parent.right
//        anchors.bottom: parent.bottom
//        anchors.leftMargin: 20
//        anchors.bottomMargin: 50

//        //padding: 20

//        text: "Version"
//        color: "white"
//        opacity: 0.5

//        font.family: "D-DIN Condensed HEMRO"
//        font.pixelSize: 40
//    }

    Image
    {
        id: idPduPresentDisplay

        visible: pduDcHallMotorDriver.motorTestOk
        z: 1

        x: 100
        y: 100

        source: "qrc:/Images/PduSymbol.png"
    }
}

