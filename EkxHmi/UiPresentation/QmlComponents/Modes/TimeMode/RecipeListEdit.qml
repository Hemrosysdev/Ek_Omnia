import QtQml 2.12
import QtQuick 2.12
import "../../CommonItems"

Rectangle
{
    id: idRoot

    width: 480
    height: 360

    color: "black"

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // self defined properties
    ///////////////////////////////////////////////////////////////////////////////////////////////

    property int recipeCount: 2
    property var recipeList: recipeControl.installedRecipeList;

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // signals
    ///////////////////////////////////////////////////////////////////////////////////////////////

    signal closeEditRecipeList()
    signal openEditRecipeList( var recipeIndex );

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // functions
    ///////////////////////////////////////////////////////////////////////////////////////////////

    function addNewRecipe( newRecipeIndex )
    {
        console.log("Call to recipeController addNewRecipe");
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // signal handlers
    ///////////////////////////////////////////////////////////////////////////////////////////////

    onVisibleChanged:
    {
        if(visible)
        {
            animationTimer.start();
        }
        else
        {
            recipeRow.positionViewAtBeginning();
        }
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // items
    ///////////////////////////////////////////////////////////////////////////////////////////////

    EkxButton
    {
        // id: idCloseButton;
        z: 2;
        buttonType: EkxButton.ButtonType.CLOSE;
        onClicked:
        {
            closeEditRecipeList();
        }
    }

    ShadowGroup
    {
        z: 1;
        shadowTopSize: ShadowGroup.ShadowTopSizes.StandardSize;
        shadowBottomSize: ShadowGroup.ShadowBottomSizes.StandardSize;
        shadowBottomEnabled: (recipeCount>3 && !recipeRow.atYEnd)? true: false;
        // onShadowBottomEnabledChanged:
        // {
        //     console.log("onShadowBottomEnabledChanged:"
        //                 +"\nrecipeCount       = "+recipeCount
        //                 +"\nrecipeRow.originY = "+recipeRow.originY
        //                 +"\nrecipeRow.atYEnd  = "+recipeRow.atYEnd);
        // }
        //visualDebug: true
    }

    ListView
    {
        id: recipeRow
        property bool recipeListMoving: false;

        anchors.left: parent.left;
        anchors.right: parent.right;
        anchors.top: parent.top;
        anchors.topMargin: 80;
        height: 226; // same bounds position like MenuPage.qml
        displayMarginBeginning: 500;
        displayMarginEnd: 500;
        preferredHighlightBegin:     0
        preferredHighlightEnd:       0
        highlightFollowsCurrentItem: true
        interactive: recipeCount > 3

        model: recipeCount
        delegate: RecipeEditListEntry
        {
            currentRecipeValue: parseFloat(recipeList[index].recipeBigValue + "." + recipeList[index].recipeCommaValue);
            currentRecipeDdd:   parseFloat(recipeList[index].dddBigValue + "." + recipeList[index].dddCommaValue);
            currentPortaFilter:  recipeList[index].portaFilterIndex
            currentTimeRecipeIndex: index;

            onOpenRecipeListEntryEdit:
            {
                openEditRecipeList( listRecipeIndex )
            }

            onRecipeListMovingChanged:
            {
                if ( recipeListMoving )
                {
                    abortLongPress()
                }
            }

            property bool recipeListMoving: recipeRow.recipeListMoving
        }

        onMovementStarted:
        {
            recipeListMoving = true;
        }

        onMovementEnded:
        {
            recipeListMoving = false;
        }
    }

    // button to add recipes right/bottom
    EkxButton
    {
        // id: idButtonAddRecipe;
        visible: recipeCount < recipeControl.maxTimeRecipesNum() ? true : false;
        z: 2;

        anchors.right: parent.right
        anchors.bottom: parent.bottom
        buttonType: EkxButton.ButtonType.PLUS;
        onClicked:
        {
            recipeControl.addTimeRecipe();
            openEditRecipeList( recipeCount - 1 );
        }
    }

    Timer
    {
        id: animationTimer
        interval: 400;
        running: false;
        repeat: true

        property int counter: 0;

        onTriggered:
        {
            if ( counter < recipeCount )
            {
                recipeRow.contentItem.children[counter].startJiggling();
                counter++;
            }
            else
            {
                counter = 0;
                animationTimer.stop();
            }
        }
    }
}
