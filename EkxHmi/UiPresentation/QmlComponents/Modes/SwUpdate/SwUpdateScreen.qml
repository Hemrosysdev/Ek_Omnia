import QtQuick 2.12
import QtQml 2.12
import "../../CommonItems"

Item
{
    id: idRoot

    anchors.centerIn: parent

    width: parent.width
    height: idMainItem.height + idAcknowlegde.height + idProgressBar.height

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // self defined properties
    ///////////////////////////////////////////////////////////////////////////////////////////////

    property string actionInfoHeadline: ekxSwUpdateController.swUpdateHeadline
    property string actionInfoText: ekxSwUpdateController.swUpdateInfo
    property bool   showProgressBar: ekxSwUpdateController.showProgressBar
    property int    progressValue: ekxSwUpdateController.progressValue
    property int    progressBarAutoTimer: ekxSwUpdateController.progressBarAutoTimer

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // signal handlers
    ///////////////////////////////////////////////////////////////////////////////////////////////

    onProgressBarAutoTimerChanged:
    {
        idProgressAutoTimer.value = 0
        idProgressAutoTimer.start()
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // items
    ///////////////////////////////////////////////////////////////////////////////////////////////

    Timer
    {
        id: idProgressAutoTimer

        interval: 100
        repeat: false

        property int maxTimeSec: ekxSwUpdateController.progressBarAutoTimer
        property double inc
        property double value

        onMaxTimeSecChanged:
        {
            value = 0
            inc = 100.0 / ( maxTimeSec * 1000.0 / interval )
        }

        onTriggered:
        {
            value += inc
            progressValue = value

            if ( progressValue >= 100 )
            {
                idProgressAutoTimer.stop()
            }
            else
            {
                idProgressAutoTimer.start()
            }
        }
    }

    Item
    {
        id: idMainItem

        anchors.top: parent.top
        anchors.horizontalCenter: parent.horizontalCenter

        width: parent.width
        height: idMainText.height

        Text
        {
            id: idMainText

            width: parent.width

            text: ( actionInfoHeadline !== "" ? actionInfoHeadline + "\n" : "" ) + actionInfoText;
            horizontalAlignment: Text.AlignHCenter

            anchors.margins: 10
            anchors.centerIn: parent

            font.pixelSize: 58
            font.letterSpacing: 2.1
            font.bold: true
            //font.family: "D-DIN Condensed HEMRO"
            font.family: "D-DIN Condensed HEMRO"

            color: idMain.colorEkxTextWhite

            wrapMode: Text.WordWrap
        }
    }

     Item
     {
         id: idProgressBarElement

         anchors.horizontalCenter: parent.horizontalCenter
         anchors.top: idMainItem.bottom

         width: idProgressBar.width
         height: showProgressBar ? ( idProgressBar.height + idSpacer.height ) : 0

         visible: showProgressBar

         Item
         {
             id: idSpacer

             anchors.top: parent.top
             height: 20
         }

         BarGraph
         {
             id: idProgressBar

             visible: showProgressBar

             anchors.horizontalCenter: parent.horizontalCenter
             anchors.top: idSpacer.bottom

             width: 350
             height: 14

             currentPercentage: progressValue * 0.01
             criticalLowPercentage: 0.0
             criticalHighPercentage: 1.0
         }
     }

    Item
    {
        id: idAcknowlegde

        anchors.horizontalCenter: parent.horizontalCenter
        // anchors.top: idMainItem.bottom
        anchors.bottom: parent.bottom;

        width: parent.width
        height: ekxSwUpdateController.showUpdateCompScreen ? idAcknowlegdeText.height : 0

        visible: ekxSwUpdateController.showUpdateCompScreen

        Text
        {
            id: idAcknowlegdeText

            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: parent.top

            height: 50
            width: parent.width

            text: "Acknowlegde";
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignBottom

            font.pixelSize: 30
            font.letterSpacing: 1.48
            font.family: "D-DIN Condensed HEMRO"

            color: idMain.colorEkxConfirmGreen
        }

        MouseArea
        {
            id: idAcknowlegdeBtn

            z: 1
            anchors.centerIn: idAcknowlegdeText

            width: idAcknowlegdeText.paintedWidth + 200
            height: idAcknowlegdeText.paintedHeight + 100

            onClicked:
            {
                ekxSwUpdateController.userAcknowlegeCall();
            }
        }
    }
}
