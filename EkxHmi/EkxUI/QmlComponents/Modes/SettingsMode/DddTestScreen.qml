import QtQuick 2.12
import QtQml 2.12

import "../../CommonItems"

Item
{
    id: idRoot

    width: 480
    height: 360

    function getValue( itemName )
    {
        if ( itemName === "dddHybridFiltered" )
        {
            return dddCouple.dddValue;
        }
        else if ( itemName === "dddHybridRaw" )
        {
            return dddCouple.rawDddValue;
        }
        else if ( itemName === "ddd1Filtered" )
        {
            return ( ddd1Driver.filterAngle10thDeg / 10 ).toFixed( 1 ) + "°";
        }
        else if ( itemName === "ddd1Raw" )
        {
            return ( ddd1Driver.rawAngle10thDeg / 10 ).toFixed( 1 ) + "°";
        }
        else if ( itemName === "ddd2Filtered" )
        {
            return ( ddd2Driver.filterAngle10thDeg / 10 ).toFixed( 1 ) + "°";
        }
        else if ( itemName === "ddd2Raw" )
        {
            return ( ddd2Driver.rawAngle10thDeg / 10 ).toFixed( 1 ) + "°";
        }
        else
        {
            return undefined;
        }
    }

    ListModel
    {
        id: idListModel

        ListElement
        {
            xlabel: "DDD Hybrid Filt."
            xname: "dddHybridFiltered"
        }
        ListElement
        {
            xlabel: "DDD Hybrid Raw"
            xname: "dddHybridRaw"
        }
        ListElement
        {
            xlabel: "DDD 1 Filt."
            xname: "ddd1Filtered"
        }
        ListElement
        {
            xlabel: "DDD 1 Raw"
            xname: "ddd1Raw"
        }
        ListElement
        {
            xlabel: "DDD 2 Filt."
            xname: "ddd2Filtered"
        }
        ListElement
        {
            xlabel: "DDD 2 Raw"
            xname: "ddd2Raw"
        }
    }

    ListView
    {
        id: idListView

        anchors.fill: parent
        // For a display height of 226.
        topMargin: 80
        bottomMargin: 54

        model: idListModel

        delegate: Rectangle
        {
            id: idListEntry
            width: 480
            height: 70//96
            color: "black"

            Text
            {
                id: idListEntryText
                anchors.fill: parent

                font.pixelSize: 58
                font.letterSpacing: 0.23
                font.bold: false
                font.family: "D-DIN Condensed HEMRO"

                horizontalAlignment: Text.AlignHCenter
                verticalAlignment:   Text.AlignVCenter

                color: idMain.colorEkxTextWhite
                text: xlabel
            }

            Text
            {
                anchors.right: parent.right
                anchors.rightMargin: 10
                anchors.verticalCenter: parent.verticalCenter
                width: 80
                height: 70//80

                font.pixelSize: 40
                font.letterSpacing: 2
                font.family: "D-DIN Condensed HEMRO _Mono_Numbers"
                font.bold: true

                horizontalAlignment: Text.AlignRight
                verticalAlignment: Text.AlignVCenter

                color: idMain.colorEkxTextGrey
                text: getValue( xname )
            }
        }
    }

    EkxButton
    {
        id: idButtonClose
        z:2
        buttonType: EkxButton.ButtonType.CLOSE
        onClicked:
        {
            settingsStateMachine.backToMainMenu()
        }
    }

    EkxButton
    {
        id: idButtonBack
        z: 2
        buttonType: EkxButton.ButtonType.PREVIOUS
        onClicked:
        {
            settingsStateMachine.back()
        }
    }

    ShadowGroup
    {
        z:1

        shadowTopSize: ShadowGroup.ShadowTopSizes.StandardSize
        shadowTopThirdGradientStopEnable:   true
        shadowTopThirdGradientStopPosition: 0.5
        shadowTopThirdGradientStopOpacity:  0.85

        shadowBottomSize: 75
    }
}
