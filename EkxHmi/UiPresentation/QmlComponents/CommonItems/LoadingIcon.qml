import QtQuick 2.12

Item
{
    id: idRoot;

    property int radius: 20;
    property int spacing: 50;
    property color color: "white";
    property int durationMs: 1000;

    function reset()
    {
        idRoot.state = "";
    }

    function step()
    {
        if(idRoot.state === "")
            idRoot.state = "B";
        else if(idRoot.state === "B")
            idRoot.state = "C";
        else if(idRoot.state === "C")
            idRoot.state = "";
        else
            idRoot.state = "";
    }

    width: 2*idRoot.radius+2*idRoot.spacing;
    height: 2*idRoot.radius;

    states: [
        // There is always the default-state "".
        State {
            name: "B";
            PropertyChanges { target: idR0; opacity: 0.0; }
            PropertyChanges { target: idR1; opacity: 1.0; }
            PropertyChanges { target: idR2; opacity: 0.0; }
        },
        State {
            name: "C";
            PropertyChanges { target: idR0; opacity: 0.0; }
            PropertyChanges { target: idR1; opacity: 0.0; }
            PropertyChanges { target: idR2; opacity: 1.0; }
        }
    ]
    transitions: [
        Transition {
            from: "*"; // any state
            to: "*"; // any state
            NumberAnimation {
                target: idR0
                property: "opacity";
                duration: idRoot.durationMs;
                easing.type: Easing.InOutQuad
            }
            NumberAnimation {
                target: idR1
                property: "opacity";
                duration: idRoot.durationMs;
                easing.type: Easing.InOutQuad
            }
            NumberAnimation {
                target: idR2
                property: "opacity";
                duration: idRoot.durationMs;
                easing.type: Easing.InOutQuad
            }
        }
    ]

    Rectangle
    {
        id: idR0;
        x: idRoot.width/2 -idRoot.spacing -width/2;
        y: idRoot.height/2 -height/2;
        width: 2*idRoot.radius;
        height: 2*idRoot.radius;
        opacity: 1.0;
        radius: idRoot.radius;
        color: idRoot.color;
    }

    Rectangle
    {
        id: idR1;
        x: idRoot.width/2 -width/2;
        y: idRoot.height/2 -height/2;
        width: 2*idRoot.radius;
        height: 2*idRoot.radius;
        opacity: 0.0;
        radius: idRoot.radius;
        color: idRoot.color;
    }

    Rectangle
    {
        id: idR2;
        x: idRoot.width/2 +idRoot.spacing -width/2;
        y: idRoot.height/2 -height/2;
        width: 2*idRoot.radius;
        height: 2*idRoot.radius;
        opacity: 0.0;
        radius: idRoot.radius;
        color: idRoot.color;
    }
}

