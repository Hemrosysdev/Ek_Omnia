import QtQuick 2.12

ShaderEffect
{
    id: idRoot

    supportsAtlasTextures: true
    blending: longPressAnimation ? longPressAnimation.blending : false
    fragmentShader: "
                uniform lowp float qt_Opacity;
                varying highp vec2 qt_TexCoord0;

                uniform lowp sampler2D maskSource;
                uniform lowp vec4 fillColor;
                uniform lowp vec4 backgroundColor;

                uniform highp float progress;
                uniform bool useLuminosityFill;

                // 1/256, i.e. smallest change in a 8bpp RGB value.
                const lowp float epsilon = 0.00390625;

                void main() {
                    highp vec4 sourceColor = texture2D(maskSource, qt_TexCoord0.st);

                    highp vec4 mixColor = backgroundColor;
                    if (qt_TexCoord0.y > (1.0 - progress)) {
                        // For the fill color use the source color's luminosity as alpha channel.
                        // This ensures that darker text is highlighted darker and a black mask
                        // isn't just completely filled with a solid intense color.
                        // Since the non-filled part is also used for colorizing the source,
                        // where we want the original unaltered color, this blending is only
                        // done for the filled area.
                        if (useLuminosityFill) {
                            lowp float luminosity = 0.2126 * sourceColor.r + 0.7152 * sourceColor.g + 0.0722 * sourceColor.b;

                            mixColor = vec4(fillColor.rgb * luminosity, luminosity);
                        } else {
                            mixColor = fillColor;
                        }
                    }

                    gl_FragColor = vec4( mix( sourceColor.rgb / max(sourceColor.a, epsilon),
                                              mixColor.rgb / max(mixColor.a, epsilon),
                                              mixColor.a)
                                         * sourceColor.a, sourceColor.a) * qt_Opacity;
                }
            "

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // self defined properties
    ///////////////////////////////////////////////////////////////////////////////////////////////

    property LongPressAnimation longPressAnimation
    property color fillColor: longPressAnimation ? longPressAnimation.fillColor : "orange";
    // Default "transparent" effectively means "use item color/background as-is".
    property color backgroundColor: "transparent"
    property real progress: longPressAnimation ? longPressAnimation.progress : 0
    /*
     * Whether to mix the fillColor with the luminosity of the source.
     */
    property bool useLuminosityFill: true
}
