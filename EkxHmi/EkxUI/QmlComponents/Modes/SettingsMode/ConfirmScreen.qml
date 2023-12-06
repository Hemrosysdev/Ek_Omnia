import QtQuick 2.12
import QtQml 2.12
import "../../CommonItems" as Common

Item
{
    id: idRoot;

    width: 480;
    height: 360;

    property int entryType;
    property string infoText: "Confirm\nUpload SW"

    property string confirmText: "Confirm"
    property string cancelText: "Cancel"
    property string okText: "Ok"

    property bool enableConfirm: true
    property bool enableCancel: true
    property bool enableOk: false

    property bool colorRedNotGreen: false;
    property bool hasBackground: true;
    property bool navAvailable_backup:false;
    property bool initDone: false;

    // for legacy compatibility: (to be removed)
    property var cancelFunction;
    property var confirmFunction;
    property var okFunction;

    signal userConfirmed();
    signal userCanceled();
    signal userNoticed();

    function save()
    {
        //do nothing
    }

    Component.onCompleted:
    {
        //console.log("ConfirmScreen.onCompleted:");
        if(visible)
        {
            navAvailable_backup = settingsMenuContentFactory.navAvailable;
            settingsMenuContentFactory.navAvailable = false;
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
                navAvailable_backup = settingsMenuContentFactory.navAvailable;
                settingsMenuContentFactory.navAvailable = false;
            }
            else
            {
                settingsMenuContentFactory.navAvailable = navAvailable_backup;
            }
        }
        else
        {
            if(visible)
            {
                navAvailable_backup = settingsMenuContentFactory.navAvailable;
                settingsMenuContentFactory.navAvailable = false;
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

    Item {
        anchors.horizontalCenter: parent.horizontalCenter;
        anchors.top: parent.top;
        anchors.topMargin: 72;
        width: 480;
        height: 208;

        Common.Label {
            anchors.verticalCenter: parent.verticalCenter
            width: parent.width
            // To make sure the LongPressAnimation doesn't have to travel too
            // far outside the visible area, we put the Label into a container
            // and made the Label only as tall as its actual text contents.
            height: Math.min(parent.height, implicitHeight)
            text: infoText
            pixelSize: 58
            bold: true
            font.letterSpacing: 2.0;
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.Wrap;

            layer.enabled: idLongPressAnimation.layerEnabled
            layer.smooth: true;

            // This item should be used as the 'mask'
            layer.samplerName: "maskSource"
            layer.effect: Common.LongPressShaderEffect
            {
                longPressAnimation: idLongPressAnimation
            }
        }
    }

    Text
    {
        //id: idLeftChoice;
        visible: enableCancel

        text: cancelText;
        color: idMain.colorEkxTextWhite

        anchors.left: parent.left;
        anchors.right: parent.horizontalCenter;
        anchors.bottom: parent.bottom;
        height: 80;

        font.pixelSize: 43;
        font.letterSpacing: 1.48;
        font.bold: false;
        font.family: "D-DIN Condensed HEMRO"
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter;
        wrapMode: Text.Wrap;

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
        visible: enableConfirm

        text: confirmText;
        color: (colorRedNotGreen===true)? idMain.colorEkxConfirmRed : idMain.colorEkxConfirmGreen;

        anchors.left: parent.horizontalCenter;
        anchors.right: parent.right;
        anchors.bottom: parent.bottom;
        height: 80;

        font.pixelSize: 43;
        font.letterSpacing: 1.48;
        font.bold: false;
        font.family: "D-DIN Condensed HEMRO"
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter;
        wrapMode: Text.Wrap;

        Common.LongPressAnimation
        {
            id: idLongPressAnimation

            duration: 1500
            longPressOnly: true
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
        visible: enableOk

        text: okText;
        color: (colorRedNotGreen===true)? idMain.colorEkxConfirmRed : idMain.colorEkxConfirmGreen;

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



