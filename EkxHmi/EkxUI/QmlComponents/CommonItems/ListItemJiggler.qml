import QtQml 2.12
import QtQuick 2.12//2.13 // TODO Qt 5.15

QtObject {
    id: idJiggler

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // properties
    ///////////////////////////////////////////////////////////////////////////////////////////////

    /** The ListView to jiggle. */
    /*required TODO Qt 5.15*/property ListView listView

    /** How many items to jiggle, starting at the first visible item. */
    property int jiggleCount: 4

    /**
     * The total amount of the animation, not including
     * any delay to jiggle the items after one another.
     */
    property int duration: 200

    /**
     * The amount of horizontal pixels the jiggled item should be jiggled.
     */
    property int amount: 30

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // functions
    //////////////////////////////////////////////////////////////////////////////////////////////

    function jiggle() {
        if (!idJiggler.listView) {
            throw "Cannot jiggle without ListView";
        }

        let y = idJiggler.listView.contentY + idJiggler.listView.topMargin;
        for (let i = 0; i < idJiggler.jiggleCount; ++i) {
            // TODO Use ListView.itemAtIndex from Qt 5.13!
            const item = idJiggler.listView.itemAt(0, y);
            if (!item) {
                continue;
            }

            if (!(item instanceof SwipableListItem)) {
                console.warn("Can only jiggle instances of SwipableListItem");
                continue;
            }

            y += item.height;

            let animation = jiggleAnimationComponent.createObject(item, {
                startDelay: idJiggler.duration * (i + 1),
                target: item
            });
        }
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // signal handlers
    ///////////////////////////////////////////////////////////////////////////////////////////////

    Component.onCompleted: {
        idJiggler.jiggle();
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // items
    ///////////////////////////////////////////////////////////////////////////////////////////////

    // QtObject has no default property so we cannot just add items directly.

    property Connections listViewConnections: Connections {
        target: idJiggler.listView
        function onVisibleChanged() {
            if (idJiggler.listView.visible) {
                // Delay jiggling in case the view is scrolled
                // in response to a visibility change.
                Qt.callLater(function() {
                    idJiggler.jiggle();
                });
            }
        }
    }

    property Component jiggleAnimationComponent: Component {
        SequentialAnimation {
            id: jiggleAnimation
            property alias startDelay: jigglePause.duration
            property Item target

            property Connections stopOnHide: Connections {
                target: jiggleAnimation.target
                function onVisibleChanged() {
                    // Finish animation immediately when item becomes invisible
                    // to avoid still jiggling when entering the same screen again.
                    if (!jiggleAnimation.target.visible) {
                        jiggleAnimation.complete();
                    }
                }
                function onDraggingChanged() {
                    if (jiggleAnimation.target.dragging) {
                        jiggleAnimation.complete();
                    }
                }
            }

            running: true
            loops: 1

            onFinished: destroy()

            PauseAnimation {
                id: jigglePause
            }
            NumberAnimation {
                target: jiggleAnimation.target
                property: "contentX"
                from: 0
                to: idJiggler.amount
                duration: idJiggler.duration / 2
            }
            NumberAnimation {
                target: jiggleAnimation.target
                property: "contentX"
                to: 0
                duration: idJiggler.duration / 2
            }
        }
    }
}
