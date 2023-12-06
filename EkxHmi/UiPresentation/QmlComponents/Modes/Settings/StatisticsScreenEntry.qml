import QtQuick 2.0
import QtQml 2.12
import "../../CommonItems"

Rectangle
{
    id: idRoot;

    property bool visualDebug: false;

    property string key: "UNDEF";
    property string value: "UNDEF\nDUMMY";
    property bool isLoading: false;
    property bool animation: false; // TODO
    property bool hasBar: false;
    property real barCurrentPercentage: 0.0;
    property real barCriticalPercentage: 0.8;
    property int iconIndex: 0;
    property string sectionTitle: "";
    property int paddingVert: 10;
    property int paddingHoriz: 16;

    width: 480;
    height: Math.max(idLabelValue.height, idKey.height, idIcon.height, (hasBar?(idBar.y+idBar.height-idKey.y):0)) + 2*idRoot.paddingVert;
    color: "black";
    //border.width: 1;
    //border.color: "orange";

    Image
    {
        id: idIcon;

        property variant portaFilterImages:
        [
            "qrc:/Icons/ICON-Strokes_Single-80px_portafilter_white.png",  // fake entry
            "qrc:/Icons/ICON-Strokes_Single-80px_portafilter_white.png",
            "qrc:/Icons/ICON-Strokes_Double-80px_portafilter_white.png",
            "qrc:/Icons/ICON-Strokes_Bottomless-80px_portafilter_white.png"
        ];

        z: 2
        anchors.left: parent.left;
        anchors.leftMargin: idRoot.paddingHoriz;
        anchors.verticalCenter: parent.verticalCenter;
        width: (idRoot.iconIndex===0)?0:implicitWidth;
        height: (idRoot.iconIndex===0)?0:implicitHeight;
        opacity: 0.5;
        source: portaFilterImages[iconIndex];
        DebugFrame { visible: idRoot.visualDebug; }
    }

    Text
    {
        id: idKey;

        anchors.left: (idRoot.iconIndex!==0)?idIcon.right:parent.left;
        anchors.leftMargin: (idRoot.iconIndex!==0)?10:idRoot.paddingHoriz;
        anchors.top: idLabelValue.top;
        opacity: 1;
        horizontalAlignment: Text.AlignLeft;
        verticalAlignment:   idRoot.hasBar?Text.AlignTop:Text.AlignVCenter;
        font.pixelSize: 36;
        font.letterSpacing: 2;
        font.bold: false;
        font.family: "D-DIN Condensed HEMRO";
        color: idMain.colorEkxTextGrey;
        text: idRoot.key;
        DebugFrame { visible: idRoot.visualDebug; }
    }

    Text
    {
        id: idLabelValue;

        visible: !isLoading;
        anchors.right: parent.right;
        anchors.rightMargin: idRoot.paddingHoriz;
        anchors.verticalCenter: parent.verticalCenter;
        opacity: 1;
        horizontalAlignment: Text.AlignRight;
        verticalAlignment:   Text.AlignVCenter;
        font.pixelSize: 36;
        font.letterSpacing: 2;
        font.bold: false;
        font.family: "D-DIN Condensed HEMRO";
        color: idMain.colorEkxTextWhite;
        text: idRoot.value;
        DebugFrame { visible: idRoot.visualDebug; }
    }

    // Rectangle
    // {
    //     id: idBar;
    //
    //     visible: idRoot.hasBar;
    //
    //     anchors.left: parent.left;
    //     anchors.leftMargin: idRoot.paddingHoriz;
    //     anchors.top: idKey.bottom;
    //     anchors.topMargin: 10;
    //     width: idRoot.hasBar?300:0;
    //     height: idRoot.hasBar?20:0;
    //
    //     color: "black";
    //
    //     Rectangle
    //     {
    //         // Color defined in abstract with opacity and respect to black background
    //         implicitWidth: parent.width;
    //         height: 20;
    //         opacity: 0.25;
    //         color: idMain.colorEkxConfirmRed;
    //     }
    //
    //     Rectangle
    //     {
    //         implicitWidth: parent.width * idRoot.barCriticalPercentage;
    //         height: 20;
    //         opacity: 1.0;
    //         color: "black";
    //     }
    //
    //     Rectangle
    //     {
    //         // Color defined in abstract withopacity and respect to black background
    //         implicitWidth: parent.width * idRoot.barCriticalPercentage;
    //         height: 20;
    //         opacity: 0.25;
    //         color: idMain.colorEkxConfirmGreen;
    //     }
    //
    //     Rectangle
    //     {
    //         implicitWidth: parent.width * idRoot.barCurrentPercentage;
    //         height: 20;
    //         opacity: 1.0;
    //         color: (idRoot.barCurrentPercentage < idRoot.barCriticalPercentage)?idMain.colorEkxConfirmGreen:idMain.colorEkxConfirmRed;
    //     }
    //
    //     //Rectangle
    //     //{
    //     //    anchors.fill: parent;
    //     //    opacity: 1.0;
    //     //    color: "transparent";
    //     //    border.width: 1;
    //     //    border.color: idMain.colorEkxTextGrey;
    //     //}
    //
    //     DebugFrame { visible: idRoot.visualDebug; }
    // }

    BarGraph
    {
        id: idBar;
        visible: idRoot.hasBar;
        anchors.left: parent.left;
        anchors.leftMargin: idRoot.paddingHoriz;
        anchors.top: idKey.bottom;
        anchors.topMargin: 10;
        width: idRoot.hasBar?350:0;
        height: idRoot.hasBar?14:0;
        currentPercentage: idRoot.barCurrentPercentage;
        criticalLowPercentage: 0.0;
        criticalHighPercentage: idRoot.barCriticalPercentage;
    }

    Rectangle
    {
        // # shade lower half (2.line) of the "value"-label
        visible: idRoot.hasBar;
        anchors.left: idLabelValue.left;
        anchors.right: idLabelValue.right;
        anchors.bottom: idLabelValue.bottom;
        height: idLabelValue.height/2;
        color: Qt.rgba(0.0, 0.0, 0.0, 0.5);
    }

    AnimatedImage
    {
        id: idLoadingIcon
        visible: isLoading
        anchors.right: parent.right;
        anchors.rightMargin: idRoot.paddingHoriz;
        anchors.verticalCenter: parent.verticalCenter;

        source: "qrc:/Icons/EKX_Loading.gif"
    }

//    LoadingIcon {
//        id: idLoadingIcon;
//        visible: isLoading;
//        anchors.right: parent.right;
//        anchors.rightMargin: idRoot.paddingHoriz;
//        anchors.verticalCenter: parent.verticalCenter;
//        radius: 4;
//        spacing: 20;

//        Timer
//        {
//            interval: 1000;
//            running: true;
//            repeat: true;

//            onTriggered:
//            {
//                idLoadingIcon.step();
//            }
//        }
//    }



}
