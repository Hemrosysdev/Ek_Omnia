import QtQml 2.12
import QtQuick 2.12
import "../../CommonItems"

Rectangle
{
    id: idRoot

    width:  480
    height: 360
    color:  "black"

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // self defined properties
    ///////////////////////////////////////////////////////////////////////////////////////////////

    property var portaFilterIcons:
        [
        "qrc:/Icons/ICON_Single-80px_pf_transparent.png",
        "qrc:/Icons/ICON_Double-80px_pf_transparent.png",
        "qrc:/Icons/ICON_Bottomless-80px_pf_transparent.png"
    ]

    property int recipeIndex;

    property int startRecipeBig: 0;
    property int startRecipeComma: 0;

    property int startDddBig: 0;
    property int startDddComma: 0;

    property int startPortaFilter: 0

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // states
    ///////////////////////////////////////////////////////////////////////////////////////////////

    states: [
        State
        {
            name: "editGrindTime"
            PropertyChanges { target: idLeftArrowIcon; visible: false }
            PropertyChanges { target: idRightArrowIcon; visible: true }
            PropertyChanges { target: idUnitSecText; visible: true }
            PropertyChanges { target: idEditTypeIcon; source: 'qrc:/Icons/ICON_Timer-56px_white 50%.png';  visible: true}
            PropertyChanges { target: idShowCoarsenessToggle; visible:false }
            PropertyChanges { target: idShowPortaFilterToggle; visible:false }
            PropertyChanges { target: idTrashIcon; visible: recipeControl.currentTimeRecipeCount > 1 }
            PropertyChanges { target: idAcceptIcon; visible:false }
            PropertyChanges { target: idPortaFilterTypeChooser; visible:false }
            PropertyChanges { target: idPreDecimalValue; visible:true }
            PropertyChanges { target: idPostDecimalValue; visible:true }
            PropertyChanges { target: idDecimalPoint; visible:true }
        },
        State
        {
            name: "editCoarseness"
            PropertyChanges { target: idLeftArrowIcon; visible: true }
            PropertyChanges { target: idRightArrowIcon; visible: true }
            PropertyChanges { target: idUnitSecText; visible: false }
            PropertyChanges { target: idEditTypeIcon; source: 'qrc:/Icons/ICON_DiscDistance-72px_white 50%.png'; visible: true }
            PropertyChanges { target: idShowCoarsenessToggle; visible:true }
            PropertyChanges { target: idShowPortaFilterToggle; visible:false }
            PropertyChanges { target: idTrashIcon; visible:false }
            PropertyChanges { target: idAcceptIcon; visible:false }
            PropertyChanges { target: idPortaFilterTypeChooser; visible:false }
            PropertyChanges { target: idPreDecimalValue; visible:true }
            PropertyChanges { target: idPostDecimalValue; visible:true }
            PropertyChanges { target: idDecimalPoint; visible:true }
        },
        State
        {
            name: "editPortaFilter"
            PropertyChanges { target: idLeftArrowIcon; visible: true }
            PropertyChanges { target: idRightArrowIcon; visible: false }
            PropertyChanges { target: idUnitSecText; visible: false }
            PropertyChanges { target: idEditTypeIcon; source: 'qrc:/Icons/ICON_DiscDistance-72px_white 50%.png'; visible: false }
            PropertyChanges { target: idShowCoarsenessToggle; visible:false }
            PropertyChanges { target: idShowPortaFilterToggle; visible:true }
            PropertyChanges { target: idTrashIcon; visible:false }
            PropertyChanges { target: idAcceptIcon; visible:true }
            PropertyChanges { target: idPortaFilterTypeChooser; visible:true }
            PropertyChanges { target: idPreDecimalValue; visible:false }
            PropertyChanges { target: idPostDecimalValue; visible:false }
            PropertyChanges { target: idDecimalPoint; visible:false }
        }
    ]

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // signals
    ///////////////////////////////////////////////////////////////////////////////////////////////

    signal closeAngleView( var recipeIndex );

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // functions
    ///////////////////////////////////////////////////////////////////////////////////////////////

    function setRecipeStartValues(big, comma, portaFilter)
    {
        startRecipeBig = big;
        startRecipeComma = comma;
        startPortaFilter = portaFilter;

        idPreDecimalValue.setStartValues(big);
        idPostDecimalValue.setStartValues(comma);
    }

    function setRecipeIndex( currentRecipeIndex )
    {
        recipeIndex = currentRecipeIndex;
    }

    function setDddStartValues(big, comma)
    {
        startDddBig = big;
        startDddComma = comma;
    }

    function handleSaveChanges()
    {
        if( startDddBig === (recipeControl.getMaxDddValue() - 1) )
        {
            startDddComma = 0;
        }

        recipeControl.editTimeRecipe( startRecipeBig,
                                     startRecipeComma,
                                     startDddBig,
                                     startDddComma,
                                     startPortaFilter,
                                     recipeList[recipeControl.currentTimeRecipeIndex].recipeIconOn,
                                     recipeList[recipeControl.currentTimeRecipeIndex].showCoarseness,
                                     recipeIndex )
        idRoot.state = "editGrindTime";
        idRoot.closeAngleView( recipeIndex );
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // signal handlers
    ///////////////////////////////////////////////////////////////////////////////////////////////

    Component.onCompleted:
    {
        idRoot.state = "editGrindTime";
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // items
    ///////////////////////////////////////////////////////////////////////////////////////////////

    ShadowGroup
    {
        z: 1;
        shadowTopSize: ShadowGroup.ShadowTopSizes.StandardSize;
        shadowBottomSize: ShadowGroup.ShadowBottomSizes.StandardSize;
        //visualDebug: true
    }

    Item
    {
        id: idEditTypeIcon

        z:4
        width: 72
        height: 72

        anchors.left: parent.left
        anchors.leftMargin: 46

        anchors.top: parent.top
        anchors.topMargin: 144

        property alias source: idEditTypeIconImage.source

        Image
        {
            id: idEditTypeIconImage
            anchors.fill: parent
        }
    }

    NumberWheel
    {
        id: idPreDecimalValue

        anchors.left: parent.left
        anchors.leftMargin: 114

        height: parent.height

        currentFontFamiliy: "D-DIN Condensed HEMRO"

        model: recipeControl.getMaxDddValue();   // TODO wrong max value
        interactive: true

        smallValue: false

        longPressAnimationInt: idSavingAnimation

        onCurrentValue:
        {
            if( idRoot.state === "editGrindTime" )
            {
                startRecipeBig = value;
            }
            if( idRoot.state === "editCoarseness" )
            {
                startDddBig = value;
            }
        }
    }

    Item
    {
        id: idDecimalPoint

        width: 32
        height: 360

        anchors.left: parent.left
        anchors.leftMargin: 254

        Text
        {
            id: idDecimalPointText

            anchors.horizontalCenter: parent.horizontalCenter
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 90

            text:  "."
            color: idMain.colorEkxTextWhite

            font.pixelSize: 174
            font.family: "D-DIN Condensed HEMRO Light_Numbers"

            layer.enabled: true
            layer.smooth: true

            // This item should be used as the 'mask'
            layer.samplerName: "maskSource"
            layer.effect: LongPressShaderEffect
            {
                longPressAnimation: idDecimalPointText.longPressAnimation
            }

            property LongPressAnimation longPressAnimation
        }
    }

    NumberWheel
    {
        id: idPostDecimalValue

        anchors.left: parent.left
        anchors.leftMargin: 270
        height: parent.height

        informAboutChange: true

        currentFontFamiliy: "D-DIN Condensed HEMRO Light_Mono_Numbers"

        itemWidth: 120

        model: 10
        interactive: true

        longPressAnimationInt: idSavingAnimation

        onCurrentValue:
        {
            if( idRoot.state === "editGrindTime" )
            {
                startRecipeComma = value;
            }

            if( idRoot.state === "editCoarseness" )
            {
                startDddComma = value;
            }
        }

        onUpChange:
        {
            idPreDecimalValue.incrementCurrentIndex();
        }

        onDownChange:
        {
            idPreDecimalValue.decrementCurrentIndex();
        }
    }

    Text
    {
        id: idUnitSecText
        objectName: "idUnitSecText"

        anchors.left: idPostDecimalValue.right
        anchors.leftMargin: -13

        anchors.top: parent.top
        anchors.topMargin: 175

        text:  "s"
        color: idMain.colorEkxTextWhite

        font.pixelSize: 80
        font.family:    "D-DIN Condensed HEMRO"

        layer.enabled: true
        layer.smooth: true

        // This item should be used as the 'mask'
        layer.samplerName: "maskSource"
        layer.effect: LongPressShaderEffect
        {
            longPressAnimation: idUnitSecText.longPressAnimation
        }

        property LongPressAnimation longPressAnimation
    }

    Column
    {
        id: idPortaFilterTypeChooser

        z:4
        anchors.left: parent.left
        anchors.leftMargin: 47
        anchors.top: parent.top
        anchors.topMargin: 44
        spacing: 16

        Repeater
        {
            model: portaFilterIcons.length

            Item
            {
                height: 80
                width: 409

                Rectangle
                {
                    anchors.fill: idPortaFilterIcon
                    anchors.centerIn: idPortaFilterIcon
                    color: startPortaFilter === index ? idMain.colorEkxTextWhite : idMain.colorEkxTextGrey
                }

                Image
                {
                    id: idPortaFilterIcon

                    width: 80
                    height: 80
                    source: portaFilterIcons[index];
                    anchors.centerIn: parent

                    MouseArea
                    {
                        anchors.fill: parent

                        onClicked:
                        {
                            startPortaFilter = index;
                        }
                    }
                }
            }
        }
    }

    ActivateToggle
    {
        id: idShowCoarsenessToggle

        visible:false
        active: true
        width: 72
        height: 72
        z:4
        anchors.right: parent.right
        anchors.top: parent.top

        onToggleChanged:
        {
            recipeList[recipeIndex].showCoarseness = active;
            active = recipeList[recipeIndex].showCoarseness;
        }

        onVisibleChanged:
        {
            active = recipeList[recipeIndex].showCoarseness;
        }
    }

    ActivateToggle
    {
        id: idShowPortaFilterToggle

        visible:false
        width: 72
        height: 72
        z:4
        anchors.right: parent.right
        anchors.top: parent.top

        onToggleChanged:
        {
            recipeList[recipeIndex].recipeIconOn = active;
            active = recipeList[recipeIndex].recipeIconOn;
        }

        onVisibleChanged:
        {
            active = recipeList[recipeIndex].recipeIconOn;
        }
    }

    EkxButton
    {
        // id: idCloseIcon;
        z: 4
        buttonType: EkxButton.ButtonType.CLOSE;
        onClicked:
        {
            idRoot.state = "editGrindTime";
            idRoot.closeAngleView( recipeIndex );
        }
    }

    // trash icon right/top
    Image
    {
        id: idTrashIcon

        z:4
        width: 72
        height: 72

        anchors.right: parent.right
        anchors.top: parent.top

        source: "qrc:/Icons/ICON_Trashcan-72px_white.png"

        layer.enabled: true
        layer.smooth: true

        // This item should be used as the 'mask'
        layer.samplerName: "maskSource"
        layer.effect: LongPressShaderEffect
        {
            longPressAnimation: idDeleteAnimation
        }

        LongPressAnimation
        {
            id: idDeleteAnimation

            anchors.fill: parent

            fillColor: idMain.colorEkxConfirmRed

            onStarted:
            {
                idUnitSecText.longPressAnimation = idDeleteAnimation
                idPreDecimalValue.longPressAnimationInt = idDeleteAnimation
                idPostDecimalValue.longPressAnimationInt = idDeleteAnimation
                idDecimalPointText.longPressAnimation = idDeleteAnimation
            }

            onLongPressed:
            {
                recipeControl.deleteTimeRecipe( recipeIndex );

                idRoot.state = "editGrindTime";
                idRoot.closeAngleView(0);
            }

            onSimplePressed:
            {
                // do nothing
            }
        }
    }

    EkxButton
    {
        id: idAcceptIcon;
        z:4
        buttonType: EkxButton.ButtonType.OK;
        onClicked:
        {
            handleSaveChanges();
        }
    }

    EkxButton
    {
        id: idLeftArrowIcon;
        z:4
        buttonType: EkxButton.ButtonType.PREVIOUS;
        onClicked:
        {
            if ( idRoot.state === "editPortaFilter" )
            {
                idRoot.state = "editCoarseness";
                idPreDecimalValue.setStartValues( startDddBig );
                idPostDecimalValue.setStartValues( startDddComma );
            }
            else
            {
                idRoot.state = "editGrindTime";
                idPreDecimalValue.setStartValues( startRecipeBig );
                idPostDecimalValue.setStartValues( startRecipeComma );
            }
        }
    }

    // right arrow for navigation and saving bottom/right
    Image
    {
        id: idRightArrowIcon

        z:4
        width: 72
        height: 72
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        mirror: true
        source: "qrc:/Icons/ICON-Strokes_Back-72px_white.png"

        layer.enabled: true
        layer.smooth: true

        // This item should be used as the 'mask'
        layer.samplerName: "maskSource"
        layer.effect: LongPressShaderEffect
        {
            id: idRightArrowIconShaderEffect
            longPressAnimation: idSavingAnimation
        }

        LongPressAnimation
        {
            id: idSavingAnimation

            fillColor: idMain.colorEkxConfirmGreen

            onStarted:
            {
                idUnitSecText.longPressAnimation = idSavingAnimation
                idPreDecimalValue.longPressAnimationInt = idSavingAnimation
                idPostDecimalValue.longPressAnimationInt = idSavingAnimation
                idDecimalPointText.longPressAnimation = idSavingAnimation
            }

            onLongPressed:
            {
                handleSaveChanges();
            }

            onSimplePressed:
            {
                if( idRoot.state === "editGrindTime")
                {
                    idRoot.state = "editCoarseness";
                }
                else
                {
                    idRoot.state = "editPortaFilter";
                }
                idPreDecimalValue.setStartValues( startDddBig );
                idPostDecimalValue.setStartValues( startDddComma );
            }
        }
    }
}
