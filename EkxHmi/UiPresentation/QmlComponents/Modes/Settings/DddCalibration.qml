import QtQuick 2.0
import "../../CommonItems"

Item
{
    width: 480
    height:360

    property int entryType;

    function save()
    {
        console.log("Save calibration values: Type: " + entryType)
    }

    ConfirmScreen
    {
        id: idQuestion;
        visible: true
        infoText:  "Zero the ruler &\nconfirm calibration";
        isInfoNotQuestion: false;
        confirmText: "Calibrate";
        onUserCanceled:
        {
            settingsStateMachine.back();
        }
        onUserConfirmed:
        {
            dddDriver.calibrateDdd();
            idInfo.visible = true;
        }
    }

    ConfirmScreen
    {
        id: idInfo;
        visible: false;
        infoText:  "Calibration\nsuccessful!"
        isInfoNotQuestion: true;
        onUserNoticed:
        {
            settingsStateMachine.back();
        }
    }
}
