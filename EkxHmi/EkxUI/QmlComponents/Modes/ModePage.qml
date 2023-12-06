import QtQml 2.12
import QtQuick 2.12
import QtGraphicalEffects 1.12

import "../CommonItems" as Common

Item
{
    id: idModePage

    width:   480
    height:  360
    enabled: isCurrentPage

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // self defined properties
    ///////////////////////////////////////////////////////////////////////////////////////////////

    /** The title of this page. */
    /*required TODO Qt 5.15*/property string title

    /** The EkxMainstateEnum value for this page. */
    /*required TODO Qt 5.15 */property int stateId // TODO Qt 5.15

    /** Whether the page is blended out when it's not the current one. */
    property bool blendOutEnabled: true

    /** Whether switching modes is locked.
        Typically enabled while editing the page */
    property bool modeIsLocked

    /** Whether grinding is locked. */
    property bool grindingLocked

    /** Convenience accessor for grindRunning while it is the current page. */
    readonly property bool productRunning: isCurrentPage && mainStatemachine.grindRunning

    /** Whether the DDD control should be shown in the header. */
    property bool showDdd: true

    /** Whether the current DDD value should be shown in the header.
      Disable this when it is already shown in the main view. */
    property alias showCurrentDdd: idDddDisplay.showCurrentDdd

    /** The page's main contents which are also shown during blend out. */
    property alias mainContent: idMainContainer.data

    /** The page's additional items (such as footers and headers) which
      are hidden during blend out. This is the default property. */
    default property alias additionalContent: idAdditionalContainer.data

    // ListView.isCurrentItem switches as soon as the item crosses the half-way point.
    // We only want our page active when the state machine has decided it is.
    readonly property bool isCurrentPage: mainStatemachine.ekxMainstate === stateId

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // signals
    ///////////////////////////////////////////////////////////////////////////////////////////////

    /** Emitted when the user long-presses the DDDModeDisplay
      and we should save the value to the current recipe. */
    signal saveCurrentDdd(int dddValue)

    /** Emitted when the page is no longer the active one.
      Use this to for example reset scroll positions. */
    signal deactivated

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // signal handlers
    ///////////////////////////////////////////////////////////////////////////////////////////////

    onIsCurrentPageChanged: {
        if (!isCurrentPage) {
            deactivated();
        }
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // items
    ///////////////////////////////////////////////////////////////////////////////////////////////

    // Contains the page's main items which are shown also during blend out.
    Item {
        id: idMainContainer
        anchors.fill: parent
    }

    // Contains the page's additional items (such as footers and headers)
    // which are hidden during blend out.
    Item {
        id: idAdditionalContainer
        anchors.fill: parent
        visible: opacity > 0
    }

    DddModeDisplay {
        id: idDddDisplay
        anchors {
            top: parent.top
            left: parent.left
        }
        showCurrentDdd: false
        visible: idModePage.showDdd && opacity > 0

        onSaveCurrentDdd: {
            idModePage.saveCurrentDdd(dddValue);
        }
    }

    FastBlur {
        id: blendOutBlur

        anchors.fill: idMainContainer
        source: idMainContainer
        radius: 0

        visible: opacity > 0
        opacity: 0
    }

    Common.Label {
        id: blendOutText

        anchors.fill: parent
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        text: idModePage.title
        pixelSize: 58
        bold: true
        font.letterSpacing: 0.23

        visible: opacity > 0
        opacity: 0
    }

    states: State {
        name: "BLENDED_OUT"
        when: !idModePage.isCurrentPage && idModePage.blendOutEnabled
        PropertyChanges {
            target: blendOutBlur
            radius: 55
            opacity: 1
        }
        PropertyChanges {
            target: blendOutText
            opacity: 1
        }
        PropertyChanges {
            target: idMainContainer
            opacity: 0
        }
        PropertyChanges {
            target: idDddDisplay
            opacity: 0
        }
        PropertyChanges {
            target: idAdditionalContainer
            opacity: 0
        }
    }

    transitions: Transition {
        from: "BLENDED_OUT"
        to: ""
        enabled: idModePage.blendOutEnabled
        SequentialAnimation {
            PauseAnimation {
                duration: 260
            }
            NumberAnimation {
                properties: "radius,opacity"
                duration: 150
            }
        }
    }
}
