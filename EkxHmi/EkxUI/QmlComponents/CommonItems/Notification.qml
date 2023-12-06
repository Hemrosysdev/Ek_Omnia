import QtQuick 2.0

import EkxSqliteTypes 1.0

Item
{
    id: idRoot

    width: 480
    height: 360

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // self defined properties
    ///////////////////////////////////////////////////////////////////////////////////////////////

    property int classId: EkxSqliteTypes.None
    property alias icon: idNotifySym.source
    property alias text: idNotifyText.text
    property alias font: idNotifyText.font
    property alias cancelLabel: idCancelText.text
    property alias confirmLabel: idConfirmText.text

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // states
    ///////////////////////////////////////////////////////////////////////////////////////////////

    states: [
        State
        {
            when: idRoot.classId === EkxSqliteTypes.None
            PropertyChanges { target: idNotifyText; color: idMain.colorEkxTextGrey }
            //PropertyChanges { target: idLongPressAnimation; fillColor: idMain.colorEkxConfirmGreen }
            PropertyChanges { target: idConfirmText; color: idMain.colorEkxConfirmGreen }
        },
        State
        {
            when: idRoot.classId === EkxSqliteTypes.Info
            PropertyChanges { target: idNotifyText; color: idMain.colorEkxTextWhite }
            //PropertyChanges { target: idLongPressAnimation; fillColor: idMain.colorEkxConfirmGreen }
            PropertyChanges { target: idConfirmText; color: idMain.colorEkxConfirmGreen }
        },
        State
        {
            when: idRoot.classId === EkxSqliteTypes.Warning
            PropertyChanges { target: idNotifyText; color: idMain.colorEkxWarning }
            //PropertyChanges { target: idLongPressAnimation; fillColor: idMain.colorEkxTextWhite }
            PropertyChanges { target: idConfirmText; color: idMain.colorEkxWarning }
        },
        State
        {
            when: idRoot.classId === EkxSqliteTypes.Error
            PropertyChanges { target: idNotifyText; color: idMain.colorEkxError }
            //PropertyChanges { target: idLongPressAnimation; fillColor: idMain.colorEkxTextWhite }
            PropertyChanges { target: idConfirmText; color: idMain.colorEkxError }
        }
    ]

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // signals
    ///////////////////////////////////////////////////////////////////////////////////////////////

    signal cancelClicked
    signal confirmClicked

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // functions
    ///////////////////////////////////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // signal handlers
    ///////////////////////////////////////////////////////////////////////////////////////////////

    Component.onCompleted:
    {
        if ( visible )
        {
            // don't manipulate navigation buttons down under
            //settingsMenuContentFactory.navAvailable = false;
        }
    }

    onVisibleChanged:
    {
        if ( visible )
        {
            // don't manipulate navigation buttons down under
            //settingsMenuContentFactory.navAvailable = false;
        }
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // items
    ///////////////////////////////////////////////////////////////////////////////////////////////

    Rectangle
    {
        anchors.fill: parent

        color: "black"

        Item
        {
            id: idBanner

            anchors.top: parent.top
            anchors.left: parent.left

            width: parent.width
            height: 72

            Image
            {
                id: idNotifySym

                anchors.centerIn: parent
            }
        }

        Text
        {
            id: idNotifyText

            anchors.top: idBanner.bottom
            anchors.left: parent.left

            width: parent.width
            height: 208

            font.pixelSize: 58
            font.letterSpacing: 2.9
            font.bold: true

            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter

            font.family: "D-DIN Condensed HEMRO"

            color: idMain.colorEkxWarning

            //            layer.enabled: true
            //            layer.smooth: true;

            //            // This item should be used as the 'mask'
            //            layer.samplerName: "maskSource"
            //            layer.effect: LongPressShaderEffect
            //            {
            //                longPressAnimation: idLongPressAnimation
            //                backgroundColor: idNotifyText.color
            //                fillColor: idLongPressAnimation.fillColor
            //            }
        }

        Item
        {
            id: idCancelBox

            anchors.left: parent.left
            anchors.top: idNotifyText.bottom
            anchors.bottom: parent.bottom

            width: parent.width / 2

            Text
            {
                id: idCancelText

                anchors.centerIn: parent

                font.pixelSize: 43
                font.letterSpacing: 1.48
                font.family: "D-DIN Condensed HEMRO"

                color: idMain.colorEkxTextWhite
            }

            MouseArea
            {
                anchors.fill: parent

                onClicked:
                {
                    idRoot.cancelClicked();
                }
            }
        }

        Item
        {
            id: idConfirmBox

            anchors.right: parent.right
            anchors.top: idNotifyText.bottom
            anchors.bottom: parent.bottom

            width: parent.width / 2

            Text
            {
                id: idConfirmText

                anchors.centerIn: parent

                font.pixelSize: 43
                font.letterSpacing: 1.48
                font.family: "D-DIN Condensed HEMRO"

                color: idMain.colorEkxTextWhite

                //                LongPressAnimation
                //                {
                //                    id: idLongPressAnimation

                //                    incrementCntStopAnimation: 15
                //                    fillColor: idMain.colorEkxConfirmGreen

                //                    onLongPressed:
                //                    {
                //                    }
                //                }
            }

            MouseArea
            {
                anchors.fill: parent

                onClicked:
                {
                    idRoot.confirmClicked();
                }
            }
        }
    }
}
