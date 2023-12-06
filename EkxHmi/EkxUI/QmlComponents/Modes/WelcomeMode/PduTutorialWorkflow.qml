import QtQuick 2.12
import QtQuick.Layouts 1.1

import ".." as Modes
import "../../CommonItems" as Common

import "../LibraryMode" as LibraryMode

import EkxMainstateEnum 1.0

import com.hemrogroup.recipes 1.0

Common.OverlayWorkflow
{
    id: idWorkflow

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // properties
    ///////////////////////////////////////////////////////////////////////////////////////////////

    objectName: "TutorialMode_PduTutorial"

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // self defined properties
    ///////////////////////////////////////////////////////////////////////////////////////////////

    required property Modes.ModeSelector modeSelector

    readonly property LibraryGrammage currentGrammage: {
        let grammage = null;
        const recipe = recipeControl.installedLibraryList[recipeControl.currentLibraryRecipeIndex];

        grammage = recipe.grammages[recipeControl.currentLibraryGrammageIndex];
        // Create a dummy grammage for the demo.
        if (!grammage) {
            grammage = recipeControl.createLibraryGrammage(recipe);
            grammage.grammageMg = 35200;
            grammage.dosingTimeMs = 17000;
            grammage.icon = 1;
        }

        return grammage;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // components
    ///////////////////////////////////////////////////////////////////////////////////////////////

    component GrammageDescriptionPage : ModeDescriptionPage {
        id: idGrammageDescriptionPage
        required property LibraryGrammage grammage
        property string highlightedSection: ""

        textVerticalCenterOffset: 43

        property alias dosingTimeVisible: idGrammage.dosingTimeVisible

        LibraryMode.Grammage {
            id: idGrammage

            // Used by the animation.
            property color highlightColor: "white"

            anchors {
                top: parent.top
                topMargin: 50
                horizontalCenter: parent.horizontalCenter
            }

            grammageMg: idGrammageDescriptionPage.grammage.grammageMg
            // Don't show a zero dosing time. We assume 2g/s, so just show half the grammage in doubt.
            dosingTimeMs: idGrammageDescriptionPage.grammage.dosingTimeMs || Math.round(idGrammageDescriptionPage.grammage.grammageMg / 2)
            iconSource: idWorkflow.modeSelector.libraryMode.icons[idGrammageDescriptionPage.grammage.icon]
            dosingTimeVisible: true

            gramageLabel.layer.enabled: highlightedSection === "grammage"
            gramageLabel.layer.samplerName: "maskSource"
            gramageLabel.layer.effect: Common.LongPressShaderEffect {
                id: idDosingTimeShaderEffect
                backgroundColor: idGrammage.highlightColor
                blending: true
            }

            dosingTimeLabel.layer.enabled: highlightedSection === "dosingTime"
            dosingTimeLabel.layer.samplerName: "maskSource"
            dosingTimeLabel.layer.effect: Common.LongPressShaderEffect {
                id: idLabelShaderEffect
                backgroundColor: idGrammage.highlightColor
                blending: true
            }

            Common.PulseAnimation {
                target: idGrammage
                property: "highlightColor"
                running: idGrammage.visible
            }
        }
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // items
    ///////////////////////////////////////////////////////////////////////////////////////////////

    ModeDescriptionPage {
        modeSelector: idWorkflow.modeSelector
        modeIndicatorVisible: false
        text: `The PDU is a volumetric
doser, and is only
available in Library mode`
        mode: EkxMainstateEnum.EKX_LIBRARY_MODE
        showMeButton: true
    }

    ModeDemoPage {
        id: idPduDemoPage
        modeSelector: idWorkflow.modeSelector
        mode: EkxMainstateEnum.EKX_LIBRARY_MODE
        // TODO select a recipe with grammages?

        onActivating: {
            idFinger.opacity = 0;

            modeSelector.libraryMode.swipableItem.reset();
        }

        Finger {
            id: idFinger
        }

        // Swipe left and right to reveal grammages animation.
        SequentialAnimation {
            running: idPduDemoPage.active

            PauseAnimation {
                duration: 1000
            }

            // Fly in finger to the center right of the screen.
            Finger.FlyInAnimation {
                target: idFinger
                to: Qt.point(idPduDemoPage.width * 0.66, idPduDemoPage.height / 2)
            }

            SequentialAnimation {
                loops: Animation.Infinite

                PauseAnimation {
                    duration: 1000
                }

                ScriptAction {
                    script: {
                        modeSelector.libraryMode.swipableItem.open();
                    }
                }
                Finger.DragAnimation {
                    target: idFinger
                    axis: Qt.Horizontal
                    from: idPduDemoPage.width * 0.66
                    to: idPduDemoPage.width * 0.33
                    duration: 150
                }

                PauseAnimation {
                    duration: 2000
                }

                ScriptAction {
                    script: {
                        modeSelector.libraryMode.swipableItem.close();
                    }
                }
                Finger.DragAnimation {
                    target: idFinger
                    axis: Qt.Horizontal
                    from: idPduDemoPage.width * 0.33
                    to: idPduDemoPage.width * 0.66
                    duration: 150
                }

                // Loops around to a total delay of 2000.
                PauseAnimation {
                    duration: 1000
                }
            }
        }
    }

    GrammageDescriptionPage {
        modeSelector: idWorkflow.modeSelector
        grammage: idWorkflow.currentGrammage
        text: `The grammage has no
functionality and acts
only as a note`
        highlightedSection: "grammage"
    }

    GrammageDescriptionPage {
        modeSelector: idWorkflow.modeSelector
        grammage: idWorkflow.currentGrammage
        text: `The time defines the PDU
dosing time to achieve
the grammage note weight`
        highlightedSection: "dosingTime"
    }

    GrammageDescriptionPage {
        modeSelector: idWorkflow.modeSelector
        grammage: idWorkflow.currentGrammage
        text: `Dosing time depends on
bean size and is
approximately 2g / s`
        highlightedSection: "dosingTime"
    }


    ModeDescriptionPage {
        modeSelector: idWorkflow.modeSelector

        GridLayout {
            anchors {
                fill: parent
                topMargin: 70
                leftMargin: 32
                rightMargin: 10
                bottomMargin: 80
            }
            rows: 5
            rowSpacing: 0
            columnSpacing: 0
            flow: GridLayout.TopToBottom

            Repeater {
                model: 5

                Common.Label {
                    Layout.fillWidth: true
                    text: index > 0 ? modelData + "." : ""
                    pixelSize: 43
                    font.letterSpacing: 1.48
                }
            }

            Repeater {
                model: [
                    "Grind process with PDU",
                    "Motor starts",
                    "PDU doses",
                    "Grinder grinds empty",
                    "Grinder stops"
                ]

                Common.Label {
                    Layout.fillWidth: true
                    text: modelData
                    pixelSize: 43
                    font.letterSpacing: 1.48
                    bold: index === 0
                }
            }
        }
    }

    GrammageDescriptionPage {
        modeSelector: idWorkflow.modeSelector
        grammage: idWorkflow.currentGrammage
        dosingTimeVisible: false
        text: `If your PDU is not available
you will only have access to
the grammage note`
    }

}
