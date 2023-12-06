import QtQuick 2.12

import "../../CommonItems" as Common

Rectangle
{
    id: idFinger

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // self defined properties
    ///////////////////////////////////////////////////////////////////////////////////////////////

    property bool pressed: false

    property int pressAnimationDuration: 100

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // properties
    ///////////////////////////////////////////////////////////////////////////////////////////////

    width: 64
    height: 64
    color: "#A0FF0000" // 0.63 opacity
    radius: width / 2
    z: 100

    scale: pressed ? 0.63 : 1

    onVisibleChanged: {
        if (!visible) {
            pressed = false;
        }
    }

    Behavior on scale {
        NumberAnimation {
            duration: idFinger.pressAnimationDuration
            easing.type: Easing.InOutQuad
        }
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // components
    ///////////////////////////////////////////////////////////////////////////////////////////////

    component FlyInAnimation : ParallelAnimation {
        id: idFlyInAnimation

        property point from: Qt.point(550, 280)
        required property point to
        required property Finger target

        NumberAnimation {
            target: idFlyInAnimation.target
            duration: 500
            easing.type: Easing.OutQuad
            property: "opacity"
            from: 0
            to: 1
        }
        PathAnimation {
            duration: 500
            easing.type: Easing.InOutQuad

            target: idFlyInAnimation.target
            path: Path {
                startX: idFlyInAnimation.from.x - idFlyInAnimation.target.width / 2
                startY: idFlyInAnimation.from.y - idFlyInAnimation.target.height / 2

                PathQuad {
                    x: idFlyInAnimation.to.x  - idFlyInAnimation.target.width / 2
                    y: idFlyInAnimation.to.y  - idFlyInAnimation.target.height / 2
                    controlX: idFlyInAnimation.to.x - idFlyInAnimation.target.width / 2
                    controlY: idFlyInAnimation.from.y - idFlyInAnimation.target.height / 2
                }
            }
        }

        PauseAnimation {
            duration: 1000
        }
    }

    component DragAnimation : SequentialAnimation {
        id: idDragAnimation

        required property Finger target
        required property int axis
        required property int from
        required property int to

        property int duration: 250

        PressAction {
            target: idDragAnimation.target
            pressed: true
        }

        ParallelAnimation {
            NumberAnimation {
                target: idDragAnimation.target
                property: idDragAnimation.axis === Qt.Horizontal ? "x" : "y"
                from: idDragAnimation.from - idDragAnimation.target.width / 2
                to: idDragAnimation.to - idDragAnimation.target.height / 2
                duration: idDragAnimation.duration
                easing.type: Easing.InOutQuad
            }
        }

        PressAction {
            target: idDragAnimation.target
            pressed: false
        }
    }

    component PressAction : PropertyAction {
        id: idPressAction
        required property bool pressed

        property: "pressed"
        value: pressed
    }

    component ShortPressAnimation : SequentialAnimation {
        id: idShortPressAnimation
        required property Finger target

        signal clicked

        PressAction {
            target: idShortPressAnimation.target
            pressed: true
        }

        PauseAnimation {
            duration: idShortPressAnimation.target.pressAnimationDuration / 2
        }

        ScriptAction {
            script: idShortPressAnimation.clicked()
        }

        PauseAnimation {
            duration: idShortPressAnimation.target.pressAnimationDuration / 2
        }

        PressAction {
            target: idShortPressAnimation.target
            pressed: false
        }
    }

    component LongPressAnimation : SequentialAnimation {
        id: idLongPressAnimation
        required property Finger target
        required property Common.LongPressAnimation longPressAnimation

        signal longPressed

        PressAction {
            target: idLongPressAnimation.target
            pressed: true
        }

        ScriptAction {
            script: {
                idLongPressAnimation.longPressAnimation.start()
            }
        }

        PauseAnimation {
            duration: idLongPressAnimation.longPressAnimation ? (idLongPressAnimation.longPressAnimation.delay + idLongPressAnimation.longPressAnimation.duration) : 0
        }

        PauseAnimation {
            // Make sure the glow is noticed.
            duration: 500
        }

        PressAction {
            target: idLongPressAnimation.target
            pressed: false
        }

        PauseAnimation {
            duration: idLongPressAnimation.target.pressAnimationDuration
        }

        ScriptAction {
            script: {
                idLongPressAnimation.longPressed();
                idLongPressAnimation.longPressAnimation.abort();
            }
        }
    }

}
