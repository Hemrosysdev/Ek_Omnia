import QtQml 2.12
import QtQuick 2.12

import ".." as Modes
import "../../CommonItems" as Common

import EkxMainstateEnum 1.0

import com.hemrogroup.recipes 1.0

ModeDemoPage {
    id: idInteractionSwipePage

    mode: EkxMainstateEnum.EKX_LIBRARY_MODE

    onActivating: {
        idInteractionSwipeFinger.opacity = 0;
        // Blur is disabled since we explained the modes already
        // and performance is too bad otherwise.
        idWorkflow.modeSelector.blendOutEnabled = false;

        mode = EkxMainstateEnum.EKX_LIBRARY_MODE
    }

    Finger {
        id: idInteractionSwipeFinger
    }

    // Swipe up and down animation.
    SequentialAnimation {
        running: idInteractionSwipePage.active

        PauseAnimation {
            duration: 1000
        }

        Finger.FlyInAnimation {
            target: idInteractionSwipeFinger
            to: Qt.point(280, idInteractionSwipePage.height - 70)
        }

        PauseAnimation {
            duration: 1000
        }

        SequentialAnimation {
            loops: Animation.Infinite

            ScriptAction {
                script: idInteractionSwipePage.mode = EkxMainstateEnum.EKX_SETTINGS_MODE;
            }
            Finger.DragAnimation {
                target: idInteractionSwipeFinger
                axis: Qt.Vertical
                from: idInteractionSwipePage.height - 70
                to: 70
            }

            PauseAnimation {
                duration: 2000
            }

            ScriptAction {
                script: idInteractionSwipePage.mode = EkxMainstateEnum.EKX_LIBRARY_MODE;
            }
            Finger.DragAnimation {
                target: idInteractionSwipeFinger
                axis: Qt.Vertical
                from: 70
                to: idInteractionSwipePage.height - 70
            }

            PauseAnimation {
                duration: 2000
            }
        }
    }
}
