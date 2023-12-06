import QtQuick 2.0

PathView
{
    property alias interactive: bhwLv.interactive
    property string currentFontFamiliy;

    property int itemOffset: 0
    property var dataList;

    property int  itemHeight:    180
    property int  itemWidth:     82

    property bool useIndexData: false

    property LongPressAnimation longPressAnimationInt: null

    signal currentValue( var value );

    readonly property var grammageIcons: [
        "qrc:/Icons/ICON_Single-80px_Cup_transparent.png",
        "qrc:/Icons/ICON-Strokes_Single-80px_Filter_white_transparent.png",
        "qrc:/Icons/ICON-Strokes_Double-V2-80px_Filter_white_transparent.png"]


    function setStartValues( startValue )
    {
        bhwLv.positionViewAtIndex( ( startValue ), PathView.Center)
    }

    id: bhwLv
    width:  itemWidth
    height: parent.height

    offset: itemOffset

    pathItemCount: 5

    clip:        true

    preferredHighlightBegin:     0.5
    preferredHighlightEnd:       0.5
    highlightRangeMode:          ListView.StrictlyEnforceRange

    model: grammageIcons

    delegate: MouseArea
    {
        id: lvdel

        property int currentAngle: PathView.pathAngle
        property int currentPixelSize: PathView.pixelsizeAngle
        property real currentOpacity:  PathView.opacityAngle

        width:  itemWidth
        height: itemHeight
        visible: PathView.onPath

        onClicked: {
            // HACK When PathView "wraps around" because there are too few items
            // in the model, as is the case with IconWheel, it behaves erratic
            // when assigning an index directly, e.g. going from 0 to 2 because
            // we can see "the other side".
            // Instead, figure out if we're above or below the current index
            // and then decrement/incrementCurrentIndex as needed.

            const centerItem = itemAt(bhwLv.width / 2, bhwLv.height / 2);
            if (y < centerItem.y) {
                bhwLv.decrementCurrentIndex();
            } else if (y > centerItem.y) {
                bhwLv.incrementCurrentIndex();
            }
            // decrement/incrementCurrentIndex does not trigger movementEnded signal.
            bhwLv.currentValue(index);
        }

        Image
        {
            id: icon

            width: 80
            height: 80

            source: grammageIcons[index]

            anchors.centerIn: parent
            // opacity:  currentOpacity
            layer.enabled: bhwLv.currentIndex === index && bhwLv.longPressAnimationInt && bhwLv.longPressAnimationInt.layerEnabled

            layer.samplerName: "maskSource"
            layer.effect: LongPressShaderEffect
            {
                id: idLongPressShaderEffect
                longPressAnimation: bhwLv.longPressAnimationInt
            }

            Rectangle
            {
                anchors.fill: parent
                color: idMain.colorEkxTextWhite
                opacity:  currentOpacity
                z: -2
            }
        }

        //opacity:  PathView.opacityAngle

        transform: Rotation
        {
            origin.x: lvdel.width/2;
            origin.y: lvdel.height/2;
            axis { x: 1; y: 0; z: 0 }
            angle: currentAngle
        }


    }
    path: Path
    {
        startX: bhwLv.width/2; startY: 45
        PathAttribute { name: "pathAngle"; value: 40 }
        PathAttribute { name: "opacityAngle"; value: 0.0 }
        PathAttribute { name: "pixelsizeAngle"; value: 82 }
        PathLine { x: bhwLv.width/2; y: bhwLv.height/2 }
        PathAttribute { name:"pathAngle"; value: 0 }
        PathAttribute { name: "opacityAngle"; value: 1.0 }
        PathAttribute { name: "pixelsizeAngle"; value: 82 }
        PathLine { x: bhwLv.width/2; y: bhwLv.height -45 }
        PathAttribute { name:"pathAngle"; value: -40 }
        PathAttribute { name: "opacityAngle"; value: 0.0 }
        PathAttribute { name: "pixelsizeAngle"; value: 82 }

    }
    onMovementEnded:
    {
        currentValue( ( currentIndex ) );
    }
}
