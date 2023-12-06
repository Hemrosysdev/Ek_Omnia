import QtQuick 2.0

Item
{
    id: idRoot

    width: 72
    height: 72

    property bool active;
    property bool toggleEnabled: true

    signal toggleChanged( var active );

    Image
    {
        id: idImage

        anchors.centerIn: parent

        source: active ? "qrc:/Icons/ICON_Toggle-72px_active.png" : "qrc:/Icons/ICON_Toggle-72px_deactive.png"
    }

    MouseArea
    {
        id: idTouchArea

        anchors.fill: parent
        enabled: toggleEnabled

        onClicked:
        {
            active = !active
            toggleChanged(active);
        }
    }
}
