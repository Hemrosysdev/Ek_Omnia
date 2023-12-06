import QtQml 2.12
import QtQuick 2.12
import QtQuick.Layouts 1.1

import ".."
import "../../CommonItems" as Common

import EkxMainstateEnum 1.0
import EkxSqliteTypes 1.0

import com.hemrogroup.recipes 1.0
import com.hemrogroup.utils 1.0 as Utils

ModePage
{
    id: libraryMode

    title: "Library Mode"
    stateId: EkxMainstateEnum.EKX_LIBRARY_MODE

    // To allow vertically scrolling grammages, lock mode changes.
    modeIsLocked: !idLibrarySwipableItem.atXBeginning
    showCurrentDdd: false

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // self defined properties
    ///////////////////////////////////////////////////////////////////////////////////////////////

    readonly property int currentLibraryRecipeIndex: recipeControl.currentLibraryRecipeIndex
    readonly property int currentGrammageIndex: recipeControl.currentLibraryGrammageIndex

    readonly property var libraryList: recipeControl.installedLibraryList;
    readonly property LibraryRecipe currentLibraryRecipe: libraryList[currentLibraryRecipeIndex]
    readonly property LibraryGrammage currentGrammage: settingsSerializer.pduInstalled ? (currentLibraryRecipe.grammages[currentGrammageIndex] || null) : null

    readonly property bool pduInstalled: settingsSerializer.pduInstalled
    readonly property bool showDosingTime: pduInstalled || settingsSerializer.showDosingTime

    readonly property bool doGrindByWeight: pduInstalled && libraryMode.currentGrammage

    readonly property var icons: [
        "ICON_Single-80px_Cup_transparent_nopadding",
        "ICON-Strokes_Single-80px_Filter_white_transparent_nopadding",
        "ICON-Strokes_Double-V2-80px_Filter_white_transparent_nopadding"
    ].map(name => "qrc:/Icons/" + name + ".png")

    // Tutorial needs to swipe this open/close.
    property alias swipableItem: idLibrarySwipableItem

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // functions
    ///////////////////////////////////////////////////////////////////////////////////////////////

    function showBeansList() {
        console.log(libraryLog, "Show beans list");

        idOverlayView.show(beansList);
    }

    function showGrammageList() {
        console.log(libraryLog, "Show grammage list");

        grammageList.recipe = libraryMode.currentLibraryRecipe;
        idOverlayView.show(grammageList);
    }

    function openRecipeEditor(recipe : LibraryRecipe, args) {
        console.log(libraryLog, "open recipe editor for", recipe);

        const editorArgs = args || {};

        editBeansWorkflow.recipe = recipe;

        editBeansWorkflow.beanName = recipe.beanName;
        editBeansWorkflow.grindingSize = recipe.grindingSize;

        editBeansWorkflow.openedFromList = editorArgs.openedFromList === true;

        editBeansWorkflow.initialPage = args.initialPage || null;

        idOverlayView.show(editBeansWorkflow);
    }

    function addNewGrammage(recipe : LibraryRecipe, args) {
        console.log(libraryLog, "add new grammage for", recipe);
        const newGrammage = recipeControl.createLibraryGrammage(recipe);
        if (newGrammage) {
            openGrammageEditor(recipe, newGrammage, args);
        }
    }

    function openGrammageEditor(recipe : LibraryRecipe, grammage : LibraryGrammage, args) {
        console.log(libraryLog, "open grammage editor for", recipe, "and", grammage);

        const editorArgs = args || {};

        editGrammageWorkflow.recipe = recipe;
        editGrammageWorkflow.grammage = grammage;

        editGrammageWorkflow.icon = grammage.icon;
        editGrammageWorkflow.grammageMg = grammage.grammageMg;
        editGrammageWorkflow.dosingTimeMs = grammage.dosingTimeMs;

        editGrammageWorkflow.openedFromList = editorArgs.openedFromList === true;

        idOverlayView.reroute(editGrammageWorkflow);
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // signal handlers
    ///////////////////////////////////////////////////////////////////////////////////////////////

    onDeactivated: {
        idLibrarySwipableItem.close();
    }

    onCurrentLibraryRecipeChanged: {
        idLibrarySwipableItem.close();
        // Avoid delayed AGSA on recipe change to scroll left when user immediately scrolled right.
        idDelayedCloseOnDddChangeTimer.restart();
    }

    onSaveCurrentDdd: {
        console.log(libraryLog, "Save DDD", dddValue, "on current bean");

        recipeControl.editLibraryRecipe(currentLibraryRecipe,
                                        currentLibraryRecipe.beanName,
                                        dddValue,
                                        currentLibraryRecipe.grindingSize);
    }

    onProductRunningChanged: {
        if (productRunning && libraryMode.doGrindByWeight) {
            console.log(libraryLog, "Started grind-by-weight, target", libraryMode.currentGrammage.grammageMg, "g, duration", libraryMode.currentGrammage.dosingTimeMs, "ms");
            idElapsedTimer.start();
        } else {
            idElapsedTimer.stop();
        }
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // items
    ///////////////////////////////////////////////////////////////////////////////////////////////

    LoggingCategory {
        id: libraryLog
        name: "ekx.library"
    }

    Connections {
        target: dddCouple
        // Scroll left so the DDD value is visible while changing.
        function onDddValueChanged() {
            // Wait a bit before closing it again since as it moves it changes continuously,
            // and also will move when clicking the DddModeDisplay or switching beans.
            if (!idDelayedCloseOnDddChangeTimer.running) {
                idLibrarySwipableItem.close();
            }
            idDelayedCloseOnDddChangeTimer.restart();
        }
    }

    Timer {
        id: idDelayedCloseOnDddChangeTimer
        // At least 2s but AGSA delay + some margin.
        interval: Math.max(2000, settingsSerializer.agsaStartDelaySec * 1000 + 150)
        repeat: false
    }

    Utils.ElapsedTimer {
        id: idElapsedTimer
        readonly property real slope: libraryMode.currentGrammage ? (libraryMode.currentGrammage.grammageMg / libraryMode.currentGrammage.dosingTimeMs) : 1
        readonly property int groundAmountMg: elapsed * slope
        reportInterval: Math.max(33, Math.min(100, slope * 100))

        onElapsedChanged: {
            if (running && elapsed >= libraryMode.currentGrammage.dosingTimeMs) {
                console.log(libraryLog, "Finished grind-by-weight, grammage reached, calculated", groundAmountMg, "g, target", libraryMode.currentGrammage.grammageMg, "g");
                mainStatemachine.grindRunning = false;
                idElapsedTimer.stop();
            }
        }
    }

    mainContent: [

        Common.SwipableListItem {
            id: idLibrarySwipableItem
            anchors.fill: parent
            padding: 80
            controlsMargin: 20
            controlsTail: 0
            closeWhenDeactivated: false
            enabled: !libraryMode.productRunning
            visible: !idTuple.visible
            onClicked: {
                idMain.showAgsaFineAdjustment()
            }
            onLongPressed: {
                libraryMode.openRecipeEditor(libraryMode.currentLibraryRecipe, {initialPage: editBeansCoarsenessPage});
            }

            Item {
                anchors.fill: parent

                DddValue {
                    anchors.centerIn: parent
                    visible: !idMain.isAgsaFineAdjustmentShown()
                    decimalValue: dddCouple.dddValue
                }
            }

            controls: ListView {
                id: idGrammageList
                // Smaller font makes for narrower text
                width: libraryMode.showDosingTime ? 220 : 290
                height: idLibrarySwipableItem.height
                highlightFollowsCurrentItem: true
                highlightRangeMode: ListView.ApplyRange
                preferredHighlightBegin: 80
                preferredHighlightEnd: 160
                highlightMoveDuration: 250
                topMargin: 80
                bottomMargin: 80
                spacing: 10
                currentIndex: libraryMode.currentGrammageIndex
                model: libraryMode.currentLibraryRecipe.grammages
                enabled: idLibrarySwipableItem.atXEnd && !libraryMode.productRunning
                // Otherwise a cup can be seen when switching modes up/down.
                clip: !idLibrarySwipableItem.atXEnd

                // Add grammage note hint when there are none
                Column {
                    id: idAddGrammageHintColumn
                    anchors.verticalCenter: parent.verticalCenter
                    width: idAddGrammageNoteLabel2.width
                    spacing: 0
                    visible: idGrammageList.count === 0

                    Image {
                        x: idAddGrammageNoteLabel1.x
                        // So that the plus icon doesn't move slightly different from the "add" label.
                        width: Math.max(72, idAddGrammageNoteLabel1.width)
                        height: 72
                        fillMode: Image.PreserveAspectFit
                        source: "qrc:/Icons/ICON-Strokes_Add-40px_white.png"
                    }

                    Common.Label {
                        id: idAddGrammageNoteLabel1
                        x: Math.max(0, Math.min(Math.round( (parent.width - width) / 2 ), idLibrarySwipableItem.contentX - idShadowGroup.shadowRightSize / 2))
                        text: "Add a"
                        pixelSize: 42
                        bold: true
                        font.letterSpacing: 1.33
                    }

                    Common.Label {
                        id: idAddGrammageNoteLabel2
                        text: "grammage note"
                        pixelSize: 42
                        bold: true
                        font.letterSpacing: 1.33
                    }
                }

                MouseArea {
                    anchors.fill: idAddGrammageHintColumn
                    visible: idAddGrammageHintColumn.visible
                    onClicked: {
                        libraryMode.addNewGrammage(libraryMode.currentLibraryRecipe);
                    }
                }

                footer: MouseArea {
                    width: idGrammageList.width
                    height: 72
                    visible: idGrammageList.count > 0 && idGrammageList.count < recipeControl.maxLibraryGrammageNum
                    onClicked: {
                        libraryMode.addNewGrammage(libraryMode.currentLibraryRecipe);
                    }

                    Image {
                        anchors.horizontalCenter: parent.horizontalCenter
                        width: 72
                        height: 72
                        source: "qrc:/Icons/ICON-Strokes_Add-40px_white.png"
                    }
                }

                delegate: Item {
                    id: idGrammageDelegate
                    width: idGrammageList.width
                    height: idGrammageItem.height

                    Common.LongPressAnimation {
                        id: idGrammageEditAnimation
                        fillColor: idMain.colorEkxConfirmGreen
                        onSimplePressed: {
                            const delta = Math.abs(idGrammageList.currentIndex - index);
                            recipeControl.currentLibraryGrammageIndex = index;
                            // HACK Skip highlight animation, which jerks back to the old index before moving to the new one...
                            if (delta > 1) {
                                idGrammageList.positionViewAtIndex(index, ListView.Center);
                            }
                        }
                        onLongPressed: {
                            const recipe = libraryMode.currentLibraryRecipe;
                            libraryMode.openGrammageEditor(recipe, recipe.grammages[index]);
                        }
                    }

                    Grammage {
                        id: idGrammageItem
                        opacity: idGrammageDelegate.ListView.isCurrentItem ? 1 : 0.5
                        width: parent.width - 25
                        grammageMg: modelData.grammageMg
                        dosingTimeMs: modelData.dosingTimeMs
                        iconSource: libraryMode.icons[modelData.icon]
                        dosingTimeVisible: libraryMode.showDosingTime
                        longPressAnimation: idGrammageEditAnimation
                    }
                }
            }
        },

        TupleValue {
            id: idTuple
            readonly property real displayGrammage: {
                let amount = 0;
                if (idElapsedTimer.running) {
                    amount = idElapsedTimer.groundAmountMg;
                } else if (libraryMode.currentGrammage) {
                    amount = libraryMode.currentGrammage.grammageMg;
                }
                return amount;
            }
            anchors.centerIn: parent
            showUnits: true
            centerXByHallbach: false
            centerXByMass: true
            unitString: "g"
            pixelSize: 200

            preDecimalValue: Math.floor(displayGrammage / 1000).toFixed(0)
            postDecimalValue: Math.floor(displayGrammage / 100 ) % 10

            visible: libraryMode.productRunning && libraryMode.doGrindByWeight
        },

        Common.ShadowGroup {
            id: idShadowGroup
            shadowTopSize: Common.ShadowGroup.ShadowTopSizes.StandardSize
            shadowTopEnabled: !idGrammageList.atYBeginning
            shadowRightSize: 100
            shadowRightEnabled: !idLibrarySwipableItem.atXEnd

            shadowLeftSize: 100

            shadowBottomSize: Common.ShadowGroup.ShadowBottomSizes.StandardSize
            // Cut off shadow sooner to avoid "new grammage" plus overlapping the bean name.
            shadowBottomThirdGradientStopEnable: true
            shadowBottomThirdGradientStopPosition: 0.4
            shadowBottomThirdGradientStopOpacity: 1
        }

    ]

    // Beans name label at the bottom of the screen.
    Common.Label {
        id: beansName

        anchors {
            bottom: parent.bottom
            bottomMargin: 10
            horizontalCenter: parent.horizontalCenter
        }
        text: libraryMode.currentLibraryRecipe.beanName
        textColor: Common.Label.Grey
        elide: Text.ElideRight
        width: Math.min(implicitWidth, parent.width)

        pixelSize: 58
        bold: true

        layer.enabled: idEditBeansNameAnimation.layerEnabled
        layer.smooth: true

        // This item should be used as the 'mask'
        layer.samplerName: "maskSource"
        layer.effect: Common.LongPressShaderEffect {
            longPressAnimation: idEditBeansNameAnimation
        }

        Common.LongPressAnimation {
            id: idEditBeansNameAnimation

            fillColor: idMain.colorEkxConfirmGreen
            enabled: !libraryMode.productRunning

            onSimplePressed: {
                showBeansList();
            }
            onLongPressed: {
                libraryMode.openRecipeEditor(libraryMode.currentLibraryRecipe, {initialPage: editBeansNamePage});
            }
        }
    }

    // Beans name for use in grammage editor overlays.
    Component {
        id: beansNameComponent

        Item {
            height: idBeansNameInnerLabel.height + idBeansNameInnerLabel.anchors.bottomMargin

            Common.ShadowGroup {
                shadowBottomSize: Common.ShadowGroup.ShadowBottomSizes.StandardSize
            }

            // TODO should it be possible to switch to beans list from here?
            Common.Label {
                id: idBeansNameInnerLabel
                anchors {
                    bottom: parent.bottom
                    bottomMargin: beansName.anchors.bottomMargin
                }
                width: parent.width
                horizontalAlignment: Text.AlignHCenter
                text: beansName.text
                textColor: beansName.textColor
                pixelSize: beansName.pixelSize
                bold: beansName.bold
                elide: beansName.elide
            }
        }
    }

    // Beans list.
    BeansListView {
        id: beansList

        onOpenRecipeEditor: {
            const recipe = libraryMode.libraryList[recipeIndex];
            libraryMode.openRecipeEditor(recipe, {openedFromList: true});
        }
        onAddNewRecipe: {
            const newRecipe = recipeControl.createLibraryRecipe();
            if (newRecipe) {
                libraryMode.openRecipeEditor(newRecipe, {openedFromList: true});
            }
        }
    }

    // Edit beans workflow.
    Common.OverlayWorkflow {
        id: editBeansWorkflow
        objectName: "LibraryMode_EditBeansWorkflow"

        property LibraryRecipe recipe: null

        property bool openedFromList: false
        // Hide delete button, disallow skipping.
        readonly property bool isNewItem: recipe ? !recipe.persisted : false

        property string beanName
        property int grindingSize

        onAboutToClose: {
            if (editBeansWorkflow.openedFromList) {
                event.accepted = true;
                libraryMode.showBeansList();
            }
        }

        onAboutToFinish: {
            if (isNewItem) {
                const newRecipeIndex = recipeControl.addLibraryRecipe(recipe);
                if (newRecipeIndex > -1) {
                    recipeControl.currentLibraryRecipeIndex = newRecipeIndex;
                }
            }

            recipeControl.editLibraryRecipe(recipe, editBeansWorkflow.beanName, editBeansWorkflow.grindingSize, recipe.msPerGram);
        }

        EditBeansNamePage {
            id: editBeansNamePage
            onRightActionLongPressed: {
                finish();
            }
        }
        EditCoarsenessPage {
            id: editBeansCoarsenessPage
            onRightActionLongPressed: {
                finish();
            }
        }
        GrammageNoteListPage {
            id: editGrammageNoteListPage
            recipe: editBeansWorkflow.recipe
            // Don't advance into editGrammageWorkflow.
            nextPage: null

            onRightActionLongPressed: {
                finish();
            }

            onAddNewGrammage: {
                libraryMode.addNewGrammage(recipe);
            }
            onEditGrammage: {
                const grammage = recipe.grammages[grammageIndex];
                libraryMode.openGrammageEditor(recipe, grammage);
            }
            onDeleteGrammate: {
                recipeControl.deleteLibraryGrammage(recipe, grammageIndex);
            }
        }
    }

    // Edit grammage note workflow.
    Common.OverlayWorkflow {
        id: editGrammageWorkflow
        objectName: "LibraryMode_EditGrammageWorkflow"

        property LibraryRecipe recipe: null
        property LibraryGrammage grammage: null

        property int icon
        property int grammageMg
        property int dosingTimeMs

        property bool openedFromList: false

        // Hide delete button, disallow skipping.
        readonly property bool isNewItem: grammage ? !grammage.persisted : false

        onAboutToClose: {
            if (editBeansWorkflow.openedFromList) {
                event.accepted = true;
                libraryMode.showBeansList();
            }
        }

        onAboutToFinish: {
            if (isNewItem) {
                const newGrammageIndex = recipeControl.addLibraryGrammage(recipe, grammage);
                if (newGrammageIndex > -1) {
                    recipeControl.currentLibraryGrammageIndex = newGrammageIndex;
                }
            }

            recipeControl.editLibraryGrammage(grammage, icon, grammageMg, dosingTimeMs);

            // If we came from editBeansWorkflow, go back there.
            if (editBeansWorkflow.inUse) {
                event.accepted = true;
                idOverlayView.go(editGrammageNoteListPage);
            }
        }

        // Edit grammage.
        EditGrammageNotePage {
            id: editGrammageNotePage
            // TODO disable rightActionLongPress if dosing time is zero.
            onRightActionLongPressed: {
                finish();
            }
            onTopActionLongPressed: {
                // no findIndex on QML list property...
                const grammages = editGrammageWorkflow.recipe.grammages;
                for (let i = 0; i < grammages.length; ++i) {
                    if (grammages[i] === editGrammageWorkflow.grammage) {
                        recipeControl.deleteLibraryGrammage(editGrammageWorkflow.recipe, i);
                        finish();
                        break;
                    }
                }
            }
        }
        EditGrammageDosingTimePage {
            id: editGrammageDosingTimePage
            navigable: libraryMode.showDosingTime
            onRightActionLongPressed: {
                finish();
            }
        }
    }

}
