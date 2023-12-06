import QtQml 2.12
import QtQuick 2.12
import "../../CommonItems" as Common

Common.SwipableListItem
{
    id: idBeansListEntryView

    height: 80 // special height for LibraryMode recipe list entries

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // self defined properties
    ///////////////////////////////////////////////////////////////////////////////////////////////

    property alias text: idBeansNameLabel.text

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

    Common.Label {
        id: idBeansNameLabel
        anchors.fill: parent
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        pixelSize: 58
        font.letterSpacing: 2
        bold: true
        textColor: idBeansListEntryView.active ? Common.Label.White : Common.Label.Grey
        elide: Text.ElideRight
    }

    controls: [
        Common.EkxButton {
            id: idPencilIcon
            buttonType: Common.EkxButton.ButtonType.EDIT

            layer.enabled: idDeleteAnimation.layerEnabled
            layer.samplerName: "maskSource"
            layer.effect: Common.LongPressShaderEffect {
                longPressAnimation: idDeleteAnimation
            }

            onClicked: {
                idBeansListEntryView.openRecipeEditor();
            }
        },

        Image {
            id: idTrashIcon

            visible: recipeControl.installedLibraryList.length > 1

            width: 72
            height: 72

            source: "qrc:/Icons/ICON_Trashcan-72px_white.png"

            layer.enabled: idDeleteAnimation.layerEnabled
            layer.samplerName: "maskSource"
            layer.effect: Common.LongPressShaderEffect {
                longPressAnimation: idDeleteAnimation
            }

            Common.LongPressAnimation {
                id: idDeleteAnimation
                fillColor: idMain.colorEkxConfirmRed
                longPressOnly: true
                onStarted: {
                    idBeansListEntryView.longPressAnimation = this;
                }
                onLongPressed: {
                    idBeansListEntryView.deleteRecipe();
                }
            }
        }
    ]
}
