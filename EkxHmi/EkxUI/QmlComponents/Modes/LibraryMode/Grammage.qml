import QtQuick 2.12
import QtQuick.Layouts 1.1

import "../../CommonItems" as Common

RowLayout {
    id: idGrammage

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // properties
    ///////////////////////////////////////////////////////////////////////////////////////////////

    spacing: 30
    layer.enabled: idGrammage.longPressAnimation ? idGrammage.longPressAnimation.layerEnabled : false
    layer.samplerName: "maskSource"
    layer.effect: Common.LongPressShaderEffect {
        longPressAnimation: idGrammage.longPressAnimation
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // self defined properties
    ///////////////////////////////////////////////////////////////////////////////////////////////

    property int grammageMg
    property int dosingTimeMs
    property alias iconSource: idImage.source

    property bool dosingTimeVisible

    property Common.LongPressAnimation longPressAnimation: null

    // Tutorial needs to highlight these.
    property alias gramageLabel: idGrammageLabel
    property alias dosingTimeLabel: idDosingTimeLabel

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // items
    ///////////////////////////////////////////////////////////////////////////////////////////////

    ColumnLayout {
        Layout.alignment: Qt.AlignVCenter
        Layout.preferredWidth: 80
        spacing: 0

        Image {
            id: idImage
            Layout.alignment: Qt.AlignHCenter
            Layout.preferredWidth: 80
            Layout.preferredHeight: Layout.preferredWidth / (sourceSize.width / sourceSize.height)
            asynchronous: true
        }
    }

    ColumnLayout {
        Layout.fillWidth: true
        spacing: 0

        Common.Label {
            id: idGrammageLabel
            Layout.fillWidth: true
            Layout.fillHeight: true
            pixelSize: idGrammage.dosingTimeVisible ? 45 : 82
            text: (idGrammage.grammageMg / 1000).toFixed(1) + " g"
            font.letterSpacing: 0.33
            horizontalAlignment: Text.AlignRight
            verticalAlignment: Text.AlignVCenter
        }

        Common.Label {
            id: idDosingTimeLabel
            Layout.fillWidth: true
            Layout.fillHeight: true
            pixelSize: 45
            text: (idGrammage.dosingTimeMs / 1000).toFixed(1) + " s"
            font.letterSpacing: 0.33
            horizontalAlignment: Text.AlignRight
            verticalAlignment: Text.AlignVCenter
            visible: idGrammage.dosingTimeVisible
        }
    }
}
