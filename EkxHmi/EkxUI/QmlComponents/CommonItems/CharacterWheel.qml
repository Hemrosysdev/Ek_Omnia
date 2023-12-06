import QtQuick 2.0

PathView
{
    property alias interactive: bhwLv.interactive
    //property alias startValue: bhwLv.currentIndex
    property string currentFontFamiliy;

    property int  itemHeight:    90
    property int  itemWidth:     84

    signal currentValue(string value)

    property var additionalCharacters: []

    readonly property var alphabet: {
        const uppercase = ["A","B","C","D","E","F","G","H","I","J","K","L","M","N","O","P","Q","R","S","T","U","V","W","X","Y","Z"];
        const lowercase = ["a","b","c","d","e","f","g","h","i","j","k","l","m","n","o","p","q","r","s","t","u","v","w","x","y","z"];
        const numbers   = ["0", "1","2","3","4","5","6","7","8","9"];

        let alphabet = [...uppercase, " ", ...lowercase, " ", ...numbers, " "];
        if (additionalCharacters.length > 0) {
            alphabet.push(...additionalCharacters, " ");
        }

        return alphabet;
    }

    // finds the "needle" in given "str" starting at "start"
    // in direction "direction", wrapping around at the end of "str".
    function findWrapAround(str, needle, start, direction) {
        if (!str) {
            return null;
        }

        let distance = 0;
        for (let i = start; distance < str.length; i += direction) {
            if (i >= str.length) {
                i = 0;
            } else if (i < 0) {
                i = str.length - 1;
            }

            const chr = str[i];
            if (chr === needle) {
                return {distance: distance, index: i};
            }

            ++distance;
        }

        return null;
    }

    function updateTextInListview()
    {
        // Don't bother if we're already displaying the right letter.
        const currentLetter = alphabet[currentIndex];
        if (letterToShow === currentLetter || (letterToShow === "\0" && currentLetter === " ")) {
            return;
        }

        // First check whether we support this character at all.
        let newIndex = alphabet.indexOf(letterToShow);

        if (newIndex === -1) {
            // Unknown/empty character, go blank, between Z and a.
            if (letterToShow !== "\0") {
                console.warn("CharacterWheel: letterToShow", letterToShow, "is not in the alphabet!");
            }
            newIndex = alphabet.indexOf(" ");
        // If visible, animate the move and search the closest matching character.
        } else if (visible) {
            const beforeIndex = findWrapAround(alphabet, letterToShow, currentIndex, -1);
            const afterIndex = findWrapAround(alphabet, letterToShow, currentIndex, +1);

            if (beforeIndex.distance < afterIndex.distance) {
                newIndex = beforeIndex.index;
            } else {
                newIndex = afterIndex.index;
            }
        }

        if (visible) { // animated
            currentIndex = newIndex;
        } else {
            bhwLv.positionViewAtIndex(newIndex, PathView.Center);
        }
    }

    property string letterToShow: ""

    onLetterToShowChanged:
    {
        updateTextInListview();
    }

    Component.onCompleted: {
        updateTextInListview();
    }

    id: bhwLv
    width:  itemWidth
    height: 360

    pathItemCount: 5

        preferredHighlightBegin:     0.5
        preferredHighlightEnd:       0.5
        highlightRangeMode:          ListView.StrictlyEnforceRange

        model: alphabet

    delegate: Item
    {
        id: lvdel

        property int angleWidth: !PathView.isCurrentItem ? ( itemWidth * 2 ) : itemWidth
        property int angleHeight: !PathView.isCurrentItem ? ( itemHeight * 2 ) : itemHeight

        property int currentAngle: PathView.pathAngle || 0

        width:  itemWidth     //angleWidth
        height: itemHeight    //angleHeight
        visible: PathView.onPath

        Label
        {
            anchors.fill: parent
            text: modelData
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            pixelSize: 82
            // Otherwise Glyph cache upload stalls the rendering for hundrets of milliseconds on first swipe.
            renderType: Text.NativeRendering

            font.family:   currentFontFamiliy
        }

        opacity: PathView.opacityAngle || 1

        transform:
            Rotation
            {
                origin.x: lvdel.width/2; origin.y: lvdel.height/2; axis { x: 1; y: 0; z: 0 } angle: currentAngle
            }


    }
    path: Path
    {
        startX: bhwLv.width/2; startY: bhwLv.height + 45
        PathAttribute { name: "pathAngle"; value: -60 }
        PathAttribute { name: "opacityAngle"; value: 0.2 }
        PathLine { x: bhwLv.width/2; y: bhwLv.height/2 }
        PathAttribute { name:"pathAngle"; value: 0 }
        PathAttribute { name: "opacityAngle"; value: 1.0 }
        PathLine { x: bhwLv.width/2; y: -45 }
        PathAttribute { name:"pathAngle"; value: 60 }
        PathAttribute { name: "opacityAngle"; value: 0.2 }
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // items
    ///////////////////////////////////////////////////////////////////////////////////////////////

    // Up mouse area.
    MouseArea {
        anchors {
            left: parent.left
            right: parent.right
            top: parent.top
        }
        height: (parent.height - bhwLv.itemHeight) / 2
        onClicked: {
            bhwLv.incrementCurrentIndex()
            currentValue(bhwLv.alphabet[bhwLv.currentIndex]);
        }
    }

    // Down mouse area.
    MouseArea {
        anchors {
            left: parent.left
            right: parent.right
            bottom: parent.bottom
        }
        height: (parent.height - bhwLv.itemHeight) / 2
        onClicked: {
            bhwLv.decrementCurrentIndex();
            currentValue(bhwLv.alphabet[bhwLv.currentIndex]);
        }
    }

    onMovementEnded:
    {
        currentValue(alphabet[currentIndex]);
    }
}
