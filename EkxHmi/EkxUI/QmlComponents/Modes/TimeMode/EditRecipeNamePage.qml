import QtQml 2.12
import QtQuick 2.12
import "../../CommonItems" as Common

Common.OverlayPage
{
    id: idEditRecipeNamePage

    rightActionLongPressEnabled: !workflow.isNewItem
    rightActionLongPressColor: idMain.colorEkxConfirmGreen

    topActionItem: Common.ActivateToggle {
        id: showRecipeNameToggle
        anchors.centerIn: parent
        onToggleChanged: workflow.showRecipeName = active;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // signal handlers
    ///////////////////////////////////////////////////////////////////////////////////////////////

    onActivated: {
        idRecipeNameWheel.init(idEditRecipeNamePage.workflow.recipeName);
        showRecipeNameToggle.active = workflow.showRecipeName;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // items
    ///////////////////////////////////////////////////////////////////////////////////////////////

    Common.ShadowGroup
    {
        shadowTopSize: Common.ShadowGroup.ShadowTopSizes.StandardSize;
        shadowBottomSize: Common.ShadowGroup.ShadowBottomSizes.StandardSize;
    }

    Common.MultiCharacterWheel
    {
        id: idRecipeNameWheel

        anchors.left: parent.left
        anchors.top:  parent.top

        onCurrentTextChanged: {
            idEditRecipeNamePage.workflow.recipeName = currentText;
        }
    }

    Common.EkxButton {
        anchors {
            right: parent.right
            verticalCenter: parent.verticalCenter
        }
        buttonType: Common.EkxButton.ButtonType.BACKSP
        onClicked: {
            idRecipeNameWheel.deleteText();
        }
    }
}
