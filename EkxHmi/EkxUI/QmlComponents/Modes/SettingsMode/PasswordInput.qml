import QtQuick 2.0
import "../../CommonItems"

Item
{
    id: idRoot

    width: 480
    height: 360

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // self defined properties
    ///////////////////////////////////////////////////////////////////////////////////////////////

    property string oldPassword;

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // functions
    ///////////////////////////////////////////////////////////////////////////////////////////////

    function save()
    {
        // nothing to do
    }

    function cancelAction()
    {
        idConfirmView.visible = false;
        idMultiWheel.visible = true;
        idButtonOk.visible = true;
        idBackspaceIcon.visible = true;

        settingsMenuContentFactory.navAvailable = true;
    }

    function confirmAction()
    {
        settingsSerializer.wifiApPassword = idMultiWheel.getBeanName();
        settingsSerializer.saveSettings();

        settingsStateMachine.back();
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // signal handlers
    ///////////////////////////////////////////////////////////////////////////////////////////////

    onOldPasswordChanged:
    {
        idMultiWheel.init( oldPassword )
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // items
    ///////////////////////////////////////////////////////////////////////////////////////////////

    MultiCharacterWheel
    {
        id: idMultiWheel

        z:1

        anchors.left: parent.left
        anchors.top:  parent.top
    }

    EkxButton
    {
        id: idBackspaceIcon
        z:1
        anchors.right: parent.right
        anchors.top: parent.top
        buttonType: EkxButton.ButtonType.BACKSP;
        onClicked:
        {
            idMultiWheel.deleteText();
        }
    }

    EkxButton
    {
        id: idButtonOk;
        z:1
        buttonType: EkxButton.ButtonType.OK;
        onClicked:
        {
            idMultiWheel.visible = false;
            idButtonOk.visible = false;
            idBackspaceIcon.visible = false;

            if ( idMultiWheel.getBeanName() === settingsSerializer.wifiApPassword )
            {
                // no dialog needed
                settingsStateMachine.back();
            }

            else if ( idMultiWheel.getBeanName().length >= 8 && idMultiWheel.getBeanName().length <= 63 )
            {
                idConfirmView.infoText = "New Password \n " + idMultiWheel.getBeanName();
                idConfirmView.enableCancel = true;
                idConfirmView.enableConfirm = true;
                idConfirmView.enableOk = false;
                idConfirmView.colorRedNotGreen = false
                idConfirmView.visible = true;
            }
            else
            {
                idConfirmView.infoText = "Password wrong\n " + idMultiWheel.getBeanName();
                idConfirmView.enableCancel = false;
                idConfirmView.enableConfirm = false;
                idConfirmView.enableOk = true;
                idConfirmView.colorRedNotGreen = true
                idConfirmView.visible = true;
            }
        }
    }

    ConfirmScreen
    {
        id: idConfirmView
        z:2
        visible: false

        onUserCanceled:
        {
            cancelAction();
        }

        onUserConfirmed:
        {
            confirmAction();
        }

        onUserNoticed:
        {
            cancelAction();
        }
    }
}
