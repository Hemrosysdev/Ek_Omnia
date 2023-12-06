import QtQml 2.12
import QtQuick 2.12
import QtQuick.Layouts 1.1
import "../../CommonItems" as Common

Common.SwipableListItem
{
    id: idRecipeListEntryView

    height: showRecipeName && recipeName !== "" ? 128 : implicitHeight

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // self defined properties
    ///////////////////////////////////////////////////////////////////////////////////////////////

    property string recipeName
    property bool showRecipeName

    property real recipeValue: 99.9
    property int  grindingSize: 999

    property int portaFilter: 0

    readonly property var portaFilterImages:
    {
        0: "qrc:/Icons/ICON-Strokes_Single-80px_portafilter_white.png",
        1: "qrc:/Icons/ICON-Strokes_Double-80px_portafilter_white.png",
        2: "qrc:/Icons/ICON-Strokes_Bottomless-80px_portafilter_white.png"
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // signals
    ///////////////////////////////////////////////////////////////////////////////////////////////

    signal selectRecipe
    signal openRecipeEditor
    signal deleteRecipe

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // signal handlers
    ///////////////////////////////////////////////////////////////////////////////////////////////

    onClicked: selectRecipe()
    onLongPressed: openRecipeEditor()

    // Reset delete animation to default when default starts.
    defaultLongPressAnimation.onStarted: {
        longPressAnimation = null;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // items
    ///////////////////////////////////////////////////////////////////////////////////////////////

    ColumnLayout {
        id: idListContentColumn
        anchors.fill: parent
        spacing: -8

        RowLayout {
            id: idListContentRow
            Layout.fillWidth: true

            Row {
                leftPadding: 9
                spacing: 2
                Layout.preferredWidth: 190
                Layout.fillHeight: true

                // clock symbol in front of recipe time
                Image {
                    width: 56
                    height: 56
                    anchors.verticalCenter: parent.verticalCenter
                    source: "qrc:/Icons/ICON_Timer-56px_white 50%.png"
                }

                // text for recipe time
                Common.Label {
                    height: parent.height
                    horizontalAlignment: Text.AlignLeft
                    verticalAlignment: Text.AlignVCenter
                    pixelSize: 58
                    font.letterSpacing: 0.23
                    textColor: idRecipeListEntryView.active ? Common.Label.TextColor.White : Common.Label.TextColor.Grey
                    text: idRecipeListEntryView.recipeValue.toFixed(1).toString()
                }

                Common.Label {
                    height: parent.height
                    verticalAlignment: Text.AlignVCenter
                    pixelSize: 58
                    font.letterSpacing: 0.23
                    textColor: Common.Label.TextColor.Grey
                    text: "s"
                }
            }

            Row {
                Layout.preferredWidth: 190
                Layout.fillHeight: true
                spacing: 2

                // DDD symbol
                Image {
                    width: 56
                    height: 56
                    anchors.verticalCenter: parent.verticalCenter
                    source: "qrc:/Icons/ICON_DiscDistance-56px_white 50%.png"
                }

                Common.Label {
                    height: parent.height
                    verticalAlignment: Text.AlignVCenter
                    pixelSize: 58
                    font.letterSpacing: 0.23
                    textColor: idRecipeListEntryView.active ? Common.Label.TextColor.White : Common.Label.TextColor.Grey
                    text: idRecipeListEntryView.grindingSize
                }
            }

            // porta filter image
            Item {
                width: 80
                height: 80

                Image {
                    anchors.fill: parent
                    source: idRecipeListEntryView.portaFilterImages[idRecipeListEntryView.portaFilter]
                }
            }
        }

        Common.Label {
            id: idRecipeNameLabel
            Layout.fillWidth: true
            leftPadding: 15
            text: idRecipeListEntryView.recipeName
            textFormat: Text.PlainText
            textColor: idRecipeListEntryView.active ? Common.Label.TextColor.White : Common.Label.TextColor.Grey
            pixelSize: 40
            font.letterSpacing: 2
            bold: true
            elide: Text.ElideRight
            visible: idRecipeListEntryView.showRecipeName && text !== ""
        }
    }

    controls: [
        Common.EkxButton {
            id: idPencilIcon
            buttonType: Common.EkxButton.ButtonType.EDIT

            layer.enabled: idDeleteAnimation.layerEnabled
            // This item should be used as the 'mask'
            layer.samplerName: "maskSource"
            layer.effect: Common.LongPressShaderEffect {
                longPressAnimation: idDeleteAnimation
            }

            onClicked: {
                idRecipeListEntryView.openRecipeEditor();
            }
        },

        // trash symbol right of pencil
        Image {
            id: idTrashIcon

            visible: recipeControl.currentTimeRecipeCount > 1

            width: 72
            height: 72

            source: "qrc:/Icons/ICON_Trashcan-72px_white.png"

            layer.enabled: idDeleteAnimation.layerEnabled
            layer.smooth: true;

            // This item should be used as the 'mask'
            layer.samplerName: "maskSource"
            layer.effect: Common.LongPressShaderEffect {
                longPressAnimation: idDeleteAnimation
            }

            // the mouse area over trash symbol
            Common.LongPressAnimation {
                id: idDeleteAnimation

                fillColor: idMain.colorEkxConfirmRed
                longPressOnly: true

                onStarted: {
                    idRecipeListEntryView.longPressAnimation = this;
                }

                onLongPressed: {
                    idRecipeListEntryView.deleteRecipe();
                }
            }
        }
    ]

}
