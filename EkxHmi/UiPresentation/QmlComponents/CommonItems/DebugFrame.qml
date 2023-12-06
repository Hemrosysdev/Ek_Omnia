import QtQuick 2.0

Rectangle {
    id: idDebugFrame;

    //property color visualDebugColor: "blue";
    property color visualDebugColor: Qt.rgba(Math.random(),Math.random(),Math.random(),1.0);
    property int visualDebugStroke: 5;
    property Item target: parent;

    visible: visualDebug;
    anchors.fill: target;
    opacity: 1;
    color: "transparent";
    border.width: visualDebugStroke;
    border.color: visualDebugColor;
}
