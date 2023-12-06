import QtQuick 2.12

Rectangle
{
    id: multiCWheel

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // properties
    ///////////////////////////////////////////////////////////////////////////////////////////////

    property Item currentTextItem1;
    property Item currentTextItem2;
    property Item currentTextItem3;

    property int activeTextIndex: 0;
    property int characterLength: 2

    property bool triggerBeanNameUpdate: false

    width: 480
    height: 360
    color:"black"

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // functions
    ///////////////////////////////////////////////////////////////////////////////////////////////

    function triggerUpdate()
    {
        cWheelRow.triggerUpdate = !cWheelRow.triggerUpdate
    }

    function deleteText()
    {
        var resetSelection = false;

        textRepeater.itemAt(characterLength-1).text = " ";
        cWheelRow.mcBeanName = cWheelRow.mcBeanName.slice(0, characterLength-1) + " ";
        characterLength --;
        cWheelRow.currentIndex = characterLength;
        triggerUpdate();


        if( resetSelection )
        {
            activeTextIndex = 0;
            checkSelection();
            setCharacterValues()
        }
    }

    function init( beanName )
    {
        var name = "" + beanName;

        cWheelRow.mcBeanName = beanName;

        multiCWheel.activeTextIndex = 0;
        multiCWheel.characterLength = beanName.length

        var namelength = name.length
        if( namelength <= 15 )
        {
            for(var i = 0; i < beanName.length; i++ )
            {
                textRepeater.itemAt(i).text = "" + beanName.charAt(i);

            }
            for( var j = beanName.length; j <= 15; j++ )
            {
                if( textRepeater.itemAt(j) !== null)
                {
                    textRepeater.itemAt(j).text = " ";
                }
            }
        }
    }

    function getBeanName()
    {
        var currentBeanName = "";

        for(var i = 0; i < characterLength; i++ )
        {
            currentBeanName = currentBeanName.concat( textRepeater.itemAt(i).text )
        }

        return currentBeanName;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // signal handlers
    ///////////////////////////////////////////////////////////////////////////////////////////////

    onVisibleChanged:
    {
        if( visible )
        {
            cWheelRow.currentIndex = 0;
            cWheelRow.positionViewAtIndex(cWheelRow.currentIndex, PathView.Left)
            triggerUpdate()
        }
    }

    onCharacterLengthChanged:
    {
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // items
    ///////////////////////////////////////////////////////////////////////////////////////////////

    ListView
    {
        id: cWheelRow
        z:0

        property string mcBeanName: ""
        property bool triggerUpdate: false

        width: 336
        height: 360

        clip: false

        cacheBuffer: 1020

        anchors.left: parent.left
        anchors.leftMargin: 72
        anchors.top: parent.top

        boundsMovement: Flickable.StopAtBounds
        snapMode: ListView.SnapToItem
        flickableDirection: Flickable.HorizontalFlick
        orientation: ListView.Horizontal

        displayMarginBeginning: 85
        displayMarginEnd: 85

        preferredHighlightBegin:     currentIndex < ( model - 4 ) ? 0 : cWheelRow.width - 85
        preferredHighlightEnd:       currentIndex < ( model - 4 ) ? 85 : cWheelRow.width
        highlightRangeMode:          ListView.StrictlyEnforceRange
        highlightFollowsCurrentItem: true

        model: 15

        delegate: CharacterWheel
        {
            currentFontFamiliy: "D-DIN Condensed HEMRO"

            interactive: true

            letterToShow: index < cWheelRow.mcBeanName.length ? cWheelRow.mcBeanName.charAt(index) : "_"

            triggerUpdateView:  cWheelRow.triggerUpdate;

            onCurrentValue:
            {
                textRepeater.itemAt(index).text = value;

                if( index + 1 >= characterLength )
                {
                    characterLength = index + 1;
                }

                cWheelRow.mcBeanName = getBeanName();
            }

            Component.onCompleted:
            {
                triggerUpdate();
            }
        }
    }

    Row
    {
        id: textRow
        z:3

        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        anchors.topMargin: 295

        Repeater
        {
            id: textRepeater

            model: 15
            Text
            {
                visible: true
                text: " "
                font.family: "D-DIN Condensed HEMRO"
                font.bold: true
                font.pixelSize: 58
                color: idMain.colorEkxTextWhite

                Rectangle
                {
                    width: parent.width
                    height: 4
                    visible: ( cWheelRow.currentIndex ) < (cWheelRow.model - 4) ? ( ( index <= cWheelRow.currentIndex + 3 ) && ( index >= cWheelRow.currentIndex ) ) : ( ( index >= cWheelRow.currentIndex - 3 ) && ( index <= cWheelRow.currentIndex ) )
                    color:  "#41ffaa"
                    anchors.bottom: parent.bottom
                    anchors.bottomMargin: 5
                }
            }

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
