import QtQml 2.12
import QtQuick 2.12
import "../../CommonItems" as Common

Common.OverlayPage
{
    id: idEditCoarsenessPage

    rightActionLongPressEnabled: !workflow.isNewItem
    rightActionLongPressColor: idMain.colorEkxConfirmGreen

    topActionItem: Common.ActivateToggle {
        id: showIconToggle
        anchors.centerIn: parent
        onToggleChanged: workflow.iconOn = active;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // self defined properties
    ///////////////////////////////////////////////////////////////////////////////////////////////

    readonly property var portaFilterIcons: [
        "qrc:/Icons/ICON_Single-80px_pf_transparent.png",
        "qrc:/Icons/ICON_Double-80px_pf_transparent.png",
        "qrc:/Icons/ICON_Bottomless-80px_pf_transparent.png"
    ]

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // signal handlers
    ///////////////////////////////////////////////////////////////////////////////////////////////

    onActivated: {
        showIconToggle.active = workflow.iconOn;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // items
    ///////////////////////////////////////////////////////////////////////////////////////////////

    Common.ShadowGroup
    {
        shadowTopSize: Common.ShadowGroup.ShadowTopSizes.StandardSize;
        shadowBottomSize: Common.ShadowGroup.ShadowBottomSizes.StandardSize;
    }

    Column
    {
        id: idPortaFilterTypeChooser

        z:4
        anchors.left: parent.left
        anchors.leftMargin: 47
        anchors.top: parent.top
        anchors.topMargin: 44
        spacing: 16

        Repeater
        {
            model: idEditCoarsenessPage.portaFilterIcons

            Item
            {
                height: 80
                width: 409

                Rectangle
                {
                    anchors.fill: idPortaFilterIcon
                    anchors.centerIn: idPortaFilterIcon
                    color: workflow.portaFilter === index ? idMain.colorEkxTextWhite : idMain.colorEkxTextGrey
                }

                Image
                {
                    id: idPortaFilterIcon

                    width: 80
                    height: 80
                    source: modelData
                    anchors.centerIn: parent

                    MouseArea
                    {
                        anchors.fill: parent

                        onClicked:
                        {
                            workflow.portaFilter = index;
                        }
                    }
                }
            }
        }
    }
}
