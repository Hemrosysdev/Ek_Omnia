import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQml.Models 2.1

import "../../CommonItems"

Item
{
    width: 480
    height: 360

    property int entryType;

    property string nameOfGrinder:    settingsSerializer.grinderName;
    property string nameOfModell:     "Hemro Ekx";
    property string nameOfConfig:     "HEM-EKX-K02";

    property int generalPixelSize: 33;
    property int generalRectHeight: generalPixelSize + 10;
    property int generalMargin: 10

    function save()
    {
        // do nothing
    }

    ShadowGroup
    {
        z: 1;
        shadowTopSize: ShadowGroup.ShadowTopSizes.StandardSize;
        shadowBottomSize: ShadowGroup.ShadowBottomSizes.StandardSize;
    }

    ObjectModel
    {
         id: modes

         Item
         {
             id: name

             width: 460
             height: generalRectHeight

             anchors.left: parent.left
             anchors.leftMargin: generalMargin

             Text
             {
                 z:2

                 property bool pressInAction: false
                 text: "Smiley Name:";

                 font.pixelSize: generalPixelSize
                 font.letterSpacing: 0.23
                 font.family: "D-DIN Condensed HEMRO"

                 color: idMain.colorEkxTextGrey

                 anchors.left: parent.left
             }

             Text
             {
                 z:2

                 text: nameOfGrinder;

                 font.pixelSize: generalPixelSize
                 font.letterSpacing: 0.23
                 font.family: "D-DIN Condensed HEMRO"

                 color: idMain.colorEkxTextWhite

                 anchors.right: parent.right
             }
         }

         Rectangle
         {
             id: spacer1
             width: 460
             height: 2
             color: idMain.colorEkxTextGrey

             anchors.left: name.left
             anchors.top: name.bottom
             anchors.topMargin: 5
         }

         Item
         {
             id: idPackageInfo

             width: 460
             height: generalRectHeight * 2

             anchors.left: parent.left
             anchors.leftMargin: generalMargin

             anchors.top: spacer1.bottom
             anchors.topMargin: 15

             Text
             {
                 z:2

                 text: "Package SW Version:";

                 font.pixelSize: generalPixelSize
                 font.letterSpacing: 0.23
                 font.family: "D-DIN Condensed HEMRO"

                 color: idMain.colorEkxTextGrey

                 anchors.left: parent.left
             }

             Text
             {
                 z:2

                 text: deviceInfoCollector.packageSwVersion;

                 font.pixelSize: generalPixelSize
                 font.letterSpacing: 0.23
                 font.family: "D-DIN Condensed HEMRO"

                 color: idMain.colorEkxTextWhite

                 anchors.right: parent.right
             }

             Text
             {
                 z:2

                 text: "Package Build No: ";

                 font.pixelSize: generalPixelSize
                 font.letterSpacing: 0.23
                 font.family: "D-DIN Condensed HEMRO"

                 color: idMain.colorEkxTextGrey

                 anchors.left: parent.left
                 anchors.top: parent.top
                 anchors.topMargin: generalRectHeight
             }

             Text
             {
                 z:2

                 text: deviceInfoCollector.packageBuildNo;

                 font.pixelSize: generalPixelSize
                 font.letterSpacing: 0.23
                 font.family: "D-DIN Condensed HEMRO"

                 color: idMain.colorEkxTextWhite

                 anchors.right: parent.right
                 anchors.top: parent.top
                 anchors.topMargin: generalRectHeight
             }

         }

         Rectangle
         {
             id: spacer2
             width: 460
             height: 2
             color: idMain.colorEkxTextGrey
             anchors.left: idPackageInfo.left
             anchors.top: idPackageInfo.bottom
             anchors.topMargin: 5
         }

         Item
         {
             id: idHmiInfo

             width: 460
             height: generalRectHeight * 4

             anchors.left: parent.left
             anchors.leftMargin: generalMargin

             anchors.top: spacer2.bottom
             anchors.topMargin: 15

             Text
             {
                 z:2

                 text: "HMI HW ID:";

                 font.pixelSize: generalPixelSize
                 font.letterSpacing: 0.23
                 font.family: "D-DIN Condensed HEMRO"

                 color: idMain.colorEkxTextGrey

                 anchors.left: parent.left
             }

             Text
             {
                 z:2

                 text: deviceInfoCollector.ekxUiHwVersion

                 font.pixelSize: generalPixelSize
                 font.letterSpacing: 0.23
                 font.family: "D-DIN Condensed HEMRO"

                 color: idMain.colorEkxTextWhite

                 anchors.right: parent.right
             }

             Text
             {
                 z:2

                 text: "HMI Serial No: ";

                 font.pixelSize: generalPixelSize
                 font.letterSpacing: 0.23
                 font.family: "D-DIN Condensed HEMRO"

                 color: idMain.colorEkxTextGrey

                 anchors.left: parent.left
                 anchors.top: parent.top
                 anchors.topMargin: generalRectHeight
             }

             Text
             {
                 z:2

                 text: deviceInfoCollector.ekxUiSerialNo

                 font.pixelSize: generalPixelSize
                 font.letterSpacing: 0.23
                 font.family: "D-DIN Condensed HEMRO"

                 color: idMain.colorEkxTextWhite

                 anchors.right: parent.right
                 anchors.top: parent.top
                 anchors.topMargin: generalRectHeight
             }

             Text
             {
                 z:2

                 text: "HMI SW Version: ";

                 font.pixelSize: generalPixelSize
                 font.letterSpacing: 0.23
                 font.family: "D-DIN Condensed HEMRO"

                 color: idMain.colorEkxTextGrey

                 anchors.left: parent.left
                 anchors.top: parent.top
                 anchors.topMargin: generalRectHeight * 2
             }

             Text
             {
                 z:2

                 text: deviceInfoCollector.ekxUiSwVersion

                 font.pixelSize: generalPixelSize
                 font.letterSpacing: 0.23
                 font.family: "D-DIN Condensed HEMRO"

                 color: idMain.colorEkxTextWhite

                 anchors.right: parent.right
                 anchors.top: parent.top
                 anchors.topMargin: generalRectHeight * 2
             }

             Text
             {
                 z:2

                 text: "HMI EHC Version: ";

                 font.pixelSize: generalPixelSize
                 font.letterSpacing: 0.23
                 font.family: "D-DIN Condensed HEMRO"

                 color: idMain.colorEkxTextGrey

                 anchors.left: parent.left
                 anchors.top: parent.top
                 anchors.topMargin: generalRectHeight * 3
             }

             Text
             {
                 z:2

                 text: deviceInfoCollector.espHostConSwVersion

                 font.pixelSize: generalPixelSize
                 font.letterSpacing: 0.23
                 font.family: "D-DIN Condensed HEMRO"

                 color: idMain.colorEkxTextWhite

                 anchors.right: parent.right
                 anchors.top: parent.top
                 anchors.topMargin: generalRectHeight * 3
             }

         }

         Rectangle
         {
             id: idSpacer3
             width: 460
             height: 2
             color: idMain.colorEkxTextGrey
             anchors.left: idHmiInfo.left
             anchors.top: idHmiInfo.bottom
             anchors.topMargin: 5
         }

         Item
         {
             id: idEspInfo

             width: 460
             height: generalRectHeight * 3

             anchors.left: idSpacer3.left

             anchors.top: idSpacer3.bottom
             anchors.topMargin: 15

             Text
             {
                 z:2

                 text: "ESP HW ID:";

                 font.pixelSize: generalPixelSize
                 font.letterSpacing: 0.23
                 font.family: "D-DIN Condensed HEMRO"

                 color: idMain.colorEkxTextGrey

                 anchors.left: parent.left
             }

             Text
             {
                 z:2

                 text: deviceInfoCollector.espProdId

                 font.pixelSize: generalPixelSize
                 font.letterSpacing: 0.23
                 font.family: "D-DIN Condensed HEMRO"

                 color: idMain.colorEkxTextWhite

                 anchors.right: parent.right
             }

             Text
             {
                 z:2

                 text: "ESP Serial No: ";

                 font.pixelSize: generalPixelSize
                 font.letterSpacing: 0.23
                 font.family: "D-DIN Condensed HEMRO"

                 color: idMain.colorEkxTextGrey

                 anchors.left: parent.left
                 anchors.top: parent.top
                 anchors.topMargin: generalRectHeight
             }

             Text
             {
                 z:2

                 text: deviceInfoCollector.espSerialNo

                 font.pixelSize: generalPixelSize
                 font.letterSpacing: 0.23
                 font.family: "D-DIN Condensed HEMRO"

                 color: idMain.colorEkxTextWhite

                 anchors.right: parent.right
                 anchors.top: parent.top
                 anchors.topMargin: generalRectHeight
             }

             Text
             {
                 z:2

                 text: "ESP SW Version: ";

                 font.pixelSize: generalPixelSize
                 font.letterSpacing: 0.23
                 font.family: "D-DIN Condensed HEMRO"

                 color: idMain.colorEkxTextGrey

                 anchors.left: parent.left
                 anchors.top: parent.top
                 anchors.topMargin: generalRectHeight * 2
             }

             Text
             {
                 z:2

                 text: deviceInfoCollector.espSwVersion

                 font.pixelSize: generalPixelSize
                 font.letterSpacing: 0.23
                 font.family: "D-DIN Condensed HEMRO"

                 color: idMain.colorEkxTextWhite

                 anchors.right: parent.right
                 anchors.top: parent.top
                 anchors.topMargin: generalRectHeight * 2
             }

         }

         Rectangle
         {
             id: idSpacer4
             width: 460
             height: 2
             color: idMain.colorEkxTextGrey

             anchors.left: idEspInfo.left
             anchors.top: idEspInfo.bottom
             anchors.topMargin: 5
         }

         Item
         {
             id: idMcuInfo

             width: 460
             height: generalRectHeight * 4

             anchors.left: idSpacer4.left

             anchors.top: idSpacer4.bottom
             anchors.topMargin: 15

             Text
             {
                 z:2

                 text: "MCU HW ID:";

                 font.pixelSize: generalPixelSize
                 font.letterSpacing: 0.23
                 font.family: "D-DIN Condensed HEMRO"

                 color: idMain.colorEkxTextGrey

                 anchors.left: parent.left
             }

             Text
             {
                 z:2

                 text: deviceInfoCollector.mcuProdId

                 font.pixelSize: generalPixelSize
                 font.letterSpacing: 0.23
                 font.family: "D-DIN Condensed HEMRO"

                 color: idMain.colorEkxTextWhite

                 anchors.right: parent.right
             }

             Text
             {
                 z:2

                 text: "MCU HW Version:";

                 font.pixelSize: generalPixelSize
                 font.letterSpacing: 0.23
                 font.family: "D-DIN Condensed HEMRO"

                 color: idMain.colorEkxTextGrey

                 anchors.left: parent.left
                 anchors.top: parent.top
                 anchors.topMargin: generalRectHeight
             }

             Text
             {
                 z:2

                 text: deviceInfoCollector.mcuHwVersion

                 font.pixelSize: generalPixelSize
                 font.letterSpacing: 0.23
                 font.family: "D-DIN Condensed HEMRO"

                 color: idMain.colorEkxTextWhite

                 anchors.right: parent.right
                 anchors.top: parent.top
                 anchors.topMargin: generalRectHeight
             }

             Text
             {
                 z:2

                 text: "MCU Serial No: ";

                 font.pixelSize: generalPixelSize
                 font.letterSpacing: 0.23
                 font.family: "D-DIN Condensed HEMRO"

                 color: idMain.colorEkxTextGrey

                 anchors.left: parent.left
                 anchors.top: parent.top
                 anchors.topMargin: generalRectHeight * 2
             }

             Text
             {
                 z:2

                 text: deviceInfoCollector.mcuSerialNo

                 font.pixelSize: generalPixelSize
                 font.letterSpacing: 0.23
                 font.family: "D-DIN Condensed HEMRO"

                 color: idMain.colorEkxTextWhite

                 anchors.right: parent.right
                 anchors.top: parent.top
                 anchors.topMargin: generalRectHeight * 2
             }

             Text
             {
                 z:2

                 text: "MCU SW Version: ";

                 font.pixelSize: generalPixelSize
                 font.letterSpacing: 0.23
                 font.family: "D-DIN Condensed HEMRO"

                 color: idMain.colorEkxTextGrey

                 anchors.left: parent.left
                 anchors.top: parent.top
                 anchors.topMargin: generalRectHeight * 3
             }

             Text
             {
                 z:2

                 text: deviceInfoCollector.mcuSwVersion

                 font.pixelSize: generalPixelSize
                 font.letterSpacing: 0.23
                 font.family: "D-DIN Condensed HEMRO"

                 color: idMain.colorEkxTextWhite

                 anchors.right: parent.right
                 anchors.top: parent.top
                 anchors.topMargin: generalRectHeight * 3
             }

         }
     }


    ListView
    {
        id: machineInfoList

        cacheBuffer: 460

        width: 480
        height: 200

        anchors.top: parent.top
        anchors.topMargin: 80

        boundsMovement: Flickable.StopAtBounds
        flickableDirection: Flickable.VerticalFlick
        interactive: true
        clip: false

        ScrollBar.vertical: ScrollBar
        {
        }

        model: modes
    }
}
