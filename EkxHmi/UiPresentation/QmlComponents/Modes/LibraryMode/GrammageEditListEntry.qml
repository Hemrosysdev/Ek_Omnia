import QtQml 2.12
import QtQuick 2.12
import "../../CommonItems"

Item
{
    id: idRoot

    width: 480
    height: 96

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // self defined properties
    ///////////////////////////////////////////////////////////////////////////////////////////////

    property real currentGrammage: 10.4
    property int  currentGrammageIcon: 0
    property int  currentEditLibraryIndex:0
    property int  currentGrammageIndex;

    property variant grammageIcons: [
        "qrc:/Icons/ICON-Strokes_Single-80px_Cup_white.png",
        "qrc:/Icons/ICON-Strokes_Single-80px_Filter_white.png",
        "qrc:/Icons/ICON-Strokes_Double-V2-80px_Filter_white.png"
    ]

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // signals
    ///////////////////////////////////////////////////////////////////////////////////////////////

    signal openGrammageEditListEntry( var listGrammageIndex );

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
        if( visible )
        {
            idFlickableItem.contentX = 0;
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

        NumberAnimation { target: idFlickableContent; property: "x"; from: 0; to: -50; duration: 200 }
        NumberAnimation { target: idFlickableContent; property: "x"; from: -50; to: 0; duration: 200 }

        onRunningChanged:
        {
            if ( !running )
            {
                idRoot.rotation = 0;
            }
        }
    }

    Flickable
    {
        id: idFlickableItem

        width: parent.width
        height:parent.height

        contentWidth: idFlickableContent.width;
        contentHeight: idFlickableContent.height

        interactive: true

        flickableDirection: Flickable.HorizontalFlick

        boundsBehavior:  Flickable.StopAtBounds

        onMovementStarted:
        {
            abortLongPress()
        }

        Item
        {
            id: idFlickableContent

            width: 480 + idPencilIcon.width + 83 + idTrashIcon.width + 87
            height: 96

            Image
            {
                id: idGrammageIcon

                width: 96
                height: 96

                anchors.left: parent.left
                anchors.leftMargin: 94

                source: grammageIcons[currentGrammageIcon]

                layer.enabled: true
                layer.smooth: true;

                layer.samplerName: "maskSource"
                layer.effect: LongPressShaderEffect
                {
                    longPressAnimation: idGrammageIcon.longPressAnimation
                    backgroundColor: idMain.colorEkxTextGrey
                }

                property LongPressAnimation longPressAnimation: idEditAnimation
            }

            Item
            {
                id: idGrammageValue

                width: 169
                height: 80

                anchors.left: parent.left
                anchors.leftMargin: 218
                anchors.top: parent.top
                anchors.topMargin: 8

                Text
                {
                    id: idGrammageText

                    anchors.centerIn: parent

                    text: currentGrammage.toFixed(1).toString() + " t"
                    color:  idMain.colorEkxTextGrey
                    horizontalAlignment: Text.AlignRight

                    font.pixelSize: 80
                    font.family:    "D-DIN Condensed HEMRO"
                    font.letterSpacing: 0.33

                    layer.enabled: true
                    layer.smooth: true;

                    // This item should be used as the 'mask'
                    layer.samplerName: "maskSource"
                    layer.effect: LongPressShaderEffect
                    {
                        longPressAnimation: idGrammageText.longPressAnimation
                        backgroundColor: idGrammageText.color
                    }

                    property LongPressAnimation longPressAnimation: idEditAnimation
                }
            }

            LongPressAnimation
            {
                id: idEditAnimation

                width: 320
                height:80
                anchors.left: idGrammageIcon.left

                fillColor: idMain.colorEkxConfirmGreen

                onStarted:
                {
                    idGrammageText.longPressAnimation = idEditAnimation
                    idGrammageIcon.longPressAnimation = idEditAnimation
                }

                onLongPressed:
                {
                    openGrammageEditListEntry( currentGrammageIndex )
                }
            }

            EkxButton
            {
                id: idPencilIcon;
                anchors.left: idGrammageValue.right
                anchors.leftMargin: 80
                anchors.verticalCenter: parent.verticalCenter
                buttonType: EkxButton.ButtonType.EDIT;
                onClicked:
                {
                    openGrammageEditListEntry( currentGrammageIndex );
                }
            }


            Image
            {
                id: idTrashIcon

                width: 72
                height: 72

                anchors.left: idPencilIcon.right
                anchors.leftMargin: 64
                anchors.verticalCenter:  parent.verticalCenter

                source: "qrc:/Icons/ICON_Trashcan-72px_white.png"

                layer.enabled: true
                layer.smooth: true;

                layer.samplerName: "maskSource"
                layer.effect: LongPressShaderEffect
                {
                    longPressAnimation: idDeleteAnimation
                }

                LongPressAnimation
                {
                    id: idDeleteAnimation

                    anchors.fill: parent

                    fillColor: idMain.colorEkxConfirmRed

                    onStarted:
                    {
                        idGrammageText.longPressAnimation = idDeleteAnimation
                        idGrammageIcon.longPressAnimation = idDeleteAnimation
                    }

                    onLongPressed:
                    {
                        recipeControl.deleteLibraryGrammage( currentEditLibraryIndex, currentGrammageIndex )
                    }
                }
            }
        }
    }
}
