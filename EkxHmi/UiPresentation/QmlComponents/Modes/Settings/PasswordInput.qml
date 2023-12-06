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
    property int entryType;

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // functions
    ///////////////////////////////////////////////////////////////////////////////////////////////

    function save()
    {
        // nothing to do
    }

    function cancelCalibration()
    {
        idConfirmView.visible = false;
        idMultiWheel.visible = true;
        idButtonOk.visible = true;
        idBackspaceIcon.visible = true;

        settingsStateMachine.navAvailable = true;
    }

    function confirmCalibration()
    {
        settingsSerializer.setWifiPw(idMultiWheel.getBeanName());
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
            idConfirmView.infoText = "New Password \n " + idMultiWheel.getBeanName();
            idMultiWheel.visible = false;
            idButtonOk.visible = false;
            idBackspaceIcon.visible = false;
            idConfirmView.visible = true;
        }
    }

    ConfirmScreen
    {
        id: idConfirmView
        z:2
        visible: false
        onUserCanceled:
        {
            cancelCalibration();
        }
        onUserConfirmed:
        {
            confirmCalibration();
        }
    }
}
