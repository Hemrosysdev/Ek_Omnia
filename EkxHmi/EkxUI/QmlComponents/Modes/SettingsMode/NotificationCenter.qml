import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQml.Models 2.1

import "../../CommonItems"

Item
{
    width: 480
    height: 360

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // self defined properties
    ///////////////////////////////////////////////////////////////////////////////////////////////

    property var notificationList: notificationCenter.notificationList
    property int selectedItem: -1

    property var colorList: [ idMain.colorEkxTextWhite, idMain.colorEkxTextWhite, idMain.colorEkxWarning, idMain.colorEkxError ];

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // functions
    ///////////////////////////////////////////////////////////////////////////////////////////////

    function save()
    {
        // do nothing
    }

    function previousClicked()
    {
        selectedItem = -1;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // signal handlers
    ///////////////////////////////////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // items
    ///////////////////////////////////////////////////////////////////////////////////////////////

    ShadowGroup
    {
        z: 1;
        shadowTopSize: ShadowGroup.ShadowTopSizes.StandardSize;
        shadowBottomSize: ShadowGroup.ShadowBottomSizes.StandardSize;
    }

    ListView
    {
        id: idNotificationList

        visible: selectedItem === -1

        anchors.fill: parent
        topMargin: 84
        bottomMargin: 0
        cacheBuffer: parent.height

        boundsMovement: Flickable.StopAtBounds
        flickableDirection: Flickable.VerticalFlick
        snapMode: ListView.SnapToItem
        orientation: ListView.Vertical

        interactive: true

        ScrollBar.vertical: ScrollBar
        {
        }

        model: notificationList

        footer: Item { height: 75 }

        delegate: Item
        {
            id: idDelegate

            width: parent.width
            height: 96

            anchors.left: parent.left

            Item
            {
                id: idNotificationImageBox

                anchors.left: parent.left
                anchors.top: parent.top

                width: 80
                height: parent.height

                Image
                {
                    anchors.centerIn: parent

                    source: modelData.listEntryImage
                    opacity: modelData.acknowledged ? 0.5 : 1.0
                }
            }

            Text
            {
                id: idShortText

                anchors.left: idNotificationImageBox.right
                anchors.top: idNotificationImageBox.top
                width: 320

                text: modelData.shortText

                color: idMain.colorEkxTextWhite
                opacity: modelData.acknowledged ? 0.25 : 1.0

                font.pixelSize: 43
                font.letterSpacing: 1.48
                font.family: "D-DIN Condensed HEMRO"
            }

            Text
            {
                id: idTimestamp

                anchors.left: idShortText.left
                anchors.bottom: idNotificationImageBox.bottom
                width: idShortText.width

                text: modelData.timestamp
                color: idMain.colorEkxTextWhite
                opacity: modelData.acknowledged ? 0.25 : 0.5

                font.pixelSize: 43
                font.letterSpacing: 1.48
                font.family: "D-DIN Condensed HEMRO"
            }

            Item
            {
                id: idAcknowledgeImageBox

                anchors.left: idShortText.right
                anchors.right: parent.right
                anchors.top: idNotificationImageBox.top
                height: idNotificationImageBox.height

                Image
                {
                    visible: !modelData.acknowledged

                    anchors.centerIn: parent

                    source: "qrc:/Icons/Checkbox-34px_white.png"
                }

                Image
                {
                    visible: modelData.acknowledged

                    anchors.centerIn: parent

                    source:  "qrc:/Icons/Checkmark-34px_white.png"
                }
            }

            MouseArea
            {
                id: idAcknowledgeArea

                z: 3

                visible: !modelData.acknowledged

                anchors.fill: idAcknowledgeImageBox

                onClicked:
                {
                    modelData.activateAcknowledge();
                }
            }

            MouseArea
            {
                id: idItemArea

                z: 2

                anchors.fill: parent

                onClicked:
                {
                    selectedItem = index;
                }
            }
        }
    }

    ListView
    {
        id: idDetailsView

        visible: selectedItem !== -1

        width: parent.width
        height: parent.height - anchors.topMargin

        anchors.top: parent.top
        anchors.topMargin: 84
        anchors.bottom: parent.bottom

        contentWidth: idDetailsColumn.width
        contentHeight: idDetailsColumn.height
        clip : true

        ScrollBar.vertical: ScrollBar
        {
        }

        //ScrollBar.vertical.policy: ScrollBar.AlwaysOn

        onVisibleChanged:
        {
            if ( visible )
            {
                // override onClicked function of the previous button
                idButtonPrevious.clickFunc = function() { previousClicked(); }
            }
            else
            {
                idButtonPrevious.clickFunc = null
            }
        }

        model: ObjectModel
        {
            Column
            {
                id: idDetailsColumn

                width: idDetailsView.width

                Item
                {
                    id: idBanner

                    width: parent.width
                    height: 72

                    Image
                    {
                        anchors.centerIn: parent
                        source: selectedItem === -1 ? "" : notificationCenter.notificationList[selectedItem].image
                    }
                }

                Text
                {
                    id: idItemLongText

                    text: selectedItem === -1 ? "" : notificationCenter.notificationList[selectedItem].longText

                    width: parent.width
                    height: 147

                    font.pixelSize: 58
                    font.letterSpacing: 2
                    font.bold: true

                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter

                    font.family: "D-DIN Condensed HEMRO"

                    color:  selectedItem === -1 ? idMain.colorEkxTextWhite : colorList[ notificationCenter.notificationList[selectedItem].classId ]
                }

                Item
                {
                    id: idDescriptionSpacer

                    width: parent.width
                    height: 20
                }

                Text
                {
                    id: idItemDescription

                    text: selectedItem === -1 ? "" : notificationCenter.notificationList[selectedItem].description

                    width: parent.width

                    font.pixelSize: 43
                    font.letterSpacing: 1.48

                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    wrapMode: Text.WordWrap

                    font.family: "D-DIN Condensed HEMRO"

                    color: idMain.colorEkxTextWhite
                }

                Text
                {
                    id: idItemNotificationId

                    text: selectedItem === -1 ? "" : ( "\n"
                                                      + "ID: " + notificationCenter.notificationList[selectedItem].typeId + "\n"
                                                      + "\n"
                                                      + "Please contact Service:\n"
                                                      + "\n"
                                                      + "service-center@hemrogroup.com\n"
                                                      + "\n"
                                                      + "+49 40 696 940 38\n"
                                                      + "or +49 40 696 940 37\n"
                                                      + "\n" );

                    width: parent.width

                    font.pixelSize: 36
                    font.letterSpacing: 1.24

                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter

                    font.family: "D-DIN Condensed HEMRO"

                    color: idMain.colorEkxTextWhite
                    opacity: 0.75
                }

                Rectangle
                {
                    id: idQrCodeHelp

                    width: parent.width
                    height: 240

                    color: selectedItem === -1 ? idMain.colorEkxTextWhite : colorList[notificationCenter.notificationList[selectedItem].classId]

                    Item
                    {
                        id: idQrCodeBox

                        anchors.top: parent.top
                        anchors.left: parent.left

                        width: parent.width
                        height: idQrCode.height + 40

                        Image
                        {
                            id: idQrCode

                            anchors.centerIn: parent

                            source: "qrc:/Icons/qr-code.png"
                        }
                    }

                    Text
                    {
                        text: "Get additional information"

                        anchors.top: idQrCodeBox.bottom

                        width: parent.width

                        font.pixelSize: 36
                        font.letterSpacing: 1.24

                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter

                        font.family: "D-DIN Condensed HEMRO"

                        color: "black"
                    }
                }

                Item
                {
                    id: idSpacer
                    height: 20
                    width: parent.width
                }
            }
        }
    }
}
