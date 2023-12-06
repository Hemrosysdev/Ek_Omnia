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

    signal currentValue( var value );

    property variant grammageIcons: [
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

    delegate: Rectangle
    {
        id: lvdel

        property int currentAngle: PathView.pathAngle
        property int currentPixelSize: PathView.pixelsizeAngle
        property real currentOpacity:  PathView.opacityAngle

        width:  itemWidth
        height: itemHeight


        color:  "transparent"

        Rectangle
        {
            anchors.fill: icon
            anchors.centerIn: icon
            color: idMain.colorEkxTextWhite
            opacity:  currentOpacity
        }

        Image
        {
            id: icon

            width: 80
            height: 80

            source: grammageIcons[index]

            anchors.centerIn: parent
            // opacity:  currentOpacity
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
