import QtQuick 2.12

import EkxMainstateEnum 1.0

Item
{
    id: idStandbyInfo

    anchors.fill: parent

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // items
    ///////////////////////////////////////////////////////////////////////////////////////////////

    Rectangle
    {
        id: idStandbyScreen

        anchors.fill: parent
        color: "black"
        visible: mainStatemachine.ekxMainstate === EkxMainstateEnum.EKX_STANDBY

        Text
        {
            text: "STANDBY";     // this is only visible on host simulations

            font.pixelSize: 40
            font.letterSpacing: 2
            font.bold: true
            font.family: "D-DIN Condensed HEMRO"

            color: idMain.colorEkxTextGrey

            anchors.centerIn: parent
        }
    }

    Rectangle
    {
        id: idDimmedStandbyScreen

        anchors.fill: parent
        color: "black"
        opacity: 0.7
        visible: standbyController.preStandbyDimActive
    }

    MouseArea
    {
        anchors.fill: parent

        onClicked:
        {
            standbyController.wakeUp();
        }
    }
}
