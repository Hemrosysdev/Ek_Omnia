import QtQuick 2.12
import QtQml 2.12
import QtQml.Models 2.1
import "ClassicMode"
import "LibraryMode"
import "TimeMode"
import "Settings"
import EkxMainstateEnum 1.0
import SettingStatesEnum 1.0
import QtQuick.Controls 2.12 // just for dev buttons

Item
{
    id: idModeSelector;

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // properties
    ///////////////////////////////////////////////////////////////////////////////////////////////

    property bool debugOverlayEnable: false;
    property bool debugPrintsEnable: false;

    property int currentMode: mainStatemachine.ekxMainstate;
    //property bool modeSwitchingIsAllowed: true;

    property bool modeSwitchingIsAllowed:    !(currentMode===EkxMainstateEnum.EKX_CLASSIC_MODE && (idClassicMode.modeIsLocked))
                                          && !(currentMode===EkxMainstateEnum.EKX_TIME_MODE && (idTimeMode.modeIsLocked))
                                          //&& !(currentMode===EkxMainstateEnum.EKX_LIBRARY_MODE && (idLibraryMode.editLibraryRecipe || idLibraryMode.showBeansList))
                                          && !(currentMode===EkxMainstateEnum.EKX_LIBRARY_MODE && (idLibraryMode.modeIsLocked))
                                          && !(currentMode===EkxMainstateEnum.EKX_MENU && (idSettingsMode.currentSettingState!==SettingStatesEnum.SETTING_MAIN));
    property bool timeModeEnabled: settingsSerializer.timeModeIsEnabled;
    property bool libraryModeEnabled: settingsSerializer.libraryModeIsEnabled;

    width: 480;
    height: 360;

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // functions
    ///////////////////////////////////////////////////////////////////////////////////////////////

    function index2mode(modeIndex)
    {

        let modeId = EkxMainstateEnum.EKX_UNKNOWN;
        let classicModeIdx  = 0;
        let timeModeIdx     = 1;
        let libraryModeIdx  = 1+(timeModeEnabled?1:0);
        let settingsModeIdx = 1+(timeModeEnabled?1:0)+(libraryModeEnabled?1:0);

        if(modeIndex===classicModeIdx)
        {
            modeId = EkxMainstateEnum.EKX_CLASSIC_MODE; // ClassicMode
        }
        else if(modeIndex===timeModeIdx && timeModeEnabled)
        {
            modeId = EkxMainstateEnum.EKX_TIME_MODE; // TimeMode
        }
        else if(modeIndex===libraryModeIdx && libraryModeEnabled)
        {
            modeId = EkxMainstateEnum.EKX_LIBRARY_MODE; // LibraryMode
        }
        else if(modeIndex===settingsModeIdx || modeIndex > settingsModeIdx)
        {
            modeId = EkxMainstateEnum.EKX_MENU; // SettingsMode
        }

        return(modeId);
    }

    function mode2index(modeId)
    {
        let modeIndex = -1;

        if(modeId===EkxMainstateEnum.EKX_CLASSIC_MODE)
            modeIndex = 0;
        else if(modeId===EkxMainstateEnum.EKX_TIME_MODE && timeModeEnabled)
            modeIndex = 1;
        else if(modeId===EkxMainstateEnum.EKX_LIBRARY_MODE && libraryModeEnabled)
            modeIndex = 1 + (timeModeEnabled?1:0);
        else if(modeId===EkxMainstateEnum.EKX_MENU)
            modeIndex = 1 + (timeModeEnabled?1:0) + (libraryModeEnabled?1:0);

        return(modeIndex);
    }

    function mode2string(modeId)
    {
        let modeName="UnknownMode";

        if(modeId===EkxMainstateEnum.EKX_CLASSIC_MODE)
            modeName = "ClassicMode";
        else if(modeId===EkxMainstateEnum.EKX_TIME_MODE && timeModeEnabled)
            modeName = "TimeMode";
        else if(modeId===EkxMainstateEnum.EKX_LIBRARY_MODE && libraryModeEnabled)
            modeName = "LibraryMode";
        else if(modeId===EkxMainstateEnum.EKX_MENU)
            modeName = "SettingsMode";

        return(modeName);
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // signal handlers
    ///////////////////////////////////////////////////////////////////////////////////////////////

    Component.onCompleted:
    {
        idClassicMode.resetModeInfo();
        idTimeMode.resetModeInfo();
        idLibraryMode.resetModeInfo();
        idSettingsMode.resetModeInfo();
        idTimeMode.visible = timeModeEnabled;
        idLibraryMode.visible = libraryModeEnabled;
    }

    onCurrentModeChanged:
    {
        if(debugPrintsEnable)
        {
            console.log("ModeSelector2: EkxMode changed to "+mode2string(currentMode))
        }
        idSwipeView.scrollToMode( currentMode )
    }

    onModeSwitchingIsAllowedChanged:
    {
        if(debugPrintsEnable)
        {
            console.log("ModeSelector2: modeSwitchingIsAllowed = "+modeSwitchingIsAllowed)
        }
    }

    onTimeModeEnabledChanged:
    {
        if(timeModeEnabled)
        {
            idTimeMode.visible = true;
            idTimeMode.width   = 480;
            idTimeMode.height  = 360;
            idSwipeView.insertItem(1, idTimeMode);
        }
        else
        {
            idTimeMode.visible = false;
            idTimeMode.width   = 0;
            idTimeMode.height  = 0;
            idSwipeView.removePage(idTimeMode);
        }
    }

    onLibraryModeEnabledChanged:
    {
        if(libraryModeEnabled)
        {
            idLibraryMode.visible = true;
            idLibraryMode.width   = 480;
            idLibraryMode.height  = 360;
            idSwipeView.insertItem((timeModeEnabled?2:1),idLibraryMode);
        }
        else
        {
            idLibraryMode.visible = false;
            idLibraryMode.width   = 0;
            idLibraryMode.height  = 0;
            idSwipeView.removePage(idLibraryMode);
        }
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // items
    ///////////////////////////////////////////////////////////////////////////////////////////////

    ObjectModel
    {
        id: idListOfAllModes;

        ClassicMode
        {
            id: idClassicMode;
        }

        TimeMode
        {
            id: idTimeMode;
        }

        LibraryMode
        {
            id: idLibraryMode;
        }

        Settings
        {
            id: idSettingsMode;
        }
    }

    SwipeView
    {
        id: idSwipeView;

        anchors.fill: parent;
        interactive: idModeSelector.modeSwitchingIsAllowed;
        orientation: Qt.Vertical;

        // https://stackoverflow.com/questions/49387541/qt-qml-swipeview-change-animation-transition-speed
        contentItem: ListView
        {
            model: idSwipeView.contentModel
            interactive: idSwipeView.interactive
            currentIndex: idSwipeView.currentIndex

            spacing: idSwipeView.spacing
            orientation: idSwipeView.orientation
            snapMode: ListView.SnapOneItem
            boundsBehavior: Flickable.StopAtBounds

            highlightRangeMode: ListView.StrictlyEnforceRange
            preferredHighlightBegin: 0
            preferredHighlightEnd: 0
            highlightMoveDuration: 250
            //                    min:10
            maximumFlickVelocity: 4 * (idSwipeView.orientation === Qt.Horizontal ? width : height)

            onMovementEnded:
            {
                let modeId =  index2mode( idSwipeView.currentIndex )
                if( modeId !== mainStatemachine.ekxMainstate )
                {
                    if(debugPrintsEnable)
                    {
                        console.log("ModeSelector2: idSwipeView/onMovementEnded: ekxMainstate ="+mode2string(modeId) );
                    }
                    mainStatemachine.ekxMainstate = modeId;
                }
                else
                {
                    if(debugPrintsEnable)
                    {
                        console.log("ModeSelector2: idSwipeView/onMovementEnded: stay in mode "+mode2string(modeId));
                    }
                }
            }
            onMovementStarted:
            {
                if(currentMode==EkxMainstateEnum.EKX_TIME_MODE)
                {
                    idTimeMode.abortLongPress();
                }
                else if(currentMode==EkxMainstateEnum.EKX_LIBRARY_MODE)
                {
                    idLibraryMode.abortLongPress();
                }
            }
        }

        Component.onCompleted:
        {
            addItem(idClassicMode);
            addItem(idTimeMode);
            addItem(idLibraryMode);
            addItem(idSettingsMode);
        }

        onCurrentIndexChanged:
        {
            if(debugPrintsEnable)
            {
                console.log("ModeSelector2: idSwipeView/onCurrentIndexChanged: recognized "+mode2string(index2mode(currentIndex))+"!");
            }
        }

        function scrollToMode(mode)
        {
            let index = mode2index(mode);
            if( index !== -1 && index!==currentIndex )
                setCurrentIndex(index);
        }

        function addPage(page) {
            addItem(page)
            page.visible = true
        }

        function removePage(page) {
            for (var n = 0; n < count; n++) { if (page === itemAt(n)) { removeItem(n) } }
            page.visible = false
        }
    }

    ModeIndicator
    {
        visible: idModeSelector.modeSwitchingIsAllowed;
        z: 1
        anchors.left: parent.left
        anchors.verticalCenter: parent.verticalCenter
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // debug helpers
    ///////////////////////////////////////////////////////////////////////////////////////////////

    Text
    {
        visible: debugOverlayEnable;

        x: 20;
        y: 20;
        color: "white";
        font.pixelSize: 20;
        text:
        {
            let modeIndex = idSwipeView.currentIndex;
            let modeId    = index2mode(modeIndex);
            return ("SwipeView currentIndex=" + modeIndex + "\nrecognizedMode = " + mode2string( modeId ) );
        }
    }

    Button
    {
        visible: debugOverlayEnable;

        x: 350;
        y: 80;
        width: 120;
        height: 30;
        font.pixelSize: 10;
        text: settingsSerializer.timeModeIsEnabled?"disable TimeMode":"enable TimeMode";
        onClicked:
        {
            if(settingsSerializer.timeModeIsEnabled)
                settingsSerializer.timeModeIsEnabled = false;
            else
                settingsSerializer.timeModeIsEnabled = true;
        }
    }

    Button
    {
        visible: debugOverlayEnable;

        x: 350;
        y: 120;
        width: 120;
        height: 30;
        font.pixelSize: 10;
        text: settingsSerializer.libraryModeIsEnabled?"disable LibraryMode":"enable LibraryMode";
        onClicked:
        {
            if(settingsSerializer.libraryModeIsEnabled)
                settingsSerializer.libraryModeIsEnabled = false;
            else
                settingsSerializer.libraryModeIsEnabled = true;
        }
    }

}
