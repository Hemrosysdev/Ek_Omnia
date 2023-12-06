import QtQuick 2.12

import ".." as Modes
import "../../CommonItems" as Common

import EkxMainstateEnum 1.0
import EkxSqliteTypes 1.0

Common.OverlayWorkflow
{
    id: idWorkflow

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // properties
    ///////////////////////////////////////////////////////////////////////////////////////////////

    objectName: "TutorialMode_InteractionTutorial"

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // signals
    ///////////////////////////////////////////////////////////////////////////////////////////////

    signal skipRequested

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // self defined properties
    ///////////////////////////////////////////////////////////////////////////////////////////////

    required property Modes.ModeSelector modeSelector

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // items
    ///////////////////////////////////////////////////////////////////////////////////////////////

    ModeDescriptionPage {
        id: interactionIntoPage
        modeSelector: idWorkflow.modeSelector
        text: `Learn the integrated
touch screen gestures!`
        skipButton: true
        onSkipClicked: {
            idWorkflow.skipRequested()
        }
    }

    ModeDescriptionPage {
        modeSelector: idWorkflow.modeSelector
        mode: EkxMainstateEnum.EKX_LIBRARY_MODE
        modeIndicatorVisible: false
        titleText: "Swipe"
        text: `Swipe vertically or
horizontally to reveal
menus and actions`
        showMeButton: true
    }

    InteractionTutorial_Swipe {
        modeSelector: idWorkflow.modeSelector
    }

    ModeDescriptionPage {
        modeSelector: idWorkflow.modeSelector
        modeIndicatorVisible: false
        titleText: "Tap"
        text: "Tap to select an item"
        showMeButton: true
    }

    InteractionTutorial_ShortPress {
        modeSelector: idWorkflow.modeSelector
    }

    ModeDescriptionPage {
        modeSelector: idWorkflow.modeSelector
        titleText: "Long press"
        text: `Long press to edit, save,
or delete`
        showMeButton: true
    }

    InteractionTutorial_LongPress {
        rightAction: Common.EkxButton.ButtonType.NEXT
        modeSelector: idWorkflow.modeSelector
    }
}
