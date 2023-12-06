import QtQuick 2.0

PathView
{
    property alias interactive: bhwLv.interactive
    //property alias startValue: bhwLv.currentIndex
    property string currentFontFamiliy;

    property int  itemHeight:    90
    property int  itemWidth:     84

    property bool upperCase: false

    signal currentValue( var value, var capitals );

    property variant alphabet: ["A","B","C","D","E","F","G","H","I","J","K","L","M","N","O","P","Q","R","S","T","U","V","W","X","Y","Z"," ","a","b","c","d","e","f","g","h","i","j","k","l","m","n","o","p","q","r","s","t","u","v","w","x","y","z"," ", "0", "1","2","3","4","5","6","7","8","9", " "]

    function updateTextInListview( )
    {
        var letter = letterToShow;
        if( letter === "_" )
        {

               letter = " ";
        }

        for(var i = 0 ; i < alphabet.length; i++ )
        {
            if( letter === alphabet[i] )
            {
                bhwLv.positionViewAtIndex(i, PathView.Center)
                break;
            }

            if( letter.toLowerCase() === alphabet[i] )
            {
                bhwLv.positionViewAtIndex(i, PathView.Center)
                break;
            }
        }
    }
    property string letterToShow: ""
    property bool triggerUpdateView: false

    onLetterToShowChanged:
    {
        updateTextInListview();
    }

    onTriggerUpdateViewChanged:
    {
        updateTextInListview();
    }



    id: bhwLv
    width:  itemWidth
    height: 360

    pathItemCount: 5

    clip:        true

        preferredHighlightBegin:     0.5
        preferredHighlightEnd:       0.5
        highlightRangeMode:          ListView.StrictlyEnforceRange

        model: alphabet.length

    delegate: Rectangle
    {
        id: lvdel

        property int angleWidth: !PathView.isCurrentItem ? ( itemWidth * 2 ) : itemWidth
        property int angleHeight: !PathView.isCurrentItem ? ( itemHeight * 2 ) : itemHeight

        property int currentAngle: PathView.pathAngle
        property int currentPixelSize: PathView.pixelsizeAngle

        width:  itemWidth     //angleWidth
        height: itemHeight    //angleHeight

        color:  "transparent"

        Text
        {
            text: upperCase ?  bhwLv.alphabet[index].toUpperCase() :  bhwLv.alphabet[index]
            color: idMain.colorEkxTextWhite

            font.pixelSize: currentPixelSize

            font.family:   currentFontFamiliy

            anchors.centerIn: parent
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
        currentValue(alphabet[currentIndex],upperCase);
    }
}
