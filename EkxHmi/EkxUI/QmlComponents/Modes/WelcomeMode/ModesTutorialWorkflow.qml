import QtQuick 2.12

import ".." as Modes
import "../../CommonItems" as Common

import EkxMainstateEnum 1.0

Common.OverlayWorkflow
{
    id: idWorkflow

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // properties
    ///////////////////////////////////////////////////////////////////////////////////////////////

    objectName: "TutorialMode_ModesTutorial"

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
        modeSelector: idWorkflow.modeSelector
        titleText: "Grinding Modes"
        text: `Learn about the Ek Omnia
grinding modes`
        skipButton: true
        // Force our demo to start in classic mode.
        mode: EkxMainstateEnum.EKX_CLASSIC_MODE
        modeIndicatorVisible: false

        onSkipClicked: {
            idWorkflow.skipRequested()
        }
    }

    ModeDescriptionPage {
        modeSelector: idWorkflow.modeSelector
        titleText: "Classic Mode"
        mode: EkxMainstateEnum.EKX_CLASSIC_MODE
        text: `Grind with manual
start-stop`
        showMeButton: true
    }

    ModeDemoPage {
        modeSelector: idWorkflow.modeSelector
        mode: EkxMainstateEnum.EKX_CLASSIC_MODE
    }

    ModeDescriptionPage {
        modeSelector: idWorkflow.modeSelector
        titleText: "Grind-by-time Mode"
        mode: EkxMainstateEnum.EKX_TIME_MODE
        text: `Grind with your stored
dosage time`
        showMeButton: true
    }

    ModeDemoPage {
        modeSelector: idWorkflow.modeSelector
        mode: EkxMainstateEnum.EKX_TIME_MODE
    }

    ModeDescriptionPage {
        modeSelector: idWorkflow.modeSelector
        titleText: "Library Mode"
        mode: EkxMainstateEnum.EKX_LIBRARY_MODE
        text: `Grind with your stored
recipe settings`
        showMeButton: true
    }

    ModeDemoPage {
        modeSelector: idWorkflow.modeSelector
        mode: EkxMainstateEnum.EKX_LIBRARY_MODE
    }

    ModeDescriptionPage {
        modeSelector: idWorkflow.modeSelector
        titleText: "Grind Size Adjustment (GSA)"
        text: `Distance adjustment
between discs is possible
in three ways`
    }

    ModeDescriptionPage {
        modeSelector: idWorkflow.modeSelector
        titleText: "Classic GSA"
        text: `Use the main knob to set
the distance between discs`
    }

    ModeDescriptionPage {
        modeSelector: idWorkflow.modeSelector
        titleText: "Touch GSA"
        text: `Use the touch function to set
the distance between discs`
        showMeButton: true
    }

    InteractionTutorial_Ddd {
        modeSelector: idWorkflow.modeSelector
    }

    ModeDescriptionPage {
        modeSelector: idWorkflow.modeSelector
        titleText: "Automatic GSA"
        text: `Adjustment through a
stored disc distance`
        showMeButton: true
    }

    InteractionTutorial_Agsa {
        modeSelector: idWorkflow.modeSelector
    }
}
