import QtQml 2.12
import QtQuick 2.12
import QtQuick.Controls 2.12 as QQC2
import "../CommonItems" as Common

Item
{
    id: idOverlayView
    anchors.fill: parent
    visible: idStack.depth > 0

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // self defined properties
    ///////////////////////////////////////////////////////////////////////////////////////////////

    readonly property bool grindingLocked: idStack.currentItem ? (idStack.currentItem.grindingLocked === true) : false

    readonly property bool backdrop: idStack.currentItem ? idStack.currentItem.backdrop === true : true

    readonly property alias currentItem: idStack.currentItem

    readonly property QtObject currentWorkflow: idStack.currentItem ? idStack.currentItem.workflow : null

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // functions
    ///////////////////////////////////////////////////////////////////////////////////////////////

    /**
     * @brief Shows the given page or workflow.
     *
     * Immediately drops all previously shown pages,
     * doesn't run any handlers (no aboutToClose, aboutToFinish, etc).
     *
     * @param item The item(s) to push. This can be an OverlayPage,
     * an OverlayWorkflow, or an Array containing these items.
     */
    function show(item) {
        console.log(overlayLog, "Show overlay", item);
        __clear();
        __push(item);
    }

    /**
     * @brief Continue with the given page or workflow.
     *
     * Immediately removes all pages after the current one, and
     * then pushes the given page or workflow.
     *
     * If there is no current page, this effectively behaves like show().
     *
     * @param item The item(s) to reroute to. This can be an OverlayPage,
     * an OverlayWorkflow, or an Array containing these items.
     */
    function reroute(item) {
        console.log(overlayLog, "Reroute overlay to", item);

        // Throw away all upcoming pages.
        idStack.pages.length = idStack.depth;

        __push(item);
    }

    /**
     * @brief Close the overlay.
     *
     * This also runs any handlers (aboutToClose, etc).
     */
    function close() {
        console.log(overlayLog, "Close overlay");

        // If the page changed as a result of the aboutToClose signal, don't run default handler.
        // Otherwise a page showing a new overlay will still immediately close.
        const oldCurrentPage = idStack.currentItem;
        let event = {accepted: false};

        if (idStack.currentItem) {
            // First ask the page to handle the event.
            idStack.currentItem.aboutToClose(event);

            // If not accepted, ask the workflow, if any.
            if (!event.accepted && oldCurrentPage === idStack.currentItem && oldCurrentPage.workflow) {
                oldCurrentPage.workflow.aboutToClose(event);
            }
        }

        if (!event.accepted && oldCurrentPage === idStack.currentItem) {
            __clear();
        } else {
            console.log(overlayLog, "  Close event was accepted");
        }
    }

    function go(page : Common.OverlayPage) {
        console.log(overlayLog, "Go to", page);
        if (!page) {
            return;
        }

        if (!page.navigable) {
            throw "Cannot go to non-navigable page";
        }

        // Check if the page is open before popping, otherwise Qt complains if it's not.
        for (let i = idStack.depth - 1; i >= 0; --i) {
            const candidate = idStack.pages[i];
            if (candidate === page) {
                idStack.pop(page);
                return;
            }
        }

        let pagesToPush = [];
        for (let i = idStack.depth - 1; i < idStack.pages.length; ++i) {
            const candidate = idStack.pages[i];
            pagesToPush.push(candidate);
            if (candidate === page) {
                break;
            }
        }

        if (pagesToPush.length === 0) {
            throw "Cannot go, failed to find page";
        }

        idStack.push(pagesToPush);
    }

    function __clear() {
        idStack.clear();
        idStack.pages = [];
    }

    function __push(item, args) {
        const items = Array.isArray(item) ? item : [item];

        let pagesToPush = [];

        for (let i = 0; i < items.length; ++i) {
            const item = items[i];

            if (item instanceof Common.OverlayPage) {
                console.log(overlayLog, "Pushed single page:", item);
                idStack.pages.push(item);

                // The first item is the one we'll navigate to, everything after
                // it is in the list of pages but will not show right away.
                if (i === 0) {
                    pagesToPush.push(item);
                }
            } else if (item instanceof Common.OverlayWorkflow) {
                console.log(overlayLog, "Pushed workflow:", item);

                for (let i in item.pages) {
                    idStack.pages.push(item.pages[i]);
                }

                if (i === 0) {
                    let initialPageFound = false;

                    for (let i in item.pages) {
                        const page = item.pages[i];
                        console.log(overlayLog, "  Pushing", page);

                        if (!initialPageFound) {
                            pagesToPush.push(page);

                            if (page === item.initialPage || !item.initialPage) {
                                initialPageFound = true
                            }
                        }
                    }

                    // Push all pages up to and including the initial page.
                    if (!initialPageFound) {
                        throw "Cannot push workflow, failed to find initial page";
                    }
                }

            } else {
                throw "Can only add OverlayPage or OverlayWorkflow to the OverlayView!";
            }
        }

        idStack.push(pagesToPush);
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // items
    ///////////////////////////////////////////////////////////////////////////////////////////////

    LoggingCategory {
        id: overlayLog
        name: "ekx.overlay"
    }

    QQC2.StackView {
        id: idStack
        anchors.fill: parent
        // Disable transition animations.
        pushEnter: null
        popEnter: null
        pushExit: null
        popExit: null

        property list<Common.OverlayPage> pages;
        property var workflows: new Set()

        readonly property bool canGoPrevious: previousPage() !== null
        readonly property bool canGoNext: nextPage() !== null

        property alias topActionAnimation: idTopButtonAnimation
        property alias topActionItemContainer: idTopActionItemContainer
        property alias rightActionAnimation: idRightButtonAnimation

        function previousPage() {
            if (!currentItem || idStack.depth < 2) {
                return null;
            }
            if (currentItem.previousPage && currentItem.previousPage instanceof Common.OverlayPage) {
                return currentItem.previousPage;
            } else if (currentItem.previousPage === undefined) {
                // Automatically determine previous page.
                const pages = idStack.pages;
                for (let i = idStack.depth - 2; i >= 0; --i) {
                    const page = pages[i];
                    if (page && page.navigable) {
                        return page;
                    }
                }
                return null;
            } else if (currentItem.previousPage === null) {
                return null;
            } else {
                console.error(overlayLog, "Failed to determine previous page", currentItem.previousPage, "on", currentItem);
                return null;
            }
        }

        function nextPage() {
            if (!currentItem) {
                return null;
            }
            if (currentItem.nextPage && currentItem.nextPage instanceof Common.OverlayPage) {
                return currentItem.nextPage;
            } else if (currentItem.nextPage === undefined) {
                // Automatically determine next page.
                const pages = idStack.pages;

                for (let i = idStack.depth; i < pages.length; ++i) {
                    const page = pages[i];
                    if (page && page.navigable) {
                        return page;
                    }
                }
                return null;
            } else if (currentItem.nextPage === null) {
                return null;
            } else {
                console.error(overlayLog, "Failed to determine next page", currentItem.nextPage, "on", currentItem);
                return null;
            }
        }

        function goPrevious() {
            idOverlayView.go(previousPage());
        }

        function goNext() {
            idOverlayView.go(nextPage());
        }

        function finish() {
            console.log(overlayLog, "Finish overlay");
            const oldCurrentPage = idStack.currentItem;

            let event = {accepted: false};
            if (idStack.currentItem) {
                // First ask the page to handle the event.
                idStack.currentItem.aboutToFinish(event);

                // If not accepted, ask the workflow, if any.
                if (!event.accepted && oldCurrentPage === idStack.currentItem && oldCurrentPage.workflow) {
                    oldCurrentPage.workflow.aboutToFinish(event);
                }
            }

            // If the page changed as a result of the finish signal, don't run default handler.
            // Otherwise a page showing a new overlay (e.g. "add beans" button) will still immediately close.
            if (oldCurrentPage === idStack.currentItem && !event.accepted) {
                idOverlayView.close();
            } else {
                console.log(overlayLog, "  Finish event was accepted");
            }
        }

        function updateWorkflowsInUse() {
            // Remember which workflows were inUse before.
            let newWorkflows = new Set();
            for (let i in pages) {
                const page = pages[i];
                if (page && page.workflow) {
                    newWorkflows.add(page.workflow);

                    // All open workflows are in use.
                    page.workflow.inUse = true;
                }
            }

            // Now mark the ones that aren't there anymore as not inUse.
            workflows.forEach((workflow) => {
                if (workflow && !newWorkflows.has(workflow)) {
                    console.log(overlayLog, "Workflow", workflow, "no longer in use");
                    workflow.inUse = false;
                }
            });

            workflows = newWorkflows;
        }

        onPagesChanged: {
            Qt.callLater(updateWorkflowsInUse);
        }
    }

    Connections {
        target: idStack.currentItem
        // currentItem can be null, suppress warning.
        ignoreUnknownSignals: true

        function onGo(page) {
            idOverlayView.go(page);
        }

        function onGoPrevious() {
            idStack.goPrevious();
        }

        function onGoNext() {
            idStack.goNext();
        }

        function onCloseOverlay() {
            idOverlayView.close();
        }

        function onFinish() {
            idStack.finish();
        }
    }

    Item {
        anchors.fill: idStack
        visible: idStack.currentItem && idStack.currentItem.controlsVisible

        // Header area
        Common.ShadowGroup {
            shadowTopSize: Common.ShadowGroup.ShadowBottomSizes.StandardSize
            shadowTopEnabled: idTitleLabel.visible
        }

        Common.EkxButton {
            id: idCloseButton
            anchors {
                top: parent.top
                left: parent.left
            }
            buttonType: Common.EkxButton.ButtonType.CLOSE
            visible: {
                const currentItem = idStack.currentItem;
                if (currentItem) {
                    if (!currentItem.closeButtonVisible) {
                        return false;
                    }

                    const workflow = currentItem.workflow;
                    if (workflow && !workflow.closeButtonVisible) {
                        return false;
                    }
                }

                return true;
            }
            onClicked: {
                // If the page changed as a result of the closeClicked signal, don't run default handler.
                const oldCurrentPage = idStack.currentItem;
                let event = {accepted: false};

                if (idStack.currentItem) {
                    // First ask the page to handle the event.
                    idStack.currentItem.closeClicked(event);

                    // If not accepted, ask the workflow, if any.
                    if (!event.accepted && oldCurrentPage === idStack.currentItem && oldCurrentPage.workflow) {
                        oldCurrentPage.workflow.closeClicked(event);
                    }
                }

                if (!event.accepted && oldCurrentPage === idStack.currentItem) {
                    idOverlayView.close();
                } else {
                    console.log(overlayLog, "  Close clicked was accepted");
                }
            }
        }

        Common.Label {
            id: idTitleLabel
            anchors {
                top: parent.top
                left: idCloseButton.right
                right: idTopButtonArea.left
                bottom: idCloseButton.bottom
            }
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            textColor: Common.Label.TextColor.Grey
            bold: true
            font.letterSpacing: 2.0
            font.capitalization: Font.AllUppercase
            text: idStack.currentItem && idStack.currentItem.title
            visible: text !== ""
        }

        // Top right button (e.g. add, delete)
        Item {
            id: idTopButtonArea
            anchors {
                top: parent.top
                right: parent.right
            }
            width: 72
            height: 72

            Common.EkxButton {
                id: idTopActionButton
                anchors {
                    top: parent.top
                    right: parent.right
                }
                buttonType: idStack.currentItem ? idStack.currentItem.topAction : 0
                visible: buttonType !== 0

                layer.enabled: idTopButtonAnimation.layerEnabled
                layer.samplerName: "maskSource"
                layer.effect: Common.LongPressShaderEffect {
                    longPressAnimation: idTopButtonAnimation
                }

                onClicked: {
                    let event = {accepted: false};
                    idStack.currentItem.topActionClicked(event);
                }

                Common.LongPressAnimation {
                    id: idTopButtonAnimation
                    visible: idStack.currentItem ? idStack.currentItem.topActionLongPressEnabled : false
                    fillColor: idStack.currentItem ? idStack.currentItem.topActionLongPressColor : idMain.colorEkxConfirmGreen
                    longPressOnly: idStack.currentItem ? idStack.currentItem.topActionLongPressOnly : false
                    onStarted: {
                        idStack.currentItem.longPressAnimationStarted(this);
                    }
                    onLongPressed: {
                        let event = {accepted: false};
                        idStack.currentItem.topActionLongPressed(event);
                    }
                    onSimplePressed: {
                        let event = {accepted: false};
                        idStack.currentItem.topActionClicked(event);
                    }
                }
            }

            Item { // topItem gets reparented here.
                id: idTopActionItemContainer
                anchors.fill: parent
            }
        }

        // Footer area.
        Common.EkxButton { // left button (go previous)
            id: idLeftButton
            anchors {
                bottom: parent.bottom
                left: parent.left
            }
            buttonType: idStack.currentItem ? idStack.currentItem.leftAction : 0
            onClicked: {
                var event = {accepted: false};
                idStack.currentItem.leftActionClicked(event);
                if (!event.accepted) {
                    if (buttonType === Common.EkxButton.ButtonType.PREVIOUS) {
                        idStack.goPrevious();
                    }
                }
            }
            visible: {
                if (buttonType === Common.EkxButton.ButtonType.PREVIOUS) {
                    return idStack.canGoPrevious;
                } else {
                    return buttonType !== 0;
                }
            }
        }

        Loader { // footer component loader.
            anchors {
                // As soon as one button is shown, anchor it to both, so it's always centered.
                left: idLeftButton.visible || idRightButton.visible ? idLeftButton.right : parent.left
                right: idLeftButton.visible || idRightButton.visible ? idRightButton.left : parent.right
                bottom: parent.bottom
            }
            sourceComponent: idStack.currentItem ? idStack.currentItem.footerComponent : null
        }

        Common.EkxButton { // right button (go next)
            id: idRightButton

            // Used by the pulse animation.
            property color pulseColor: "transparent"

            anchors {
                bottom: parent.bottom
                right: parent.right
            }
            buttonType: {
                const type = (idStack.currentItem ? idStack.currentItem.rightAction : 0);
                if (type === Common.EkxButton.ButtonType.NEXT && !idStack.canGoNext) {
                    return Common.EkxButton.ButtonType.OK;
                } else {
                    return type;
                }
            }
            visible: {
                if (buttonType === Common.EkxButton.ButtonType.NEXT) {
                    return idStack.canGoNext;
                } else {
                    return buttonType !== 0;
                }
            }

            layer.enabled: idStack.currentItem ? (idRightButtonPulseAnimation.running || (idStack.currentItem.rightActionLongPressEnabled && idRightButtonAnimation.layerEnabled)) : false
            layer.samplerName: "maskSource"
            layer.effect: Common.LongPressShaderEffect {
                longPressAnimation: idRightButtonAnimation
                backgroundColor: idRightButton.pulseColor
                // During tutorial the finger tends to get hidden behind the pulse animation.
                blending: true
            }

            onClicked: {
                buttonClicked();
            }

            function buttonClicked() {
                const oldCurrentPage = idStack.currentItem;

                let event = {accepted: false};
                idStack.currentItem.rightActionClicked(event);

                // If the page changed as a result of the action click, don't run default handler.
                // Otherwise a page showing a new overlay (e.g. "add beans" button) will also immediately
                // "go next" upon entering.
                if (oldCurrentPage === idStack.currentItem && !event.accepted) {
                    if (idRightButton.buttonType === Common.EkxButton.ButtonType.NEXT) {
                        idStack.goNext();
                    } else if (idRightButton.buttonType === Common.EkxButton.ButtonType.OK) {
                        idStack.finish();
                    }
                }
            }

            Common.LongPressAnimation {
                id: idRightButtonAnimation
                visible: idStack.currentItem ? idStack.currentItem.rightActionLongPressEnabled : false
                fillColor: idStack.currentItem ? idStack.currentItem.rightActionLongPressColor : idMain.colorEkxConfirmGreen
                longPressOnly: idStack.currentItem ? idStack.currentItem.rightActionLongPressOnly : false
                onStarted: {
                    if (idStack.currentItem.rightActionLongPressEnabled) {
                        idStack.currentItem.longPressAnimationStarted(this);
                    }
                }
                onLongPressed: {
                    abort();
                    if (idStack.currentItem.rightActionLongPressEnabled) {
                        let event = {accepted: false};
                        idStack.currentItem.rightActionLongPressed(event);
                    }
                }
                onSimplePressed: {
                    abort();
                    idRightButton.buttonClicked();
                }
            }

            Common.PulseAnimation {
                id: idRightButtonPulseAnimation
                target: idRightButton
                property: "pulseColor"
                // The icon is white already, let it just mix it.
                // TODO Fix the Shader so it treats "transparent" properly. This is transparent "white" here.
                color: "#00ffffff"
                running: idStack.currentItem ? idStack.currentItem.rightActionPulseAnimationEnabled : false
                highlightColor: idStack.currentItem ? idStack.currentItem.rightActionLongPressColor : idMain.colorEkxConfirmGreen
            }
        }
    }
}
