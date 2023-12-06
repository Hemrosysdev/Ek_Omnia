import QtQuick 2.12

import "../../CommonItems" as Common

import EkxMainstateEnum 1.0

import com.hemrogroup.recipes 1.0

ModeDemoPage
{
    id: idTutorialPage

    mode: EkxMainstateEnum.EKX_TIME_MODE

    onActivating: {
        idFinger.opacity = 0;
        recipeControl.currentTimeRecipeIndex = 0;
    }

    onDeactivating: {
        recipeControl.agsaAllowed = false;
    }

    Finger {
        id: idFinger
    }

    component AgsaSwipeAnimation : SequentialAnimation {
        id: idAgsaSwipeAnimation
        required property int recipeIndex
        required property int direction
        property int delay: 2000

        ScriptAction {
            script: recipeControl.currentTimeRecipeIndex = idAgsaSwipeAnimation.recipeIndex
        }

        Finger.DragAnimation {
            target: idFinger
            axis: Qt.Horizontal
            from: idTutorialPage.width * (idAgsaSwipeAnimation.direction === Qt.RightToLeft ? 0.66 : 0.33)
            to: idTutorialPage.width * (idAgsaSwipeAnimation.direction === Qt.RightToLeft ? 0.33 : 0.66)
        }

        PauseAnimation {
            duration: idAgsaSwipeAnimation.delay
        }
    }

    // Swipe Time mode recipes animation.
    SequentialAnimation {
        id: idAgsaAnimation
        running: idTutorialPage.active

        PauseAnimation {
            duration: 1000
        }

        // Fly in finger to the center right of the screen.
        Finger.FlyInAnimation {
            target: idFinger
            to: Qt.point(idTutorialPage.width * 0.66, idTutorialPage.height / 2)
        }

        ScriptAction {
            script: recipeControl.agsaAllowed = true;
        }

        PauseAnimation {
            duration: 1000
        }

        // Demonstrate the real AGSA once, then just swipe to reduce wear and tear.
        AgsaSwipeAnimation {
            recipeIndex: 1
            direction: Qt.RightToLeft
            // Don't wait if AGSA wouldn't run or we don't have a recipe to switch to.
            delay: settingsSerializer.agsaEnabled && recipeControl.currentTimeRecipeCount > 1 ? (settingsSerializer.agsaStartDelaySec * 1000 + 5000) : 2000
        }
        AgsaSwipeAnimation {
            recipeIndex: 0
            direction: Qt.LeftToRight
            delay: settingsSerializer.agsaEnabled && recipeControl.currentTimeRecipeCount > 1 ? (settingsSerializer.agsaStartDelaySec * 1000 + 5000) : 2000
        }

        ScriptAction {
            script: recipeControl.agsaAllowed = false;
        }

        SequentialAnimation {
            loops: Animation.Infinite

            AgsaSwipeAnimation {
                recipeIndex: 1
                direction: Qt.RightToLeft
            }

            AgsaSwipeAnimation {
                recipeIndex: 0
                direction: Qt.LeftToRight
            }
        }

    }

}
