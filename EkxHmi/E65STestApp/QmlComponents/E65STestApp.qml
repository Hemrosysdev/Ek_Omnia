import QtQml 2.12
import QtQuick 2.12
import QtQuick.Window 2.12

Rectangle
{
    id: idMain

    visible: true
    width: 480
    height: 360
    color: "black"
    clip: true

    border.color: "blue"
    border.width: 1

    property color colorEkxTextWhite:    "white"
    property color colorEkxTextGrey:     Qt.rgba( 0.5, 0.5, 0.5, 1.0 )
    property color colorEkxConfirmGreen: "#00ffb3"
    property color colorEkxConfirmRed:   "#ff465f"
    property color colorEkxDarkGreen:    "#00805a"
    property color colorEkxWarning:      "#ff8b00"
    property color colorEkxError:        "#ff465f"

    property bool automated: true

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // self defined properties
    ///////////////////////////////////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // signal handlers
    ///////////////////////////////////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // items
    ///////////////////////////////////////////////////////////////////////////////////////////////

    Timer
    {
        interval: 500
        running: automated
        repeat: true

        property bool scrollDown: true

        onTriggered:
        {
            idPathView.incrementCurrentIndex()

            if ( idTextView.currentIndex === idTextView.count - 1 )
            {
                scrollDown = false
            }
            else if ( idTextView.currentIndex === 0 )
            {
                scrollDown = true
            }

            if ( scrollDown )
            {
                idTextView.incrementCurrentIndex()
            }
            else
            {
                idTextView.decrementCurrentIndex()
            }
        }
    }

    Text
    {
        id: idInfo
        text: "Auto: " + automated
        color: "yellow"
        x: 0
        y: 0
        font.pixelSize: 20
        font.family:    "D-DIN Condensed HEMRO"
        font.bold: true
    }

    MouseArea
    {
        anchors.fill: parent
        onClicked: automated = !automated
        z: automated ? 1 : 0
    }

    ListModel
    {
        id: idMenuModel

        ListElement
        {
            file: "qrc:/Icons/ICON-Strokes_Double-80px_portafilter_white.png"
            name: "1"
        }
        ListElement
        {
            file: "qrc:/Icons/ICON-Strokes_Single-80px_portafilter_white.png"
            name: "2"
        }
        ListElement
        {
            file: "qrc:/Icons/ICON-Strokes_Single-80px_Cup_white.png"
            name: "3"
        }
        ListElement
        {
            file: "qrc:/Icons/ICON-Strokes_Single-80px_Filter_white.png"
            name: "4"
        }
        ListElement
        {
            file: "qrc:/Icons/ICON-Strokes_Double-80px_portafilter_white.png"
            name: "5"
        }
        ListElement
        {
            file: "qrc:/Icons/ICON-Strokes_Single-80px_portafilter_white.png"
            name: "6"
        }
        ListElement
        {
            file: "qrc:/Icons/ICON-Strokes_Single-80px_Cup_white.png"
            name: "7"
        }
        ListElement
        {
            file: "qrc:/Icons/ICON-Strokes_Single-80px_Filter_white.png"
            name: "8"
        }
    }

    Rectangle
    {
        id: idMenuList

        visible: true

        x: 280
        width: parent.width - x
        anchors.top: parent.top
        anchors.bottom: parent.bottom

        border.color: "yellow"
        border.width: 1
        color: "transparent"
        clip: true

        Component
        {
            id: idDelegate

            Rectangle
            {
                property bool isCurrent: PathView.isCurrentItem

                width: 80
                height: 80
                border.width: isCurrent ? 3 : 1
                color: "transparent"

                Column
                {
                    id: idMenuColumn
                    anchors.fill: parent
                    opacity: isCurrent ? 1 : 0.5
                    Image
                    {
                        source: file
                    }
                    Text
                    {
                        width: parent.width
                        color: "white"
                        anchors.bottom: parent.bottom
                        text: name
                        font.bold: isCurrent ? true : false
                        font.pixelSize: 20
                        font.family:    "D-DIN Condensed HEMRO"
                    }
                }
                //                OpacityAnimator on opacity
                //                {
                //                    from: 0.5
                //                    to: isCurrent ? 1 : 0.5
                //                    duration: 500
                //                    target: idMenuColumn
                //                    running: true
                //                }
            }
        }

        PathView
        {
            id: idPathView
            anchors.fill: parent
            model: idMenuModel
            delegate: idDelegate
            snapMode: PathView.SnapToItem
            path: Path
            {
                PathAngleArc
                {
                    sweepAngle: 360
                    startAngle: -180
                    centerX: idMenuList.width + 40
                    centerY: idMenuList.height / 2
                    radiusX: idMenuList.height / 2
                    radiusY: idMenuList.height / 2
                }
            }
        }
    }

    ListModel
    {
        id: idContactModel

        ListElement
        {
            name: "Bill Smith"
            number: "555 3264"
        }
        ListElement
        {
            name: "John Brown"
            number: "555 8426"
        }
        ListElement
        {
            name: "Sam Wise"
            number: "555 0473"
        }
        ListElement
        {
            name: "Bill Smith"
            number: "555 3264"
        }
        ListElement
        {
            name: "John Brown"
            number: "555 8426"
        }
        ListElement
        {
            name: "Sam Wise"
            number: "555 0473"
        }
        ListElement
        {
            name: "Bill Smith"
            number: "555 3264"
        }
        ListElement
        {
            name: "John Brown"
            number: "555 8426"
        }
        ListElement
        {
            name: "Sam Wise"
            number: "555 0473"
        }
    }

    Rectangle
    {
        id: isTextList

        visible: true

        anchors.margins: 20
        anchors.left: parent.left
        anchors.right: idMenuList.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom

        border.color: "white"
        border.width: 1
        color: "transparent"
        clip: true

        ListView
        {
            id: idTextView

            anchors.fill: parent

            model: idContactModel
            delegate: Row
            {
                Image
                {
                    source: "qrc:/Icons/ICON-Strokes_Timer-30px_white_50%"
                }

                Text
                {
                    height: 100
                    font.pixelSize: 40
                    font.family:    "D-DIN Condensed HEMRO"

                    color: "white"
                    text: name + "\n" + number
                }
            }
            snapMode: ListView.SnapToItem;
            boundsMovement: Flickable.StopAtBounds
            //boundsMovement: Flickable.FollowBoundsBehavior

            //boundsBehavior: Flickable.StopAtBounds;
            //boundsBehavior: Flickable.DragOverBounds;
            //boundsBehavior: Flickable.OvershootBounds;
            boundsBehavior: Flickable.DragAndOvershootBounds;
            //flickDeceleration: 10
        }
    }
}

