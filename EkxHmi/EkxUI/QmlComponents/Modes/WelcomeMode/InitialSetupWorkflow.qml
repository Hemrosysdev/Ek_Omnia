import QtQuick 2.12

import "../../CommonItems" as Common
import "../SettingsMode" as SettingsMode

import EkxSqliteTypes 1.0
import SettingsStateEnum 1.0

Common.OverlayWorkflow {
    id: idWorkflow

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // properties
    ///////////////////////////////////////////////////////////////////////////////////////////////

    objectName: "TutorialMode_InitialSetup"
    closeButtonVisible: false

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // signals
    ///////////////////////////////////////////////////////////////////////////////////////////////

    signal tutorialRequested

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // components
    ///////////////////////////////////////////////////////////////////////////////////////////////

    // A page hosting a page from SettingsMode.
    component SettingsPage : Common.OverlayPage {
        required default property Item settingsItem

        children: [settingsItem]

        onAboutToClose: {
            if (typeof settingsItem.beforeExitOnClose === "function") {
                settingsItem.beforeExitOnClose();
            }
        }

        onLeftActionClicked: {
            if (typeof settingsItem.beforeExitOnPrevious === "function") {
                settingsItem.beforeExitOnPrevious();
            }
        }

        onRightActionClicked: {
            if (typeof settingsItem.beforeExitOnOk === "function") {
                settingsItem.beforeExitOnOk();
            }
            if (typeof settingsItem.save === "function") {
                settingsItem.save();
            }
        }
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // items
    ///////////////////////////////////////////////////////////////////////////////////////////////

    TextPage {
        text: `Hello!
Let’s set up your Ek Omnia
for the best experience!`
    }

    // Date settings.
    SettingsPage {
        title: "Date"

        onActivating: {
            const d = new Date();
            idDateAdjustment.setDateStartValues(d.getDate(), d.getMonth() + 1, d.getFullYear());
        }

        SettingsMode.DateAdjustment {
            id: idDateAdjustment
            entryType: SettingsStateEnum.SETTING_SYSTEM_DATE
        }
    }

    // Time settings.
    SettingsPage {
        title: "Time"

        onActivating: {
            const d = new Date();
            idTimeAdjustment.setTimeStartValues(d.getHours(), d.getMinutes());
        }

        SettingsMode.TimeAdjustment {
            id: idTimeAdjustment
            entryType: SettingsStateEnum.SETTING_SYSTEM_TIME
        }
    }

    // Unit settings (Celsius/Fahrenheit).
    SettingsPage {
        title: "Temp. Unit"

        SettingsMode.RadioButtons {
            // NOTE Keep in sync with MenuPage.qml buildUpComboInput()!
            valueList: ["Celsius", "Fahrenheit"]
            entryType: SettingsStateEnum.SETTING_SYSTEM_TEMP_UNIT
        }
    }

    TextPage {
        text: "Great! You’re done!"
        leftAction: 0
    }

    PromptPage {
        id: idWereDonePage

        text: `Enhance your experience
through a short tutorial!`
        cancelLabel: "No, let’s grind"
        confirmLabel: "Tutorial"

        onCancelClicked: idWereDonePage.finish()
        onConfirmClicked: idWorkflow.tutorialRequested()

        onActivated: {
            // Record that the initial setup has been completed.
            settingsSerializer.initialSetupDone = true;
            settingsSerializer.saveSettings();
        }
    }
}
