import QtQuick 2.0

import SettingsSerializerEnum 1.0

Item
{
    id: idTemperature

    visible: tempValid

    width: 240
    height: 64

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // self defined properties
    ///////////////////////////////////////////////////////////////////////////////////////////////

    property int   currentTemp: ntcTempDriver.currentTemp
    property bool  tempValid:  ntcTempDriver.currentTempValid
    property bool  overtempWarningActive:  ntcTempDriver.overtempWarningActive
    property color tempColor: tempValid ? ( overtempWarningActive ? idMain.colorEkxConfirmRed : idMain.colorEkxTextGrey ) : idMain.colorEkxWarning

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // functions
    ///////////////////////////////////////////////////////////////////////////////////////////////

    function calculateFahrenheit( celsiusTemp )
    {
        var fahrenheit = Math.round( ( ( celsiusTemp * 9.0 ) / 5.0 ) + 32.0 );

        return fahrenheit;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // items
    ///////////////////////////////////////////////////////////////////////////////////////////////

    Item
    {
        id: idTemperatureSub

        visible: settingsSerializer.showTempEnable

        anchors.fill: parent

        Image
        {
            id: idHeatIcon

            anchors.bottom: parent.bottom
            anchors.bottomMargin: 16
            anchors.right: idTemperatureText.left
            anchors.rightMargin: 8

            source: "qrc:/Icons/ICON_Heat-40px.png"

            layer.enabled: true;
            layer.smooth: true;
            layer.samplerName: "maskSource"
            layer.effect: ShaderEffect
            {
                property var colorSource: idHeatBackground
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
            }

            Rectangle
            {
                id: idHeatBackground

                visible: false
                anchors.fill: parent

                color: tempColor

                layer.enabled: true;
                layer.smooth: true;
            }
        }

        Text
        {
            id: idTemperatureText

            text: ( settingsSerializer.tempUnit === SettingsSerializerEnum.TEMPUNIT_CELSIUS ) ? currentTemp : calculateFahrenheit( currentTemp );

            anchors.horizontalCenter: parent.horizontalCenter
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 7

            font.pixelSize: 58
            font.letterSpacing: 0.23
            font.family: "D-DIN Condensed HEMRO"

            color: tempColor
        }

        Text
        {
            id: idUnitText

            text: ( settingsSerializer.tempUnit === SettingsSerializerEnum.TEMPUNIT_CELSIUS ) ? "°C" : "°F";

            anchors.bottom: parent.bottom
            anchors.bottomMargin: 16
            anchors.left: idTemperatureText.right
            anchors.rightMargin: 4

            font.pixelSize: 46
            font.letterSpacing: 0.18
            font.family: "D-DIN Condensed HEMRO"

            color: tempColor
        }
    }
}
