import QtQuick 2.0
import "../../CommonItems"
import SettingStatesEnum 1.0
import SettingsSerializerEnum 1.0

Item
{
    width: 480
    height: 360

    property int entryType;

    function save()
    {
        // do nothing
    }


    function confirmSaveFunction()
    {
        var relateduser;

        if (entryType === SettingStatesEnum.SETTING_USER_CPC_STORE)
        {
            relateduser = SettingsSerializerEnum.LOGGEDIN_STOREOWNER;
        }
        else
        {
             relateduser = SettingsSerializerEnum.LOGGEDIN_TECHNICIAN;
        }

        var save = settingsStateMachine.savePinCode( pin.currentPin, relateduser )
        settingsStateMachine.back()
    }

    function cancelSaveFunction()
    {
        pin.visible = true
        infoText.visible = true
        successScreen.visible = false;
    }

    Item
    {
        width: 480
        height: 72

        z: 2

        anchors.top: parent.top

        Text
        {
            id: infoText
            text: "Enter current PIN"

            font.pixelSize: 40
            font.bold: true
            opacity: 0.7
            font.letterSpacing: 2
            font.family: "D-DIN Condensed HEMRO"

            color: idMain.colorEkxTextWhite

            anchors.centerIn: parent
        }

    }

    PinScreen
    {
        id: pin
        currentPin: "0000"
        checkPinScreen: true

        storeOwnerLogin: entryType === SettingStatesEnum.SETTING_USER_CPC_STORE ? true : false
        loginNeeded: false;

        onSavedPinResult:
        {
            pin.visible = false
            infoText.visible = false
            successScreen.visible = true;
        }
        onCheckedPinResult:
        {            
            if( success )
            {
                currentPin = "0000";
                checkPinScreen = false;                
                infoText.text = "Enter new PIN"
                infoText.color = idMain.colorEkxConfirmGreen
                infoText.opacity = 1.0
                setStartPin();
            }
            else
            {
                pin.visible = true
                infoText.visible = true;
                infoText.color = idMain.colorEkxConfirmRed;
                infoText.opacity = 1.0
            }
        }
        onInfoTimeOut:
        {
            infoText.color = idMain.colorEkxTextWhite;
            infoText.opacity = 0.7
        }
    }


    ConfirmScreen
    {
        id: successScreen
        visible: false
        anchors.centerIn: parent
        infoText: "Change PIN Code to:  \n " + pin.currentPin
        isInfoNotQuestion : false
        onUserConfirmed:
        {
            confirmSaveFunction();
        }
        onUserCanceled:
        {
            cancelSaveFunction();
        }
    }
}
