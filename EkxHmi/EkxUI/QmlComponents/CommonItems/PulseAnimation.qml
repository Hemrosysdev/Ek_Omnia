import QtQuick 2.12

SequentialAnimation
{
    id: idAnimation
    loops: Animation.Infinite

    required property Item target
    required property string property

    property color color: "white"
    property color highlightColor: idMain.colorEkxConfirmGreen

    ColorAnimation {
        target: idAnimation.target
        property: idAnimation.property
        duration: 500
        from: idAnimation.color
        to: idAnimation.highlightColor
    }
    PauseAnimation {
        duration: 1000
    }
    ColorAnimation {
        target: idAnimation.target
        property: idAnimation.property
        duration: 500
        to: idAnimation.color
    }
}
