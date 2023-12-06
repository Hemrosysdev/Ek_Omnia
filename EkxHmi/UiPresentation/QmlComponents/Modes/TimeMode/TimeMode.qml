import QtQml 2.12
import QtQuick 2.12
import QtQml.Models 2.1
import QtGraphicalEffects 1.12
import ".."
import "../../CommonItems"

import EkxMainstateEnum 1.0
import SettingStatesEnum 1.0

Item
{
    id: timeMode

    width:  settingsSerializer.timeModeIsEnabled ? 480 : 0
    height: settingsSerializer.timeModeIsEnabled ? 360 : 0
    visible: settingsSerializer.timeModeIsEnabled

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // self defined properties
    ///////////////////////////////////////////////////////////////////////////////////////////////

    property bool visualDebug: false;
    property color visualDebugColor: "green";
    property bool interactive: false;
    property bool modeIsLocked: editRecipeListActive || editRecipeActive || productRunning;
    property bool lockCoffeeCondition: editRecipeListActive || editRecipeActive;
    property int currentEkxMainstate: mainStatemachine.ekxMainstate

    // DDD values
    property int dddValueBig: dddDriver.dddBigValue;
    property int dddValueSmall: dddDriver.dddCommaValue;

    // RecipeValues

    property int runningBig: 0
    property int runningComma: 0
    property bool productRunning: mainStatemachine.isCoffeeRunning;
    property variant portaFilterImages: [
        "qrc:/Icons/ICON-Strokes_Single-80px_portafilter_white.png",
        "qrc:/Icons/ICON-Strokes_Double-80px_portafilter_white.png",
        "qrc:/Icons/ICON-Strokes_Bottomless-80px_portafilter_white.png"]

    property int recipeCount: recipeControl.currentTimeRecipeCount;
    property int saveCurrentRecipe: recipeControl.currentTimeRecipeIndex

    property var recipeList: recipeControl.installedRecipeList;

    property int recipeDddBig: recipeList[recipeControl.currentTimeRecipeIndex] ? recipeList[recipeControl.currentTimeRecipeIndex].dddBigValue : 0;
    property int recipeDddComma: recipeList[recipeControl.currentTimeRecipeIndex] ? recipeList[recipeControl.currentTimeRecipeIndex].dddCommaValue : 0;

    property int portaFilter:  recipeList[recipeControl.currentTimeRecipeIndex] ? recipeList[recipeControl.currentTimeRecipeIndex].portaFilterIndex : 0;

    property bool editRecipeActive: false
    property bool editRecipeAllowed: true;
    property bool startUp: true;

    property bool editRecipeListActive: false

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // signals
    ///////////////////////////////////////////////////////////////////////////////////////////////

    signal closeEditRecipe();
    signal openEditRecipe();
    signal recipeIndexChanged( var recipeIndex )

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // functions
    ///////////////////////////////////////////////////////////////////////////////////////////////

    function getTypeOfItem()
    {
        return 1;
    }

    function showEditRecipe( currentTimeRecipeIndex )
    {
        editRecipeActive = true;
        openEditRecipe();
        idEditRecipeView.setRecipeStartValues(recipeList[currentTimeRecipeIndex].recipeBigValue,
                                              recipeList[currentTimeRecipeIndex].recipeCommaValue,
                                              portaFilter)
        idEditRecipeView.setDddStartValues( recipeList[currentTimeRecipeIndex].dddBigValue,
                                           recipeList[currentTimeRecipeIndex].dddCommaValue )
        idEditRecipeView.setRecipeIndex(currentTimeRecipeIndex);
    }

    function hideEditRecipe()
    {
        editRecipeActive = false;
        recipeIndexChanged( recipeControl.currentTimeRecipeIndex );
        closeEditRecipe();
    }

    function showEditListRecipe()
    {
        editRecipeListActive = true;
        editRecipeActive = false;
        openEditRecipe();
    }

    function hideEditListRecipe()
    {
        editRecipeListActive = false;
        recipeIndexChanged(recipeControl.currentTimeRecipeIndex);
        closeEditRecipe();
    }

    function resetModeInfo()
    {
        timeMode.interactive = false;
        blendOutblur.opacity = 1;
        blendOutblur.radius = 55;
        blendoutText.opacity = 1;
        timeModeRecipeList.opacity = 0;
        temperatureView.opacity = 0;
        idPortaFilterIcon.opacity = 0;
        adjustment.opacity = 0;

        blendOutTimer.stop();
        blendOut.stop();
        hideEditListRecipe();
        hideEditRecipe();
    }

    function startBlendoutInfo()
    {
        blendOutTimer.start();
    }

    function abortLongPress()
    {
        idEditAnimation.abort()
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // signal handlers
    ///////////////////////////////////////////////////////////////////////////////////////////////

    onRecipeDddBigChanged:
    {
        adjustment.currentRecipeDdd = parseFloat(recipeDddBig + "."+ recipeDddComma)
    }

    onRecipeDddCommaChanged:
    {
        adjustment.currentRecipeDdd = parseFloat(recipeDddBig + "."+ recipeDddComma)
    }

    onProductRunningChanged:
    {
        if( productRunning )
        {
            timeModeTimer.start();
        }
        else
        {
            timeModeTimer.resetTimeModeTimer();
        }
    }

    onSaveCurrentRecipeChanged:
    {
        mainStatemachine.currentRecipe = recipeControl.currentTimeRecipeIndex;

        if(startUp)
        {
            startUp = false;
            timeModeRecipeList.currentIndex = recipeControl.currentTimeRecipeIndex;
            recipeIndexChanged(recipeControl.currentTimeRecipeIndex);

        }
    }

    onCurrentEkxMainstateChanged:
    {
        if( currentEkxMainstate == EkxMainstateEnum.EKX_TIME_MODE )
        {
            resetModeInfo();
            startBlendoutInfo();
        }
        else
        {
            resetModeInfo();
        }
    }

    onLockCoffeeConditionChanged:
    {
        //console.log("onLockCoffeeConditionChanged(): changed to " + lockCoffeeCondition);
        mainStatemachine.isCoffeeRunAllowed = !lockCoffeeCondition;
        //console.log("mainStatemachine.isCoffeeRunAllowed = " + mainStatemachine.isCoffeeRunAllowed);
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // items
    ///////////////////////////////////////////////////////////////////////////////////////////////

    // wait timer after mode change to start fade-over
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
        id: timeModeTimer
        interval: 100;
        running: false;
        repeat: true

        property int endBigValue: recipeList[recipeControl.currentTimeRecipeIndex] ? recipeList[recipeControl.currentTimeRecipeIndex].recipeBigValue : 0
        property int endCommaValue: recipeList[recipeControl.currentTimeRecipeIndex] ? recipeList[recipeControl.currentTimeRecipeIndex].recipeCommaValue : 0

        function resetTimeModeTimer()
        {
            timeModeTimer.stop();
            runningComma = 0;
            runningBig   = 0;
        }

        onTriggered:
        {
            runningComma++
            if ( runningComma === 10 )
            {
                runningComma = 0
                runningBig++;
            }

            if( ( runningBig == endBigValue ) && ( runningComma == endCommaValue ) )
            {
                resetTimeModeTimer();
                mainStatemachine.isCoffeeRunning = false;
            }
        }
    }

    TimeModeEditRecipeView
    {
        id: idEditRecipeView

        z:2
        anchors.centerIn: parent
        visible: editRecipeActive

        onCloseAngleView:
        {
            //console.log("close: " + recipeIndex);
            recipeControl.currentTimeRecipeIndex = recipeIndex;
            timeModeRecipeList.positionViewAtIndex(recipeIndex, ListView.Center);
            hideEditRecipe();
        }
    }

    RecipeListEdit
    {
        id: recipeListEdit

        z:2
        visible: editRecipeListActive
        recipeCount: timeMode.recipeCount

        onCloseEditRecipeList:
        {
            hideEditListRecipe()
        }

        onOpenEditRecipeList:
        {
            // open next dialog before closing previous one to prevent short-time switch to mainmenu (glitch)
            recipeControl.currentTimeRecipeIndex = recipeIndex;
            timeModeRecipeList.positionViewAtIndex(recipeIndex, ListView.Visible);
            showEditRecipe( recipeIndex )
            hideEditListRecipe();
        }
    }

    // horizontal list view for recipes
    ListView
    {
        id: timeModeRecipeList;
        anchors.fill: parent;
        displayMarginBeginning: 480;
        displayMarginEnd: 480;
        spacing:0;
        highlightFollowsCurrentItem: true;
        highlightRangeMode: ListView.StrictlyEnforceRange;
        preferredHighlightBegin: 60;
        preferredHighlightEnd: 480;
        boundsMovement: Flickable.StopAtBounds;
        boundsBehavior: Flickable.StopAtBounds;
        snapMode: ListView.SnapOneItem
        flickableDirection: Flickable.HorizontalFlick
        orientation: ListView.Horizontal
        interactive: !productRunning
        model: recipeList

        delegate: Rectangle {
            id: idListItem;
            //width: 480;
            width: 480-60;
            height: 360;
            color: "transparent";
            border.width: visualDebug?10:0;
            border.color: visualDebug?"yellow":"transparent";

            TupleValue
            {
                id: idTuple;
                showUnits: true
                anchors.verticalCenter: parent.verticalCenter;
                //anchors.horizontalCenter: parent.horizontalCenter;
                x: 480/2-width/2-60;
                centerXByHallbach: false;
                centerXByMass: true;
                preDecimalValue: productRunning ? runningBig : modelData.recipeBigValue
                postDecimalValue: productRunning ? runningComma : modelData.recipeCommaValue

                tupleAnimation: idEditAnimation
            }
            Rectangle
            {
                anchors.fill: parent;
                //anchors.leftMargin: 60;
                anchors.leftMargin: 20;
                anchors.rightMargin: 80;
                anchors.topMargin: 65;
                anchors.bottomMargin: 75;
                color: "transparent";
                border.width: 5;
                border.color: timeMode.visualDebug?timeMode.visualDebugColor:"transparent";

                MouseArea
                {
                    anchors.fill: parent
                    enabled: !productRunning

                    onReleased:
                    {
                        idEditAnimation.stop()
                    }
                }
            }

        }

        onMovingChanged:
        {
            idEditAnimation.abort()
            recipeControl.currentTimeRecipeIndex = currentIndex;
            recipeIndexChanged(recipeControl.currentTimeRecipeIndex);

        }

        onModelChanged:
        {
            timeModeRecipeList.positionViewAtIndex(recipeControl.currentTimeRecipeIndex, ListView.Visible);
        }

        onMovementStarted:
        {
            editRecipeAllowed = false;
        }

        onMovementEnded:
        {
            //console.log("TimeMode/ListView/onMovementEnded");
            editRecipeAllowed = true;
            idPortaFilterIconAnimation.to=1.0;
            idPortaFilterIconAnimation.start();
        }

        onContentXChanged:
        {
            if(horizontalVelocity!==0.0 && moving===true && idPortaFilterIconAnimation.to===1.0)
            {
                idPortaFilterIconAnimation.to=0.0;
                idPortaFilterIconAnimation.start();
            }
        }
    }

    Rectangle {
        id: idDbgRecipeList;
        visible: visualDebug;
        anchors.fill: timeModeRecipeList;
        color: "transparent";
        border.width: visualDebug?5:0;
        border.color: visualDebug?visualDebugColor:"transparent";
    }

    NumberAnimation
    {
        id: idPortaFilterIconAnimation;
        target: idPortaFilterIcon;
        property: "opacity";
        to: 1;
        duration: 200
    }

    // right aligned porta filter icon
    Image
    {
        id: idPortaFilterIcon
        z:2
        visible: recipeList[recipeControl.currentTimeRecipeIndex].recipeIconOn && !editRecipeListActive && !editRecipeActive
        width: 80
        height: 80
        opacity: 0
        anchors.right: parent.right
        anchors.verticalCenter:  parent.verticalCenter
        source: portaFilterImages[ recipeList[recipeControl.currentTimeRecipeIndex].portaFilterIndex]
    }

    // addtional centered mouse area to catch simple press/long press actions
    //Rectangle{
    //    z:10
    //    visible: upperMouseArea.enabled?true:false;
    //    anchors.fill: upperMouseArea;
    //    border.width: 5;
    //    border.color: "orange";
    //    color: "transparent";
    //}

    // this animation is not visible and will be
    // driven by outside MouseAreas. This is due to the overlapped
    // horizontal movements and click events
    LongPressAnimation
    {
        id: idEditAnimation

        visible: false
        enabled: !productRunning && editRecipeAllowed && timeMode.interactive

        anchors.fill: timeModeRecipeList
        anchors.leftMargin: 100;
        anchors.rightMargin: 80;
        anchors.topMargin: 65;
        anchors.bottomMargin: 75;

        fillColor: idMain.colorEkxConfirmGreen

        onSimplePressed:
        {
            if ( !editRecipeActive )
            {
                showEditListRecipe();
            }
        }

        onLongPressed:
        {
            showEditRecipe( recipeControl.currentTimeRecipeIndex )
        }
    }

    MouseArea
    {
        id: upperMouseArea
        anchors.fill: timeModeRecipeList
        anchors.leftMargin: 80;
        anchors.rightMargin: 80;
        anchors.topMargin: 65;
        anchors.bottomMargin: 75;

        enabled: !productRunning
        propagateComposedEvents: true

        onPressed:
        {
            mouse.accepted = false

            // start long press timer to enter edit mode
            if ( editRecipeAllowed )
            {
                idEditAnimation.start()
            }
        }

        onReleased:
        {
            idEditAnimation.stop()
        }
    }

    Rectangle {
        id: idDbgUpperMouseArea;
        visible: visualDebug;
        anchors.fill: upperMouseArea;
        color: "transparent";
        border.width: visualDebug?5:0;
        border.color: visualDebug?"pink":"transparent";
    }
    // top aligned DDD values
    DddAdjustment
    {
        id: adjustment

        z: 2;
        anchors.horizontalCenter: parent.horizontalCenter;
        showCurrentDdd: true
        currentDddValue: parseFloat(dddValueBig + "."+ dddValueSmall)
        anchors.top: parent.top
        opacity: 0

        visible: settingsSerializer.showDddEnable
                 && recipeList[recipeControl.currentTimeRecipeIndex].showCoarseness
                 && !editRecipeActive
                 && !editRecipeListActive

        onSaveCurrentDdd:
        {
            recipeControl.editTimeRecipe(recipeList[recipeControl.currentTimeRecipeIndex].recipeBigValue,
                                         recipeList[recipeControl.currentTimeRecipeIndex].recipeCommaValue,
                                         dddValueBig,
                                         dddValueSmall,
                                         portaFilter,
                                         recipeList[recipeControl.currentTimeRecipeIndex].recipeIconOn,
                                         recipeList[recipeControl.currentTimeRecipeIndex].showCoarseness,
                                         recipeControl.currentTimeRecipeIndex)
        }
    }

    // bottom aligned temperature symbol
    TemperatureView
    {
        id: temperatureView
        opacity: 0
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
    }

    FastBlur
    {
        id: blendOutblur
        z:1
        visible: true
        anchors.fill: parent;
        opacity: 1.0
        source: timeModeRecipeList
        radius: 55
    }

    // fade-over-text during mode change
    Text
    {
        id: blendoutText
        text: "Driver Mode"
        anchors.centerIn: parent
        opacity: 1
        color:  idMain.colorEkxTextWhite
        font.pixelSize: 58
        font.family:    "D-DIN Condensed HEMRO"
        font.bold:      true
        font.letterSpacing: 0.23
    }

    // fade-over animation during mode change
    ParallelAnimation
    {
        id: blendOut
        running: false
        NumberAnimation { target: blendOutblur; property: "radius"; to: 0; duration: 150 }
        NumberAnimation { target: blendOutblur; property: "opacity"; to: 0; duration: 150 }
        NumberAnimation { target: blendoutText; property: "opacity"; to: 0; duration: 150 }
        NumberAnimation { target: idPortaFilterIcon; property: "opacity"; to: 1; duration: 150 }
        NumberAnimation { target: timeModeRecipeList; property: "opacity"; to: 1; duration: 150 }
        NumberAnimation { target: adjustment; property: "opacity"; to: 1; duration: 150 }
        NumberAnimation { target: temperatureView; property: "opacity"; to: 1; duration: 150 }
        PropertyAnimation {
            property: "editRecipeAllowed"; to: true;
        }
        onFinished: {
            timeMode.interactive = true;
        }
    }

    ShadowGroup
    {
        shadowRightSize: 100;
        shadowRightThirdGradientStopEnable:   true;
        shadowRightThirdGradientStopPosition: 0.4;
        shadowRightThirdGradientStopOpacity:  0.6;

        shadowLeftSize: 115;
        shadowLeftThirdGradientStopEnable:   true;
        shadowLeftThirdGradientStopPosition: 0.4;
        shadowLeftThirdGradientStopOpacity:  0.6;

        //visualDebug: true;
    }
}
