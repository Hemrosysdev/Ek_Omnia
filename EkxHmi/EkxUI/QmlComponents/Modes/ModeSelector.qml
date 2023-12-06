import QtQuick 2.12
import QtQml 2.12
import QtQml.Models 2.1
import "ClassicMode"
import "LibraryMode"
import "TimeMode"
import "SettingsMode"
import EkxMainstateEnum 1.0
import SettingsStateEnum 1.0
import QtQuick.Controls 2.12 // just for dev buttons

Item
{
    id: idModeSelector;

    anchors.fill: parent

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // properties
    ///////////////////////////////////////////////////////////////////////////////////////////////

    property bool debugOverlayEnable: false;
    property bool debugPrintsEnable: false;

    property int currentMode: mainStatemachine.ekxMainstate;
    readonly property alias currentPage: swipeListView.currentItem

    property bool blendOutEnabled: true

    readonly property bool modeSwitchingIsAllowed: {
        if (mainStatemachine.grindRunning) {
            return false;
        }

        if (idOverlayView.visible) {
            return false;
        }

        if (!currentPage || !currentPage.hasOwnProperty("modeIsLocked")) {
            return true;
        }

        return currentPage.modeIsLocked === false;
    }

    readonly property bool grindingAllowed: {
        if (idOverlayView.visible && idOverlayView.grindingLocked) {
            return false;
        }

        if (!currentPage || !currentPage.hasOwnProperty("grindingLocked")) {
            return true;
        }

        return currentPage.grindingLocked === false;
    }

    // Tutorial needs all modes available.
    property bool allModesEnabled: false
    property bool timeModeEnabled: allModesEnabled || settingsSerializer.timeModeIsEnabled;
    property bool libraryModeEnabled: allModesEnabled || settingsSerializer.libraryModeIsEnabled;

    // Tutorial needs access to these modes.
    property alias classicMode: idClassicMode
    property alias timeMode: idTimeMode
    property alias libraryMode: idLibraryMode

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
            modeId = EkxMainstateEnum.EKX_SETTINGS_MODE; // SettingsMode
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
        else if(modeId===EkxMainstateEnum.EKX_SETTINGS_MODE)
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
        else if(modeId===EkxMainstateEnum.EKX_SETTINGS_MODE)
            modeName = "SettingsMode";

        return(modeName);
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // signal handlers
    ///////////////////////////////////////////////////////////////////////////////////////////////

    Component.onCompleted:
    {
        //idTimeMode.visible = timeModeEnabled;
        //idLibraryMode.visible = libraryModeEnabled;
    }

    onCurrentModeChanged:
    {
        if(debugPrintsEnable)
        {
            console.log("ModeSelector: EkxMode changed to "+mode2string(currentMode))
        }
        idSwipeView.scrollToMode( currentMode )
    }

    onModeSwitchingIsAllowedChanged:
    {
        if(debugPrintsEnable)
        {
            console.log("ModeSelector: modeSwitchingIsAllowed = "+modeSwitchingIsAllowed)
        }
    }

    onTimeModeEnabledChanged:
    {
        if(timeModeEnabled)
        {
            idSwipeView.insertItem(1, idTimeMode);
        }
        else
        {
            idSwipeView.removePage(idTimeMode);
        }

        // When a mode gets added/removed, make sure really hard that the current mode is displayed correctly.
        idSwipeView.scrollToMode(idModeSelector.currentMode);
    }

    onLibraryModeEnabledChanged:
    {
        if(libraryModeEnabled)
        {
            idSwipeView.insertItem((timeModeEnabled?2:1),idLibraryMode);
        }
        else
        {
            idSwipeView.removePage(idLibraryMode);
        }

        idSwipeView.scrollToMode(idModeSelector.currentMode);
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // items
    ///////////////////////////////////////////////////////////////////////////////////////////////

    Binding
    {
        target: mainStatemachine
        property: "grindingAllowed"
        value: idModeSelector.grindingAllowed
    }

    ObjectModel
    {
        id: idListOfAllModes;

        ClassicMode
        {
            id: idClassicMode;
            blendOutEnabled: idModeSelector.blendOutEnabled
            visible: idMain.isDelegateEffectivelyVisible(this)
        }

        TimeMode
        {
            id: idTimeMode;
            blendOutEnabled: idModeSelector.blendOutEnabled
            visible: idMain.isDelegateEffectivelyVisible(this)
        }

        LibraryMode
        {
            id: idLibraryMode;
            blendOutEnabled: idModeSelector.blendOutEnabled
            visible: idMain.isDelegateEffectivelyVisible(this)

        }

        SettingsMode
        {
            id: idSettingsMode;
            blendOutEnabled: idModeSelector.blendOutEnabled
            visible: idMain.isDelegateEffectivelyVisible(this)
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
            id: swipeListView;

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
                        console.log("ModeSelector: idSwipeView/onMovementEnded: ekxMainstate ="+mode2string(modeId) );
                    }
                    mainStatemachine.ekxMainstate = modeId;
                }
                else
                {
                    if(debugPrintsEnable)
                    {
                        console.log("ModeSelector: idSwipeView/onMovementEnded: stay in mode "+mode2string(modeId));
                    }
                }
            }
        }

        Component.onCompleted:
        {
            addItem(idClassicMode);
            if ( idModeSelector.timeModeEnabled )
                addItem(idTimeMode);
            if ( idModeSelector.libraryModeEnabled )
                addItem(idLibraryMode);
            addItem(idSettingsMode);

            scrollToMode(idModeSelector.currentMode);
        }

        onCurrentIndexChanged:
        {
            if(debugPrintsEnable)
            {
                console.log("ModeSelector: idSwipeView.onCurrentIndexChanged: recognized "+mode2string(index2mode(currentIndex))+"!");
            }
        }

        function scrollToMode(mode)
        {
            let index = mode2index(mode);
            if( index !== -1 && index!==currentIndex )
                setCurrentIndex( index );
        }

        function removePage( page )
        {
            for (var n = 0; n < count; n++)
            {
                if (page === itemAt(n))
                {
                    removeItem(n)
                }
            }
            //page.visible = false
        }
    }

    ModeIndicator
    {
        anchors.left: parent.left
        anchors.verticalCenter: parent.verticalCenter
        currentMode: idModeSelector.currentMode
        timeModeEnabled: idModeSelector.timeModeEnabled
        libraryModeEnabled: idModeSelector.libraryModeEnabled
        visible: idModeSelector.modeSwitchingIsAllowed;
        z: 1

        onSelectMode: {
            mainStatemachine.ekxMainstate = mode;
        }
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // debug helpers
    ///////////////////////////////////////////////////////////////////////////////////////////////

    //    Text
    //    {
    //        visible: debugOverlayEnable;

    //        x: 20;
    //        y: 20;
    //        color: "white";
    //        font.pixelSize: 20;
    //        text:
    //        {
    //            let modeIndex = idSwipeView.currentIndex;
    //            let modeId    = index2mode(modeIndex);
    //            return ("SwipeView currentIndex=" + modeIndex + "\nrecognizedMode = " + mode2string( modeId ) );
    //        }
    //    }

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
