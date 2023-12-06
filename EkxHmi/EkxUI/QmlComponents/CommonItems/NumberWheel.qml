import QtQuick 2.12

PathView
{
    id: idRoot

    width:  itemWidth
    height: parent.height

    pathItemCount: 3

    preferredHighlightBegin:     0.5
    preferredHighlightEnd:       0.5
    highlightRangeMode:          ListView.StrictlyEnforceRange

    maximumFlickVelocity:  2200

    flickDeceleration:     400

    delegate: MouseArea
    {
        id: idDelegate

        width:  itemWidth     //angleWidth
        height: itemHeight    //angleHeight
        visible: PathView.onPath

        property int angleWidth: !PathView.isCurrentItem ? ( itemWidth * 2 ) : itemWidth
        property int angleHeight: !PathView.isCurrentItem ? ( itemHeight * 2 ) : itemHeight

        property int currentAngle: PathView.pathAngle

        onClicked: {
            const oldInformAboutChange = idRoot.informAboutChange;
            idRoot.informAboutChange = true; // Reuse onCurrentIndexChanged logic.
            idRoot.currentIndex = index;
            idRoot.informAboutChange = oldInformAboutChange;

            idRoot.currentValue(index);
        }

        Text
        {
            id: idNumberWheelText

            anchors.horizontalCenter: ( (index < 10 ) && !smallValue ) ? undefined : parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter

            anchors.right: ( (index < 10 ) && !smallValue ) ? parent.right : undefined
            anchors.rightMargin: width/3 - 5

            text:  "" + index
            color: idRoot.currentIndex === index ? idRoot.numberColor : idMain.colorEkxTextWhite

            font.pixelSize: 174

            font.family:   currentFontFamiliy

            antialiasing: true
            renderType: Text.NativeRendering

            layer.enabled: currentIndex === index && idRoot.longPressAnimationInt && idRoot.longPressAnimationInt.layerEnabled
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
            origin.x: Math.round(idDelegate.width / 2)
            origin.y: Math.round(idDelegate.height / 2)
            axis { x: 1; y: 0; z: 0 }
            angle: currentAngle
        }
    }

    path: Path
    {
        startX: Math.round(idRoot.width / 2)
        startY: idRoot.height + 45
        PathAttribute { name: "pathAngle"; value: -60 }
        PathAttribute { name: "opacityAngle"; value: 0.2 }

        PathLine { x: Math.round(idRoot.width / 2); y: Math.round(idRoot.height / 2) }
        PathAttribute { name: "pathAngle"; value: 0 }
        PathAttribute { name: "opacityAngle"; value: 1 }

        PathLine { x: Math.round(idRoot.width / 2); y: -45 }
        PathAttribute { name: "pathAngle"; value: +60 }
        PathAttribute { name: "opacityAngle"; value: 0.2 }
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

    property color numberColor: idMain.colorEkxTextWhite

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
        oldIndex = startValue;
        idRoot.positionViewAtIndex(oldIndex, PathView.Center)
    }

    function incByNeighbour()
    {
        incrementCurrentIndex();
        currentValue( currentIndex );
    }

    function decByNeighbour()
    {
        decrementCurrentIndex();
        currentValue( currentIndex );
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // signal handlers
    ///////////////////////////////////////////////////////////////////////////////////////////////

    onMovementEnded:
    {
        currentValue( currentIndex );
    }

    onCurrentIndexChanged:
    {
        if (  informAboutChange )
        {
            if ( oldIndex === ( idRoot.count - 1 ) && idRoot.currentIndex === 0 )
            {
                upChange();
            }
            else if ( oldIndex === 0 && idRoot.currentIndex === ( idRoot.count - 1  ) )
            {
                downChange()
            }
        }
        oldIndex = idRoot.currentIndex;
    }
}
