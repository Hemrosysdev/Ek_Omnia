import QtQuick 2.12

Item
{
    id: multiCWheel

    width: 480
    height: 360

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // properties
    ///////////////////////////////////////////////////////////////////////////////////////////////

    property string currentText: ""

    property var additionalCharacters: []

    property int maximumLength: 15

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // functions
    ///////////////////////////////////////////////////////////////////////////////////////////////

    function deleteText()
    {
        multiCWheel.currentText = multiCWheel.currentText.slice(0, -1);

        const length = multiCWheel.currentText.length;

        // If the end of the text where we have just deleted from is out of view, scroll there.
        if (cWheelRow.currentIndex < length - cWheelRow.endGap || cWheelRow.currentIndex > length - 1) {
            cWheelRow.currentIndex = Math.max(0, length - 1);
        }
    }

    function init(text)
    {
        multiCWheel.currentText = text;
    }

    // TODO remove
    function getBeanName()
    {
        return multiCWheel.currentText;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // signal handlers
    ///////////////////////////////////////////////////////////////////////////////////////////////

    onVisibleChanged:
    {
        if( visible )
        {
            cWheelRow.init();

            cWheelRow.currentIndex = 0;
            cWheelRow.positionViewAtIndex(cWheelRow.currentIndex, PathView.Left)
            idTextLabel.x = 0;
        }
    }

    Component.onCompleted: {
        if (visible) {
            cWheelRow.init();
        }
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // items
    ///////////////////////////////////////////////////////////////////////////////////////////////

    ListView
    {
        id: cWheelRow

        readonly property int endGap: 4

        // Qt's JavaScript environment doesn't support String.trimEnd.
        function trimEnd(str) {
            return str.replace(/\s+$/, "");
        }

        function init() {
            cWheelRow.model = Qt.binding(() => {
                return multiCWheel.maximumLength;
            });
        }

        anchors.fill: parent
        // Don't use leftMargin as in conjunction with StrictlyEnforceRange
        // lets the ListView overshoot the left edge despite StopAtBounds behavior!
        // rightMargin so that the last page is fully covered in letters.
        rightMargin: -85 * (endGap - 1)

        boundsBehavior: Flickable.StopAtBounds
        snapMode: ListView.SnapToItem
        flickableDirection: Flickable.HorizontalFlick
        orientation: ListView.Horizontal

        preferredHighlightBegin: 72
        preferredHighlightEnd: 72 + 85
        highlightMoveDuration: 250

        highlightRangeMode: ListView.StrictlyEnforceRange
        highlightFollowsCurrentItem: true

        // Deferred until the wheel is shown for the first time.
        model: null

        delegate: CharacterWheel {
            currentFontFamiliy: "D-DIN Condensed HEMRO"

            letterToShow: multiCWheel.currentText[index] || "\0"
            additionalCharacters: multiCWheel.additionalCharacters

            // Block interacting with the characters on the edge.
            enabled: index >= cWheelRow.currentIndex && index < cWheelRow.currentIndex + cWheelRow.endGap

            onCurrentValue: {
                // JS strings are immutable, have to splice it apart to insert our new item.
                multiCWheel.currentText = cWheelRow.trimEnd(
                    multiCWheel.currentText.substring(0, index).padEnd(index)
                        + value
                        + multiCWheel.currentText.substring(index + 1));

                // When editing the last visible character, advance view one forward.
                if (index === cWheelRow.currentIndex + cWheelRow.endGap - 1 && index < cWheelRow.count - 1) {
                    ++cWheelRow.currentIndex;
                }
            }
        }
    }

    // The text display with position indicator.
    Item {
        id: idTextLabelContainer
        anchors {
            horizontalCenter: parent.horizontalCenter
            top: parent.top
            topMargin: 295
        }
        width: Math.min(parent.width - 2 * 72, idTextLabel.width)
        height: idTextLabel.height
        z: 3
        clip: idTextLabel.width > width
        onWidthChanged: idTextLabelUnderline.returnToBounds()

        Label {
            id: idTextLabel
            bold: true
            pixelSize: 58
            text: multiCWheel.currentText.substr(0, multiCWheel.maximumLength).padEnd(multiCWheel.maximumLength)
            width: implicitWidth + idPlaceholderMetrics.advanceWidth * Math.max(0, cWheelRow.count - text.length)
            x: 0

            // Metrics for the "empty" spots. averageCharacterWidth is too wide.
            TextMetrics {
                id: idPlaceholderMetrics
                font: idTextLabel.font
                text: " "
            }

            // Metrics for the text before the underline.
            TextMetrics {
                id: idOffsetMetrics
                font: idTextLabel.font
                text: idTextLabel.text.substr(0, cWheelRow.currentIndex)
            }

            // Metrics for the underline width.
            TextMetrics {
                id: idIndicatorMetrics
                font: idTextLabel.font
                text: idTextLabel.text.substr(cWheelRow.currentIndex, cWheelRow.endGap)
            }

            Rectangle {
                id: idTextLabelUnderline
                anchors {
                    bottom: parent.bottom
                    bottomMargin: 5
                }
                x: idOffsetMetrics.advanceWidth
                width: idIndicatorMetrics.advanceWidth + idPlaceholderMetrics.advanceWidth * Math.max(0, cWheelRow.endGap - idTextLabel.text.substr(cWheelRow.currentIndex, cWheelRow.endGap).length)
                height: 4
                color: "#41ffaa"

                onXChanged: returnToBounds()
                onWidthChanged: returnToBounds()

                // Let the label scroll ensuring the highlighted part is always visible.
                function returnToBounds() {
                    if (idTextLabel.width > idTextLabelContainer.width) {
                        if (x > idTextLabelContainer.width - width - idTextLabel.x) { // scroll right.
                            idTextLabel.x = (idTextLabelContainer.width - width - x);
                        } else if (x < -idTextLabel.x) { // scroll left.
                            idTextLabel.x = -x;
                        }
                    } else {
                        idTextLabel.x = 0;
                    }
                }

                Behavior on x {
                    SmoothedAnimation {
                        duration: 250
                    }
                }
            }
        }

        ShadowGroup {
            shadowLeftSize: 40
            shadowRightSize: 40
            shadowLeftEnabled: idTextLabel.x < 0
            shadowRightEnabled: idTextLabel.x + idTextLabel.width > multiCWheel.width - 2 * 72
        }
    }

    ShadowGroup
    {
        z: 2;
        shadowLeftSize:   85;
        shadowRightSize:  85;
        shadowTopSize:    135;
        shadowBottomSize: 135;
    }

    Rectangle
    {
        z:1;
        anchors.left: parent.left;
        anchors.top: parent.top;
        width: 72;
        height: 130;
        color: "black";
    }

    Rectangle
    {
        z:1;
        anchors.left: parent.left;
        anchors.bottom: parent.bottom;
        width: 72;
        height:130;
        color: "black";
    }

    Rectangle
    {
        z:1;
        width: 72;
        height: 130;
        anchors.right: parent.right;
        anchors.bottom: parent.bottom;
        color: "black";
    }

    Rectangle
    {
        z:1;
        anchors.right: parent.right;
        anchors.top: parent.top;
        width: 72;
        height:130;
        color: "black";
    }

}
