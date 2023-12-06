import QtQml 2.12
import QtQuick 2.0
import "../../CommonItems"
import SettingsSerializerEnum 1.0

Item
{
    width: 480
    height:360

    property string currentPin: "0000";
    property bool checkPinScreen: true
    property bool loginNeeded: true
    property bool storeOwnerLogin: true

    signal checkedPinResult( var success );
    signal savedPinResult( var success );
    signal infoTimeOut();

    function checkOrSavePW()
    {
        var relateduser;
        if(storeOwnerLogin)
        {
            relateduser = SettingsSerializerEnum.LOGGEDIN_STOREOWNER;
        }
        else
        {
             relateduser = SettingsSerializerEnum.LOGGEDIN_TECHNICIAN;
        }

        if( checkPinScreen )
        {
            // check Pin and report
            var check = settingsStateMachine.checkPinCode( currentPin, relateduser, loginNeeded )
            console.log("checkPinCode returned: ",check);
            if( !check )
            {
                pinCheckTimer.interval = 2000;
                setPinWrong();
                pinCheckTimer.start();
            }
            else
            {
                pinCheckTimer.interval = 1000;
                setPinRight();
                pinCheckTimer.start();
            }


        }
        else
        {
            // save Pin and report
            savedPinResult(true)
        }
    }

    function setStartPin()
    {
        if( currentPin.length === 4 )
        {
            cWheelRepeater.itemAt(0).setStartValues( parseInt( currentPin.charAt(0) ) );
            cWheelRepeater.itemAt(1).setStartValues( parseInt( currentPin.charAt(1) ) );
            cWheelRepeater.itemAt(2).setStartValues( parseInt( currentPin.charAt(2) ) );
            cWheelRepeater.itemAt(3).setStartValues( parseInt( currentPin.charAt(3) ) );
        }
        else
        {
            console.log("A terrible Mistake has happened!");
        }
    }

    function setPinWrong( )
    {
        cWheelRepeater.itemAt(0).pinWrong = true;
        cWheelRepeater.itemAt(1).pinWrong = true;
        cWheelRepeater.itemAt(2).pinWrong = true;
        cWheelRepeater.itemAt(3).pinWrong = true;
        cWheelRepeater.itemAt(0).pinRight = false;
        cWheelRepeater.itemAt(1).pinRight = false;
        cWheelRepeater.itemAt(2).pinRight = false;
        cWheelRepeater.itemAt(3).pinRight = false;

        acceptColorRect.color = idMain.colorEkxConfirmRed
    }

    function setPinRight( )
    {
        cWheelRepeater.itemAt(0).pinWrong = false;
        cWheelRepeater.itemAt(1).pinWrong = false;
        cWheelRepeater.itemAt(2).pinWrong = false;
        cWheelRepeater.itemAt(3).pinWrong = false;
        cWheelRepeater.itemAt(0).pinRight = true;
        cWheelRepeater.itemAt(1).pinRight = true;
        cWheelRepeater.itemAt(2).pinRight = true;
        cWheelRepeater.itemAt(3).pinRight = true;

         acceptColorRect.color = idMain.colorEkxConfirmGreen
    }

    function setPinNeutral( )
    {
        cWheelRepeater.itemAt(0).pinWrong = false;
        cWheelRepeater.itemAt(1).pinWrong = false;
        cWheelRepeater.itemAt(2).pinWrong = false;
        cWheelRepeater.itemAt(3).pinWrong = false;
        cWheelRepeater.itemAt(0).pinRight = false;
        cWheelRepeater.itemAt(1).pinRight = false;
        cWheelRepeater.itemAt(2).pinRight = false;
        cWheelRepeater.itemAt(3).pinRight = false;

        acceptColorRect.color = idMain.colorEkxTextWhite
    }

    Timer
    {
        id: pinCheckTimer
        interval: 2000
        repeat: false

        onTriggered:
        {
            if( interval === 1000 )
            {
                setPinNeutral();
                checkedPinResult(true)
                pinSuccessTimer.start();
            }
            else
            {
                setPinNeutral();
                checkedPinResult(false)
                pinSuccessTimer.start();
            }
        }
    }

    Timer
    {
        id: pinSuccessTimer
        interval: 1000
        repeat: false

        onTriggered:
        {
            infoTimeOut();
        }

    }

    ShadowGroup
    {
        z: 1;

        shadowTopSize: 135;
        shadowTopThirdGradientStopEnable:   true;
        shadowTopThirdGradientStopPosition: 0.6;
        shadowTopThirdGradientStopOpacity:  0.8;

        shadowBottomSize: 145;
        shadowBottomThirdGradientStopEnable:   true;
        shadowBottomThirdGradientStopPosition: 0.6;
        shadowBottomThirdGradientStopOpacity:  0.8;
    }


    Row {
        id: cWheelRow
        width: 326
        height:360

        anchors.left: parent.left
        anchors.leftMargin: 77
        anchors.top: parent.top
        spacing: 0

        Repeater {
            id: cWheelRepeater
            model: 4
            SettingsNumberWheel
            {
                id: cwheel
                currentFontFamiliy: "D-DIN Condensed HEMRO"

                model: 10

                interactive: true

                onCurrentValue:
                {
                     currentPin   = currentPin.substring(0, index) + value.toString() + currentPin.substring(index + 1);
                }
            }

        }
    }

    Rectangle
    {
        id: acceptColorRect

        z:4

        anchors.fill: acceptIcon
        color:  idMain.colorEkxTextWhite
    }

    Image
    {
        id: acceptIcon
        z:4

        width: 72
        height: 72

        source: "qrc:/Icons/ICON_Confirm-72px_white_transparent.png"

        anchors.right: parent.right
        anchors.bottom: parent.bottom


        MouseArea
        {
            anchors.fill: parent

            onClicked:
            {
                checkOrSavePW();
            }
        }
    }

    Component.onCompleted:
    {
        setStartPin();
    }
}
