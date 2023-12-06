import QtQml 2.12
import QtQuick 2.12

import ".."
import EkxMainstateEnum 1.0

ModePage
{
    id: idClassicMode

    objectName: "classic"
    title: "Classic Mode"
    stateId: EkxMainstateEnum.EKX_CLASSIC_MODE

    showDdd: false

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // items
    ///////////////////////////////////////////////////////////////////////////////////////////////

    mainContent: DddValue
    {
        id: idDddValue

        visible: !idMain.isAgsaFineAdjustmentShown()

        centerWithUnits: false
        anchors.centerIn: parent

        decimalValue: dddCouple.dddValue;

        MouseArea
        {
            anchors.fill: parent

            onClicked:
            {
                idMain.showAgsaFineAdjustment()
            }
        }
    }

    TemperatureView
    {
        id: idTemperatureView

        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
    }

}
