import QtQml 2.12
import QtQuick 2.12
import "../../CommonItems" as Common

Common.OverlayPage
{
    id: idEditGrammageNotePage
    objectName: "LibraryMode_EditGrammageNotePage"

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // properties
    ///////////////////////////////////////////////////////////////////////////////////////////////

    title: "Grammage Note"

    rightActionLongPressEnabled: !workflow.isNewItem
    rightActionLongPressColor: idMain.colorEkxConfirmGreen

    topAction: !workflow.isNewItem ? Common.EkxButton.DELETE : 0
    topActionLongPressEnabled: true
    topActionLongPressOnly: true
    topActionLongPressColor: idMain.colorEkxConfirmRed

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // signal handlers
    ///////////////////////////////////////////////////////////////////////////////////////////////

    onActivated: {
        idIconWheel.setStartValues(workflow.icon);

        idPreDecimalValue.setStartValues(Math.floor(workflow.grammageMg / 1000));
        idPostDecimalValue.setStartValues(Math.floor(workflow.grammageMg / 100) % 10);
    }

    onLongPressAnimationStarted: {
        idIconWheel.longPressAnimationInt = animation;
        idPreDecimalValue.longPressAnimationInt = animation;
        point.longPressAnimation = animation;
        idPostDecimalValue.longPressAnimationInt = animation;
        unit.longPressAnimation = animation;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // items
    ///////////////////////////////////////////////////////////////////////////////////////////////

    Common.IconWheel {
        id: idIconWheel
        anchors.left: parent.left
        anchors.leftMargin: 110
        longPressAnimationInt: idEditGrammageNotePage.rightActionLongPressAnimation

        onCurrentValue: {
            idEditGrammageNotePage.workflow.icon = value;
        }
    }

   Common.SettingsNumberWheel {
       id: idPreDecimalValue

       anchors.left: parent.left
       anchors.leftMargin: 207

       currentFontFamiliy: "D-DIN Condensed HEMRO _Mono_Numbers"

       itemOffset: 0

       model: 99

       longPressAnimationInt: idEditGrammageNotePage.rightActionLongPressAnimation

       onCurrentValue: {
            idEditGrammageNotePage.workflow.grammageMg = value * 1000 + idPostDecimalValue.currentIndex * 100
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

       longPressAnimationInt: idEditGrammageNotePage.rightActionLongPressAnimation

       informAboutChange: true

       onCurrentValue:
       {
           idEditGrammageNotePage.workflow.grammageMg = idPreDecimalValue.currentIndex * 1000 + value * 100;
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

       text:  "g"

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
