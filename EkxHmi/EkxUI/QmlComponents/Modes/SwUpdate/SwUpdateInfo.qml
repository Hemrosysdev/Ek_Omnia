import QtQuick 2.12

Rectangle
{
    anchors.fill: parent
    anchors.centerIn: parent

    color: "black"

    // this catcher blocks mouse events for the still active main operational views in background
    MouseArea
    {
        id: clickCatcher
        anchors.fill: parent
    }

    SwUpdateScreen
    {
    }
}
