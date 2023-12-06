import QtQuick 2.12
import EkxMainstateEnum 1.0
import "../CommonItems" as Common

Item
{
    id: idModeIndicator

    width: 85;
    height: idSettingsModeIndicator.y+idSettingsModeIndicator.height;

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // self defined properties
    ///////////////////////////////////////////////////////////////////////////////////////////////

    property bool visualDebug: false;
    property real radiusValue: 2.0
    property real spacingVertical: 16;
    property color highlightColor: idMain.colorEkxTextWhite
    property color inactiveColor: idMain.colorEkxTextGrey

    property int currentMode: -1

    property bool timeModeEnabled: true
    property bool libraryModeEnabled: true

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // signals
    ///////////////////////////////////////////////////////////////////////////////////////////////

    signal selectMode(int mode)

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // items
    ///////////////////////////////////////////////////////////////////////////////////////////////

    // ClassicMode-Indicator:
    Rectangle
    {
        id: idClassicModeIndicator;
        anchors.horizontalCenter: idSettingsModeIndicator.horizontalCenter;
        y: 0
        width: 6
        height: 32
        radius: radiusValue
        enabled: idModeIndicator.currentMode === EkxMainstateEnum.EKX_CLASSIC_MODE
        color: enabled ? idModeIndicator.highlightColor : idModeIndicator.inactiveColor
    }

    MouseArea
    {
        id: idClassicModeSelectArea

        anchors.left: parent.left
        width: parent.width;
        height: 60;
        y: idClassicModeIndicator.y + idClassicModeIndicator.height + spacingVertical/2 -height;
        onClicked:
        {
            idModeIndicator.selectMode(EkxMainstateEnum.EKX_CLASSIC_MODE);
        }
    }

    // TimeMode-indicator:
    Row
    {
        id: idTimeModeIndicator
        visible: idModeIndicator.timeModeEnabled
        anchors.left: idClassicModeIndicator.left;
        anchors.top: idClassicModeIndicator.bottom
        anchors.topMargin: idModeIndicator.timeModeEnabled ? spacingVertical : 0
        spacing: recipeControl.currentTimeRecipeCount > 5 ? 4 : 6
        enabled: idModeIndicator.currentMode === EkxMainstateEnum.EKX_TIME_MODE

        Repeater
        {
            model: recipeControl.currentTimeRecipeCount

            Rectangle
            {
                property int currentRecipe: index;
                color: (enabled && currentRecipe === recipeControl.currentTimeRecipeIndex) ? idModeIndicator.highlightColor : idModeIndicator.inactiveColor
                height: idModeIndicator.timeModeEnabled ? 32 : 0;
                width: (recipeControl.currentTimeRecipeCount > 4 && ( currentRecipe !== recipeControl.currentTimeRecipeIndex ) )  ? 3 : 6;

                radius: radiusValue
            }
        }
    }

    MouseArea
    {
        id: idTimeModeSelectArea
        visible: idModeIndicator.timeModeEnabled;
        anchors.left: parent.left
        anchors.verticalCenter: idTimeModeIndicator.verticalCenter;
        width: parent.width;
        height: idTimeModeIndicator.height + spacingVertical;
        onClicked:
        {
            idModeIndicator.selectMode(EkxMainstateEnum.EKX_TIME_MODE);
        }
    }

    // LibraryMode-Indicator:
    Rectangle
    {
        id: idLibraryModeIndicator;
        visible: idModeIndicator.libraryModeEnabled
        anchors.horizontalCenter: idSettingsModeIndicator.horizontalCenter;
        anchors.top: idTimeModeIndicator.bottom
        anchors.topMargin: idModeIndicator.libraryModeEnabled ? spacingVertical : 0
        width: 6
        height: idModeIndicator.libraryModeEnabled ? 32 : 0
        radius: radiusValue
        enabled: idModeIndicator.currentMode === EkxMainstateEnum.EKX_LIBRARY_MODE
        color: enabled ? idModeIndicator.highlightColor : idModeIndicator.inactiveColor
    }

    MouseArea
    {
        id: idLibraryModeSelectArea

        visible: idModeIndicator.libraryModeEnabled;
        anchors.left: parent.left
        anchors.verticalCenter: idLibraryModeIndicator.verticalCenter;
        width: parent.width;
        height: idLibraryModeIndicator.height + spacingVertical;
        onClicked:
        {
            idModeIndicator.selectMode(EkxMainstateEnum.EKX_LIBRARY_MODE);
        }
    }

    // SettingsMode-Indicator:
    Item
    {
        id: idSettingsModeIndicator
        anchors.top: idLibraryModeIndicator.bottom
        anchors.topMargin: 20
        anchors.left: parent.left
        anchors.leftMargin: 8
        width: 32
        height: 32
        enabled: idModeIndicator.currentMode === EkxMainstateEnum.EKX_SETTINGS_MODE
        // Colorize the icon if the desired highlightColor is different from white.
        layer.enabled: enabled && !Qt.colorEqual(idModeIndicator.highlightColor, idMain.colorEkxTextWhite)
        layer.samplerName: "maskSource"
        layer.effect: Common.LongPressShaderEffect {
            backgroundColor: idModeIndicator.highlightColor
        }

        Image
        {
            id: idSettingsImage
            anchors.top: idSettingsModeIndicator.top
            width: 32
            height: 32
            source: "qrc:/Icons/settings_indicator.png"
            opacity: enabled ? 1.0 : 0.5
        }

        Image
        {
            id: idNotificationImage
            anchors.top: idSettingsImage.top
            anchors.topMargin: 8
            anchors.left: idSettingsImage.right
            width: 16
            height: 16
            source: notificationCenter.modeIndicatorImage
            opacity: enabled ? 1.0 : 0.5
        }
    }

    MouseArea
    {
        id: idSettingsModeSelectArea

        anchors.left: parent.left
        y: idSettingsModeIndicator.y -20 +spacingVertical/2;
        width: parent.width;
        height: 60;
        onClicked:
        {
            idModeIndicator.selectMode(EkxMainstateEnum.EKX_SETTINGS_MODE);
        }
    }
}

