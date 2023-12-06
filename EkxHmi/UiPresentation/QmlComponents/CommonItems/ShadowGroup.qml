import QtQuick 2.12

Rectangle {
    id: idShadowGroup;

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // custom properties
    ///////////////////////////////////////////////////////////////////////////////////////////////

    property bool visualDebug: false;

    property Item target: parent;




    // standard two-point gradient shadows:
    property bool shadowLeftEnabled:   true;
    property int  shadowLeftSize:      0;

    property bool shadowRightEnabled:  true;
    property int  shadowRightSize:     0;

    property bool shadowTopEnabled:    true;
    property int  shadowTopSize:       0;

    property bool shadowBottomEnabled: true;
    property int  shadowBottomSize:    0;

    enum ShadowLeftSizes {
        NoShadow = 0
    }

    enum ShadowRightSizes {
        NoShadow = 0
    }

    enum ShadowTopSizes {
        NoShadow = 0,
        StandardSize = 100
    }

    enum ShadowBottomSizes {
        NoShadow = 0,
        StandardSize = 100
    }

    // Optional setting for opacity at end position (at border):
    property real shadowLeftOuterOpacity:   1.0;
    property real shadowRightOuterOpacity:  1.0;
    property real shadowTopOuterOpacity:    1.0;
    property real shadowBottomOuterOpacity: 1.0;

    // Additional optional third gradient stop (e.g. at 40% of size already 60% opacity)
    property bool shadowLeftThirdGradientStopEnable:     false;
    property real shadowLeftThirdGradientStopPosition:   0.4;
    property real shadowLeftThirdGradientStopOpacity:    0.6;

    property bool shadowRightThirdGradientStopEnable:    false;
    property real shadowRightThirdGradientStopPosition:  0.4;
    property real shadowRightThirdGradientStopOpacity:   0.6;

    property bool shadowTopThirdGradientStopEnable:      false;
    property real shadowTopThirdGradientStopPosition:    0.4;
    property real shadowTopThirdGradientStopOpacity:     0.6;

    property bool shadowBottomThirdGradientStopEnable:   false;
    property real shadowBottomThirdGradientStopPosition: 0.4;
    property real shadowBottomThirdGradientStopOpacity:  0.6;

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // standard properties
    ///////////////////////////////////////////////////////////////////////////////////////////////

    anchors.fill: target;
    color: "transparent";

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // functions
    ///////////////////////////////////////////////////////////////////////////////////////////////

    // common functions:
    function addThirdGradientStop(idGradient, gradientStopPosition, gradientStopOpacity)
    {
        if(idGradient.stops.length !== 2)
        {
            //console.log("addThirdGradientStop(): stops array has not length equal to 2");
            return;
        }

        let gradStopObj = Qt.createQmlObject("import QtQuick 2.12;"
                                             +"GradientStop {"
                                             +"position: 0.5;"
                                             +"color: Qt.rgba(1.0,0.0,0.0,1.0);"
                                             +"}"
                                             ,idGradient);
        gradStopObj.position = gradientStopPosition;
        gradStopObj.color    = Qt.rgba(0.0,0.0,0.0,gradientStopOpacity);
        idGradient.stops.push(gradStopObj);
    }

    function removeThirdGradientStop(idGradient)
    {
        if(idGradient.stops.length !== 3)
        {
            //console.log("removeThirdGradientStop(): stops array has not length equal to 3");
            return;
        }

        idGradient.stops.pop();
    }

    // specialized functions for left, right, top and  bottom:
    function updateShadowLeftThirdGradientStop()
    {
        if(shadowLeftThirdGradientStopEnable)
            addThirdGradientStop(idShadowLeft.gradient, 1.0-shadowLeftThirdGradientStopPosition, shadowLeftThirdGradientStopOpacity);
        else
            removeThirdGradientStop(idShadowLeft.gradient);
    }

    function updateShadowRightThirdGradientStop()
    {
        if(shadowRightThirdGradientStopEnable)
            addThirdGradientStop(idShadowRight.gradient, shadowRightThirdGradientStopPosition, shadowRightThirdGradientStopOpacity);
        else
            removeThirdGradientStop(idShadowRight.gradient);
    }

    function updateShadowTopThirdGradientStop()
    {
        if(shadowTopThirdGradientStopEnable)
            addThirdGradientStop(idShadowTop.gradient, 1.0-shadowTopThirdGradientStopPosition, shadowTopThirdGradientStopOpacity);
        else
            removeThirdGradientStop(idShadowTop.gradient);
    }

    function updateShadowBottomThirdGradientStop()
    {
        if(shadowBottomThirdGradientStopEnable)
            addThirdGradientStop(idShadowBottom.gradient, shadowBottomThirdGradientStopPosition, shadowBottomThirdGradientStopOpacity);
        else
            removeThirdGradientStop(idShadowBottom.gradient);
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // handlers
    ///////////////////////////////////////////////////////////////////////////////////////////////

    onShadowLeftThirdGradientStopEnableChanged:
    {
        updateShadowLeftThirdGradientStop();
    }

    onShadowRightThirdGradientStopEnableChanged:
    {
        updateShadowRightThirdGradientStop();
    }

    onShadowTopThirdGradientStopEnableChanged:
    {
        updateShadowTopThirdGradientStop();
    }

    onShadowBottomThirdGradientStopEnableChanged:
    {
        updateShadowBottomThirdGradientStop();
    }

    Component.onCompleted:
    {
        updateShadowLeftThirdGradientStop();
        updateShadowRightThirdGradientStop();
        updateShadowTopThirdGradientStop();
        updateShadowBottomThirdGradientStop();
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // items
    ///////////////////////////////////////////////////////////////////////////////////////////////

    Rectangle
    {
        id: idShadowLeft;
        visible: shadowLeftEnabled;
        z:1
        anchors.left: parent.left
        width: shadowLeftSize;
        height: parent.height

        gradient: Gradient
        {
            orientation: Gradient.Horizontal;
            GradientStop { position: 1.0-0.0; color: Qt.rgba(0.0,0.0,0.0,0.0); }
            GradientStop { position: 1.0-1.0; color: Qt.rgba(0.0,0.0,0.0,shadowLeftOuterOpacity); }
        }
        DebugFrame{visible: visualDebug}
    }

    Rectangle
    {
        id: idShadowRight;
        visible: shadowRightEnabled;
        z:1
        anchors.right: parent.right
        width: shadowRightSize;
        height: parent.height

        gradient: Gradient
        {
            orientation: Gradient.Horizontal;
            GradientStop { position: 0.0; color: Qt.rgba(1.0,0.0,0.0,0.0); }
            GradientStop { position: 1.0; color: Qt.rgba(0.0,0.0,0.0,shadowRightOuterOpacity); }
        }
        DebugFrame{visible: visualDebug}
    }

    Rectangle
    {
        id: idShadowTop;
        visible: shadowTopEnabled;
        z:1
        anchors.top: parent.top;
        width: parent.width;
        height: shadowTopSize;

        gradient: Gradient
        {
            orientation: Gradient.Vertical;
            GradientStop { position: 1.0-0.0; color: Qt.rgba(0.0,0.0,0.0,0.0); }
            GradientStop { position: 1.0-1.0; color: Qt.rgba(0.0,0.0,0.0,shadowTopOuterOpacity); }
        }
        DebugFrame{visible: visualDebug}
    }

    Rectangle
    {
        id: idShadowBottom;
        visible: shadowBottomEnabled;
        z:1
        anchors.bottom: parent.bottom;
        width: parent.width;
        height: shadowBottomSize;

        gradient: Gradient
        {
            orientation: Gradient.Vertical;
            GradientStop { position: 0.0; color: Qt.rgba(0.0,0.0,0.0,0.0); }
            GradientStop { position: 1.0; color: Qt.rgba(0.0,0.0,0.0,shadowBottomOuterOpacity); }
        }
        DebugFrame{visible: visualDebug}
    }
}
