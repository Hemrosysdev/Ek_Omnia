import QtQuick 2.12
import QtQuick.Controls 2.12 as QQC2

Item
{
    id: overlayPage

    visible: false

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // self defined properties
    ///////////////////////////////////////////////////////////////////////////////////////////////

    /**
     * @brief The title of this page.
     *
     * It is shown in the header area of the overlay view.
     */
    property string title

    /**
     * @brief Whether overlay controls are visible
     *
     * When disabled, no title and no action buttons will
     * be displayed. This can be useful to show
     * notifications or automated parts in the workflow.
     */
    property bool controlsVisible: true

    /**
     * @brief Whether to show the close button
     */
    property bool closeButtonVisible: true

    /** @brief The action to show in the top right corner. */
    property int topAction
    /** @brief The item to show in the top right corner.
      Use this if a simple button isn't enough. */
    property Item topActionItem: null
    property bool topActionLongPressEnabled: false
    /** @brief Whether long-press is the only way to trigger the top action. */
    property bool topActionLongPressOnly: false
    property color topActionLongPressColor: "cyan"
    readonly property LongPressAnimation topActionLongPressAnimation: QQC2.StackView.view ? QQC2.StackView.view.topActionAnimation : null

    /** @brief The action to show in the bottom left corner. */
    property int leftAction: EkxButton.ButtonType.PREVIOUS

    /** @brief The action to show in the bottom right corner. */
    property int rightAction: EkxButton.ButtonType.NEXT
    /** @brief Whether the long-press animation for the bottom right action is enabled. */
    property bool rightActionLongPressEnabled: false
    /** @brief Whether the bottom right action pulses. It uses the rightActionLongPressColor. */
    property bool rightActionPulseAnimationEnabled: false
    /** @brief Whether long-press is the only way to trigger the right action. */
    property bool rightActionLongPressOnly: false
    /** @brief The color for the long-press animation for the bottom right action. */
    property color rightActionLongPressColor: idMain.colorEkxConfirmGreen
    // Binding on stackView.currentItem asserts in Qt when switching pages :-(
    readonly property LongPressAnimation rightActionLongPressAnimation: QQC2.StackView.view ? QQC2.StackView.view.rightActionAnimation : null

    /** @brief The QML component to shoe in the footer area.
      This item will be shown between the left and right action buttons, if any. */
    property Component footerComponent

    /**
     * @brief The previous page in the overlay.
     *
     * If undefined (the default), the overlay view will automatically
     * determine the page based on the pages currently in the stack.
     *
     * If null, this will be considered trhe first page in the overlay.
     */
    property var /*OverlayPage*/ previousPage: undefined

    /**
     * @brief The next page in the overlay.
     *
     * If undefined (the default), the overlay view will automatically
     * determine the page based on the curent OverlayWorkflow.
     *
     * If null, this will be considered the last page in the overlay.
     */
    property var /*OverlayPage*/ nextPage: undefined

    /**
     * @brief Whether this page can be navigated to
     *
     * If this is false, OverlayView pretends this page doesn't
     * exist in the workflow.
     *
     * This is useful for edit pages that are only shown conditionally.
     */
    property bool navigable: true

    /**
     * @brief Whether the page has a black backdrop.
     *
     * The default is true. Only disable this if you really want
     * to see through to the home screen below, e.g. during a tutorial.
     */
    property bool backdrop: true

    /** @brief Whether grinding is disallowed on this page. */
    property bool grindingLocked: true

    /** @brief The workflow this page is currently a part of. */
    property QtObject /*OverlayWorkflow*/ workflow: null

    /** @brief Whether the page is currently active. */
    readonly property bool active: QQC2.StackView.status === QQC2.StackView.Active

    /** @brief Whether the page is currently inactive. */
    readonly property bool inView: QQC2.StackView.status !== QQC2.StackView.Inactive

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // signals
    ///////////////////////////////////////////////////////////////////////////////////////////////

    signal aboutToClose(var event)
    signal aboutToFinish(var event)

    signal closeClicked(var event)

    signal leftActionClicked(var event)

    signal rightActionClicked(var event)
    signal rightActionLongPressed(var event)

    signal topActionClicked(var event)
    signal topActionLongPressed(var event)

    signal longPressAnimationStarted(LongPressAnimation animation)

    signal activating
    signal activated
    signal deactivating

    // For use by the page.
    signal go(QtObject /*OverlayPage*/ page)
    signal goPrevious
    signal goNext
    signal closeOverlay
    /** Ask the view to finish the workflow. */
    signal finish

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // signal handlers
    ///////////////////////////////////////////////////////////////////////////////////////////////

    QQC2.StackView.onActivating: overlayPage.activating()
    QQC2.StackView.onActivated: overlayPage.activated()
    QQC2.StackView.onDeactivating: overlayPage.deactivating()

    onActiveChanged: {
        if (topActionItem) {
            if (active) {
                topActionItem.parent = QQC2.StackView.view.topActionItemContainer;
            } else {
                topActionItem.parent = null;
            }
        }
    }

}
