import QtQml 2.12
import QtQuick 2.12
import "../../CommonItems" as Common

Common.OverlayPage
{
    id: idEditDurationPage

    rightActionLongPressEnabled: !workflow.isNewItem
    rightActionLongPressColor: idMain.colorEkxConfirmGreen

    topAction: !workflow.isNewItem && recipeControl.currentTimeRecipeCount > 1 ? Common.EkxButton.DELETE : 0
    topActionLongPressEnabled: true
    topActionLongPressOnly: true
    topActionLongPressColor: idMain.colorEkxConfirmRed

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // self defined properties
    ///////////////////////////////////////////////////////////////////////////////////////////////

    // For the tutorial so it can change the number.
    property alias preDecimalNumberWheel: idPreDecimalValue
    property alias postDecimalNumberWheel: idPostDecimalValue

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // signal handlers
    ///////////////////////////////////////////////////////////////////////////////////////////////

    onActivated: {
        idPreDecimalValue.setStartValues(workflow.recipeBig);
        idPostDecimalValue.setStartValues(workflow.recipeComma);
    }

    onLongPressAnimationStarted: {
        idPreDecimalValue.longPressAnimationInt = animation;
        idDecimalPointText.longPressAnimation = animation;
        idPostDecimalValue.longPressAnimationInt = animation;
        idUnitSecText.longPressAnimation = animation;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // items
    ///////////////////////////////////////////////////////////////////////////////////////////////

    Common.ShadowGroup
    {
        shadowTopSize: Common.ShadowGroup.ShadowTopSizes.StandardSize;
        shadowBottomSize: Common.ShadowGroup.ShadowBottomSizes.StandardSize;
    }

    Item
    {
        z:4
        width: 72
        height: 72

        anchors.left:parent.left
        anchors.leftMargin: 46

        anchors.top: parent.top
        anchors.topMargin: 144

        Image
        {
            id: idEditTypeIconImage
            anchors.fill: parent
            source: "qrc:/Icons/ICON_Timer-56px_white 50%.png"
        }
    }

    Common.SettingsNumberWheel {
        id: idPreDecimalValue

        anchors.left: parent.left
        anchors.leftMargin: 171
        itemWidth: 36 * 3

        currentFontFamiliy: "D-DIN Condensed HEMRO _Mono_Numbers"

        itemOffset: 0

        model: mainStatemachine.maxGrindDuration / 1000

        longPressAnimationInt: idEditDurationPage.rightActionLongPressAnimation

        onCurrentValue: {
             idEditDurationPage.workflow.recipeBig = value;
        }
    }

    Common.Label {
        id: idDecimalPointText
        text:  "."
        pixelSize: 81
        fontFamily: Common.Label.FontFamily.MonoNumbers

        anchors.left: idPreDecimalValue.right
        anchors.leftMargin: 3
        anchors.top: parent.top
        anchors.topMargin: 140

        layer.enabled: idDecimalPointText.longPressAnimation ? idDecimalPointText.longPressAnimation.layerEnabled : false
        layer.samplerName: "maskSource"
        layer.effect: Common.LongPressShaderEffect {
            longPressAnimation: idDecimalPointText.longPressAnimation
        }

        property Common.LongPressAnimation longPressAnimation
    }


    Common.SettingsNumberWheel {
        id: idPostDecimalValue

        itemWidth: 36
        width: 72 // Make it easier to scroll.

        anchors.left: idDecimalPointText.right
        anchors.leftMargin: 3

        currentFontFamiliy: "D-DIN Condensed HEMRO _Mono_Numbers"

        model: 10

        longPressAnimationInt: idEditDurationPage.rightActionLongPressAnimation

        informAboutChange: true

        onCurrentValue:
        {
            idEditDurationPage.workflow.recipeComma = value;
        }

        onUpChange:
        {
           idPreDecimalValue.incByNeighbour();
        }

        onDownChange:
        {
           idPreDecimalValue.decByNeighbour();
        }
    }

    Common.Label {
        id: idUnitSecText
        anchors.left: parent.left
        anchors.leftMargin: 347
        anchors.top: parent.top
        anchors.topMargin: 140

        text:  "s"

        width: 36
        height: 90

        pixelSize: 81
        fontFamily: Common.Label.FontFamily.MonoNumbers

        horizontalAlignment: Text.AlignHCenter

        layer.enabled: idUnitSecText.longPressAnimation ? idUnitSecText.longPressAnimation.layerEnabled : false
        layer.samplerName: "maskSource"
        layer.effect: Common.LongPressShaderEffect {
            longPressAnimation:  idUnitSecText.longPressAnimation
        }

        property Common.LongPressAnimation longPressAnimation
    }

    Common.ShadowGroup {
        shadowTopSize: 135
        shadowBottomSize: 135
    }

}
