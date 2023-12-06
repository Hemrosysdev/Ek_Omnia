import QtQml 2.12
import QtQuick 2.12
import "../../CommonItems" as Common

Common.OverlayPage
{
    id: idEditCoarsenessPage
    objectName: "LibraryMode_EditCoarsenessPage"

    rightActionLongPressEnabled: !workflow.isNewItem
    rightActionLongPressColor: idMain.colorEkxConfirmGreen

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // signal handlers
    ///////////////////////////////////////////////////////////////////////////////////////////////

    onActivated: {
        idDigit100.setStartValues( ( workflow.grindingSize / 100 ) % 10 );
        idDigit10.setStartValues( ( workflow.grindingSize / 10 ) % 10 );
        idDigit1.setStartValues( workflow.grindingSize % 10 );
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
            source: "qrc:/Icons/ICON_DiscDistance-72px_white 50%.png"
        }
    }

    Item
       {
           id: idCoarsenessWheels

           anchors.top: parent.top
           anchors.left: parent.left
           anchors.leftMargin: 114

           width: idDigit100.width + idDigit10.width + idDigit1.width
           height: parent.height

           Common.NumberWheel
           {
               id: idDigit100

               anchors.top: parent.top
               anchors.left: parent.left

               currentFontFamiliy: "D-DIN Condensed HEMRO"

               model: 9
               interactive: true

               itemWidth: 90
               longPressAnimationInt: idEditCoarsenessPage.rightActionLongPressAnimation

               numberColor: idEditCoarsenessPage.workflow.grindingSize > 800 ? idMain.colorEkxWarning : idMain.colorEkxTextWhite

               onCurrentValue:
               {
                   idEditCoarsenessPage.workflow.grindingSize = value * 100 + idEditCoarsenessPage.workflow.grindingSize % 100;
               }
           }

           Common.NumberWheel
           {
               id: idDigit10

               anchors.top: parent.top
               anchors.left: idDigit100.right

               currentFontFamiliy: "D-DIN Condensed HEMRO"

               model: 10
               interactive: true

               itemWidth: 90
               informAboutChange: true
               longPressAnimationInt: idEditCoarsenessPage.rightActionLongPressAnimation

               numberColor: idEditCoarsenessPage.workflow.grindingSize > 800 ? idMain.colorEkxWarning : idMain.colorEkxTextWhite

               onCurrentValue:
               {
                   idEditCoarsenessPage.workflow.grindingSize = value * 10 + parseInt( idEditCoarsenessPage.workflow.grindingSize / 100 ) * 100 + ( idEditCoarsenessPage.workflow.grindingSize % 10 );
               }
           }

           Common.NumberWheel
           {
               id: idDigit1

               anchors.top: parent.top
               anchors.left: idDigit10.right

               currentFontFamiliy: "D-DIN Condensed HEMRO"

               model: 10
               interactive: true

               itemWidth: 90
               informAboutChange: true

               longPressAnimationInt: idEditCoarsenessPage.rightActionLongPressAnimation

               numberColor: idEditCoarsenessPage.workflow.grindingSize > 800 ? idMain.colorEkxWarning : idMain.colorEkxTextWhite

               onCurrentValue:
               {
                   idEditCoarsenessPage.workflow.grindingSize = value + parseInt( idEditCoarsenessPage.workflow.grindingSize / 10 ) * 10;
               }
           }
       }

    Common.ShadowGroup {
        shadowTopSize: 135
        shadowBottomSize: 135
    }

}
