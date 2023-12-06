import QtQuick 2.12
import QtQml 2.12
import "../../CommonItems"

Item
{
    id: idRoot;
    property int entryType;
    property string infoText: "Confirm \n Upload SW"
    property string confirmText: "Confirm"
    property string cancelText: "Cancel";
    property string okText: "Ok";
    property bool isInfoNotQuestion: false
    property bool colorRedNotGreen: false;
    property bool hasBackground: true;
    property bool navAvailable_backup:false;
    property bool initDone: false;

    signal userConfirmed();
    signal userCanceled();
    signal userNoticed();

    // for legacy compatibility: (to be removed)
    property var cancelFunction;
    property var confirmFunction;
    property var okFunction;

    function save()
    {
        //do nothing
    }

    width: 480;
    height: 360;

    Component.onCompleted:
    {
        //console.log("ConfirmScreen.onCompleted:");
        if(visible)
        {
            navAvailable_backup = settingsStateMachine.navAvailable;
            settingsStateMachine.navAvailable = false;
        }
        initDone = true;
    }

    onVisibleChanged:
    {
        //console.log("ConfirmScreen.onVisibleChanged: "+!visible+"->"+visible
        //            +" navAvailableBackup="+navAvailable_backup
        //            +" initDone="+initDone);
        if(initDone)
        {
            if(visible)
            {
                navAvailable_backup = settingsStateMachine.navAvailable;
                settingsStateMachine.navAvailable = false;
            }
            else
            {
                settingsStateMachine.navAvailable = navAvailable_backup;
            }
        }
        else
        {
            if(visible)
            {
                navAvailable_backup = settingsStateMachine.navAvailable;
                settingsStateMachine.navAvailable = false;
            }
            else
            {
                // do nothing
            }
        }
    }

    Rectangle
    {
        visible: hasBackground;
        anchors.fill: parent;
        color: "black";
    }

    Text
    {
        //id: idInfoText;
        anchors.horizontalCenter: parent.horizontalCenter;
        anchors.top: parent.top;
        anchors.topMargin: 72;
        width: 480;
        height: 208;
        font.pixelSize: 58;
        font.letterSpacing: 2.0;
        font.bold: true;
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter;
        font.family: "D-DIN Condensed HEMRO"
        wrapMode: Text.Wrap;
        color: idMain.colorEkxTextWhite
        text: infoText;

        layer.enabled: true
        layer.smooth: true;

        // This item should be used as the 'mask'
        layer.samplerName: "maskSource"
        layer.effect: LongPressShaderEffect
        {
            longPressAnimation: idLongPressAnimation
        }
    }

    Text
    {
        //id: idLeftChoice;
        visible: !isInfoNotQuestion;
        anchors.left: parent.left;
        anchors.right: parent.horizontalCenter;
        anchors.bottom: parent.bottom;
        height: 80;

        font.pixelSize: 43;
        font.letterSpacing: 1.48;
        font.bold: false;
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter;
        font.family: "D-DIN Condensed HEMRO"
        wrapMode: Text.Wrap;
        color: idMain.colorEkxTextWhite
        text: cancelText;

        MouseArea
        {
            anchors.fill: parent;
            onClicked:
            {
                idRoot.visible = false;
                userCanceled();
                if( typeof cancelFunction == "function" )
                    cancelFunction();
            }
        }
    }

    Text
    {
        //id: idRightChoice;
        visible: !isInfoNotQuestion;
        anchors.left: parent.horizontalCenter;
        anchors.right: parent.right;
        anchors.bottom: parent.bottom;
        height: 80;

        font.pixelSize: 43;
        font.letterSpacing: 1.48;
        font.bold: false;
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter;
        font.family: "D-DIN Condensed HEMRO"
        wrapMode: Text.Wrap;
        color: (colorRedNotGreen===true)? idMain.colorEkxConfirmRed : idMain.colorEkxConfirmGreen;
        text: confirmText;

        LongPressAnimation
        {
            id: idLongPressAnimation

            incrementCntStopAnimation: 15
            fillColor: (colorRedNotGreen===true)? idMain.colorEkxConfirmRed : idMain.colorEkxConfirmGreen;

            onLongPressed:
            {
                idRoot.visible = false;
                userConfirmed();
                if( typeof confirmFunction == "function" )
                    confirmFunction(); // Used by MenuPage.qml to set callback on dynamically created object (for "LogoutScreen").
            }
        }
    }

    Text
    {
        //id: idOk;
        visible: isInfoNotQuestion;
        anchors.horizontalCenter: parent.horizontalCenter;
        anchors.bottom: parent.bottom;
        width: parent.width/2;
        height: 80;

        font.pixelSize: 43;
        font.letterSpacing: 1.48;
        font.bold: false;
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter;
        font.family: "D-DIN Condensed HEMRO"
        wrapMode: Text.Wrap;
        color: (colorRedNotGreen===true)? idMain.colorEkxConfirmRed : idMain.colorEkxConfirmGreen;
        text: okText;

        MouseArea
        {
            anchors.fill: parent;
            onClicked:
            {
                idRoot.visible = false;
                userNoticed();
                if( typeof okFunction == "function" )
                    okFunction();
            }
        }
    }
}



