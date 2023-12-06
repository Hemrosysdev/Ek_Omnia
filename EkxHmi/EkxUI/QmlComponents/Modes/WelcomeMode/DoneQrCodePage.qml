import QtQml 2.12
import QtQuick 2.12
import QtQuick.Layouts 1.1

import ".." as Modes
import "../../CommonItems" as Common

import EkxMainstateEnum 1.0

Common.OverlayPage {
    id: idTutorialDonePage

    readonly property string displayUrl: "hemrogroup.com"

    closeButtonVisible: false

    Common.ShadowGroup {
        z: 1;
        shadowBottomSize: Common.ShadowGroup.ShadowBottomSizes.StandardSize;
        opacity: idFlickable.atYEnd ? 0 : 1
        visible: opacity > 0

        Behavior on opacity {
            NumberAnimation {
                duration: 200
            }
        }
    }

    Common.ShadowGroup {
        z: 1;
        shadowTopSize: Common.ShadowGroup.ShadowTopSizes.StandardSize;
        opacity: idFlickable.atYBeginning || idFlickable.atYEnd ? 0 : 1
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
        topMargin: 60
        contentHeight: idQrCodeColumn.height

        ColumnLayout {
            id: idQrCodeColumn
            width: idFlickable.width
            spacing: 20

            Common.Label {
                Layout.fillWidth: true
                text: `Great!
Discover and play around!`
                horizontalAlignment: Text.AlignHCenter
                pixelSize: 43
                font.letterSpacing: 1.48
            }

            Common.Label {
                Layout.fillWidth: true
                Layout.topMargin: 20
                horizontalAlignment: Text.AlignHCenter
                text: `For inspiration visit our video
channel:`
                pixelSize: 36
                font.letterSpacing: 1.24
                textColor: Common.Label.TextColor.Grey
            }

            Rectangle {
                radius: 50
                Layout.alignment: Qt.AlignHCenter
                Layout.preferredHeight: idTutorialDonePage.height
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
                        text: idTutorialDonePage.displayUrl
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
