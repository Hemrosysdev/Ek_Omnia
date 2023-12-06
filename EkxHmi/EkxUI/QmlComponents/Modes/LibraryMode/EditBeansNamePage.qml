import QtQml 2.12
import QtQuick 2.12
import "../../CommonItems" as Common

Common.OverlayPage
{
    id: idEditBeansNamePage
    objectName: "LibraryMode_EditBeansNamePage"

    topAction: Common.EkxButton.ButtonType.BACKSP
    // Don't allow continuing with empty bean name.
    rightAction: workflow.beanName.length > 0 ? Common.EkxButton.NEXT : 0
    rightActionLongPressEnabled: !workflow.isNewItem
    rightActionLongPressColor: idMain.colorEkxConfirmGreen

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // signal handlers
    ///////////////////////////////////////////////////////////////////////////////////////////////

    onTopActionClicked: {
        idEditBeansName.deleteText();
    }

    onActivated: {
        idEditBeansName.init(idEditBeansNamePage.workflow.beanName);
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
        id: idEditBeansName

        anchors.left: parent.left
        anchors.top:  parent.top

        onCurrentTextChanged: {
            idEditBeansNamePage.workflow.beanName = currentText;
        }
    }
}
