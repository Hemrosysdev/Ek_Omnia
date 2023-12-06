import QtQml 2.12
import QtQuick 2.12
import "../../CommonItems" as Common

Common.OverlayPage
{
    id: idRoot

    topAction: recipeRow.count < recipeControl.maxTimeRecipesNum ? Common.EkxButton.PLUS : 0
    rightAction: 0

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // self defined properties
    ///////////////////////////////////////////////////////////////////////////////////////////////

    property var recipeList: recipeControl.installedRecipeList;
    property alias listView: recipeRow
    // For Tutorial, so the delegate sizes are uniform.
    property bool showRecipeNames: true

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // signals
    ///////////////////////////////////////////////////////////////////////////////////////////////

    signal selectRecipe(int recipeIndex)
    signal openRecipeEditor(int recipeIndex)
    signal addNewRecipe

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // signal handlers
    ///////////////////////////////////////////////////////////////////////////////////////////////

    onTopActionClicked: {
        addNewRecipe();
    }

    onActivated: {
        recipeRow.positionViewAtBeginning();
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // items
    ///////////////////////////////////////////////////////////////////////////////////////////////

    Common.ShadowGroup
    {
        z: 1;
        shadowTopSize: Common.ShadowGroup.ShadowTopSizes.StandardSize;
        shadowBottomSize: Common.ShadowGroup.ShadowBottomSizes.StandardSize;
        shadowBottomEnabled: (recipeRow.interactive && !recipeRow.atYEnd)? true: false;
    }

    ListView
    {
        id: recipeRow
        anchors.fill: parent
        // For a display height of 226, same bounds position like MenuPage.qml
        topMargin: 80
        bottomMargin: 54
        preferredHighlightBegin:     0
        preferredHighlightEnd:       0
        highlightFollowsCurrentItem: true
        interactive: count > 2
        currentIndex: recipeControl.currentTimeRecipeIndex

        model: idRoot.recipeList
        delegate: RecipeEditListEntry {
            active: index === recipeControl.currentTimeRecipeIndex
            recipeName: modelData.recipeName
            showRecipeName: modelData.showRecipeName && idRoot.showRecipeNames
            recipeValue: parseFloat(modelData.recipeBigValue + "." + modelData.recipeCommaValue)
            grindingSize: modelData.grindingSize
            portaFilter: modelData.portaFilterIndex

            onSelectRecipe: {
                idRoot.selectRecipe(index);
            }
            onOpenRecipeEditor:{
                idRoot.openRecipeEditor(index);
            }
            onDeleteRecipe: {
                recipeControl.deleteTimeRecipe(index);
            }
        }

        Common.ListItemJiggler {
            listView: recipeRow
        }
    }

}
