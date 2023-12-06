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

    property real currentRecipeValue: 99.9
    property real currentRecipeDdd: 99.9
    property int  currentTimeRecipeIndex

    property int currentPortaFilter: 0

    property variant portaFilterImages:
    {
        0: "qrc:/Icons/ICON-Strokes_Single-80px_portafilter_white.png",
        1: "qrc:/Icons/ICON-Strokes_Double-80px_portafilter_white.png",
        2: "qrc:/Icons/ICON-Strokes_Bottomless-80px_portafilter_white.png"
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // signals
    ///////////////////////////////////////////////////////////////////////////////////////////////

    signal openRecipeListEntryEdit( var listRecipeIndex )

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // functions
    ///////////////////////////////////////////////////////////////////////////////////////////////

    function startJiggling()
    {
        idJiggleAnimation.running = true
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
        if ( idRoot.visible )
        {
            idRecipeEntry.contentX = 0
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
            if( !running )
            {
                idRoot.rotation = 0;
            }
        }
    }

    Flickable
    {
        id: idRecipeEntry

        width: parent.width
        height: parent.height

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

            width: 480 + idPencilIcon.width + 83 + idTrashIcon.width + 87
            height: parent.height

            // clock symbol in front of recipe time
            Image
            {
                id: idClockIcon

                width: 56
                height: 56

                anchors.left: parent.left
                anchors.leftMargin: 9
                anchors.verticalCenter: parent.verticalCenter;

                source: "qrc:/Icons/ICON_Timer-56px_white 50%.png"

                layer.enabled: true
                layer.smooth: true;

                // This item should be used as the 'mask'
                layer.samplerName: "maskSource"
                layer.effect: LongPressShaderEffect
                {
                    longPressAnimation: idClockIcon.longPressAnimation
                }

                property LongPressAnimation longPressAnimation: idEditAnimation
            }

            // text for recipe time
            Text
            {
                id: idRecipeTime

                anchors.left: idClockIcon.right
                anchors.leftMargin: 2
                anchors.top: parent.top
                anchors.bottom: parent.bottom

                horizontalAlignment: Text.AlignLeft;
                verticalAlignment: Text.AlignVCenter

                //text: currentRecipeValue.toFixed(1).toString()+"s"
                text: currentRecipeValue.toFixed(1).toString()

                font.pixelSize: 58
                font.family:    "D-DIN Condensed HEMRO"
                font.letterSpacing: 0.23
                color: (recipeControl.currentTimeRecipeIndex === currentTimeRecipeIndex)?idMain.colorEkxTextWhite:idMain.colorEkxTextGrey;

                layer.enabled: true
                layer.smooth: true;

                // This item should be used as the 'mask'
                layer.samplerName: "maskSource"
                layer.effect: LongPressShaderEffect
                {
                    longPressAnimation: idRecipeTime.longPressAnimation
                    backgroundColor: idRecipeTime.color
                }

                property LongPressAnimation longPressAnimation: idEditAnimation
            }

            Text
            {
                id: idRecipeTimeUnit

                anchors.left: idRecipeTime.right
                anchors.leftMargin: 2
                anchors.top: parent.top
                anchors.bottom: parent.bottom

                horizontalAlignment: Text.AlignLeft;
                verticalAlignment: Text.AlignVCenter

                text: "s"

                font.pixelSize: 58
                font.family:    "D-DIN Condensed HEMRO"
                font.letterSpacing: 0.23
                //color: (recipeControl.currentTimeRecipeIndex === currentTimeRecipeIndex)?idMain.colorEkxTextWhite:idMain.colorEkxTextGrey;
                color: idMain.colorEkxTextGrey;

                layer.enabled: true
                layer.smooth: true;

                // This item should be used as the 'mask'
                layer.samplerName: "maskSource"
                layer.effect: LongPressShaderEffect
                {
                    longPressAnimation: idRecipeTimeUnit.longPressAnimation
                    backgroundColor: idRecipeTimeUnit.color
                }

                property LongPressAnimation longPressAnimation: idEditAnimation
            }

            // DDD symbol
            Image
            {
                id: idDddIcon

                width: 56
                height: 56

                anchors.left: parent.left
                anchors.leftMargin: 200
                anchors.verticalCenter: parent.verticalCenter;

                source: "qrc:/Icons/ICON_DiscDistance-56px_white 50%.png"

                layer.enabled: true
                layer.smooth: true;

                // This item should be used as the 'mask'
                layer.samplerName: "maskSource"
                layer.effect: LongPressShaderEffect
                {
                    longPressAnimation: idDddIcon.longPressAnimation
                }

                property LongPressAnimation longPressAnimation: idEditAnimation
            }

            // DDD value text
            Text
            {
                id: idDddValue

                anchors.left: idDddIcon.right
                anchors.leftMargin: 2
                anchors.top: parent.top
                anchors.bottom: parent.bottom

                horizontalAlignment: Text.AlignLeft;
                verticalAlignment: Text.AlignVCenter

                text: currentRecipeDdd.toFixed(1).toString()

                font.pixelSize: 58
                font.family:    "D-DIN Condensed HEMRO"
                font.letterSpacing: 0.23

                color: (recipeControl.currentTimeRecipeIndex === currentTimeRecipeIndex)?idMain.colorEkxTextWhite:idMain.colorEkxTextGrey;

                layer.enabled: true
                layer.smooth: true;

                // This item should be used as the 'mask'
                layer.samplerName: "maskSource"
                layer.effect: LongPressShaderEffect
                {
                    longPressAnimation: idDddValue.longPressAnimation
                    backgroundColor: idDddValue.color
                }

                property LongPressAnimation longPressAnimation: idEditAnimation
            }

            // porta filter image
            Image
            {
                id: idPortaFilterIcon
                x: 480 - width;
                anchors.verticalCenter: parent.verticalCenter;
                width: 80;
                height: 80;
                source: portaFilterImages[currentPortaFilter]

                layer.enabled: true
                layer.smooth: true;
                // This item should be used as the 'mask'
                layer.samplerName: "maskSource"
                layer.effect: LongPressShaderEffect
                {
                    longPressAnimation: idPortaFilterIcon.longPressAnimation
                    backgroundColor: (recipeControl.currentTimeRecipeIndex === currentTimeRecipeIndex)?idMain.colorEkxTextWhite:idMain.colorEkxTextGrey;
                }

                property LongPressAnimation longPressAnimation: idEditAnimation
            }

            // the mouse area over time and DDD value area
            LongPressAnimation
            {
                id: idEditAnimation

                width: 480
                height: 80

                anchors.left: parent.left

                fillColor: idMain.colorEkxConfirmGreen

                onStarted:
                {
                    idClockIcon.longPressAnimation = idEditAnimation
                    idRecipeTime.longPressAnimation = idEditAnimation
                    idDddIcon.longPressAnimation = idEditAnimation
                    idDddValue.longPressAnimation = idEditAnimation
                    idPortaFilterIcon.longPressAnimation = idEditAnimation
                }

                onLongPressed:
                {
                    openRecipeListEntryEdit( currentTimeRecipeIndex )
                }
            }

            EkxButton
            {
                id: idPencilIcon;
                anchors.left: idPortaFilterIcon.right
                anchors.leftMargin: 64
                anchors.top: parent.top
                anchors.topMargin: 7
                buttonType: EkxButton.ButtonType.EDIT;
                onClicked:
                {
                    openRecipeListEntryEdit( currentTimeRecipeIndex );
                }
            }

            // trash symbol right of pencil
            Image
            {
                id: idTrashIcon

                visible: recipeControl.currentTimeRecipeCount > 1

                width: 72
                height: 72

                anchors.left: idPencilIcon.right
                anchors.leftMargin: 64
                anchors.top: parent.top
                anchors.topMargin: 7

                source: "qrc:/Icons/ICON_Trashcan-72px_white.png"

                layer.enabled: true
                layer.smooth: true;

                // This item should be used as the 'mask'
                layer.samplerName: "maskSource"
                layer.effect: LongPressShaderEffect
                {
                    longPressAnimation: idDeleteAnimation
                }

                // the mouse area over time and DDD value area
                LongPressAnimation
                {
                    id: idDeleteAnimation

                    fillColor: idMain.colorEkxConfirmRed

                    onStarted:
                    {
                        idClockIcon.longPressAnimation = idDeleteAnimation
                        idRecipeTime.longPressAnimation = idDeleteAnimation
                        idDddIcon.longPressAnimation = idDeleteAnimation
                        idDddValue.longPressAnimation = idDeleteAnimation
                        idPortaFilterIcon.longPressAnimation = idDeleteAnimation
                    }

                    onLongPressed:
                    {
                        recipeControl.deleteTimeRecipe( currentTimeRecipeIndex )
                    }
                }
            }
        }
    }
}
