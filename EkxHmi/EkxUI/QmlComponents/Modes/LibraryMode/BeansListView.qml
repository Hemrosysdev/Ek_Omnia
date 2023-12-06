import QtQml 2.12
import QtQuick 2.12

import "../../CommonItems" as Common

Common.OverlayPage
{
    id: root
    objectName: "LibraryMode_BeansListView"

    rightAction: idBeansList.count < recipeControl.maxLibraryRecipesNum ? Common.EkxButton.PLUS : 0

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // self defined properties
    ///////////////////////////////////////////////////////////////////////////////////////////////

    property var libraryRecipeList: recipeControl.installedLibraryList;

    property color currentLibraryIndexColor: idMain.colorEkxTextWhite
    property color otherColor: idMain.colorEkxTextGrey

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // signals
    ///////////////////////////////////////////////////////////////////////////////////////////////

    signal openRecipeEditor(int recipeIndex);
    signal addNewRecipe

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // signal handlers
    ///////////////////////////////////////////////////////////////////////////////////////////////

    onRightActionClicked: {
        addNewRecipe();
    }

    onActivated: {
        idBeansList.positionViewAtIndex( recipeControl.currentLibraryRecipeIndex, ListView.Center );
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // items
    ///////////////////////////////////////////////////////////////////////////////////////////////

    Timer {
        id: idSelectCloseTimer
        interval: 700;
        onTriggered: {
            root.closeOverlay();
        }
    }

    ListView {
        id: idBeansList
        anchors.fill: parent
        // For a display height of 226, same bounds position like MenuPage.qml
        topMargin: 67
        bottomMargin: 67
        highlightFollowsCurrentItem: true
        flickableDirection: Flickable.VerticalFlick
        orientation: ListView.Vertical
        interactive: count > 3

        model: libraryRecipeList
        delegate: BeansListEntryView
        {
            id: idBeansListEntryDelegate
            text: modelData.beanName
            active: index === recipeControl.currentLibraryRecipeIndex
            onSelectRecipe: {
                recipeControl.currentLibraryRecipeIndex = index;
                idSelectCloseTimer.restart();
            }
            onOpenRecipeEditor: {
                root.openRecipeEditor(index);
            }
            onDeleteRecipe: {
                recipeControl.deleteLibraryRecipe(index);
            }
        }

        //DebugFrame{visible: true}

        Common.ListItemJiggler {
            listView: idBeansList
        }
    }

    Common.ShadowGroup {
        shadowTopSize: Common.ShadowGroup.ShadowTopSizes.StandardSize;
        shadowBottomSize: Common.ShadowGroup.ShadowBottomSizes.StandardSize;
        shadowBottomEnabled: (idBeansList.count>3 && !idBeansList.atYEnd)?true:false;
        //visualDebug: true
    }
}
