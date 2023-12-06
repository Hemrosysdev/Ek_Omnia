import QtQml 2.12
import QtQuick 2.12
import QtQuick.Layouts 1.1

Item
{
    id: idSwipableItem

    implicitWidth: 480
    implicitHeight: 96

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // self defined properties
    ///////////////////////////////////////////////////////////////////////////////////////////////

    /** The items that should be shown in the list, icons, labels, etc. */
    default property alias content: idContentItem.data

    /** Whether the content item is enabled, i.e. it can be interacted with. */
    property alias contentEnabled: idContentItem.enabled

    /** The items that should be shown in the swipe-in controls area, e.g. edit button */
    property alias controls: idControlsRow.data

    /** Whether the item slides shut when it becomes invisible or gets deactivated. */
    property bool closeWhenDeactivated: true

    /** Padding around the content item. */
    property int padding: 0

    property int leftPadding: padding
    property int rightPadding: padding
    property int topPadding: padding
    property int bottomPadding: padding

    /** Margin between content item and controls item. */
    property int controlsMargin: 64

    /** Distance to screen edge when controls are open. */
    property int controlsTail: 36 // 100-64

    /** Spacing between controls. */
    property int controlsSpacing: 64

    /** The custom long-press animation to use for the items.
      If no animation is provided, the default animation will be used. */
    property LongPressAnimation longPressAnimation: null

    /** The default long-press animation.
      Allows to do bindings, such as defaultLongPressAnimation.onStarted. */
    readonly property alias defaultLongPressAnimation: idDefaultLongPressAnimation

    /** Whether the item is currently being dragged. */
    property alias dragging: idFlickable.dragging

    /** The internal flickable's horizontal content position. */
    property alias contentX: idFlickable.contentX

    property alias atXBeginning: idFlickable.atXBeginning

    property alias atXEnd: idFlickable.atXEnd

    // Cannot be an alias since we have to temporarily override it in close().
    property bool interactive: true

    /** Whether the item is currently highlighted. */
    property bool active

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // signals
    ///////////////////////////////////////////////////////////////////////////////////////////////

    signal clicked
    signal longPressed

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // functions
    ///////////////////////////////////////////////////////////////////////////////////////////////

    function reset() {
        idFlickable.contentX = 0;
    }

    function close() {
        // Needed to break free from an active drag.
        // Otherwise we cannot flick closed the flickable where the mouse started scrolling.
        idFlickable.interactive = false;
        idFlickable.flick(idFlickable.maximumFlickVelocity, 0);
        idFlickable.interactive = Qt.binding(() => {
            return idSwipableItem.interactive;
        });
    }

    function closeAll() {
        const listView = idSwipableItem.ListView.view;
        if (listView) {
            for (let i = -3; i <= 3; ++i) {
                const item = listView.itemAtIndex(index - i);
                if (item && item !== this && item instanceof SwipableListItem) {
                    item.close();
                }
            }
        }
    }

    function open() {
        idFlickable.flick(-idFlickable.maximumFlickVelocity, 0);
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // signal handlers
    ///////////////////////////////////////////////////////////////////////////////////////////////

    // Handler not on the Flickable since we have to override it in close().
    onInteractiveChanged: {
        if (!interactive) {
            close();
        }
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // items
    ///////////////////////////////////////////////////////////////////////////////////////////////

    Connections {
        target: idSwipableItem.ListView.view
        function onDraggingChanged() {
            if (idSwipableItem.ListView.view.dragging) {
                idSwipableItem.close();
            }
        }
    }

    Flickable {
        id: idFlickable

        property var dragStartX: undefined

        anchors.fill: parent

        contentWidth: idFlickItem.width + idSwipableItem.controlsTail
        contentHeight: idFlickItem.height

        flickableDirection: Flickable.HorizontalFlick
        boundsBehavior:  Flickable.StopAtBounds

        onEnabledChanged: {
            if (idSwipableItem.closeWhenDeactivated && !enabled) {
                idSwipableItem.close();
            }
        }
        onVisibleChanged: {
            if (idSwipableItem.closeWhenDeactivated && !visible) {
                idSwipableItem.reset();
            }
        }
        // onInteractiveChanged in parent item.

        onDraggingChanged: {
            if (dragging) {
                dragStartX = contentX;

                idSwipableItem.closeAll();
            } else if (dragStartX !== undefined) {
                const dx = contentX - dragStartX;
                if (dx < 0) {
                    idSwipableItem.close();
                } else if (dx > 0) {
                    idSwipableItem.open();
                }
                dragStartX = undefined;
            }
        }

        Item {
            id: idFlickItem

            // Not adding rightPadding as it is purely visual for the closed state, has no impact on where the controls are placed.
            width: idContentItem.width + idSwipableItem.leftPadding + idSwipableItem.controlsMargin + idControlsRow.implicitWidth
            height: idSwipableItem.height

            // All content is parented here.
            Item {
                id: idContentItem
                x: idSwipableItem.leftPadding
                y: idSwipableItem.topPadding
                width: idSwipableItem.implicitWidth - idSwipableItem.rightPadding - x
                height: parent.height - idSwipableItem.bottomPadding - y

                layer.enabled: (idSwipableItem.longPressAnimation ? idSwipableItem.longPressAnimation : idDefaultLongPressAnimation).layerEnabled
                layer.samplerName: "maskSource"
                layer.effect: LongPressShaderEffect {
                    longPressAnimation: idSwipableItem.longPressAnimation ? idSwipableItem.longPressAnimation : idDefaultLongPressAnimation
                }
            }

            LongPressAnimation {
                id: idDefaultLongPressAnimation
                anchors.fill: idContentItem
                fillColor: idMain.colorEkxConfirmGreen
                enabled: idContentItem.enabled

                onSimplePressed: {
                    idSwipableItem.clicked();
                }

                onLongPressed: {
                    idSwipableItem.longPressed();
                }
            }

            // Controls area for edit, delete, etc.
            Row {
                id: idControlsRow
                spacing: idSwipableItem.controlsSpacing
                anchors {
                    left: idContentItem.right
                    leftMargin: idSwipableItem.controlsMargin
                }
                // Vertically center around implicitHeight of swipable item.
                y: Math.max(0, Math.round(Math.min(idSwipableItem.height, idSwipableItem.implicitHeight) - implicitHeight) / 2)
                // Render controls only when they are swiped in.
                visible: x - idFlickable.contentX < idFlickable.width
            }

        }
    }
}
