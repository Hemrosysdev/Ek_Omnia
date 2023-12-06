import QtQuick 2.0
import QtQuick.Layouts 1.12
import "../CommonItems"

Rectangle
{
    id: idRoot

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // signals
    ///////////////////////////////////////////////////////////////////////////////////////////////


    ///////////////////////////////////////////////////////////////////////////////////////////////
    // custom properties
    ///////////////////////////////////////////////////////////////////////////////////////////////

    property bool centerWithUnits: true
    property bool showUnit: true
    property bool visualDebug: false
    property color visualDebugColor: "blue"
    property color visualDebugColorInside: "red"
    property int decimalValue: 0
    property string unitString: "um"
//    property bool centerXByHallbach: false
//    property bool centerXByMass: false
    property LongPressAnimation dddAnimation: null
    property int pixelSize: 200

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // standard properties
    ///////////////////////////////////////////////////////////////////////////////////////////////

    width: idContent.width
//    {
//        let xOffset = 0
//        if (centerXByHallbach)
//            xOffset = xOffsetForHallbachCentering()
//        else if (centerXByMass)
//            xOffset = xOffsetForCenterOfMassCentering()
//        else
//            xOffset = 0
//        return(xOffset + idContent.width)
//    }
    height: idContent.y + idContent.height

    border.width: visualDebug ? 6 : 0
    border.color: visualDebug ? visualDebugColor : "transparent"
    color: "transparent"
    //clip: true

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // functions
    ///////////////////////////////////////////////////////////////////////////////////////////////

//    function xOffsetForHallbachCentering()
//    {
//        let hallbachCenterX = (idContent.width*idContent.height)/(idContent.width+idContent.height)
//        return( Math.abs(hallbachCenterX-idContent.width/2) )
//    }

//    function xOffsetForCenterOfMassCentering()
//    {
//                let x1 = (idPreDecimalText.x+idPreDecimalText.width/2)
//                let A1 = idPreDecimalText.width*idPreDecimalText.height
//                let x2 = 0//(idPostDecimalText.x+idPostDecimalText.width/2)
//                let A2 = 0//idPostDecimalText.width*idPostDecimalText.height
//                let x3 = (idUnitText.x+idUnitText.width/2)
//                let A3 = idUnitText.width*idUnitText.height

//                let centerOfMassX = (A1*x1+A2*x2+A3*x3)/(A1+A2+A3)
//        return( Math.abs(centerOfMassX-idContent.width/2) )
//    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // items
    ///////////////////////////////////////////////////////////////////////////////////////////////

    Rectangle
    {
        id: idContent

        width: idPreDecimalText.width + ( showUnit ? idUnitImage.width : 0 ) + idSpacer.width // ( centerWithUnits ? idUnitImage.width : 0 )
        height: idPreDecimalText.contentHeight

//        x:
//        {
//            if(centerXByHallbach)
//                return(xOffsetForHallbachCentering())
//            else if(centerXByMass)
//                return(xOffsetForCenterOfMassCentering())
//            else
//                return(0)
//        }
        border.width: visualDebug ? 5 : 0
        border.color: visualDebug ? visualDebugColorInside : "transparent"
        color: "transparent"

        Item
        {
            id: idSpacer
            height: parent.height
            width: ( ( centerWithUnits || !showUnit ) ? 0 : idUnitImage.width )
        }

//        Rectangle
//        {
//            anchors.fill: idPreDecimalText
//            color: "transparent"
//            border.width: 1
//            border.color: "blue"
//        }

        Text
        {
            id: idPreDecimalText

            text: decimalValue

            anchors.left: idSpacer.right

            color:  idMain.colorEkxTextWhite
            horizontalAlignment: Text.AlignRight

            font.letterSpacing: -10
            font.pixelSize: pixelSize
            font.family: "D-DIN Condensed HEMRO _Mono_Numbers"

            antialiasing: true
            renderType: Text.NativeRendering
            layer.enabled: idRoot.dddAnimation ? idRoot.dddAnimation.layerEnabled : false
            layer.smooth: true
            // This item should be used as the 'mask'
            layer.samplerName: "maskSource"
            layer.effect: LongPressShaderEffect
            {
                longPressAnimation: idRoot.dddAnimation
            }
        }

        Image
        {
            id: idUnitImage
            visible: showUnit
            anchors.left: idPreDecimalText.right
            anchors.leftMargin: 0
            anchors.bottom: idPreDecimalText.bottom
            anchors.bottomMargin: 5

            source: "qrc:/Icons/um.png"
        }

        ///////////////////////////////////////////////////////////////////////////////////////////////
        // debug helpers
        ///////////////////////////////////////////////////////////////////////////////////////////////

        Rectangle
        {
            id: idDbdPreDecimal
            visible: visualDebug
            anchors.fill: idPreDecimalText
            opacity: 0.5
            color: "transparent"
            border.width: 5
            border.color: visualDebugColorInside
        }

        Rectangle
        {
            id: idDbdHCenterLine
            visible: visualDebug
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: parent.left
            anchors.right: parent.right
            height: 2
            opacity: 0.5
            color: "transparent"
            border.width: 5
            border.color: visualDebugColorInside
        }

        Rectangle
        {
            id: idDbdVCenterLine
            visible: visualDebug
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            width: 2
            opacity: 0.5
            color: "transparent"
            border.width: 5
            border.color: visualDebugColorInside
        }
    }
}
