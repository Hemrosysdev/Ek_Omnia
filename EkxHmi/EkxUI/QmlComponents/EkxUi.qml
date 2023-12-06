import QtQml 2.12
import QtQuick 2.12
import QtQuick.Window 2.12
import QtGraphicalEffects 1.12
import "Modes"
import "Modes/SwUpdate"
import "Modes/Standby"
import "Modes/SettingsMode"
import "CommonItems"
import EkxMainstateEnum 1.0
import EkxSqliteTypes 1.0
import SettingsStateEnum 1.0

Item
{
    id: idMain

    width: 480
    height: 360

    property color colorEkxTextWhite:    "white"
    property color colorEkxTextGrey:     Qt.rgba( 0.5, 0.5, 0.5, 1.0 )
    property color colorEkxConfirmGreen: "#00ffb3"
    property color colorEkxConfirmRed:   "#ff465f"
    property color colorEkxDarkGreen:    "#00805a"
    property color colorEkxWarning:      "#ff8b00"
    property color colorEkxError:        "#ff465f"

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // self defined properties
    ///////////////////////////////////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // signal handlers
    ///////////////////////////////////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // functions
    ///////////////////////////////////////////////////////////////////////////////////////////////

    function showAgsaFineAdjustment()
    {
        if ( settingsSerializer.agsaEnabled )
        {
            idAgsaFineAdjustment.visible = true
        }
    }

    function isAgsaFineAdjustmentShown()
    {
        return idAgsaFineAdjustment.visible
    }

    // Returns true when the item is within visible bounds of a ListView.
    // ListView does culling on its own but the items immediately adjacent
    // to the current one are typically still visible.
    // Our mode pages are quite heavy, so it's worth it adding special
    // logic for those to improve rendering performance.
    function isDelegateEffectivelyVisible(item)
    {
        const listView = item.ListView.view;
        if (!listView || !item) { // in doubt, we're visible.
            return true;
        }

        // One would think to subtract the view's origin (which can be non-zero if items
        // are removed, e.g. when dsiabling a mode in "Customize" settings) but that is
        // only really relevant for the Flickable. Our delegates inside are pretty much
        // still placed properly relative to contentX/Y.

        const xContains = listView.contentX > item.x - item.width && listView.contentX < item.x + item.width;
        const yContains = listView.contentY > item.y - item.height && listView.contentY < item.y + item.height;

        return xContains && yContains;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // items
    ///////////////////////////////////////////////////////////////////////////////////////////////

    ModeSelector
    {
        id: idModeSelector
        z: 0
        enabled: !idOverlayView.visible
        visible: !idOverlayView.visible || !idOverlayView.backdrop
    }

    OverlayView
    {
        id: idOverlayView
        z: 1
        onVisibleChanged: recipeControl.agsaAllowed = !visible
    }

    AgsaFineAdjustment
    {
        id: idAgsaFineAdjustment

        z: 2
        visible: false
    }

    WelcomeHandler
    {
        id: idWelcomeHandler
    }

    SwUpdateInfo
    {
        z: 20
        visible: mainStatemachine.ekxMainstate === EkxMainstateEnum.EKX_SWUPDATE
    }

    StandbyInfo
    {
        z: 100
        visible: mainStatemachine.ekxMainstate === EkxMainstateEnum.EKX_STANDBY || standbyController.preStandbyDimActive
    }

    Notification
    {
        z: 10
        classId: notificationCenter.anyActive ? notificationCenter.activeNotification.classId : EkxSqliteTypes.None
        icon: notificationCenter.anyActive ? notificationCenter.activeNotification.image : ""
        text: notificationCenter.anyActive ? notificationCenter.activeNotification.longText : ""
        cancelLabel: notificationCenter.anyActive ? notificationCenter.activeNotification.cancelLabel : ""
        confirmLabel: notificationCenter.anyActive ? notificationCenter.activeNotification.confirmLabel : ""
        visible: notificationCenter.anyActive

        onCancelClicked: {
            notificationCenter.activeNotification.activateCancel();
        }
        onConfirmClicked: {
            notificationCenter.activeNotification.activateAcknowledge();
        }
    }

    MenuPage
    {
        z: 0
        visible: settingsStateMachine.settingsState !== SettingsStateEnum.SETTING_MAIN && !idOverlayView.visible
    }
}

