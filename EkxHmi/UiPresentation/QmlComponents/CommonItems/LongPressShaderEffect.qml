import QtQuick 2.0

ShaderEffect
{
    id: idRoot

    fragmentShader: "
                uniform lowp sampler2D colorSource;
                uniform lowp sampler2D maskSource;
                uniform lowp float qt_Opacity;
                varying highp vec2 qt_TexCoord0;
                void main() {
                    gl_FragColor =
                        texture2D(colorSource, qt_TexCoord0)
                        * texture2D(maskSource, qt_TexCoord0).a
                        * qt_Opacity;
                }
            "

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // self defined properties
    ///////////////////////////////////////////////////////////////////////////////////////////////

    property LongPressAnimation longPressAnimation
    property color fillColor: longPressAnimation ? longPressAnimation.fillColor : "orange";
    property color backgroundColor: idMain.colorEkxTextWhite;
    property var colorSource: idAnimatedFillRect;

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // functions
    ///////////////////////////////////////////////////////////////////////////////////////////////

    function incrementFillHeight()
    {
        if ( longPressAnimation )
        {
            idFillerRect.height += idBackgroundRect.height / ( longPressAnimation.incrementCntStopAnimation - longPressAnimation.incrementCntStartAnimation - 1 );
        }
    }

    function resetFillHeight()
    {
        if ( longPressAnimation )
        {
            idFillerRect.height = 0;
        }
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // signal handlers
    ///////////////////////////////////////////////////////////////////////////////////////////////

    onLongPressAnimationChanged:
    {
        if ( longPressAnimation )
        {
            //console.log("onLongPressAnimationChanged to " + longPressAnimation.objectName )
            longPressAnimation.fillerIncremented.connect( incrementFillHeight )
            longPressAnimation.fillerReset.connect( resetFillHeight )
            fillColor = longPressAnimation.fillColor
        }
        else
        {
            //console.log("onLongPressAnimationChanged null " + parent)
        }
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // items
    ///////////////////////////////////////////////////////////////////////////////////////////////

    Rectangle
    {
        id: idAnimatedFillRect

        visible: false
        width: 10
        height: 100

        layer.enabled: true;
        layer.smooth: true;

        Rectangle
        {
            id: idBackgroundRect;

            visible: true;
            anchors.fill: parent
            anchors.centerIn: parent
            color: backgroundColor

            Rectangle
            {
                id: idFillerRect

                width: parent.width
                height: 0
                anchors.bottom: parent.bottom
                color: fillColor
            }
        }
    }
}
