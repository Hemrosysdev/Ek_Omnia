import QtQuick 2.12
import QtQuick.Layouts 1.1

import "../../CommonItems" as Common

Item
{
    id: idRoot

    width: 480
    height: 360

    readonly property string displayUrl: "hemrogroup.com"

    property int generalPixelSize: 38
    property int generalRectHeight: generalPixelSize + 20
    property int generalMargin: 10

    Component.onDestruction: {
        idTitle.opacity = 1;
    }

    function save() {
        // do nothing
    }


    Common.ShadowGroup {
        z: 1;
        shadowTopSize: Common.ShadowGroup.ShadowTopSizes.StandardSize;
        shadowBottomSize: Common.ShadowGroup.ShadowBottomSizes.StandardSize;
        opacity: idFlickable.atYEnd ? 0 : 1
        visible: opacity > 0

        Behavior on opacity {
            NumberAnimation {
                duration: 200
            }
        }
    }

    Flickable {
        id: idFlickable
        anchors.fill: parent
        boundsMovement: Flickable.StopAtBounds
        flickableDirection: Flickable.VerticalFlick
        topMargin: 95
        contentHeight: idFlickableColumn.height
        onAtYEndChanged: {
            idTitle.opacity = atYEnd ? 0 : 1;
        }

        ColumnLayout {
            id: idFlickableColumn
            width: idFlickable.width

            Common.Label {
                Layout.fillWidth: true
                height: idRoot.generalRectHeight
                horizontalAlignment: Text.AlignHCenter
                text: "For further information go to:"
                textColor: Common.Label.TextColor.Grey
                pixelSize: idRoot.generalPixelSize
                font.letterSpacing: 0.43
            }

            Rectangle {
                radius: 50
                Layout.alignment: Qt.AlignHCenter
                Layout.preferredHeight: idRoot.height
                width: idQrColumn.width + 2 * idQrColumn.x
                color: "white"

                ColumnLayout {
                    id: idQrColumn
                    x: 30
                    y: 30
                    spacing: 10

                    Common.Label {
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignHCenter
                        text: idRoot.displayUrl
                        pixelSize: 36
                        font.letterSpacing: 1.24
                        color: "black"
                    }

                    Image {
                        id: idQrCode
                        Layout.alignment: Qt.AlignHCenter
                        asynchronous: true
                        // Original image is 132px, only do integer scaling and no interpolation!
                        Layout.preferredWidth: 132 * 2
                        Layout.preferredHeight: Layout.preferredWidth
                        smooth: false
                        source: "qrc:/Icons/qr-code-hemrogroup-com.png"
                    }
                }
            }
        }
    }

}
