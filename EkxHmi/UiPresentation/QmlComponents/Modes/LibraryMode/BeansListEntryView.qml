import QtQml 2.12
import QtQuick 2.12
import "../../CommonItems"

Item
{
    id: idRoot

    width: 480
    // height: 96; // uniform height throughout the UI
    height: 80; // special height for LibraryMode recipe list enties

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // self defined properties
    ///////////////////////////////////////////////////////////////////////////////////////////////

    //property string currentBeanName: ""
    property color  currentLibraryIndexColor: idMain.colorEkxTextWhite
    property color  otherColor: idMain.colorEkxTextGrey
    property int    recipeIndex;

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // signals
    ///////////////////////////////////////////////////////////////////////////////////////////////

    signal openLibraryListEntryEdit( var listLibraryIndex );
    signal newLibraryIndexChoosen()

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // functions
    ///////////////////////////////////////////////////////////////////////////////////////////////

    function startJiggling()
    {
        idJiggleAnimation.running = true;
    }

    function abortLongPress()
    {
        idEditAnimation.abort()
        idDeleteAnimation.abort()
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // signal handlers
    ///////////////////////////////////////////////////////////////////////////////////////////////

    onVisibleChanged:
    {
        if ( root.visible )
        {
            currentLibraryIndexColor = idMain.colorEkxTextWhite;
            idFlickableEntry.contentX = 0
        }
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // items
    ///////////////////////////////////////////////////////////////////////////////////////////////

    SequentialAnimation
    {
        id: idJiggleAnimation

        running: false
        loops: 1

        NumberAnimation { target: idFlickItem; property: "x"; from: 0; to: -50; duration: 200 }
        NumberAnimation { target: idFlickItem; property: "x"; from: -50; to: 0; duration: 200 }

        onRunningChanged:
        {
            if ( !running )
            {
                root.rotation = 0;
            }
        }
    }

    Flickable
    {
        id: idFlickableEntry

        width: parent.width
        height :parent.height

        contentWidth: idFlickItem.width;
        contentHeight: idFlickItem.height
        interactive: true
        flickableDirection: Flickable.HorizontalFlick
        boundsBehavior:  Flickable.StopAtBounds

        onMovementStarted:
        {
            abortLongPress()
        }

        Item
        {
            id: idFlickItem

            width: idBeansName.width + idPencilIcon.width + 83 + idTrashIcon.width + 87
            height: parent.height

            Rectangle
            {
                id: idBeansName

                width: 480
                height: parent.height

                color: "black"

                Text
                {
                    id: idBeansNameText
                    property LongPressAnimation longPressAnimation: idEditAnimation

                    anchors.fill: parent
                    horizontalAlignment: Text.AlignHCenter;
                    verticalAlignment: Text.AlignVCenter;
                    color: ( recipeIndex === recipeControl.currentLibraryRecipeIndex ) ? idRoot.currentLibraryIndexColor : idRoot.otherColor
                    font.pixelSize: 58
                    font.family:    "D-DIN Condensed HEMRO"
                    font.letterSpacing: 2
                    font.bold: true
                    text: modelData.beanName

                    layer.enabled: true
                    layer.smooth:  true
                    // This item should be used as the 'mask'
                    layer.samplerName: "maskSource"
                    layer.effect: LongPressShaderEffect
                    {
                        longPressAnimation: idBeansNameText.longPressAnimation
                        backgroundColor: idBeansNameText.color
                    }
                }

                LongPressAnimation
                {
                    id: idEditAnimation

                    fillColor: idMain.colorEkxConfirmGreen

                    onStarted:
                    {
                        idBeansNameText.longPressAnimation = idEditAnimation
                    }

                    onLongPressed:
                    {
                        openLibraryListEntryEdit( recipeIndex )
                    }

                    onSimplePressed:
                    {
                        recipeControl.currentLibraryRecipeIndex = index
                        newLibraryIndexChoosen()
                    }
                }
            }

            EkxButton
            {
                id: idPencilIcon;
                anchors.left: idBeansName.right
                anchors.leftMargin: 64
                anchors.top: parent.top
                anchors.topMargin: 7
                buttonType: EkxButton.ButtonType.EDIT;
                onClicked:
                {
                    openLibraryListEntryEdit( recipeIndex );
                }
            }

            Image
            {
                id: idTrashIcon

                visible: recipeControl.installedLibraryList.length > 1

                width: 72
                height: 72

                anchors.left: idPencilIcon.right
                anchors.leftMargin: 64
                anchors.top: parent.top
                anchors.topMargin: 7

                source: "qrc:/Icons/ICON_Trashcan-72px_white.png"

                layer.enabled: true
                layer.smooth:  true

                // This item should be used as the 'mask'
                layer.samplerName: "maskSource"
                layer.effect: LongPressShaderEffect
                {
                    longPressAnimation: idDeleteAnimation
                }

                LongPressAnimation
                {
                    id: idDeleteAnimation

                    fillColor: idMain.colorEkxConfirmRed

                    onStarted:
                    {
                        idBeansNameText.longPressAnimation = idDeleteAnimation
                    }

                    onLongPressed:
                    {
                        recipeControl.deleteLibraryRecipe( recipeIndex )
                    }
                }
            }
        }
    }
}
