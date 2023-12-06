import QtQuick 2.0
import EkxMainstateEnum 1.0

//Item
Rectangle
{
    id: idModeIndicator

    width: 85;
    height: idSettingsModeIndicator.y+idSettingsModeIndicator.height;
    color: "transparent";
    border.width: idModeIndicator.visualDebug?5:0;
    border.color: idModeIndicator.visualDebug?"dark red":"transparent";

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // self defined properties
    ///////////////////////////////////////////////////////////////////////////////////////////////

    property bool visualDebug: false;
    property real radiusValue: 2.0
    property real spacingVertical: 16;

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // items
    ///////////////////////////////////////////////////////////////////////////////////////////////

    // ClassicMode-Indicator:
    Rectangle
    {
        id: idClassicModeIndicator;
        property int indicatorIndex : 0
        anchors.horizontalCenter: idSettingsModeIndicator.horizontalCenter;
        y: 0
        width: 6
        height: 32
        radius: radiusValue
        color: (mainStatemachine.ekxMainstate === EkxMainstateEnum.EKX_CLASSIC_MODE) ? idMain.colorEkxTextWhite : idMain.colorEkxTextGrey
    }

    Rectangle
    {
        id: idClassicModeSelectArea

        anchors.left: parent.left
        width: parent.width;
        height: 60;
        y: idClassicModeIndicator.y + idClassicModeIndicator.height + spacingVertical/2 -height;
        color: "transparent";
        border.width: idModeIndicator.visualDebug?1:0;
        border.color: idModeIndicator.visualDebug?"red":"transparent";

        MouseArea
        {
            anchors.fill: parent;
            onClicked:
            {
                mainStatemachine.ekxMainstate = EkxMainstateEnum.EKX_CLASSIC_MODE;
            }
        }
    }

    // TimeMode-indicator:
    Row
    {
        id: idTimeModeIndicator
        property int indicatorIndex : 1
        visible: settingsSerializer.timeModeIsEnabled
        anchors.left: idClassicModeIndicator.left;
        anchors.top: idClassicModeIndicator.bottom
        anchors.topMargin: settingsSerializer.timeModeIsEnabled ? spacingVertical : 0
        spacing: recipeControl.currentTimeRecipeCount > 5 ? 4 : 6

        Repeater
        {
            model: recipeControl.currentTimeRecipeCount

            Rectangle
            {
                property int currentRecipe: index;
                color: ( (mainStatemachine.ekxMainstate === EkxMainstateEnum.EKX_TIME_MODE) && ( currentRecipe == recipeControl.currentTimeRecipeIndex ) ) ? idMain.colorEkxTextWhite : idMain.colorEkxTextGrey;
                height: settingsSerializer.timeModeIsEnabled ? 32 : 0;
                width: (recipeControl.currentTimeRecipeCount > 4 && ( currentRecipe != recipeControl.currentTimeRecipeIndex ) )  ? 3 : 6;

                radius: radiusValue
            }
        }
    }

    Rectangle
    {
        id: idTimeModeSelectArea

        visible: settingsSerializer.timeModeIsEnabled;
        anchors.left: parent.left
        anchors.verticalCenter: idTimeModeIndicator.verticalCenter;
        width: parent.width;
        height: idTimeModeIndicator.height + spacingVertical;
        color: "transparent";
        border.width: idModeIndicator.visualDebug?1:0;
        border.color: idModeIndicator.visualDebug?"orange":"transparent";

        MouseArea
        {
            anchors.fill: parent;
            onClicked:
            {
                mainStatemachine.ekxMainstate = EkxMainstateEnum.EKX_TIME_MODE;
            }
        }
    }

    // LibraryMode-Indicator:
    Rectangle
    {
        id: idLibraryModeIndicator;
        property int indicatorIndex : 2
        visible: settingsSerializer.libraryModeIsEnabled
        anchors.horizontalCenter: idSettingsModeIndicator.horizontalCenter;
        anchors.top: idTimeModeIndicator.bottom
        anchors.topMargin: settingsSerializer.libraryModeIsEnabled ? spacingVertical : 0
        width: 6
        height: settingsSerializer.libraryModeIsEnabled ? 32 : 0
        radius: radiusValue
        //color: indicatorIndex == currentModeIndex ? idMain.colorEkxTextWhite : idMain.colorEkxTextGrey
        color: (mainStatemachine.ekxMainstate === EkxMainstateEnum.EKX_LIBRARY_MODE) ? idMain.colorEkxTextWhite : idMain.colorEkxTextGrey
    }

    Rectangle
    {
        id: idLibraryModeSelectArea

        visible: settingsSerializer.libraryModeIsEnabled;
        anchors.left: parent.left
        anchors.verticalCenter: idLibraryModeIndicator.verticalCenter;
        width: parent.width;
        height: idLibraryModeIndicator.height + spacingVertical;
        color: "transparent";
        border.width: idModeIndicator.visualDebug?1:0;
        border.color: idModeIndicator.visualDebug?"green":"transparent";

        MouseArea
        {
            anchors.fill: parent;
            onClicked:
            {
                mainStatemachine.ekxMainstate = EkxMainstateEnum.EKX_LIBRARY_MODE;
            }
        }
    }

    // SettingsMode-Indicator:
    Item
    {
        id: idSettingsModeIndicator
        property int indicatorIndex : 3
        anchors.top: idLibraryModeIndicator.bottom
        anchors.topMargin: 20
        anchors.left: parent.left
        anchors.leftMargin: 8
        width: 32
        height: 32

        Image
        {
            id: settingImage
            anchors.top: idSettingsModeIndicator.top
            width: 32
            height: 32
            source: "qrc:/Icons/settings_indicator.png"
            opacity: (mainStatemachine.ekxMainstate === EkxMainstateEnum.EKX_MENU) ? 1.0 : 0.5
        }
    }

    Rectangle
    {
        id: idSettingsModeSelectArea

        anchors.left: parent.left
        y: idSettingsModeIndicator.y -20 +spacingVertical/2;
        width: parent.width;
        height: 60;
        color: "transparent";
        border.width: idModeIndicator.visualDebug?1:0;
        border.color: idModeIndicator.visualDebug?"blue":"transparent";

        MouseArea
        {
            anchors.fill: parent;
            onClicked:
            {
                mainStatemachine.ekxMainstate = EkxMainstateEnum.EKX_MENU;
            }
        }
    }
}

