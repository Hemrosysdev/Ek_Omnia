import QtQuick 2.12
import QtQml.Models 2.1

import SettingStatesEnum 1.0
import EkxMainstateEnum 1.0
import MenuEnums 1.0

import "../../CommonItems"

//import QtQuick.Controls 2.12

Rectangle
{
    id: mainMenuPage
    width: 480
    height: 360
    property int currentEkxMainstate: mainStatemachine.ekxMainstate
    color: "black"

    ObjectModel
    {
        id: mainMenuEntries
        MainMenuEntry
        {
            iconImage : "qrc:/Icons/ICON-Strokes_DDD-112px_white.png";
            menuText: "FROGS";
            linkedState: SettingStatesEnum.SETTING_DISCS;
        }
        MainMenuEntry
        {
            iconImage :  "qrc:/Icons/ICON-Strokes_Settings-112px_white.png";
            menuText: "SYSTEM";
            linkedState: SettingStatesEnum.SETTING_SYSTEM;
        }
        MainMenuEntry
        {
            iconImage : "qrc:/Icons/ICON-Strokes_Customize-112px_white.png";
            menuText: "CUSTOMIZE";
            linkedState: SettingStatesEnum.SETTING_CUSTOMIZE;
        }
        MainMenuEntry
        {
            iconImage :"qrc:/Icons/ICON-Strokes_User-MGMT-112px_white.png";
            menuText: "USER MGMT";
            linkedState: SettingStatesEnum.SETTING_USER;
        }
        MainMenuEntry
        {
            iconImage : "qrc:/Icons/ICON-Strokes_Notifications-112px_white.png";
            menuText: "INFO";
            linkedState: SettingStatesEnum.SETTING_INFO;
        }
    }

    ListView
    {
        id: modeList
        width: parent.width/1;
        height: 360;
        orientation: ListView.Horizontal
        flickableDirection: Flickable.HorizontalFlick
        maximumFlickVelocity: 1500; // px/sec
        interactive: true
        clip:false
        spacing: 10
        model: mainMenuEntries
        pressDelay: 300;
        displayMarginBeginning: (parent.width-width)/2;
        displayMarginEnd: (parent.width-width)/2;

        anchors.horizontalCenter: parent.horizontalCenter;
        //anchors.leftMargin: 92
        highlightFollowsCurrentItem: true;
        preferredHighlightBegin: width/2-160/2; // 160 equals item width (MainMenuEntry)
        preferredHighlightEnd: width/2+160/2; // 160 equals item width (MainMenuEntry)

        //highlightRangeMode:  ListView.ApplyRange;
        highlightRangeMode:  ListView.StrictlyEnforceRange;
        //highlightRangeMode:  ListView.NoHighlightRange;

        //boundsMovement: Flickable.StopAtBounds
        boundsMovement: Flickable.FollowBoundsBehavior

        //boundsBehavior: Flickable.StopAtBounds;
        //boundsBehavior: Flickable.DragOverBounds;
        //boundsBehavior: Flickable.OvershootBounds;
        boundsBehavior: Flickable.DragAndOvershootBounds;

        //snapMode: ListView.NoSnap;
        snapMode: ListView.SnapToItem;
        //snapMode: ListView.SnapOneItem;
    }
    onCurrentEkxMainstateChanged: {
        if( currentEkxMainstate !== EkxMainstateEnum.EKX_MENU )
        {
            modeList.positionViewAtBeginning();
        }
    }

    ShadowGroup
    {
        z: 1;
        shadowRightSize: 150;
        shadowLeftSize:  150;
    }

}

