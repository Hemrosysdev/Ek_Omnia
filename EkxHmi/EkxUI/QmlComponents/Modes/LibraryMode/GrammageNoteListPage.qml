import QtQml 2.12
import QtQuick 2.12
import "../../CommonItems" as Common

import com.hemrogroup.recipes 1.0

Common.OverlayPage
{
    id: root
    objectName: "LibraryMode_GrammageNoteListPage"

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // self defined properties
    ///////////////////////////////////////////////////////////////////////////////////////////////

    property LibraryRecipe recipe

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // properties
    ///////////////////////////////////////////////////////////////////////////////////////////////

    topAction: idGrammageList.count < recipeControl.maxLibraryGrammageNum ? Common.EkxButton.PLUS : 0
    rightActionLongPressEnabled: !workflow.isNewItem
    rightActionLongPressColor: idMain.colorEkxConfirmGreen

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // signals
    ///////////////////////////////////////////////////////////////////////////////////////////////

    signal addNewGrammage
    signal editGrammage(int grammageIndex)
    signal deleteGrammate(int grammageIndex)

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // signal handlers
    ///////////////////////////////////////////////////////////////////////////////////////////////

    onTopActionClicked: addNewGrammage()

    onActivated: {
        idGrammageList.positionViewAtIndex(recipeControl.currentLibraryGrammageIndex, ListView.Center);
    }

    ListView {
        id: idGrammageList
        anchors.fill: parent
        // For a display height of 226, same bounds position like MenuPage.qml
        topMargin: 80
        bottomMargin: 54
        preferredHighlightBegin: 0
        preferredHighlightEnd: 0
        highlightFollowsCurrentItem: true
        interactive: count > 2

        model: root.recipe ? root.recipe.grammages : null

        delegate: GrammageListEntryView {
            grammageMg: modelData.grammageMg
            dosingTimeMs: modelData.dosingTimeMs
            iconSource: libraryMode.icons[modelData.icon]
            dosingTimeVisible: libraryMode.showDosingTime

            onEditGrammage: {
                root.editGrammage(index);
            }
            onDeleteGrammage: {
                root.deleteGrammate(index);
            }
        }

        Common.ListItemJiggler {
            id: idGrammageListJiggler
            listView: idGrammageList
        }
    }

    Common.Label {
        anchors.centerIn: parent
        horizontalAlignment: Text.AlignHCenter
        text: `Add a
grammage note`
        textColor: Common.Label.Grey
        pixelSize: 58
        font.letterSpacing: 2
        bold: true
        visible: idGrammageList.count === 0

        MouseArea {
            anchors.fill: parent
            onClicked: root.addNewGrammage()
        }
    }

    Common.ShadowGroup {
        shadowTopSize: Common.ShadowGroup.ShadowTopSizes.StandardSize
        shadowBottomSize: Common.ShadowGroup.ShadowBottomSizes.StandardSize
        shadowTopEnabled: idGrammageList.interactive
        shadowBottomEnabled: idGrammageList.interactive
    }
}
