import QtQuick 2.12
import QtQml 2.12

Item
{
    id: idRoot

    anchors.fill: parent

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // self defined properties
    ///////////////////////////////////////////////////////////////////////////////////////////////

    /** Whether the effect is currently active.
      Can be used to disable the layer effect on an item when the animation isn't
     running to improve rendering performance. */
    readonly property bool layerEnabled: progress > 0

    /** Whether the shader effect needs to do alpha-blending of its background.
      If there is nothing behind it and the backdrop is black (the usual case),
      this can be disabled for performance. However, if an item can be seen
      behind it, it needs to be enabled. */
    property bool blending: false

    /**
     * @brief Whether only long-press results in an action.
     *
     * When set, the animation will flash in reverse when releasing the finger
     * before the timeout is up. This serves as a hint to the user that a
     * long-press is required here, for example when confirming a deletion.
     */
    property bool longPressOnly: false

    /** Animation progress. */
    property real  progress: 0

    property color fillColor: "cyan"
    property int   delay: 200
    property int   duration: 700
    property int   interruptedHintDuration: 350
    property int   mouseX: 0
    property int   mouseY: 0

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // functions
    ///////////////////////////////////////////////////////////////////////////////////////////////

    function start()
    {
        //console.log("function start")
        longPressInterruptedHint.stop();
        longPressAnimation.start();
        started();
    }

    function stop()
    {
        //console.log("function stop")
        longPressAnimation.stop();
    }

    function abort()
    {
        //console.log("function abort")
        longPressInterruptedHint.stop();
        longPressAnimation.stop();
        progress = 0;
    }

    function interrupt()
    {
        abort();
        if (longPressOnly) {
            longPressInterruptedHint.start();
        }
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // signals
    ///////////////////////////////////////////////////////////////////////////////////////////////

    signal started();
    signal simplePressed();
    signal longPressed();

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // items
    ///////////////////////////////////////////////////////////////////////////////////////////////

    SequentialAnimation {
        id: longPressAnimation

        PauseAnimation {
            duration: idRoot.delay
        }
        NumberAnimation {
            target: idRoot
            property: "progress"
            from: 0
            to: 1
            duration: idRoot.duration - idRoot.delay
        }
    }

    NumberAnimation {
        id: longPressInterruptedHint
        target: idRoot
        property: "progress"
        from: 1
        to: 0
        duration: idRoot.interruptedHintDuration
    }

    MouseArea
    {
        id: area
        anchors.fill: parent
        pressAndHoldInterval: idRoot.duration

        onPressAndHold:
        {
            idRoot.longPressed();
        }
        onClicked:
        {
            idRoot.simplePressed();
        }

        onEnabledChanged:
        {
            if (!enabled) {
                idRoot.abort();
            }
        }

        onVisibleChanged:
        {
            if (!visible) {
                idRoot.abort();
            }
        }

        onPressed:
        {
            idRoot.mouseX = mouse.x
            idRoot.mouseY = mouse.y
            idRoot.start()
        }

        onReleased:
        {
            idRoot.interrupt();
        }

        onCanceled:
        {
            idRoot.abort();
        }
    }
}
