import QtQuick 2.12

import ".." as Modes
import "../../CommonItems" as Common

import EkxMainstateEnum 1.0
import EkxSqliteTypes 1.0
import SettingsStateEnum 1.0

QtObject
{
    id: idTutorialMode

    required property string stage
    property int previousMainState: EkxMainstateEnum.EKX_CLASSIC_MODE;

    property bool returnToSettings

    required property Modes.OverlayView view
    required property Modes.ModeSelector modeSelector

    signal finished

    function executeStage(stage : string) {
        switch (stage) {
        case "initialSetup":
            view.show(initialSetup);
            break;
        case "modesTutorial":
            view.show(interactionTutorial);
            break;
        case "pduTutorial":
            view.show(pduTutorial);
            break;
        case "pduTutorialPrompt":
            view.show(pduTutorialPrompt);
            break;
        default:
            throw "Unknown stage " + stage;
        }
    }

    function confirmClose(event) {
        event.accepted = true;

        closePrompt.visible = true;
    }

    onFinished: {
        // Restore this at all cost!
        idTutorialMode.modeSelector.opacity = 1;

        idTutorialMode.modeSelector.blendOutEnabled = true;

        idTutorialMode.modeSelector.allModesEnabled = false;

        // Only return to known-good states.
        if ([EkxMainstateEnum.EKX_CLASSIC_MODE,
             EkxMainstateEnum.EKX_TIME_MODE,
             EkxMainstateEnum.EKX_LIBRARY_MODE,
             EkxMainstateEnum.EKX_SETTINGS_MODE].includes(previousMainState)) {
            mainStatemachine.ekxMainstate = previousMainState;
        } else {
            mainStatemachine.ekxMainstate = EkxMainstateEnum.EKX_CLASSIC_MODE;
        }

        if (returnToSettings) {
            mainStatemachine.ekxMainstate = EkxMainstateEnum.EKX_SETTINGS_MODE;
            settingsStateMachine.navigateToMenu(SettingsStateEnum.SETTING_INFO);
        }
    }

    Component.onCompleted: {
        executeStage(stage);
    }

    property InitialSetupWorkflow initialSetup: InitialSetupWorkflow {
        onTutorialRequested: {
            idTutorialMode.executeStage("modesTutorial");
        }
        onAboutToFinish: {
            idTutorialMode.view.reroute(doneQrCodePage);
        }
        onAboutToClose: {
            idTutorialMode.finished();
        }
    }

    property InteractionTutorialWorkflow interactionTutorial: InteractionTutorialWorkflow {
        modeSelector: idTutorialMode.modeSelector
        onAboutToFinish: {
            idTutorialMode.view.reroute(modesTutorial);
        }
        onAboutToClose: {
            idTutorialMode.finished();
        }
        onCloseClicked: (event) => {
            idTutorialMode.confirmClose(event);
        }
        onSkipRequested: {
            idTutorialMode.view.reroute(modesTutorial);
        }
    }

    property ModesTutorialWorkflow modesTutorial: ModesTutorialWorkflow {
        modeSelector: idTutorialMode.modeSelector
        onAboutToFinish: {
            idTutorialMode.view.reroute(pduTutorialPrompt);
        }
        onAboutToClose: {
            idTutorialMode.finished();
        }
        onCloseClicked: (event) => {
            idTutorialMode.confirmClose(event);
        }
        onSkipRequested: {
            idTutorialMode.view.reroute(pduTutorialPrompt);
        }
    }

    property PduTutorialWorkflow pduTutorial: PduTutorialWorkflow {
        modeSelector: idTutorialMode.modeSelector
        onAboutToFinish: {
            idTutorialMode.view.reroute(doneQrCodePage);
        }
        onAboutToClose: {
            idTutorialMode.finished();
        }
        onCloseClicked: (event) => {
            idTutorialMode.confirmClose(event);
        }
    }

    property PromptPage closePrompt: PromptPage {
        parent: idTutorialMode.view
        anchors.fill: parent
        text: `Do you want to
cancel the tutorial?`
        cancelLabel: "Yes, Cancel"
        confirmLabel: "No, go on!"
        visible: false

        onCancelClicked: { // Yes, cancel.
            idTutorialMode.view.close();
        }
        onConfirmClicked: { // No, go on.
            visible = false;
        }
    }

    property PromptPage pduTutorialPrompt: PromptPage {
        text: `Learn about the Pre-Dosing
Unit (PDU) or about why you
would need one`
        cancelLabel: "Let’s grind"
        confirmLabel: "PDU Intro"

        onCancelClicked: {
            idTutorialMode.view.reroute(doneQrCodePage);
        }
        onConfirmClicked: {
            idTutorialMode.executeStage("pduTutorial");
        }
    }

    // Final "Play around" page that is shown when leaving after *completing* a section.
    property DoneQrCodePage doneQrCodePage: DoneQrCodePage {
        onAboutToClose: {
            idTutorialMode.finished();
        }
    }

}
