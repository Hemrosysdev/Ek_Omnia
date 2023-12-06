import QtQuick 2.12
import QtQml 2.12
import QtGraphicalEffects 1.12

Item {
    id: idRoot;

    property bool visualDebug: false;
    property real currentPercentage: 0.4;
    property real criticalHighPercentage: 1.0;
    property real criticalLowPercentage: 0.0;

    width: 200;
    height: 14;

    Rectangle
    {
        // Color defined in abstract with opacity and respect to black background
        id: idContent;
        visible: false;
        x:0;
        y:0;
        implicitWidth: parent.width;
        implicitHeight: parent.height;
        opacity: 1.0;
        color: "black"; // keep black background!

        Rectangle
        {
            // Lower Critical region background:
            // Color defined in abstract with opacity and respect to black background
            id: idCriticalLowRegion;
            x: 0;
            y: 0;
            width: parent.width * criticalLowPercentage;
            height: parent.height;
            opacity: 0.25;
            color: idMain.colorEkxConfirmRed;
        }

        Rectangle
        {
            // Higher Critical region background:
            // Color defined in abstract with opacity and respect to black background
            id: idCriticalHighRegion;

            x: parent.width - width;
            y: 0;
            width: parent.width * (1.0-criticalHighPercentage);
            height: parent.height;
            opacity: 0.25;
            color: idMain.colorEkxConfirmRed;
        }

        Rectangle
        {
            // Normal region background:
            // Color defined in abstract withopacity and respect to black background
            x: idCriticalLowRegion.x + idCriticalLowRegion.width;
            implicitWidth: parent.width - idCriticalLowRegion.width - idCriticalHighRegion.width;
            height: parent.height;
            opacity: 0.25;
            color: idMain.colorEkxConfirmGreen;
        }

        Rectangle
        {
            // Active region foreground:
            x: 0
            y: 0
            width: Math.min( parent.width, parent.width * currentPercentage )
            height: parent.height
            opacity: 1.0
            color: ( width < idCriticalLowRegion.width
                    || width > (parent.width-idCriticalHighRegion.width) )? idMain.colorEkxConfirmRed : idMain.colorEkxConfirmGreen
        }
    }

    Rectangle
    {
        id: idMaskSource;
        visible: false;
        width: idContent.width;
        height: idContent.height;
        radius: height/2;
    }

    OpacityMask
    {
        anchors.fill: idContent
        source: idContent
        maskSource: idMaskSource;
        antialiasing: true;
    }
}
