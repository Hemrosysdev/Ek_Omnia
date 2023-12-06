import QtQuick 2.0
import "../../CommonItems"
import EkxSqliteTypes 1.0

Item
{
    width: 480
    height: 360

    property int entryType;
    function save()
    {
        // nothing to do
    }

    function cancelEditGrindername()
    {
        confirm.visible = false;
        multiWheel.visible = true;
        idButtonOk.visible = true;
        idButtonBackspace.visible = true;
        settingsStateMachine.navAvailable = true;
    }

    function confirmEditGrinderName()
    {
        settingsSerializer.grinderName = multiWheel.getBeanName();

        sqliteInterface.addEvent( EkxSqliteTypes.SqliteEventType_CHANGE_GRINDER_NAME,
                                  multiWheel.getBeanName() );

        settingsSerializer.saveSettings();
        settingsStateMachine.back();
    }


    property string oldGrindername;

    onOldGrindernameChanged:
    {
        multiWheel.init( oldGrindername )
        multiWheel.setCharacterValues();
        multiWheel.checkSelection();
    }

    MultiCharacterWheel
    {
        id: multiWheel

        z:1

        anchors.left: parent.left
        anchors.top:  parent.top
    }

    EkxButton
    {
        id: idButtonBackspace;
        z:1
        anchors.right: parent.right
        anchors.top: parent.top
        buttonType: EkxButton.ButtonType.BACKSP;
        onClicked:
        {
            multiWheel.deleteText();
        }
    }


    EkxButton
    {
        id: idButtonOk;
        z:1
        buttonType: EkxButton.ButtonType.OK;
        onClicked:
        {
            confirm.infoText = "Set Smiley-name to: \n " + multiWheel.getBeanName();
            multiWheel.visible = false;
            idButtonOk.visible = false;
            idButtonBackspace.visible = false;
            confirm.visible = true;
        }
    }

    ConfirmScreen
    {
        id: confirm
        z:2
        visible: false
        onUserCanceled:
        {
            cancelEditGrindername();
        }
        onUserConfirmed:
        {
            confirmEditGrinderName();
        }
    }


}
