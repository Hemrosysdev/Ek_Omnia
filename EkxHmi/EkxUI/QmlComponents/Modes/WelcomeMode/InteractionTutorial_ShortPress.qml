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

    backdrop: idRecipeList.visible
    modeIndicatorVisible: !backdrop

    onActivating: {
        idFinger.opacity = 0;
        idRecipeList.visible = false;

        recipeControl.currentTimeRecipeIndex = 0;
    }

    Finger {
        id: idFinger
    }

    TimeMode.RecipeListEdit {
        id: idRecipeList

        anchors.fill: parent
        visible: false
        // So the delegate sizes are uniform and predictable.
        showRecipeNames: false
    }

    component ClickOnRecipeAnimation : SequentialAnimation {
        id: idClickOnRecipeAnimation
        required property int recipeIndex
        property int delegateHeight: 96 // SwiableListItem default.

        PathAnimation {
            duration: 500
            easing.type: Easing.InOutQuad
            target: idFinger

            path: Path {
                startX: idFinger.x
                startY: idFinger.y

                PathQuad {
                    x: 100
                    y: 80 + idClickOnRecipeAnimation.delegateHeight * idClickOnRecipeAnimation.recipeIndex + (idClickOnRecipeAnimation.delegateHeight - idFinger.height) / 2
                    controlX: 100
                    controlY: idFinger.y
                }
            }
        }

        PauseAnimation {
            duration: 1000
        }

        Finger.ShortPressAnimation {
            target: idFinger
            onClicked: {
                recipeControl.currentTimeRecipeIndex = idClickOnRecipeAnimation.recipeIndex;
            }
        }

        PauseAnimation {
            duration: 500
        }

        ScriptAction {
            script: idRecipeList.visible = false
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

    // Click into list animation.
    SequentialAnimation {
        running: idTutorialPage.active

        PauseAnimation {
            duration: 1000
        }

        ScriptAction {
            script: idRecipeList.visible = false;
        }

        // Fly in finger to the center of the screen.
        Finger.FlyInAnimation {
            target: idFinger
            to: Qt.point(idTutorialPage.width / 2, idTutorialPage.height / 2)
        }

        SequentialAnimation {
            loops: Animation.Infinite

            PauseAnimation {
                duration: 1000
            }

            // Open "Time Mode Recipe Edit List" and wait a bit.
            Finger.ShortPressAnimation {
                target: idFinger
                onClicked: {
                    idRecipeList.visible = true;
                }
            }

            PauseAnimation {
                duration: 1000
            }

            ClickOnRecipeAnimation {
                // Since we operate on live data, if the tutorial is executed
                // after the fact and there is only one time recipe,
                // just click on the same one again.
                recipeIndex: Math.min(idRecipeList.listView.count - 1, 1)
            }

            PauseAnimation {
                duration: 1000
            }

            // Open "Time Mode Recipe Edit List" and wait a bit.
            Finger.ShortPressAnimation {
                target: idFinger
                onClicked: {
                    idRecipeList.visible = true;
                }
            }

            PauseAnimation {
                duration: 1000
            }

            ClickOnRecipeAnimation {
                recipeIndex: 0
            }

        }
    }

}
