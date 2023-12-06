import QtQuick 2.12

PathView
{
    id: idRoot

    width:  itemWidth
    height: parent.height

    pathItemCount: 3

    clip:        true

    preferredHighlightBegin:     0.5
    preferredHighlightEnd:       0.5
    highlightRangeMode:          ListView.StrictlyEnforceRange

    maximumFlickVelocity:  2200

    flickDeceleration:     400

    delegate: Item
    {
        id: idDelegate

        width:  itemWidth     //angleWidth
        height: itemHeight    //angleHeight

        property int angleWidth: !PathView.isCurrentItem ? ( itemWidth * 2 ) : itemWidth
        property int angleHeight: !PathView.isCurrentItem ? ( itemHeight * 2 ) : itemHeight

        property int currentAngle: PathView.pathAngle
        property int currentPixelSize: PathView.pixelsizeAngle

        Text
        {
            id: idNumberWheelText

            anchors.horizontalCenter: ( (index < 10 ) && !smallValue ) ? undefined : parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter

            anchors.right: ( (index < 10 ) && !smallValue ) ? parent.right : undefined
            anchors.rightMargin: width/3 - 5

            text:  "" + index
            color: idMain.colorEkxTextWhite

            font.pixelSize: currentPixelSize

            font.family:   currentFontFamiliy

            antialiasing: true
            renderType: Text.NativeRendering

            layer.enabled: idRoot.currentIndex === index ? true : false
            layer.smooth: true

            layer.samplerName: "maskSource"
            layer.effect: LongPressShaderEffect
            {
                id: idLongPressShaderEffect
                longPressAnimation: longPressAnimationInt
            }
        }

        opacity:  PathView.opacityAngle

        transform: Rotation
        {
            origin.x: idDelegate.width/2;
            origin.y: idDelegate.height/2;
            axis { x: 1; y: 0; z: 0 }
            angle: currentAngle
        }
    }

    path: Path
    {
        startX: idRoot.width/2;
        startY: idRoot.height+40
        PathAttribute { name: "pathAngle"; value: -95 }
        PathAttribute { name: "opacityAngle"; value: 0.5 }
        PathAttribute { name: "pixelsizeAngle"; value: 160 }
        PathLine { x: idRoot.width/2; y: idRoot.height/2 }
        PathAttribute { name:"pathAngle"; value: 0 }
        PathAttribute { name: "opacityAngle"; value: 1.0 }
        PathAttribute { name: "pixelsizeAngle"; value: 174 }
        PathLine { x: idRoot.width/2; y: -30  }
        PathAttribute { name:"pathAngle"; value: 95 }
        PathAttribute { name: "opacityAngle"; value: 0.5 }
        PathAttribute { name: "pixelsizeAngle"; value: 160 }

    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // self defined properties
    ///////////////////////////////////////////////////////////////////////////////////////////////

    property alias model: idRoot.model
    property alias interactive: idRoot.interactive
    property string currentFontFamiliy;

    property bool informAboutChange: false;
    property bool smallValue: true

    property int  itemHeight:    200
    property int  itemWidth:     152

    property int oldIndex: 0;

    property color fillColor: idMain.colorEkxConfirmRed

    property LongPressAnimation longPressAnimationInt: null

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // signals
    ///////////////////////////////////////////////////////////////////////////////////////////////

    signal upChange();
    signal downChange();
    signal currentValue( var value );

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // functions
    ///////////////////////////////////////////////////////////////////////////////////////////////

    function setStartValues( startValue )
    {
        idRoot.positionViewAtIndex(startValue, PathView.Center)
        oldIndex = startValue;
    }

    function addRedHeight()
    {
        redRect.height = redRect.height + 3
    }

    function resetRedHeight()
    {
        redRect.height = 0;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // signal handlers
    ///////////////////////////////////////////////////////////////////////////////////////////////

    onMovementEnded:
    {
        currentValue(currentIndex);
    }

    onCurrentIndexChanged:
    {
        if (  informAboutChange )
        {
            if ( oldIndex === ( idRoot.count - 1 ) && idRoot.currentIndex === 0 )
            {
                upChange();
            }
            if ( oldIndex === 0 && idRoot.currentIndex === ( idRoot.count - 1  ) )
            {
                downChange()
            }
            oldIndex = idRoot.currentIndex;
        }
    }
}
