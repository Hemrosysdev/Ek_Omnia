import QtQuick 2.12

Text
{
    id: idText

    /** Whether font is bold. */
    property bool bold: false
    /** The font size in px. */
    property int pixelSize: 40

    /** The known font family. */
    enum FontFamily
    {
        Condensed, ///< D-DIN Condensed HEMRO
        MonoNumbers, ///< D-DIN Condensed HEMRO _Mono_Numbers
        LightNumbers ///<D-DIN Condensed HEMRO Light_Numbers
    }
    /** The font family to use, default is Condensed. */
    property int fontFamily: Label.Condensed

    /** The known text colors. */
    enum TextColor
    {
        White,
        Grey,
        ConfirmGreen,
        ConfirmRed
        // Add new colors as needed.
    }
    /** The known text color to use. */
    property int textColor: Label.TextColor.White

    color: {
        switch (textColor) {
        case Label.White:
            // TODO Move colors into a singleton type
            return idMain.colorEkxTextWhite;
        case Label.Grey:
            return idMain.colorEkxTextGrey;
        case Label.ConfirmGreen:
            return idMain.colorEkxConfirmGreen
        case Label.ConfirmRed:
            return idMain.colorEkxConfirmRed
        default:
            return enabled ? idMain.colorEkxTextWhite : idMain.colorEkxTextGrey;
        }
    }

    font.family: {
        switch (fontFamily) {
        case Label.Condensed:
            return "D-DIN Condensed HEMRO";
        case Label.MonoNumbers:
            return "D-DIN Condensed HEMRO _Mono_Numbers";
        case Label.LightNumbers:
            return "D-DIN Condensed HEMRO Light_Numbers";
        }
    }

    font.bold: idText.bold
    font.pixelSize: idText.pixelSize

    textFormat: Text.PlainText
}
