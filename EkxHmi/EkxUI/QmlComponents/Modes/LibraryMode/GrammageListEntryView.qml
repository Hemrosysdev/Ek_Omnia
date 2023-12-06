import QtQml 2.12
import QtQuick 2.12
import QtQuick.Layouts 1.1
import "../../CommonItems" as Common

Common.SwipableListItem
{
    id: idGrammageListEntryView

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // self defined properties
    ///////////////////////////////////////////////////////////////////////////////////////////////

    property alias grammageMg: idGrammage.grammageMg
    property alias dosingTimeMs: idGrammage.dosingTimeMs
    property alias iconSource: idGrammage.iconSource
    property alias dosingTimeVisible: idGrammage.dosingTimeVisible

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // signals
    ///////////////////////////////////////////////////////////////////////////////////////////////

    signal editGrammage
    signal deleteGrammage

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // signal handlers
    ///////////////////////////////////////////////////////////////////////////////////////////////

    onLongPressed: editGrammage()

    // Reset delete animation to default when default starts.
    defaultLongPressAnimation.onStarted: {
        longPressAnimation = null;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // items
    ///////////////////////////////////////////////////////////////////////////////////////////////

    Grammage {
        id: idGrammage
        anchors {
            left: parent.left
            leftMargin: 100
        }
        width: 280
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
                idGrammageListEntryView.editGrammage();
            }
        },

        Image {
            id: idTrashIcon

            width: 72
            height: 72

            source: "qrc:/Icons/ICON_Trashcan-72px_white.png"

            layer.enabled: idDeleteAnimation.layerEnabled

            // This item should be used as the 'mask'
            layer.samplerName: "maskSource"
            layer.effect: Common.LongPressShaderEffect
            {
                longPressAnimation: idDeleteAnimation
            }

            Common.LongPressAnimation {
                id: idDeleteAnimation

                fillColor: idMain.colorEkxConfirmRed
                longPressOnly: true

                onStarted: {
                    idGrammageListEntryView.longPressAnimation = this;
                }

                onLongPressed: {
                    idGrammageListEntryView.deleteGrammage();
                }
            }
        }
    ]
}
