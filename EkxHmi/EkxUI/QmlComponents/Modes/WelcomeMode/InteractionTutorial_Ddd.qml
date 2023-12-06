import QtQuick 2.12

import ".." as Modes
import "../../CommonItems" as Common

import EkxMainstateEnum 1.0

import com.hemrogroup.recipes 1.0

ModeDemoPage
{
    id: idTutorialPage

    readonly property int recipeGrindingSize: {
        // Make sure there's always an offset to the Ddd
        return (idDddModeDisplay.currentDddValue + 123) % 800; // TODO use CONSTANT property.
    }

    mode: EkxMainstateEnum.EKX_TIME_MODE

    onActivating: {
        idFinger.opacity = 0;
        idDddModeDisplay.currentGrindingSize = idTutorialPage.recipeGrindingSize;
        idLongPressAnimation.abort();
    }

    Finger {
        id: idFinger
    }

    // Fake DddModeDisplay
    // NOTE Make sure to adjust TimeMode.qml if you change this.

    Rectangle {
        width: parent.width
        height: idDddModeDisplay.height
        color: "black"

        Modes.DddModeDisplay {
            id: idDddModeDisplay
            longPressAnimationInt: idLongPressAnimation
        }
    }

    Common.LongPressAnimation {
        id: idLongPressAnimation
        fillColor: idMain.colorEkxConfirmGreen
    }

    // Long-press for save DDD animation.
    SequentialAnimation {
        running: idTutorialPage.active
        loops: Animation.Infinite

        PauseAnimation {
            duration: 1000
        }

        ScriptAction {
            script: idDddModeDisplay.currentGrindingSize = idTutorialPage.recipeGrindingSize
        }

        // Fly in finger to the center of the screen.
        Finger.FlyInAnimation {
            target: idFinger
            to: {
                let point = Qt.point(idTutorialPage.width / 2, idDddModeDisplay.height / 2)
                if (idDddModeDisplay.recipeValueIsSmaller) {
                    point.x = idTutorialPage.width * 0.25;
                } else if (idDddModeDisplay.recipeValueIsGreater) {
                    point.x = idTutorialPage.width * 0.75;
                }
                return point;
            }
        }

        PauseAnimation {
            duration: 500
        }

        Finger.PressAction {
            target: idFinger
            pressed: true
        }

        ScriptAction {
            script: idLongPressAnimation.start()
        }

        PauseAnimation {
            duration: idLongPressAnimation.delay + idLongPressAnimation.duration
        }

        PauseAnimation {
            // Make sure the glow is noticed.
            duration: 500
        }

        Finger.PressAction {
            target: idFinger
            pressed: false
        }

        PauseAnimation {
            duration: idFinger.pressAnimationDuration
        }

        ScriptAction {
            script: {
                idLongPressAnimation.abort();

                idDddModeDisplay.currentGrindingSize = idDddModeDisplay.currentDddValue;
            }
        }

        PauseAnimation {
            duration: 1000
        }
    }

}
