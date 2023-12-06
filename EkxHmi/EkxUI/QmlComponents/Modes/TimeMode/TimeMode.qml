import QtQml 2.12
import QtQuick 2.12
import QtQml.Models 2.1
import QtGraphicalEffects 1.12
import ".."
import "../../CommonItems" as Common

import EkxMainstateEnum 1.0
import SettingsStateEnum 1.0

import com.hemrogroup.recipes 1.0

ModePage
{
    id: timeMode

    title: "Time Mode"
    stateId: EkxMainstateEnum.EKX_TIME_MODE

    showDdd: currentTimeRecipe.showCoarseness
    showCurrentDdd: true

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // self defined properties
    ///////////////////////////////////////////////////////////////////////////////////////////////

    property int currentEkxMainstate: mainStatemachine.ekxMainstate
    property int currentRecipeIndex: recipeControl.currentTimeRecipeIndex
    readonly property TimeRecipe currentTimeRecipe: recipeList[currentRecipeIndex]

    // RecipeValues

    property int runningBig: 0
    property int runningComma: 0
    property bool productRunning: mainStatemachine.grindRunning;
    readonly property var portaFilterImages: [
        "qrc:/Icons/ICON-Strokes_Single-80px_portafilter_white.png",
        "qrc:/Icons/ICON-Strokes_Double-80px_portafilter_white.png",
        "qrc:/Icons/ICON-Strokes_Bottomless-80px_portafilter_white.png"]

    property var recipeList: recipeControl.installedRecipeList;

    property int portaFilter:  recipeList[recipeControl.currentTimeRecipeIndex] ? recipeList[recipeControl.currentTimeRecipeIndex].portaFilterIndex : 0;

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // functions
    ///////////////////////////////////////////////////////////////////////////////////////////////

    function showRecipeList() {
        idOverlayView.show(recipeListEdit);
    }

    function openRecipeEditor(recipe, args) {
        editRecipeWorkflow.recipe = recipe;

        editRecipeWorkflow.recipeBig = recipe.recipeBigValue;
        editRecipeWorkflow.recipeComma = recipe.recipeCommaValue;
        editRecipeWorkflow.grindingSize = recipe.grindingSize;
        editRecipeWorkflow.portaFilter = recipe.portaFilterIndex;
        editRecipeWorkflow.iconOn = recipe.recipeIconOn;
        editRecipeWorkflow.showCoarseness = recipe.showCoarseness;
        editRecipeWorkflow.recipeName = recipe.recipeName;
        editRecipeWorkflow.showRecipeName = recipe.showRecipeName;

        const editorArgs = args || {};
        editRecipeWorkflow.openedFromList = editorArgs.openedFromList === true;
        editRecipeWorkflow.initialPage = editorArgs.initialPage || null;

        // TODO editorArgs.initialPage?

        idOverlayView.show(editRecipeWorkflow);
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // signal handlers
    ///////////////////////////////////////////////////////////////////////////////////////////////

    onCurrentTimeRecipeChanged: {
        if (currentTimeRecipe.recipeName && currentTimeRecipe.showRecipeName) {
            idFlashRecipeNameTimer.restart();
        } else {
            idFlashRecipeNameTimer.stop();
        }
    }

    onIsCurrentPageChanged: {
        if (isCurrentPage && currentTimeRecipe.recipeName && currentTimeRecipe.showRecipeName) {
            idFlashRecipeNameTimer.restart();
        } else {
            idFlashRecipeNameTimer.stop();
        }
    }

    onSaveCurrentDdd: {
        recipeControl.editTimeRecipe(currentTimeRecipe,
                                     currentTimeRecipe.recipeBigValue,
                                     currentTimeRecipe.recipeCommaValue,
                                     dddValue,
                                     currentTimeRecipe.portaFilterIndex,
                                     currentTimeRecipe.recipeIconOn,
                                     currentTimeRecipe.showCoarseness,
                                     currentTimeRecipe.recipeName,
                                     currentTimeRecipe.showRecipeName);
    }

    onProductRunningChanged: {
        if (productRunning && isCurrentPage) {
            timeModeTimer.start();
        } else {
            timeModeTimer.resetTimeModeTimer();
        }
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // items
    ///////////////////////////////////////////////////////////////////////////////////////////////

    Timer {
        id: idFlashRecipeNameTimer
        interval: 2000
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

            if ( ( runningBig == endBigValue ) && ( runningComma == endCommaValue ) )
            {
                resetTimeModeTimer();
                mainStatemachine.grindRunning = false;
            }
        }
    }

    Common.OverlayWorkflow {
        id: editRecipeWorkflow

        property TimeRecipe recipe: null

        property bool openedFromList: false
        // Hide delete button, disallow skipping.
        readonly property bool isNewItem: recipe ? !recipe.persisted : false

        property int recipeBig
        property int recipeComma
        property int grindingSize
        property int portaFilter
        property bool iconOn
        property bool showCoarseness
        property string recipeName
        property bool showRecipeName

        function saveChanges(event) {
            event.accepted = true;

            if (isNewItem) {
                const newRecipeIndex = recipeControl.addTimeRecipe(recipe);
                if (newRecipeIndex > -1) {
                    recipeControl.currentTimeRecipeIndex = newRecipeIndex;
                }
            }

            recipeControl.editTimeRecipe(recipe, recipeBig, recipeComma,
                                         grindingSize, portaFilter,
                                         iconOn, showCoarseness,
                                         recipeName, showRecipeName);

            finish();
        }

        function finish() {
            if (openedFromList) {
                showRecipeList();
            } else {
                if (recipe.recipeName && recipe.showRecipeName) {
                    idFlashRecipeNameTimer.restart();
                }

                idOverlayView.close();
            }
        }

        EditDurationPage {
            onAboutToClose: {
                if (editRecipeWorkflow.openedFromList) {
                    event.accepted = true;
                    timeMode.showRecipeList();
                }
            }
            onRightActionLongPressed: {
                editRecipeWorkflow.saveChanges(event);
            }
            onTopActionLongPressed: {
                // no findIndex on QML list property...
                for (let i = 0; i < timeMode.recipeList.length; ++i) {
                    if (timeMode.recipeList[i] === editRecipeWorkflow.recipe) {
                        recipeControl.deleteTimeRecipe(i);
                        editRecipeWorkflow.finish();
                        break;
                    }
                }
            }
        }
        EditCoarsenessPage {
            onAboutToClose: {
                if (editRecipeWorkflow.openedFromList) {
                    event.accepted = true;
                    timeMode.showRecipeList();
                }
            }
            onRightActionLongPressed: {
                editRecipeWorkflow.saveChanges(event);
            }
        }
        EditRecipeAppearancePage {
            onAboutToClose: {
                if (editRecipeWorkflow.openedFromList) {
                    event.accepted = true;
                    timeMode.showRecipeList();
                }
            }
            onRightActionLongPressed: {
                editRecipeWorkflow.saveChanges(event);
            }
        }
        EditRecipeNamePage {
            id: editRecipeNamePage
            onAboutToClose: {
                if (editRecipeWorkflow.openedFromList) {
                    event.accepted = true;
                    timeMode.showRecipeList();
                }
            }
            onRightActionClicked: {
                editRecipeWorkflow.saveChanges(event);
            }
            onRightActionLongPressed: {
                editRecipeWorkflow.saveChanges(event);
            }
        }
    }

    RecipeListEdit
    {
        id: recipeListEdit

        onSelectRecipe: {
            recipeControl.currentTimeRecipeIndex = recipeIndex;
            timeModeRecipeList.positionViewAtIndex(recipeIndex, ListView.Visible);
            idOverlayView.close();
        }
        onOpenRecipeEditor: {
            const recipe = timeMode.recipeList[recipeIndex];
            timeMode.openRecipeEditor(recipe, {openedFromList: true});
        }
        onAddNewRecipe: {
            const newRecipe = recipeControl.createTimeRecipe();
            if (newRecipe) {
                timeMode.openRecipeEditor(newRecipe, {openedFromList: true});
            }
        }
    }

    // horizontal list view for recipes
    mainContent: [
        ListView {
            id: timeModeRecipeList;

            anchors.fill: parent;
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
            interactive: !timeMode.productRunning

            currentIndex: recipeControl.currentTimeRecipeIndex

            model: recipeList

            delegate: Item
            {
                id: idListItem;
                //width: 480;
                width: 480-60;
                height: 360;

                // NOTE Make sure to adjust TutorialMode/ModesTutorialWorkflow.qml if you change this.
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

                Common.LongPressAnimation
                {
                    id: idEditAnimation

                    enabled: !timeMode.productRunning

                    anchors.fill: parent
                    anchors.leftMargin: 40; // 100 - 60 (preferredHighlightBegin)
                    anchors.rightMargin: 80;
                    anchors.topMargin: 65;
                    anchors.bottomMargin: 75;

                    fillColor: idMain.colorEkxConfirmGreen

                    onSimplePressed:
                    {
                        showRecipeList();
                    }

                    onLongPressed:
                    {
                        timeMode.openRecipeEditor(timeMode.currentTimeRecipe);
                    }
                }
            }

            onVisibleChanged: {
                if (visible) {
                    timeModeRecipeList.positionViewAtIndex(recipeControl.currentTimeRecipeIndex, ListView.Visible);
                }
            }

            onMovementEnded: {
                recipeControl.currentTimeRecipeIndex = currentIndex;
            }

            onModelChanged: {
                // When model is reset, ListView resets its current index to zero.
                timeModeRecipeList.currentIndex = Qt.binding(function() {
                    return recipeControl.currentTimeRecipeIndex;
                });
                timeModeRecipeList.positionViewAtIndex(recipeControl.currentTimeRecipeIndex, ListView.Visible);
            }
        },

        Common.ShadowGroup
        {
            shadowRightSize: 100;
            shadowRightThirdGradientStopEnable:   true;
            shadowRightThirdGradientStopPosition: 0.4;
            shadowRightThirdGradientStopOpacity:  0.6;

            shadowLeftSize: 115;
            shadowLeftThirdGradientStopEnable:   true;
            shadowLeftThirdGradientStopPosition: 0.4;
            shadowLeftThirdGradientStopOpacity:  0.6;
        }
    ]

    // right aligned porta filter icon
    Image
    {
        id: idPortaFilterIcon
        z:2
        visible: timeMode.currentTimeRecipe.recipeIconOn
        width: 80
        height: 80
        opacity: timeModeRecipeList.moving ? 0 : 1
        anchors.right: parent.right
        anchors.verticalCenter:  parent.verticalCenter
        source: timeMode.portaFilterImages[timeMode.currentTimeRecipe.portaFilterIndex]
        Behavior on opacity {
            NumberAnimation {
                duration: 200
            }
        }
    }

    // bottom aligned temperature symbol
    TemperatureView
    {
        id: temperatureView
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        opacity: idRecipeNameLabel.opacity === 0 && !idFlashRecipeNameTimer.running ? 1 : 0
        visible: opacity > 0
        Behavior on opacity {
            NumberAnimation {
                duration: 200
            }
        }
    }

    // Recipe name during transition.
    Common.Label {
        id: idRecipeNameLabel
        anchors {
            bottom: parent.bottom
            bottomMargin: 10
            horizontalCenter: parent.horizontalCenter
        }
        text: timeMode.currentTimeRecipe.recipeName
        textColor: Common.Label.Grey
        elide: Text.ElideRight
        width: parent.width
        horizontalAlignment: Text.AlignHCenter

        pixelSize: 58
        bold: true
        opacity: {
            const recipe = timeMode.currentTimeRecipe;
            if (!recipe || !recipe.showRecipeName || !recipe.recipeName) {
                return 0;
            }

            // Fade out the name while scrolling, matching the porta filter icon.
            if (timeModeRecipeList.moving) {
                return 0;
            }

            // If temperature display is enabled, flash it.
            if (settingsSerializer.showTempEnable && !idFlashRecipeNameTimer.running) {
                return 0;
            }

            // Without temperature display, we can show the name permanently.
            return 1;
        }
        visible: opacity > 0
        Behavior on opacity {
            NumberAnimation {
                duration: 200
            }
        }

        layer.enabled: idEditRecipeNameAnimation.layerEnabled
        layer.samplerName: "maskSource"
        layer.effect: Common.LongPressShaderEffect {
            longPressAnimation: idEditRecipeNameAnimation
        }

        Common.LongPressAnimation {
            id: idEditRecipeNameAnimation

            fillColor: idMain.colorEkxConfirmGreen
            enabled: !libraryMode.productRunning

            onStarted: {
                // Prolong display of label for better interaction with it.
                idFlashRecipeNameTimer.restart();
            }
            onSimplePressed: {
                timeMode.showRecipeList();
            }
            onLongPressed: {
                timeMode.openRecipeEditor(timeMode.currentTimeRecipe, {initialPage: editRecipeNamePage});
            }
        }
    }

}
