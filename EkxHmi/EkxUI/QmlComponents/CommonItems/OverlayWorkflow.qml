import QtQml 2.12

QtObject
{
    id: idWorkflow

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // self defined properties
    ///////////////////////////////////////////////////////////////////////////////////////////////

    /** The pages in this workflow. */
    default property list<OverlayPage> pages

    /**
     * The first page that should be shown.
     * By default, this is the first page in the pages
     * list but can be any one of them.
     */
    property OverlayPage initialPage: null

    /*
     * @brief Whether to show the close button.
     */
    property bool closeButtonVisible: true

    /**
     * @brief Whether the workflow is currently part of the OverlayView.
     */
    property bool inUse: false

    /**
     * A list of items to be placed above all
     * pages in this workflow.
     */
    //property list<Item> commonItems

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // signals
    ///////////////////////////////////////////////////////////////////////////////////////////////

    /*
     * @brief Emitted when the overlay view is about to close.
     *
     * This is emitted regardless of whether the close request
     * was issued programmatically, by the user, or as the resuklt
     * of advancing beyond the last page.
     *
     * @param event The close event. Set accepted to true to
     * skip default handling of closing the overlay view.
     */
    signal aboutToClose(var event)

    /**
     * @brief Emitted when the close button was clicked by the user.
     *
     * This is emitted only when the user closes the view.
     *
     * @note that aboutToClose will be emitted afterwards if this
     * event hasn't been accepted.
     */
    signal closeClicked(var event)

    /**
     * @brief Emitted when the workflow is about to finish.
     *
     * This means that the last page in the workflow has
     * the Next/finish button clicked by the user, or the
     * request has been issued programmatically.
     *
     * It is not emitted, when the workflow is just closed.
     *
     * @param event The close event. Set accepted to true to
     * skip default handling of closing the overlay view.
     */
    signal aboutToFinish(var event)

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // functions
    ///////////////////////////////////////////////////////////////////////////////////////////////

    function __linkWorkflowProperty() {
        for (let i = 0, length = pages.length; i < length; ++i) {
            pages[i].workflow = idWorkflow;
        }
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // signal handlers
    ///////////////////////////////////////////////////////////////////////////////////////////////

    onPagesChanged: Qt.callLater(__linkWorkflowProperty)

    Component.onCompleted: __linkWorkflowProperty()

}
