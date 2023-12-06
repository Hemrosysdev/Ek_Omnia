import QtQml 2.12
import QtQuick 2.0

import "../../CommonItems"

Rectangle
{
    width: 480
    height: 360

    color: "black"

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // self defined properties
    ///////////////////////////////////////////////////////////////////////////////////////////////

    property int grammageCount: 2
    property var recipeList: recipeControl.installedLibraryList;
    property int editLibraryIndex;

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // signals
    ///////////////////////////////////////////////////////////////////////////////////////////////

    signal closeEditRecipeList()
    signal openGrammageEditList( var grammageIndex );

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // functions
    ///////////////////////////////////////////////////////////////////////////////////////////////

    function getIndexedGrammageValue( cGrammageIndex )
    {
        var grammageValue = 0.0;
        if( cGrammageIndex === 0 )
        {
            grammageValue = parseFloat(recipeList[editLibraryIndex].grammage1Big + "." + recipeList[editLibraryIndex].grammage1Comma);
        }
        if( cGrammageIndex === 1 )
        {
            grammageValue = parseFloat(recipeList[editLibraryIndex].grammage2Big + "." + recipeList[editLibraryIndex].grammage2Comma);
        }
        return grammageValue;

    }

    function getIndexedGrammageIcon( cGrammageIndex )
    {
        var grammageIcon = 0;

        if( cGrammageIndex === 0 )
        {
            grammageIcon = recipeList[editLibraryIndex].grammage1Icon
        }
        if( cGrammageIndex === 1 )
        {
            grammageIcon = recipeList[editLibraryIndex].grammage2Icon
        }

        return grammageIcon;

    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // signal handlers
    ///////////////////////////////////////////////////////////////////////////////////////////////

    onVisibleChanged:
    {
        if(visible)
        {
            animationTimer.start();
        }
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // items
    ///////////////////////////////////////////////////////////////////////////////////////////////

    Timer
    {
        id: animationTimer

        property int counter: 0;

        interval: 400; running: false; repeat: true

        onTriggered:
        {
            if( counter < grammageCount )
            {
                grammageListRepeater.itemAt(counter).startJiggling();
                counter++;
            }
            else
            {
                counter = 0;
                animationTimer.stop();
            }

        }
    }

    Item
    {
        id: textInfoItem

        width: 480
        height: 152
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.topMargin: 72

        visible: grammageCount === 0;

        Text
        {
            id: textInfo
            text: "Add a \n grammage note"

            anchors.centerIn: parent
            anchors.fill: parent

            font.pixelSize: 58
            font.letterSpacing: 2.0
            font.bold: true
            horizontalAlignment: Text.AlignHCenter
            font.family: "D-DIN Condensed HEMRO"

            wrapMode: Text.Wrap

            color: idMain.colorEkxTextWhite
        }
    }

    Column
    {
        id: grammageRow

        anchors.left: parent.left
        anchors.top: parent.top
        anchors.topMargin: 72
        spacing: 16

        width: 480
        height: 152

        visible: grammageCount > 0;

        Repeater
        {
            id: grammageListRepeater

            model: grammageCount

            GrammageEditListEntry
            {
                currentGrammageIndex: index;
                currentEditLibraryIndex: editLibraryIndex

                onOpenGrammageEditListEntry:
                {
                    openGrammageEditList( listGrammageIndex )
                }

                onVisibleChanged:
                {
                    if( visible )
                    {
                        currentGrammage = getIndexedGrammageValue(index)
                        currentGrammageIcon = getIndexedGrammageIcon(index)
                    }
                }

                Component.onCompleted:
                {
                    currentGrammage = getIndexedGrammageValue(index)
                    currentGrammageIcon = getIndexedGrammageIcon(index)
                }
            }
        }
    }

    EkxButton
    {
        // id: idButtonClose;
        buttonType: EkxButton.ButtonType.CLOSE;
        onClicked:
        {
            closeEditRecipeList();
        }
    }

    EkxButton
    {
        // id: idButtonAdd;
        visible: grammageCount < 2 ? true : false
        anchors.horizontalCenter: grammageRow.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 90
        buttonType: EkxButton.ButtonType.PLUS;
        onClicked:
        {
            recipeControl.addLibraryGrammage( editLibraryIndex )
            openGrammageEditList( grammageCount - 1 )
        }
    }
}
