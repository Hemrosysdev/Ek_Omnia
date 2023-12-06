import QtQml 2.12
import QtQuick 2.0
import RecipeControl 1.0
import "../../CommonItems"

Item
{
    id: idRoot

    height: 177
    width: 256

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // self defined properties
    ///////////////////////////////////////////////////////////////////////////////////////////////

    property bool visualDebug: false;

    property variant grammageIcons: [
        "qrc:/Icons/ICON-Strokes_Single-80px_Cup_white.png",
        "qrc:/Icons/ICON-Strokes_Single-80px_Filter_white.png",
        "qrc:/Icons/ICON-Strokes_Double-V2-80px_Filter_white.png"
    ]

    property int editLibraryIndex: recipeControl.currentLibraryRecipeIndex

    property real currentGrammage1;
    property real currentGrammage2;

    property bool grammageOneExists;
    property bool grammageTwoExists;

    property bool grammageVisible: recipeControl.installedLibraryList[editLibraryIndex].showGrammage

    property bool rowOneVisible: grammageOneExists && grammageVisible;
    property bool rowTwoVisible: grammageTwoExists && grammageVisible;

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // signals
    ///////////////////////////////////////////////////////////////////////////////////////////////

    signal editGrammage(var grammageIndex)

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // functions
    ///////////////////////////////////////////////////////////////////////////////////////////////

    function abortLongPress()
    {
        idRow1Animation.abort()
        idRow2Animation.abort()
    }

    function setGrammageIcon()
    {
        idRow1Icon.source = grammageIcons[recipeControl.installedLibraryList[editLibraryIndex].grammage1Icon]
        idRow2Icon.source = grammageIcons[recipeControl.installedLibraryList[editLibraryIndex].grammage2Icon]
    }

    function setGrammageText()
    {
        currentGrammage1 = parseFloat(recipeControl.installedLibraryList[editLibraryIndex].grammage1Big
                                      + "."
                                      + recipeControl.installedLibraryList[editLibraryIndex].grammage1Comma);

        currentGrammage2 = parseFloat(recipeControl.installedLibraryList[editLibraryIndex].grammage2Big
                                      + "."
                                      + recipeControl.installedLibraryList[editLibraryIndex].grammage2Comma);

        grammageOneExists = !(recipeControl.installedLibraryList[editLibraryIndex].grammage1Icon === 0
                              && recipeControl.installedLibraryList[editLibraryIndex].grammage1Big === 0
                              && recipeControl.installedLibraryList[editLibraryIndex].grammage1Comma === 0 )

        grammageTwoExists = !(recipeControl.installedLibraryList[editLibraryIndex].grammage2Icon === 0
                              && recipeControl.installedLibraryList[editLibraryIndex].grammage2Big === 0
                              && recipeControl.installedLibraryList[editLibraryIndex].grammage2Comma === 0 )

        grammageVisible =  recipeControl.installedLibraryList[editLibraryIndex].showGrammage;
    }

    function updateGrammageView()
    {
        setGrammageIcon();
        setGrammageText();
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // signal handlers
    ///////////////////////////////////////////////////////////////////////////////////////////////

    onEditLibraryIndexChanged:
    {
        updateGrammageView();
    }

    onVisibleChanged:
    {
        updateGrammageView();
    }

    Component.onCompleted:
    {
        updateGrammageView();
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // items
    ///////////////////////////////////////////////////////////////////////////////////////////////

    Item
    {
        id: idRow1

        visible: rowOneVisible

        width: 256
        height: 96

        anchors.top: parent.top
        anchors.left: parent.left

        Image
        {
            id: idRow1Icon

            width: 96
            height: 80

            anchors.left: parent.left

            layer.enabled: true
            layer.smooth: true

            // This item should be used as the 'mask'
            layer.samplerName: "maskSource"
            layer.effect: LongPressShaderEffect
            {
                longPressAnimation: idRow1Animation
            }
        }

        Text
        {
            id: idRow1Text

            anchors.left: idRow1Icon.right
            anchors.leftMargin: 13
            anchors.top: parent.top
            anchors.topMargin: 8

            text: currentGrammage1.toFixed(1).toString() + " t"
            color:  idMain.colorEkxTextWhite

            font.pixelSize: 82
            font.family:    "D-DIN Condensed HEMRO"
            font.letterSpacing: 0.33

            horizontalAlignment: Text.AlignRight

            layer.enabled: true
            layer.smooth: true;

            // This item should be used as the 'mask'
            layer.samplerName: "maskSource"
            layer.effect: LongPressShaderEffect
            {
                longPressAnimation: idRow1Animation
            }
        }

        // the mouse area over icon and text - complete row
        LongPressAnimation
        {
            id: idRow1Animation

            anchors.fill: parent

            fillColor: idMain.colorEkxConfirmGreen

            onLongPressed:
            {
                editGrammage(0);
            }
        }
    }

    Item
    {
        id: idRow2

        visible: rowTwoVisible

        width: 256
        height: 96

        anchors.top: idRow1.bottom
        anchors.topMargin: 13
        anchors.left: parent.left

        Image
        {
            id: idRow2Icon

            width: 96
            height: 96

            anchors.left: parent.left

            layer.enabled: true
            layer.smooth: true;

            // This item should be used as the 'mask'
            layer.samplerName: "maskSource"
            layer.effect: LongPressShaderEffect
            {
                longPressAnimation: idRow2Animation
            }
        }

        Text
        {
            id: idRow2Text

            anchors.left: idRow2Icon.right
            anchors.leftMargin: 13
            anchors.top: parent.top
            anchors.topMargin: 8

            text: currentGrammage2.toFixed(1).toString() + " t"
            color:  idMain.colorEkxTextWhite

            font.pixelSize: 82
            font.family:    "D-DIN Condensed HEMRO"
            font.letterSpacing: 0.33

            horizontalAlignment: Text.AlignRight

            layer.enabled: true
            layer.smooth: true;

            // This item should be used as the 'mask'
            layer.samplerName: "maskSource"
            layer.effect: LongPressShaderEffect
            {
                longPressAnimation: idRow2Animation
            }
        }

        // the mouse area over icon and text - complete row
        LongPressAnimation
        {
            id: idRow2Animation

            anchors.fill: parent

            fillColor: idMain.colorEkxConfirmGreen

            onLongPressed:
            {
                editGrammage( 1 );
            }
        }
    }

    Rectangle
    {
        id: idDbgGrammage0;
        visible: visualDebug;
        anchors.fill: parent;
        color: "transparent";
        border.width: 6;
        border.color: "yellow";
    }

    Rectangle
    {
        id: idDbgGrammage1;
        visible: visualDebug;
        x: 0;
        y: 0;
        width: parent.childrenRect.width;
        height: parent.childrenRect.height;
        color: "transparent";
        border.width: 2;
        border.color: "orange";
    }
}
