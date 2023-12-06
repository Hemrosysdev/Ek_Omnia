import QtQml 2.12
import QtQuick 2.12
import "../../CommonItems"
import "../LibraryMode"

Rectangle
{
    id: idRoot

    width:  480
    height: 360
    color:  "black"

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // self defined properties
    ///////////////////////////////////////////////////////////////////////////////////////////////

    property int recipeIndex;

    property int startDddBig: 0;
    property int startDddComma: 0;

    property color fillColor: idMain.colorEkxConfirmRed
    property bool  continueEdit: false

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // states
    ///////////////////////////////////////////////////////////////////////////////////////////////

    states: [
        State
        {
            name: "editBeansName"
            PropertyChanges { target: idLeftArrowIcon; visible: false }
            PropertyChanges { target: idRightArrowIcon; visible: true }
            PropertyChanges { target: idEditTypeIcon; source: 'qrc:/Icons/ICON-Strokes_Timer-30px_white_50%.png';  visible: false}
            PropertyChanges { target: idBackspaceIcon; visible:true }
            PropertyChanges { target: idShowGrammageToggle; visible:false }
            PropertyChanges { target: idAcceptIcon; visible:false }
            PropertyChanges { target: idPreDecimalValue; visible:false }
            PropertyChanges { target: idPostDecimalValue; visible:false }
            PropertyChanges { target: idDecimalPoint; visible:false }
            PropertyChanges { target: idEditBeansName; visible:true }
            PropertyChanges { target: grammageEditList; visible:false }
            PropertyChanges { target: grammageEdit; visible:false }
            PropertyChanges { target: idCloseIcon; visible:true }
        },
        State
        {
            name: "editCoarseness"
            PropertyChanges { target: idLeftArrowIcon; visible: true }
            PropertyChanges { target: idRightArrowIcon; visible: true }
            PropertyChanges { target: idEditTypeIcon; source: 'qrc:/Icons/ICON_DiscDistance-72px_white 50%.png'; visible: true }
            PropertyChanges { target: idBackspaceIcon; visible:false }
            PropertyChanges { target: idShowGrammageToggle; visible:false }
            PropertyChanges { target: idAcceptIcon; visible:false }
            PropertyChanges { target: idPreDecimalValue; visible:true }
            PropertyChanges { target: idPostDecimalValue; visible:true }
            PropertyChanges { target: idDecimalPoint; visible:true }
            PropertyChanges { target: idEditBeansName; visible:false }
            PropertyChanges { target: grammageEditList; visible:false }
            PropertyChanges { target: grammageEdit; visible:false }
            PropertyChanges { target: idCloseIcon; visible:true }
        },
        State
        {
            name: "editGrammageList"
            PropertyChanges { target: idLeftArrowIcon; visible: true }
            PropertyChanges { target: idRightArrowIcon; visible: false }
            PropertyChanges { target: idEditTypeIcon; source: 'qrc:/Icons/ICON_DiscDistance-72px_white 50%.png'; visible: false }
            PropertyChanges { target: idBackspaceIcon; visible:false }
            PropertyChanges { target: idShowGrammageToggle; visible:true }
            PropertyChanges { target: idAcceptIcon; visible:true }
            PropertyChanges { target: idPreDecimalValue; visible:false }
            PropertyChanges { target: idPostDecimalValue; visible:false }
            PropertyChanges { target: idDecimalPoint; visible:false }
            PropertyChanges { target: idEditBeansName; visible:false }
            PropertyChanges { target: grammageEditList; visible:true }
            PropertyChanges { target: grammageEdit; visible:false }
            PropertyChanges { target: idCloseIcon; visible:true }
        },
        State
        {
            name: "editGrammage"
            PropertyChanges { target: idLeftArrowIcon; visible: true }
            PropertyChanges { target: idRightArrowIcon; visible: false }
            PropertyChanges { target: idEditTypeIcon; source: 'qrc:/Icons/ICON_DiscDistance-72px_white 50%.png'; visible: false }
            PropertyChanges { target: idBackspaceIcon; visible:false }
            PropertyChanges { target: idShowGrammageToggle; visible:false }
            PropertyChanges { target: idAcceptIcon; visible:false }
            PropertyChanges { target: idPreDecimalValue; visible:false }
            PropertyChanges { target: idPostDecimalValue; visible:false }
            PropertyChanges { target: idDecimalPoint; visible:false }
            PropertyChanges { target: idEditBeansName; visible:false }
            PropertyChanges { target: grammageEditList; visible:false }
            PropertyChanges { target: grammageEdit; visible:true }
            PropertyChanges { target: idCloseIcon; visible:true }
        }
    ]

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // signals
    ///////////////////////////////////////////////////////////////////////////////////////////////

    signal closeAngleView();

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // functions
    ///////////////////////////////////////////////////////////////////////////////////////////////

    function setRecipeIndex( currentRecipeIndex )
    {
        recipeIndex = currentRecipeIndex;
        idEditBeansName.init( recipeControl.installedLibraryList[currentRecipeIndex].beanName )
    }

    function setDddStartValues(big, comma)
    {
        startDddBig = big;
        startDddComma = comma;

        idPreDecimalValue.setStartValues(startDddBig);
        idPostDecimalValue.setStartValues(startDddComma);
    }

    function setStartState( startState )
    {
        idRoot.state = startState;
    }

    function setGrammageEntry( gramIndex )
    {
        grammageEdit.setGrammageStartValues( recipeIndex, gramIndex );
        idRoot.state = "editGrammage";
    }

    function addRedHeight()
    {
        redRect.height = redRect.height + 3
    }

    function resetRedHeight()
    {
        redRect.height = 0;
    }

    function handleSaveChanges()
    {
        var beanName = idEditBeansName.getBeanName();

        if( startDddBig === (recipeControl.getMaxDddValue() - 1) )
        {
            startDddComma = 0;
        }

        recipeControl.editLibraryRecipe( startDddBig,
                                        startDddComma,
                                        beanName,
                                        recipeControl.installedLibraryList[recipeIndex].grammage1Big,
                                        recipeControl.installedLibraryList[recipeIndex].grammage1Comma,
                                        recipeControl.installedLibraryList[recipeIndex].grammage1Icon,
                                        recipeControl.installedLibraryList[recipeIndex].grammage2Big,
                                        recipeControl.installedLibraryList[recipeIndex].grammage2Comma,
                                        recipeControl.installedLibraryList[recipeIndex].grammage2Icon,
                                        recipeControl.installedLibraryList[recipeIndex].grammageCount,
                                        recipeControl.installedLibraryList[recipeIndex].showGrammage,
                                        recipeIndex );
        idRoot.state = "editBeansName";
        idRoot.closeAngleView();
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // signal handlers
    ///////////////////////////////////////////////////////////////////////////////////////////////

    Component.onCompleted:
    {
        idRoot.state = "editBeansName";
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // items
    ///////////////////////////////////////////////////////////////////////////////////////////////

    ShadowGroup
    {
        z: 1;
        shadowTopSize: ShadowGroup.ShadowTopSizes.StandardSize;
        shadowBottomSize: ShadowGroup.ShadowBottomSizes.StandardSize;
    }

    MultiCharacterWheel
    {
        id: idEditBeansName

        visible: false
        z:2
        anchors.left: parent.left
        anchors.top:  parent.top
    }

    Item
    {
        id: idEditTypeIcon

        z:4
        width: 72
        height: 72

        anchors.left:parent.left
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

        z:0
        anchors.left: parent.left
        anchors.leftMargin: 114

        currentFontFamiliy: "D-DIN Condensed HEMRO"

        model: recipeControl.getMaxDddValue();
        interactive: true

        smallValue: false

        longPressAnimationInt: idSavingAnimation

        onCurrentValue:
        {
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
            text:  "."
            color: idMain.colorEkxTextWhite

            font.pixelSize: 174
            font.family: "D-DIN Condensed HEMRO Light_Numbers"

            anchors.horizontalCenter: parent.horizontalCenter
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 90

            layer.enabled: true
            layer.smooth: true

            layer.samplerName: "maskSource"
            layer.effect: LongPressShaderEffect
            {
                longPressAnimation: idSavingAnimation
            }
        }
    }

    NumberWheel
    {
        id: idPostDecimalValue

        z:0
        anchors.left: parent.left
        anchors.leftMargin: 270

        informAboutChange: true

        itemWidth: 120

        currentFontFamiliy: "D-DIN Condensed HEMRO Light_Mono_Numbers"

        model: 10
        interactive: true

        longPressAnimationInt: idSavingAnimation

        onCurrentValue:
        {
            if ( idRoot.state === "editCoarseness" )
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

    GrammageEditList
    {
        id: grammageEditList

        visible: false
        z:2

        editLibraryIndex: recipeIndex
        grammageCount: recipeControl.installedLibraryList[recipeIndex].grammageCount

        anchors.left: parent.left
        anchors.top:  parent.top

        onOpenGrammageEditList:
        {
            if(idRoot.state==="editGrammageList")
            {
                grammageEdit.setGrammageStartValues( recipeIndex, grammageIndex );
                idRoot.state = "editGrammage";
            }
        }

        onGrammageCountChanged:
        {
            idShowGrammageToggle.updateToggleValue();
        }
    }

    GrammageEdit
    {
        id: grammageEdit

        visible: false
        z:2

        anchors.left: parent.left
        anchors.top:  parent.top

        onSavedGrammageChanges:
        {
            idRoot.state = "editGrammageList";
        }
    }

    ActivateToggle
    {
        id: idShowGrammageToggle

        visible:false
        width: 72
        height: 72
        z:4

        anchors.right: parent.right
        anchors.top: parent.top

        function updateToggleValue()
        {
            active = recipeControl.installedLibraryList[recipeIndex].showGrammage;

        }

        onToggleChanged:
        {
            recipeControl.installedLibraryList[recipeIndex].showGrammage = active;
            updateToggleValue();
        }

        onVisibleChanged:
        {
            updateToggleValue();
        }
    }

    EkxButton
    {
        id: idBackspaceIcon
        z:4
        anchors.right: parent.right
        anchors.top: parent.top
        buttonType: EkxButton.ButtonType.BACKSP;
        onClicked:
        {
            idEditBeansName.deleteText();
        }
    }

    EkxButton
    {
        id: idCloseIcon
        z:4
        buttonType: EkxButton.ButtonType.CLOSE;
        onClicked:
        {
            idRoot.state = "editBeansName";
            idRoot.closeAngleView();
        }
    }

    EkxButton
    {
        id: idAcceptIcon
        z:4
        buttonType: EkxButton.ButtonType.OK;
        onClicked:
        {
            handleSaveChanges();
        }
    }

    EkxButton
    {
        id: idLeftArrowIcon
        z:4
        buttonType: EkxButton.ButtonType.PREVIOUS;
        onClicked:
        {
            if ( idRoot.state === "editCoarseness" )
            {
                idRoot.state = "editBeansName";
            }
            else if(idRoot.state==="editGrammageList")
            {
                idRoot.state = "editCoarseness";
            }
            else if(idRoot.state==="editGrammage")
            {
                idRoot.state = "editGrammageList";
            }
        }
    }

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
            longPressAnimation: idSavingAnimation
        }

        LongPressAnimation
        {
            id: idSavingAnimation

            fillColor: idMain.colorEkxConfirmGreen

            onLongPressed:
            {
                handleSaveChanges();
            }

            onSimplePressed:
            {
                if( idRoot.state === "editBeansName")
                {
                    idRoot.state = "editCoarseness";
                }
                else
                {
                    idRoot.state = "editGrammageList";
                }
            }
        }
    }
}
