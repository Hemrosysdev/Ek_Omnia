import QtQuick 2.12

Item
{
    id: idTestScreen

    anchors.fill: parent

    states: [
        State
        {
            name: "stateHorizontalGradient"
            PropertyChanges { target: idHorizontalGradientTest; visible: true }
            PropertyChanges { target: idVerticalGradientTest; visible: false }
        },
        State
        {
            name: "stateVerticalGradient"
            PropertyChanges { target: idHorizontalGradientTest; visible: false }
            PropertyChanges { target: idVerticalGradientTest; visible: true }
        }
    ]

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // self defined properties
    ///////////////////////////////////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // functions
    ///////////////////////////////////////////////////////////////////////////////////////////////

    function save()
    {
        // do nothing
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // signals
    ///////////////////////////////////////////////////////////////////////////////////////////////

    signal nextTestScreenTriggerd()
    signal finalTestScreenTriggered()

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // signal handlers
    ///////////////////////////////////////////////////////////////////////////////////////////////

    Component.onCompleted:
    {
        idTestScreen.state = "stateHorizontalGradient"
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // items
    ///////////////////////////////////////////////////////////////////////////////////////////////

    MouseArea
    {
        id: idStateToggle

        anchors.fill: parent

        onClicked:
        {
            if ( idTestScreen.state === "stateHorizontalGradient" )
            {
                idTestScreen.state = "stateVerticalGradient"
                nextTestScreenTriggerd()
            }
            else
            {
                idTestScreen.state = "stateHorizontalGradient"
                nextTestScreenTriggerd()
                finalTestScreenTriggered();
            }
        }
    }

    Item
    {
        id: idHorizontalGradientTest

        anchors.fill: parent

        Rectangle
        {
            id: idHWhite

            width: parent.width;
            height: parent.height / 4

            anchors.top: parent.top
            anchors.left: parent.left

            gradient: Gradient
            {
                orientation: Gradient.Horizontal
                GradientStop { position: 0.0; color: "#ffffff" }
                GradientStop { position: 1.0; color: "black"}
            }
        }

        Rectangle
        {
            id: idHRed

            width: parent.width;
            height: parent.height / 4

            anchors.top: idHWhite.bottom
            anchors.left: parent.left

            gradient: Gradient
            {
                orientation: Gradient.Horizontal
                GradientStop { position: 0.0; color: "red" }
                GradientStop { position: 1.0; color: "black"}
            }
        }

        Rectangle
        {
            id: idHGreen

            width: parent.width;
            height: parent.height / 4

            anchors.top: idHRed.bottom
            anchors.left: parent.left

            gradient: Gradient
            {
                orientation: Gradient.Horizontal
                GradientStop { position: 0.0; color: "#00fd00" }
                GradientStop { position: 1.0; color: "black"}
            }
        }

        Rectangle
        {
            id: idHBlue

            width: parent.width;
            height: parent.height / 4

            anchors.top: idHGreen.bottom
            anchors.left: parent.left

            gradient: Gradient
            {
                orientation: Gradient.Horizontal
                GradientStop { position: 0.0; color: "blue" }
                GradientStop { position: 1.0; color: "black"}
            }
        }
    }

    Item
    {
        id: idVerticalGradientTest

        anchors.fill: parent

        Rectangle
        {
            id: idVWhite

            width: parent.width / 4;
            height: parent.height

            anchors.top: parent.top
            anchors.left: parent.left

            gradient: Gradient
            {
                GradientStop { position: 0.0; color: "#ffffff" }
                GradientStop { position: 1.0; color: "black"}
            }
        }

        Rectangle
        {
            id: idVRed

            width: parent.width / 4;
            height: parent.height

            anchors.top: parent.top
            anchors.left: idVWhite.right

            gradient: Gradient
            {
                GradientStop { position: 0.0; color: "red" }
                GradientStop { position: 1.0; color: "black"}
            }
        }

        Rectangle
        {
            id: idVGreen

            width: parent.width / 4;
            height: parent.height

            anchors.top: parent.top
            anchors.left: idVRed.right

            gradient: Gradient
            {
                GradientStop { position: 0.0; color: "#00fd00" }
                GradientStop { position: 1.0; color: "black"}
            }
        }

        Rectangle
        {
            id: idVBlue

            width: parent.width / 4;
            height: parent.height

            anchors.top: parent.top
            anchors.left: idVGreen.right

            gradient: Gradient
            {
                GradientStop { position: 0.0; color: "blue" }
                GradientStop { position: 1.0; color: "black"}
            }
        }
    }
}
