import QtQuick 2.12

PathView
{
    id: bhwLv

    width:  itemWidth + rightOversize
    height: parent.height

    offset: itemOffset

    pathItemCount: 5

    preferredHighlightBegin:     0.5
    preferredHighlightEnd:       0.5
    highlightRangeMode:          ListView.StrictlyEnforceRange

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // self defined properties
    ///////////////////////////////////////////////////////////////////////////////////////////////

    property alias model: bhwLv.model
    property alias interactive: bhwLv.interactive
    property int pixelSize: 82
    property bool bold: false
    property string currentFontFamiliy;

    property int rightOversize: 0
    property int itemOffset: 0
    property var dataList;

    property int  itemHeight:    90
    property int  itemWidth:     72
    property int  textAlignment: Text.AlignRight
    property string suffix: ""

    property bool informAboutChange: false;

    property int oldIndex: 0;

    property int conversionNumber: 0
    property bool useIndexData: false
    property bool pinWrong: false
    property bool pinRight: false
    property int zeroLeadDigits: 0

    property LongPressAnimation longPressAnimationInt: null

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // signals
    ///////////////////////////////////////////////////////////////////////////////////////////////

    signal currentValue( var value );
    signal upChange();
    signal downChange();

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // functions
    ///////////////////////////////////////////////////////////////////////////////////////////////

    function setStartValues( startValue )
    {
        var tempInformAboutChange = informAboutChange
        informAboutChange = false

        bhwLv.positionViewAtIndex( ( startValue - conversionNumber ), PathView.Center)
        oldIndex = startValue

        informAboutChange = tempInformAboutChange
    }

    function padWithZero( num, targetLength )
    {
        return String( num ).padStart(targetLength, '0')
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

    delegate: Item
    {
        id: lvdel

        property int currentAngle: PathView.pathAngle
        property int currentPixelSize: PathView.pixelsizeAngle

        width:  parent.width
        height: itemHeight

        opacity:  PathView.opacityAngle
        visible: PathView.onPath

        transform: Rotation
        {
            origin.x: lvdel.width/2; origin.y: lvdel.height/2; axis { x: 1; y: 0; z: 0 } angle: currentAngle
        }

        Text
        {
            width: itemWidth
            height: parent.height

            anchors.rightMargin: rightOversize

            text: (useIndexData ? dataList[index] : number) + bhwLv.suffix

            color: ( currentIndex === index && pinWrong ) ? idMain.colorEkxConfirmRed : ( currentIndex === index && pinRight ) ? idMain.colorEkxConfirmGreen : idMain.colorEkxTextWhite
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: bhwLv.textAlignment
            renderType: Text.NativeRendering

            font.pixelSize: currentPixelSize
            font.family:   currentFontFamiliy
            font.bold: bhwLv.bold

            property string number: ( zeroLeadDigits === 0 ) ? String( index + conversionNumber ) : padWithZero( index + conversionNumber, zeroLeadDigits )

            layer.enabled: currentIndex === index && bhwLv.longPressAnimationInt && bhwLv.longPressAnimationInt.layerEnabled
            layer.smooth: true

            layer.samplerName: "maskSource"
            layer.effect: LongPressShaderEffect
            {
                longPressAnimation: longPressAnimationInt
            }
        }
    }

    path: Path
    {
        startX: bhwLv.width/2; startY: bhwLv.height + 45
        PathAttribute { name: "pathAngle"; value: -60 }
        PathAttribute { name: "opacityAngle"; value: 0.2 }
        PathAttribute { name: "pixelsizeAngle"; value: bhwLv.pixelSize }
        PathLine { x: bhwLv.width/2; y: bhwLv.height/2 }
        PathAttribute { name:"pathAngle"; value: 0 }
        PathAttribute { name: "opacityAngle"; value: 1.0 }
        PathAttribute { name: "pixelsizeAngle"; value: bhwLv.pixelSize }
        PathLine { x: bhwLv.width/2; y: -45 }
        PathAttribute { name:"pathAngle"; value: 60 }
        PathAttribute { name: "opacityAngle"; value: 0.2 }
        PathAttribute { name: "pixelsizeAngle"; value: bhwLv.pixelSize }
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // signal handlers
    ///////////////////////////////////////////////////////////////////////////////////////////////

    onMovementEnded:
    {
        currentValue( ( currentIndex + conversionNumber ) );
    }

    onCurrentIndexChanged:
    {
        if (  informAboutChange )
        {
            if( oldIndex === ( bhwLv.count - 1 ) && bhwLv.currentIndex === 0 )
            {
                upChange();
            }
            if( oldIndex === 0 && bhwLv.currentIndex === ( bhwLv.count - 1  ) )
            {
                downChange()
            }

            oldIndex = bhwLv.currentIndex;

            currentValue( ( currentIndex + conversionNumber ) );
        }
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
            const oldInformAboutChange = bhwLv.informAboutChange;
            bhwLv.informAboutChange = true; // Reuse onCurrentIndexChanged logic.
            ++bhwLv.currentIndex;
            bhwLv.informAboutChange = oldInformAboutChange;
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
            const oldInformAboutChange = bhwLv.informAboutChange;
            bhwLv.informAboutChange = true; // Reuse onCurrentIndexChanged logic.
            --bhwLv.currentIndex;
            bhwLv.informAboutChange = oldInformAboutChange;
        }
    }
}
