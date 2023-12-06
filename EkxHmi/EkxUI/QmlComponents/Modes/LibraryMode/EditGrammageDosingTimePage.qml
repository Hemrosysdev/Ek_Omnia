import QtQml 2.12
import QtQuick 2.12

import ".."
import "../../CommonItems" as Common

Common.OverlayPage
{
    id: idEditGrammageDosingTimePage
    objectName: "LibraryMode_EditGrammageDosingTimePage"

    title: "Dosing Time"

    rightAction: workflow.dosingTimeMs > 0 ? Common.EkxButton.NEXT : 0
    rightActionLongPressEnabled: !workflow.isNewItem && workflow.dosingTimeMs > 0
    rightActionLongPressColor: idMain.colorEkxConfirmGreen

    onActivated: {
        idPreDecimalValue.setStartValues(Math.floor(workflow.dosingTimeMs / 1000));
        idPostDecimalValue.setStartValues(Math.floor(workflow.dosingTimeMs / 100) % 10);
    }

    onLongPressAnimationStarted: {
        idPreDecimalValue.longPressAnimationInt = animation;
        point.longPressAnimation = animation;
        idPostDecimalValue.longPressAnimationInt = animation;
        unit.longPressAnimation = animation;
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
            // FIXME correct icon
            source: "qrc:/Icons/ICON_Timer-56px_white 50%.png"
            asynchronous: true
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

        longPressAnimationInt: idEditGrammageDosingTimePage.rightActionLongPressAnimation

        onCurrentValue: {
             idEditGrammageDosingTimePage.workflow.dosingTimeMs = value * 1000 + idPostDecimalValue.currentIndex * 100
        }
    }

    Common.Label {
        id: point
        text:  "."
        pixelSize: 81
        fontFamily: Common.Label.FontFamily.MonoNumbers

        anchors.left: idPreDecimalValue.right
        anchors.leftMargin: 3
        anchors.top: parent.top
        anchors.topMargin: 140

        layer.enabled: point.longPressAnimation ? point.longPressAnimation.layerEnabled : false
        layer.samplerName: "maskSource"
        layer.effect: Common.LongPressShaderEffect {
            longPressAnimation: point.longPressAnimation
        }

        property Common.LongPressAnimation longPressAnimation
    }


    Common.SettingsNumberWheel {
        id: idPostDecimalValue

        itemWidth: 36
        width: 72 // Make it easier to scroll.

        anchors.left: point.right
        anchors.leftMargin: 3

        currentFontFamiliy: "D-DIN Condensed HEMRO _Mono_Numbers"

        model: 10

        longPressAnimationInt: idEditGrammageDosingTimePage.rightActionLongPressAnimation

        informAboutChange: true

        onCurrentValue:
        {
            idEditGrammageDosingTimePage.workflow.dosingTimeMs = idPreDecimalValue.currentIndex * 1000 + value * 100;
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
        id: unit
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

        layer.enabled: unit.longPressAnimation ? unit.longPressAnimation.layerEnabled : false
        layer.samplerName: "maskSource"
        layer.effect: Common.LongPressShaderEffect {
            longPressAnimation:  unit.longPressAnimation
        }

        property Common.LongPressAnimation longPressAnimation
    }

    Common.ShadowGroup {
        shadowTopSize: 135
        shadowBottomSize: 135
    }

}
