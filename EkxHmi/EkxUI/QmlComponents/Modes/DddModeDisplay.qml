import QtQml 2.12
import QtQuick 2.0
import "../CommonItems"

Item
{
    id: idDddModeDisplay

    width: 480
    height: 72

    anchors.top: parent.top
    anchors.left: parent.left

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // self defined properties
    ///////////////////////////////////////////////////////////////////////////////////////////////

    property int  currentDddValue: dddCouple.dddValue
    // Avoid showing -1 to the user when there is no grinding size.
    property int  currentGrindingSize: recipeControl.currentGrindingSize > -1 ? recipeControl.currentGrindingSize : currentDddValue

    property bool showCurrentDdd: true
    property LongPressAnimation longPressAnimationInt: idSaveAnimation

    property bool recipeValueIsEqual: currentDddValue === currentGrindingSize
    property bool recipeValueIsGreater: ( currentDddValue < currentGrindingSize) ? true : false
    property bool recipeValueIsSmaller: ( currentDddValue > currentGrindingSize) ? true : false

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // signals
    ///////////////////////////////////////////////////////////////////////////////////////////////

    signal saveCurrentDdd(int dddValue);

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // functions
    ///////////////////////////////////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // signal handlers
    ///////////////////////////////////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // items
    ///////////////////////////////////////////////////////////////////////////////////////////////

    Item
    {
        id: idLeftValue

        visible: showCurrentDdd || recipeValueIsGreater

        anchors.centerIn: recipeValueIsEqual ? parent : undefined
        anchors.top: parent.top
        anchors.right: recipeValueIsEqual ? undefined : idArrow.left
        anchors.rightMargin: 0

        height: parent.height
        width: idLeftCoarsenessIconBox.width + idLeftText.width + idLeftUnitIconBox.width

        property bool  isRecipeValue: ( recipeValueIsGreater && !recipeValueIsEqual )
        property color itemColor: ( isRecipeValue ) ?  idMain.colorEkxConfirmRed : idMain.colorEkxTextGrey

        Item
        {
            id: idLeftCoarsenessIconBox

            visible: !idLeftValue.isRecipeValue

            anchors.top: parent.top

            width: visible ? idLeftCoarsenessIcon.width : 0
            height: parent.height

            Image
            {
                id: idLeftCoarsenessIcon

                anchors.centerIn: parent

                width: 60
                height: 60

                source: "qrc:/Icons/ICON-Strokes_DDD-112px_white.png"

                layer.enabled: true
                layer.smooth: true;

                layer.samplerName: "maskSource"
                layer.effect: LongPressShaderEffect
                {
                    longPressAnimation: idDddModeDisplay.longPressAnimationInt
                    backgroundColor: idLeftValue.itemColor
                    useLuminosityFill: false
                }
            }
        }

        Text
        {
            id: idLeftText

            anchors.top: parent.top
            anchors.left: idLeftCoarsenessIcon.visible ? idLeftCoarsenessIconBox.right : parent.left
            anchors.leftMargin: 0
            height: parent.height
            width: paintedWidth

            text: ( recipeValueIsGreater && !recipeValueIsEqual ) ? currentGrindingSize : currentDddValue
            color: idLeftValue.itemColor

            verticalAlignment: Text.AlignVCenter
            font.pixelSize: 58
            font.letterSpacing: 0.23
            font.family: "D-DIN Condensed HEMRO _Mono_Numbers"

            layer.enabled: idDddModeDisplay.longPressAnimationInt ? idDddModeDisplay.longPressAnimationInt.layerEnabled : false
            layer.smooth: true

            // This item should be used as the 'mask'
            layer.samplerName: "maskSource"
            layer.effect: LongPressShaderEffect
            {
                longPressAnimation: idDddModeDisplay.longPressAnimationInt
                useLuminosityFill: false
            }
        }

        Item
        {
            id: idLeftUnitIconBox

            anchors.top: parent.top
            anchors.left: idLeftText.right
            anchors.leftMargin: 0
            anchors.topMargin: 10

            width: idLeftUnitIcon.width
            height: parent.height

            Image
            {
                id: idLeftUnitIcon

                anchors.centerIn: parent

                width: 60
                height: 60

                source: "qrc:/Icons/um.png"

                layer.enabled: true
                layer.smooth: true

                layer.samplerName: "maskSource"
                layer.effect: LongPressShaderEffect
                {
                    longPressAnimation: idDddModeDisplay.longPressAnimationInt
                    backgroundColor: idLeftValue.itemColor
                    useLuminosityFill: false
                }
            }
        }
    }

    Image
    {
        id: idArrow

        visible: !recipeValueIsEqual

        width:  72
        height: 72

        anchors.centerIn: parent

        source: "qrc:/Icons/redArrowlo.png"
        mirror: recipeValueIsSmaller

        layer.enabled: idDddModeDisplay.longPressAnimationInt ? idDddModeDisplay.longPressAnimationInt.layerEnabled : false
        layer.smooth: true

        // This item should be used as the 'mask'
        layer.samplerName: "maskSource"
        layer.effect: LongPressShaderEffect
        {
            longPressAnimation: idDddModeDisplay.longPressAnimationInt
            useLuminosityFill: false
        }
    }

    Item
    {
        id: idRightValue

        visible: ( showCurrentDdd || recipeValueIsSmaller ) && !recipeValueIsEqual

        anchors.top: parent.top
        anchors.left: idArrow.right
        anchors.leftMargin: 0

        height: parent.height
        width: idRightCoarsenessIconBox.width + idRightText.width + idRightUnitIconBox.width

        property bool  isRecipeValue: ( recipeValueIsSmaller && !recipeValueIsEqual )
        property color itemColor: ( isRecipeValue ) ? idMain.colorEkxConfirmRed : idMain.colorEkxTextGrey

        Item
        {
            id: idRightCoarsenessIconBox

            visible: !idRightValue.isRecipeValue

            anchors.top: parent.top

            width: idRightCoarsenessIcon.width
            height: parent.height

            Image
            {
                id: idRightCoarsenessIcon

                anchors.centerIn: parent

                width: 60
                height: 60

                source: "qrc:/Icons/ICON-Strokes_DDD-112px_white.png"

                layer.enabled: true
                layer.smooth: true;

                layer.samplerName: "maskSource"
                layer.effect: LongPressShaderEffect
                {
                    longPressAnimation: idDddModeDisplay.longPressAnimationInt
                    backgroundColor: idRightValue.itemColor
                }
            }
        }

        Text
        {
            id: idRightText

            anchors.top: parent.top
            anchors.left: idRightCoarsenessIconBox.visible ? idRightCoarsenessIconBox.right : parent.left
            anchors.leftMargin: 0
            height: parent.height
            width: paintedWidth

            text: ( recipeValueIsSmaller && !recipeValueIsEqual ) ? currentGrindingSize : currentDddValue
            color: idRightValue.itemColor

            verticalAlignment: Text.AlignVCenter
            font.pixelSize: 58
            font.letterSpacing: 0.23
            font.family: "D-DIN Condensed HEMRO _Mono_Numbers"

            layer.enabled: idDddModeDisplay.longPressAnimationInt ? idDddModeDisplay.longPressAnimationInt.layerEnabled : idDddModeDisplay.longPressAnimationInt
            layer.smooth: true

            // This item should be used as the 'mask'
            layer.samplerName: "maskSource"
            layer.effect: LongPressShaderEffect
            {
                longPressAnimation: idDddModeDisplay.longPressAnimationInt
                useLuminosityFill: false
            }
        }

        Item
        {
            id: idRightUnitIconBox

            anchors.top: parent.top
            anchors.left: idRightText.right
            anchors.topMargin: 10

            width: idRightUnitIcon.width
            height: parent.height

            Image
            {
                id: idRightUnitIcon

                anchors.centerIn: parent

                width: 60
                height: 60

                source: "qrc:/Icons/um.png"

                layer.enabled: true
                layer.smooth: true

                layer.samplerName: "maskSource"
                layer.effect: LongPressShaderEffect
                {
                    longPressAnimation: idDddModeDisplay.longPressAnimationInt
                    backgroundColor: idRightValue.itemColor
                    useLuminosityFill: false
                }
            }
        }
    }

    Item
    {
        id: idActiveArea

        anchors.fill: parent

        LongPressAnimation
        {
            id: idSaveAnimation

            visible: !recipeValueIsEqual

            fillColor: idMain.colorEkxConfirmGreen

            onSimplePressed:
            {
                if ( settingsSerializer.agsaEnabled )
                {
                    var posLeft = idLeftValue.mapFromItem( idSaveAnimation, mouseX, mouseY )
                    var posRight = idRightValue.mapFromItem( idSaveAnimation, mouseX, mouseY )

                    if ( idLeftValue.contains( posLeft ) )
                    {
                        if ( idLeftValue.itemColor == idMain.colorEkxTextGrey )
                        {
                            idMain.showAgsaFineAdjustment()
                        }
                        else
                        {
                            agsaControl.moveToDddValue( currentGrindingSize )
                        }
                    }
                    else if ( idRightValue.contains( posRight ) )
                    {
                        if ( idRightValue.itemColor == idMain.colorEkxTextGrey )
                        {
                            idMain.showAgsaFineAdjustment()
                        }
                        else
                        {
                            agsaControl.moveToDddValue( currentGrindingSize )
                        }
                    }
                }
            }

            onLongPressed:
            {
                saveCurrentDdd(idDddModeDisplay.currentDddValue);
            }
        }

        MouseArea
        {
            id: idSimpleMouseOnCurrentValue1

            visible: recipeValueIsEqual && showCurrentDdd
            anchors.fill: parent

            onClicked:
            {
                idMain.showAgsaFineAdjustment()
            }
        }
    }
}
