import QtQuick 2.12

Rectangle {
    id: idLinearShadow;
    //property bool visualDebug: false;
    property bool visualDebug: true;
    property color visualDebugColor: "red";
    property color visualDebugLabelText: "LinearShadow";
    property alias orientation: idGradient.orientation;
    property alias gradient: idShadowLeft.gradient;
    z: 1
    width: -1; // to be overwritten
    height: -1; // to be overwritten

    color: "transparent";
    border.width: visualDebug?5:0;
    border.color: visualDebug?visualDebugColor:"transparent";

    Component.onCompleted: {
        // default behaviour is shadow on top of left side of parent.
        if(width===-1)
            width = parent.width/2;
        if(height===-1)
            height= parent.height;
    }

    Rectangle
    {
        id: idShadowLeft
        anchors.fill: parent;
        gradient: Gradient {
            id: idGradient;
            GradientStop { position: 0.0; color: Qt.rgba(0.0,0.0,0.0,1.0) }
            GradientStop { position: 0.6; color: Qt.rgba(0.0,0.0,0.0,0.6) }
            GradientStop { position: 1.0; color: Qt.rgba(0.0,0.0,0.0,0.0) }
            orientation: Gradient.Horizontal;
        }
    }

    Rectangle {
        id: idDbgLabelBox
        anchors.centerIn: parent;
        width: childrenRect.width;
        height: childrenRect.height;
        color: visualDebugColor;
        opacity: 0.5;
        Text {
            id: idDbgLabel;
            visible: visualDebug;
            anchors.centerIn: parent;
            horizontalAlignment: Text.AlignHCenter;
            verticalAlignment: Text.AlignVCenter;
            text: visualDebugLabelText;
        }
    }
}
