import QtQml 2.12
import QtQuick 2.12
import QtGraphicalEffects 1.12

import ".."
import "../../CommonItems"

import EkxMainstateEnum 1.0
import SettingStatesEnum 1.0

Item
{
    id: libraryMode
    visible: settingsSerializer.libraryModeIsEnabled
    width:   settingsSerializer.libraryModeIsEnabled ? 480 : 0
    height:  settingsSerializer.libraryModeIsEnabled ? 360 : 0

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // self defined properties
    ///////////////////////////////////////////////////////////////////////////////////////////////

    property bool visualDebug: false;
    property bool splashScreenActive: true;
    property bool modeIsLocked: editLibraryRecipe || showBeansList || mainStatemachine.isCoffeeRunning;
    property bool lockCoffeeCondition: editLibraryRecipe || showBeansList;
    property var libraryList: recipeControl.installedLibraryList;
    property int currentLibraryRecipeIndex: recipeControl.currentLibraryRecipeIndex

    property int currentEkxMainstate: mainStatemachine.ekxMainstate

    property bool showBeansList: false;
    property bool editLibraryRecipe: false
    property bool editLibraryRecipeAllowed: false

    property int dddValueBig: dddDriver.dddBigValue;
    property int dddValueSmall: dddDriver.dddCommaValue;

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // signals
    ///////////////////////////////////////////////////////////////////////////////////////////////

    signal activated();
    signal editBeansList();
    signal hideBeansList();

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // functions
    ///////////////////////////////////////////////////////////////////////////////////////////////

    function updateDddAdjustment()
    {
        adjustment.currentRecipeDdd = parseFloat(libraryList[currentLibraryRecipeIndex].dddBigValue + "."+ libraryList[currentLibraryRecipeIndex].dddCommaValue)

    }

    function setBackGrammage()
    {
        grammageFlick.contentX = 0;
    }

    function showLibraryModeRecipe( currentLibraryRecipeIndex, grammage, grammageIndex, editStartState )
    {
        //console.log("LibraryMode/showLibraryModeRecipe");
        characterWheelView.setDddStartValues( libraryList[currentLibraryRecipeIndex].dddBigValue,
                                             libraryList[currentLibraryRecipeIndex].dddCommaValue)
        characterWheelView.setRecipeIndex(currentLibraryRecipeIndex);
        editLibraryRecipe = true;
        if( grammage )
        {
            characterWheelView.setGrammageEntry(grammageIndex);
        }
        characterWheelView.setStartState(editStartState);

        editBeansList();
    }

    function hideLibraryRecipe()
    {
        editLibraryRecipe = false;
        grammage.updateGrammageView();
        hideBeansList();
    }

    function getTypeOfItem()
    {
        return 2;
    }

    function resetModeInfo()
    {
        splashScreenActive = true;
        blendOutblur.opacity = 1;
        blendOutblur.radius = 55;
        blendoutText.opacity = 1;
        grammageFlick.opacity = 0;
        beansName.opacity = 0;
        blendOutTimer.stop();
        blendOut.stop();
        showBeansList = false;
        hideLibraryRecipe();
    }

    function startBlendoutInfo()
    {
        //console.log("LibraryMode: onEnter");
        blendOutTimer.start();
    }

    function abortLongPress()
    {
        idEditAnimation.abort()
        idEditBeansNameAnimation.abort()
        grammage.abortLongPress()
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // signal handlers
    ///////////////////////////////////////////////////////////////////////////////////////////////

    Component.onCompleted:
    {
        resetModeInfo();
        updateDddAdjustment()
    }

    onEditLibraryRecipeAllowedChanged:
    {
        abortLongPress()
    }

    onCurrentLibraryRecipeIndexChanged:
    {
        updateDddAdjustment()
    }

    onCurrentEkxMainstateChanged:
    {
        if ( currentEkxMainstate == EkxMainstateEnum.EKX_LIBRARY_MODE )
        {
            startBlendoutInfo();
        }
        else
        {
            resetModeInfo();
        }
    }

    onLockCoffeeConditionChanged:
    {
        //console.log("onLockCoffeeConditionChanged(): changed to "+lockCoffeeCondition);
        mainStatemachine.isCoffeeRunAllowed = !lockCoffeeCondition;
        //console.log("mainStatemachine.isCoffeeRunAllowed = " +mainStatemachine.isCoffeeRunAllowed);
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // items
    ///////////////////////////////////////////////////////////////////////////////////////////////

    Timer
    {
        id: blendOutTimer

        interval: 260
        running: false
        repeat:  false

        onTriggered:
        {
            blendOut.start();
        }
    }

    Timer
    {
        id: moveTimer
        // SnapIn Animation for Flickable "grammageFlickable"
        interval: 10
        repeat: true
        property int snapPointLeft: 0;
        //property int snapPointRight: 256;
        //property int snapPointRight: 216;
        property int snapPointRight: grammageFlick.contentWidth-grammageFlick.width;
        property int snapBorderBetweenLeftRight: (snapPointLeft+snapPointRight)/2;
        property int stepsize: 5;

        onTriggered:
        {
            if( snapBorderBetweenLeftRight < grammageFlick.contentX
                    && grammageFlick.contentX <(snapPointRight-stepsize) )
            {
                grammageFlick.contentX += stepsize;
            }
            else if( (snapPointLeft+stepsize) < grammageFlick.contentX
                    && grammageFlick.contentX <= snapBorderBetweenLeftRight )
            {
                grammageFlick.contentX -= stepsize;
            }
            else
            {
                moveTimer.stop();
            }

            if( grammageFlick.contentX <= (snapPointLeft+stepsize) )
            {
                grammageFlick.contentX = snapPointLeft;
            }
            if( grammageFlick.contentX >= (snapPointRight-stepsize) )
            {
                grammageFlick.contentX = snapPointRight;
            }
        }
    }

    Flickable
    {
        id: grammageFlick

        width: 480; // display resolution
        height: 360; // diplays resolution
        x:0;
        y:0;
        opacity: 0
        contentWidth: idDbgGrammageFlickContent.width;
        contentHeight: idDbgGrammageFlickContent.height;
        interactive: grammage.rowOneVisible || grammage.rowTwoVisible

        flickableDirection: Flickable.HorizontalFlick

        boundsBehavior:  Flickable.StopAtBounds

        onMovementStarted:
        {
            editLibraryRecipeAllowed = false
            abortLongPress()
        }

        onMovementEnded:
        {
            editLibraryRecipeAllowed = true
            moveTimer.start();

        }

        onContentXChanged:
        {
            if( contentX > 50 &&  contentX <= 200 )
            {
                currentValue.opacity = 0.8
                adjustment.opacity = 0.8
            }
            else if( contentX > 200 &&  contentX <= 220 )
            {
                currentValue.opacity = 0.7
                adjustment.opacity = 0.7
            }
            else if( contentX > 220 &&  contentX <= 240 )
            {
                currentValue.opacity = 0.6
                adjustment.opacity = 0.6
            }
            else if( contentX > 240 &&  contentX <= 246 )
            {
                currentValue.opacity = 0.5
                adjustment.opacity = 0.5
            }
            else if( contentX > 246 &&  contentX <= 250 )
            {
                currentValue.opacity = 0.4
                adjustment.opacity = 0.4
            }
            else if( contentX > 250 &&  contentX <= 256 )
            {
                currentValue.opacity = 0.3
                adjustment.opacity = 0.3
            }
            else
            {
                currentValue.opacity = 1.0
                adjustment.opacity = 1.0
            }

            if(contentX>200)
                //idShadowRight.visible=false;
                idShadowGroup.shadowRightEnabled = false;
            else
                //idShadowRight.visible=true;
                idShadowGroup.shadowRightEnabled = true;
        }

        Rectangle
        {
            id: idDbgGrammageFlickContent;
            z: 10
            width: 700
            height: parent.height;
            color: "transparent";

            DddAdjustment
            {
                id: adjustment

                anchors.top: parent.top;
                anchors.left: parent.left;

                currentDddValue: parseFloat(dddValueBig + "."+ dddValueSmall)

                showCurrentDdd: false

                onSaveCurrentDdd:
                {
                    recipeControl.editLibraryRecipe( dddValueBig,
                                                    dddValueSmall,
                                                    recipeControl.installedLibraryList[currentLibraryRecipeIndex].beanName,
                                                    recipeControl.installedLibraryList[currentLibraryRecipeIndex].grammage1Big,
                                                    recipeControl.installedLibraryList[currentLibraryRecipeIndex].grammage1Comma,
                                                    recipeControl.installedLibraryList[currentLibraryRecipeIndex].grammage1Icon,
                                                    recipeControl.installedLibraryList[currentLibraryRecipeIndex].grammage2Big,
                                                    recipeControl.installedLibraryList[currentLibraryRecipeIndex].grammage2Comma,
                                                    recipeControl.installedLibraryList[currentLibraryRecipeIndex].grammage2Icon,
                                                    recipeControl.installedLibraryList[currentLibraryRecipeIndex].grammageCount,
                                                    recipeControl.installedLibraryList[currentLibraryRecipeIndex].showGrammage,
                                                    currentLibraryRecipeIndex );
                    updateDddAdjustment();
                }
                DebugFrame{visible: visualDebug; visualDebugColor: "lime"; visualDebugStroke: 7;}
            }

            TupleValue
            {
                id: currentValue
                visualDebug: visualDebug;
                anchors.verticalCenter: parent.verticalCenter;
                x: grammageFlick.width/2-width/2;

                centerXByHallbach: false;
                centerXByMass: true;
                preDecimalValue: dddValueBig
                postDecimalValue: dddValueSmall

                tupleAnimation: idEditAnimation

                LongPressAnimation
                {
                    id: idEditAnimation
                    anchors.left: parent.left
                    anchors.top: parent.top
                    anchors.topMargin: 80
                    width: 320
                    height: 280
                    enabled: !mainStatemachine.isCoffeeRunning && (mainStatemachine.ekxMainstate===EkxMainstateEnum.EKX_LIBRARY_MODE) && !splashScreenActive;
                    fillColor: idMain.colorEkxConfirmGreen

                    onSimplePressed:
                    {
                        //console.log("LibrarayMode/LongPressAnimation/onSimplePressed");
                        if( !editLibraryRecipe )
                        {
                            libraryMode.showBeansList = true
                            editBeansList();
                        }
                    }

                    onLongPressed:
                    {
                        //console.log("LibraryMode/currentValue/idEditAnimation/onLongPressed");
                        showLibraryModeRecipe( recipeControl.currentLibraryRecipeIndex, false, 0, "editCoarseness" );
                    }
                }
                DebugFrame{visible: visualDebug;}
            }

            Grammage
            {
                id: grammage
                visualDebug: visualDebug;
                x: 400;
                y: 70

                onEditGrammage:
                {
                    console.log("LibraryMode/grammage/onEditGrammage");
                    grammageFlick.contentX = 0;
                    showLibraryModeRecipe( recipeControl.currentLibraryRecipeIndex, true, grammageIndex, "editGrammage" );
                }
                DebugFrame{visible: visualDebug;}
            }
            DebugFrame{visible: visualDebug;}
        }
    }
    DebugFrame{visible: visualDebug; target: grammageFlick; visualDebugColor: "red"}


    Text
    {
        id: beansName

        anchors.bottom: parent.bottom
        anchors.bottomMargin: 10
        anchors.horizontalCenter: parent.horizontalCenter

        text:  libraryList[recipeControl.currentLibraryRecipeIndex].beanName
        color:  idMain.colorEkxTextGrey

        font.pixelSize: 58
        font.family:    "D-DIN Condensed HEMRO"
        font.bold: true

        opacity: 0

        layer.enabled: true
        layer.smooth: true;

        // This item should be used as the 'mask'
        layer.samplerName: "maskSource"
        layer.effect: LongPressShaderEffect
        {
            longPressAnimation: idEditBeansNameAnimation
            backgroundColor: beansName.color
        }

        LongPressAnimation
        {
            id: idEditBeansNameAnimation

            fillColor: idMain.colorEkxConfirmGreen

            onSimplePressed:
            {
                if( !editLibraryRecipe )
                {
                    libraryMode.showBeansList = true
                    editBeansList();
                }
            }

            onLongPressed:
            {
                console.log("LibraryMode/beansName/onLongPressed");
                showLibraryModeRecipe( recipeControl.currentLibraryRecipeIndex, false, 0, "editBeansName" );
            }
        }
        DebugFrame{visible: visualDebug; visualDebugColor: "light blue"}
    }

    LibraryModeEditRecipeView
    {
        id: characterWheelView

        visible: editLibraryRecipe

        z:2
        anchors.centerIn: parent

        onCloseAngleView:
        {
            updateDddAdjustment();
            hideLibraryRecipe();
        }
    }

    BeansListView
    {
        id: beansList

        visible: showBeansList

        z: 2
        anchors.left:parent.left
        anchors.top: parent.top

        onCloseBeansListView:
        {
            libraryMode.showBeansList = false;
            hideBeansList();
        }

        onOpenLibraryListEdit:
        {
            //console.log("LibraryMode/beansList/onOpenLibraryListEdit");
            // open next dialog before closing previous one to prevent short-time switch to mainmenu (glitch)
            showLibraryModeRecipe( recipeIndex, false, 0, "editBeansName" );
            libraryMode.showBeansList = false;

        }
    }

    FastBlur
    {
        id: blendOutblur

        visible: true

        width: 480
        height: 360
        z:1

        anchors.centerIn: parent

        opacity: 1.0

        source: grammageFlick
        radius: 55
    }

    Text
    {
        id: blendoutText

        anchors.centerIn: parent

        text: "Space Mode"
        color:  idMain.colorEkxTextWhite
        opacity: 1

        font.pixelSize: 58
        font.family:    "D-DIN Condensed HEMRO"
        font.bold:      true
        font.letterSpacing: 0.23
    }

    ParallelAnimation
    {
        id: blendOut
        running: false
        NumberAnimation { target: blendOutblur; property: "radius"; to: 0; duration: 150 }
        NumberAnimation { target: blendOutblur; property: "opacity"; to: 0; duration: 150 }
        NumberAnimation { target: blendoutText; property: "opacity"; to: 0; duration: 150 }
        NumberAnimation { target: grammageFlick; property: "opacity"; to: 1; duration: 150 }
        NumberAnimation { target: beansName; property: "opacity"; to: 1; duration: 150 }
        onFinished: {
            //console.log("LibraryMode: blendout animation finished");
            splashScreenActive = false;
        }
    }

    ShadowGroup
    {
        id: idShadowGroup;
        visible: !showBeansList
        //visualDebug: true;
        shadowRightSize: 100;
        shadowLeftSize: 100;
    }

    ShadowGroup
    {
        id: idShadowGroup2;
        visible: !showBeansList
        //visualDebug: true;
        shadowRightSize: 35;
    }
}
