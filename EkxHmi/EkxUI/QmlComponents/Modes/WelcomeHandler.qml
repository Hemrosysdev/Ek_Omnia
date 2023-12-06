import QtQml 2.12
import QtQuick 2.12

import "../CommonItems" as Common

QtObject
{
    id: idWelcomeHandler

    // Component is loaded on-demand, can't use real type here.
    property QtObject welcomeMode: null

    function start(args) {
        let initialProperties = args || {};

        initialProperties.view = idOverlayView;
        initialProperties.modeSelector =  idModeSelector;
        initialProperties.previousMainState = mainStatemachine.ekxMainstate;

        console.log("WelcomeHandler: start", initialProperties.stage);

        if (welcomeMode) {
            throw "A tutorial is already running";
        }

        const welcomeComponent = Qt.createComponent("WelcomeMode/WelcomeMode.qml");
        if (welcomeComponent.status === Component.Error) {
            throw "Failed to load WelcomeMode.qml: " + welcomeComponent.errorString();
        }

        welcomeMode = welcomeComponent.createObject(null /*parent*/, initialProperties);

        welcomeMode.finished.connect(() => {
            console.log("WelcomeHandler: Workflow has finished");
            welcomeMode.destroy();
            welcomeMode = null;
        });
    }

    function systemSetup() {
        let systemSetupWrapper = systemSetupWrapperComponent.createObject();
        idOverlayView.show(systemSetupWrapper);
    }

    function autostart() {
        if (!settingsSerializer.systemSetupDone) {
            console.log("System setup not done yet, starting system setup");
            systemSetup();
        } else if (!settingsSerializer.initialSetupDone) {
            console.log("Initial setup not done yet, starting tutorial mode");
            start({stage: "initialSetup"});
        }
    }

    Component.onCompleted: {
        autostart();
    }

    property Connections pduTutorialConnection: Connections {
        target: mainStatemachine
        function onPduTutorialRequested() {
            console.log("WelcomeHandler: PDU Tutorial requested");
            idWelcomeHandler.start({stage: "pduTutorialPrompt"});
            settingsSerializer.pduAcknowledged = true;
        }
    }

    property Component systemSetupWrapperComponent: Component {
        Common.OverlayPage {
            id: systemSetupPage

            rightAction: 0
            title: "System Setup"
            grindingLocked: false

            Connections {
                target: systemSetupLoader.item
                function onFinished() {
                    idOverlayView.close();
                    console.assert(settingsSerializer.systemSetupDone, "systemSetupDone must be true after finishing setup");
                    // Advance to the tutorial
                    idWelcomeHandler.autostart();
                }
            }

            Loader {
                id: systemSetupLoader
                source: "SettingsMode/SystemSetupScreen.qml"
            }
        }
    }
}
