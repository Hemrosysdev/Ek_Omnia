import QtQuick 2.0
import "../../CommonItems"
import SettingsStateEnum 1.0

Item
{
    width: 480
    height: 360

    property int entryType;

    function save()
    {
        // do nothing
    }

    PinScreen
    {
        id: pin
        currentPin: "0000"
        checkPinScreen: true
        loginNeeded: true

        storeOwnerLogin: entryType ===  SettingsStateEnum.SETTING_USER_STORE ? true : false

        onCheckedPinResult:
        {
            if( success )
            {
                settingsStateMachine.back()
            }
        }
    }
}
