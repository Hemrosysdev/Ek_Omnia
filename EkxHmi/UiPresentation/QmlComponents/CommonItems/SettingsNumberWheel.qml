import QtQuick 2.12

PathView
{
    property alias model: bhwLv.model
    property alias interactive: bhwLv.interactive
    //property alias startValue: bhwLv.currentIndex
    property string currentFontFamiliy;

    property int itemOffset: 0
    property var dataList;

    property int  itemHeight:    90
    property int  itemWidth:     72

    property bool informAboutChange: false;

    property int oldIndex: 0;

    signal upChange();
    signal downChange();

    property int conversionNumber: 0
    property bool useIndexData: false
    property bool pinWrong: false
    property bool pinRight: false

    signal currentValue( var value );

    function setStartValues( startValue )
    {
        bhwLv.positionViewAtIndex( ( startValue - conversionNumber ), PathView.Center)
        oldIndex = startValue
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

    delegate: Rectangle
    {
        id: lvdel

        property int currentAngle: PathView.pathAngle
        property int currentPixelSize: PathView.pixelsizeAngle

        width:  itemWidth
        height: itemHeight

        color:  "transparent"

        Text
        {
            width: parent.width
            height: parent.height
            text:  useIndexData ? dataList[index] : "" + (index + conversionNumber)
            color: ( currentIndex === index && pinWrong ) ? idMain.colorEkxConfirmRed : ( currentIndex === index && pinRight ) ? idMain.colorEkxConfirmGreen : idMain.colorEkxTextWhite

            font.pixelSize: currentPixelSize

            font.family:   currentFontFamiliy

            horizontalAlignment: Text.AlignRight

        }

        opacity:  PathView.opacityAngle

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
        PathAttribute { name: "pixelsizeAngle"; value: 82 }
        PathLine { x: bhwLv.width/2; y: bhwLv.height/2 }
        PathAttribute { name:"pathAngle"; value: 0 }
        PathAttribute { name: "opacityAngle"; value: 1.0 }
        PathAttribute { name: "pixelsizeAngle"; value: 82 }
        PathLine { x: bhwLv.width/2; y: -45 }
        PathAttribute { name:"pathAngle"; value: 60 }
        PathAttribute { name: "opacityAngle"; value: 0.2 }
        PathAttribute { name: "pixelsizeAngle"; value: 82 }

    }
    onMovementEnded:
    {
        currentValue( ( currentIndex + conversionNumber ) );
    }

    onCurrentIndexChanged:
    {
        if(  informAboutChange )
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

}
