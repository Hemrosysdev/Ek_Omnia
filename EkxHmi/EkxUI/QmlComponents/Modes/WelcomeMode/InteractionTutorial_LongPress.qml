import QtQuick 2.12

import ".." as Modes
import "../../CommonItems" as Common
import "../TimeMode" as TimeMode

import EkxMainstateEnum 1.0

import com.hemrogroup.recipes 1.0

ModeDemoPage
{
    id: idTutorialPage

    mode: EkxMainstateEnum.EKX_TIME_MODE

    backdrop: idEditDurationPage.visible
    modeIndicatorVisible: !backdrop

    rightActionLongPressColor: idMain.colorEkxConfirmGreen

    onActivating: {
        idFinger.opacity = 0;
        idEditDurationPage.visible = false;
        idLongPressAnimation.abort();
        idTutorialPage.rightActionLongPressAnimation.abort();

        const recipe = recipeControl.installedRecipeList[recipeControl.currentTimeRecipeIndex];
        idEditRecipeWorkflow.recipeBig = recipe.recipeBigValue;
        idEditRecipeWorkflow.recipeComma = recipe.recipeCommaValue;
    }

    Finger {
        id: idFinger
    }

    // Fake TimeMode.
    // NOTE Make sure to adjust TimeMode.qml if you change this.
    Item {
        width: 480-60
        height: 360
        // preferredHighlightBegin: 60
        x: 60
        visible: !idEditDurationPage.visible

        Modes.TupleValue {
            id: idLongPressTuple;
            showUnits: true
            anchors.verticalCenter: parent.verticalCenter;
            //anchors.horizontalCenter: parent.horizontalCenter;
            x: 480/2-width/2-60;
            centerXByHallbach: false;
            centerXByMass: true;

            preDecimalValue: idEditRecipeWorkflow.recipeBig
            postDecimalValue: idEditRecipeWorkflow.recipeComma

            tupleAnimation: idLongPressAnimation

            Rectangle {
                y: 20
                width: parent.width
                height: parent.height - y * 2
                // Avoid fringed edges since we just overlay the exisitng number.
                color: "black"
                z: -2
            }
        }

        Common.LongPressAnimation {
            id: idLongPressAnimation
            fillColor: idMain.colorEkxConfirmGreen
            // One of the rare cases we actually need alpha-blending in our effect,
            // since we see through to the TimeMode item and the TupleValue slightly
            // overlaps the header.
            blending: true
        }
    }

    // Fake TimeMode duration edit page.
    Common.OverlayWorkflow {
        id: idEditRecipeWorkflow
        // Properties accessed by the EditDurationPage.
        property int recipeBig
        property int recipeComma

        TimeMode.EditDurationPage {
            id: idEditDurationPage
            anchors.fill: parent
            // OverlayWorkflow is a non-visible item.
            // Manually parent it to our page so it can be displayed.
            parent: idTutorialPage
            visible: false

            // Ensure all handlers are run as if the page became visible in the OverlayView.
            onVisibleChanged: {
                activated();
            }
        }
    }

    // Long-press for edit animation.
    SequentialAnimation {
        running: idTutorialPage.active

        PauseAnimation {
            duration: 1000
        }

        ScriptAction {
            script: idEditDurationPage.visible = false;
        }

        // Fly in finger to the center of the screen.
        Finger.FlyInAnimation {
            target: idFinger
            to: Qt.point(idTutorialPage.width / 2, idTutorialPage.height / 2)
        }

        SequentialAnimation {
            loops: Animation.Infinite

            PauseAnimation {
                duration: 500
            }

            // Long-press on the number.
            Finger.LongPressAnimation {
                target: idFinger
                longPressAnimation: idLongPressAnimation
                onLongPressed: {
                    idEditDurationPage.visible = true;
                }
            }

            PauseAnimation {
                duration: 1000
            }

            ScriptAction {
                script: {
                    // incrementCurrentIndex does not trigger currentValue signal...
                    idEditDurationPage.preDecimalNumberWheel.incByNeighbour();
                }
            }

            // Flick downwards to change the integer number.
            Finger.DragAnimation {
                target: idFinger
                axis: Qt.Vertical
                from: idTutorialPage.height / 2
                to: idTutorialPage.height * 0.8
            }

            PauseAnimation {
                duration: 1000
            }

            // Move finger to bottom right "next" icon.
            PathAnimation {
                duration: 500
                easing.type: Easing.InOutQuad
                target: idFinger

                path: Path {
                    startX: idFinger.x
                    startY: idFinger.y

                    PathQuad {
                        x: idEditDurationPage.width - idFinger.width - 10
                        y: idEditDurationPage.height - idFinger.height - 8
                        controlX: idEditDurationPage.width - idFinger.width * 2
                        controlY: idFinger.y
                    }
                }
            }

            PauseAnimation {
                duration: 1000
            }

            // Need to enable this for the animation to render in OverlayView.
            ScriptAction {
                script: {
                    idTutorialPage.rightActionLongPressEnabled = true;
                    // The page isn't in the OverlayView, so the automatic signal stuff doesn't work.
                    idEditDurationPage.longPressAnimationStarted(idTutorialPage.rightActionLongPressAnimation);
                }
            }

            // "Long press" on the next icon.
            Finger.LongPressAnimation {
                target: idFinger
                longPressAnimation: idTutorialPage.rightActionLongPressAnimation
                onLongPressed: {
                    // "Save" new value.
                    idEditDurationPage.visible = false;

                    idTutorialPage.rightActionLongPressEnabled = false;
                }
            }

            PauseAnimation {
                duration: 500
            }

            // Move cursor back to the middle of the screen.
            PathAnimation {
                duration: 500
                target: idFinger

                path: Path {
                    startX: idFinger.x
                    startY: idFinger.y

                    PathQuad {
                        x: (idTutorialPage.width - idFinger.width) / 2
                        y: (idTutorialPage.height - idFinger.height) / 2
                        controlX:  idFinger.x
                        controlY: idFinger.y
                    }
                }
            }

        }
    }

}
