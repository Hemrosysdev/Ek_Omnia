import QtQuick 2.0
import QtQuick.Layouts 1.12
import "../CommonItems"

Item
{
    id: idRoot

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // signals
    ///////////////////////////////////////////////////////////////////////////////////////////////


    ///////////////////////////////////////////////////////////////////////////////////////////////
    // custom properties
    ///////////////////////////////////////////////////////////////////////////////////////////////

    property bool visualDebug: false;
    property color visualDebugColor: "blue";
    property color visualDebugColorInside: "red";
    property int preDecimalValue: 0;
    property int postDecimalValue: 0;
    property bool showUnits: false;
    property string unitString: "s";
    property int spacePreCommaPx: 8;
    property bool centerXByHallbach: false;
    property bool centerXByMass: false;
    property int pixelSize: 266
    property LongPressAnimation tupleAnimation: null

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // standard properties
    ///////////////////////////////////////////////////////////////////////////////////////////////

    width: {
        let xOffset = 0;
        if(centerXByHallbach)
            xOffset = xOffsetForHallbachCentering();
        else if(centerXByMass)
            xOffset = xOffsetForCenterOfMassCentering();
        else
            xOffset = 0;
        return(xOffset + idContent.width);
    }
    height: idContent.y + idContent.height;

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // functions
    ///////////////////////////////////////////////////////////////////////////////////////////////

    function xOffsetForHallbachCentering()
    {
        let hallbachCenterX = (idContent.width*idContent.height)/(idContent.width+idContent.height+0.0);
        return( Math.abs(hallbachCenterX-idContent.width/2) );
    }

    function xOffsetForCenterOfMassCentering()
    {
        let x1 = (idPreDecimalText.x+idPreDecimalText.width/2)
        let A1 = idPreDecimalText.width*idPreDecimalText.height;
        let x2 = (idPostDecimalText.x+idPostDecimalText.width/2)
        let A2 = idPostDecimalText.width*idPostDecimalText.height;
        let x3 = (idUnitText.x+idUnitText.width/2)
        let A3 = idUnitText.width*idUnitText.height;

        let centerOfMassX = (A1*x1+A2*x2+A3*x3)/(A1+A2+A3+0.0);
        return( Math.abs(centerOfMassX-idContent.width/2) );
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // items
    ///////////////////////////////////////////////////////////////////////////////////////////////

    Item
    {
        id: idContent;

        //width: childrenRect.width;
        //width: idPreDecimalText.contentWidth + Math.max(idPostDecimalText.contentWidth,idUnitText.contentWidth) + spacePreCommaPx;
        width: idPreDecimalText.width + Math.max(idPostDecimalText.width,idUnitText.width) + spacePreCommaPx;
        height: idPreDecimalText.contentHeight;
        //height: childrenRect.height;
        x: {
            if(centerXByHallbach)
                return(xOffsetForHallbachCentering());
            else if(centerXByMass)
                return(xOffsetForCenterOfMassCentering())
            else
                return(0);
        }

        Text
        {
            id: idPreDecimalText
            color:  idMain.colorEkxTextWhite;
            font.letterSpacing: -10
            font.pixelSize: idRoot.pixelSize // 266
            font.family: "D-DIN Condensed HEMRO _Mono_Numbers"
            horizontalAlignment: Text.AlignRight;
            text: preDecimalValue;

            antialiasing: true
            renderType: Text.NativeRendering
            layer.enabled: tupleAnimation && tupleAnimation.layerEnabled
            layer.smooth: true;
            // This item should be used as the 'mask'
            layer.samplerName: "maskSource"
            layer.effect: LongPressShaderEffect
            {
                longPressAnimation: tupleAnimation
            }
        }

        Text
        {
            id: idPostDecimalText
            anchors.left: idPreDecimalText.right;
            anchors.leftMargin: spacePreCommaPx;
            anchors.top: idPreDecimalText.top;
            anchors.topMargin: idRoot.pixelSize / 14.7 // 18
            color:  idMain.colorEkxTextWhite;
            font.pixelSize: idRoot.pixelSize / 2 // 133
            font.letterSpacing: 0.5
            font.family: "D-DIN Condensed HEMRO Light_Mono_Numbers"
            horizontalAlignment: Text.AlignLeft;
            text: "." + postDecimalValue

            renderType: Text.NativeRendering
            layer.enabled: tupleAnimation && tupleAnimation.layerEnabled
            layer.smooth: true;
            // This item should be used as the 'mask'
            layer.samplerName: "maskSource"
            layer.effect: LongPressShaderEffect
            {
                longPressAnimation: tupleAnimation
            }
        }

        Text
        {
            id: idUnitText
            visible: showUnits;
            anchors.left: idPreDecimalText.right;
            anchors.leftMargin: spacePreCommaPx;
            anchors.bottom: idPreDecimalText.bottom;
            anchors.bottomMargin: idRoot.pixelSize / 8.87 // 30
            color:  idMain.colorEkxTextGrey
            font.pixelSize: idRoot.pixelSize / 2.66 // 100
            font.family: "D-DIN Condensed HEMRO"
            horizontalAlignment: Text.AlignLeft;
            text:  unitString;
        }

        ///////////////////////////////////////////////////////////////////////////////////////////////
        // debug helpers
        ///////////////////////////////////////////////////////////////////////////////////////////////

        Rectangle
        {
            id: idDbdContent
            visible: visualDebug;
            anchors.fill: idContent;
            color: "transparent";
            border.width: 5;
            border.color: visualDebugColorInside;
        }

        Rectangle
        {
            id: idDbdPreDecimal;
            visible: visualDebug;
            anchors.fill: idPreDecimalText;
            opacity: 0.5;
            color: "transparent";
            border.width: 5;
            border.color: visualDebugColorInside;
        }

        Rectangle
        {
            id: idDbdPostDecimal;
            visible: visualDebug;
            anchors.fill: idPostDecimalText;
            opacity: 0.5;
            color: "transparent";
            border.width: 5;
            border.color: visualDebugColorInside;
        }

        Rectangle
        {
            id: idDbdUnitText;
            visible: visualDebug;
            anchors.fill: idUnitText;
            opacity: 0.5;
            color: "transparent";
            border.width: 5;
            border.color: visualDebugColorInside;
        }

        Rectangle
        {
            id: idDbdHCenterLine;
            visible: visualDebug;
            anchors.verticalCenter: parent.verticalCenter;
            anchors.left: parent.left;
            anchors.right: parent.right;
            height: 2;
            opacity: 0.5;
            color: "transparent";
            border.width: 5;
            border.color: visualDebugColorInside;
        }

        Rectangle
        {
            id: idDbdVCenterLine;
            visible: visualDebug;
            anchors.horizontalCenter: parent.horizontalCenter;
            anchors.top: parent.top;
            anchors.bottom: parent.bottom;
            width: 2;
            opacity: 0.5;
            color: "transparent";
            border.width: 5;
            border.color: visualDebugColorInside;
        }
    }

    Rectangle
    {
        id: idDbdRoot
        visible: visualDebug;
        anchors.fill: idRoot;
        color: "transparent";
        border.width: 5;
        border.color: visualDebugColorInside;
    }

}
