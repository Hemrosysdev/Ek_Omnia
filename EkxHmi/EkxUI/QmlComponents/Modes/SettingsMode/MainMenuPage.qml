import QtQuick 2.12
import QtQml.Models 2.1

import SettingsStateEnum 1.0
import EkxMainstateEnum 1.0
import MenuEnums 1.0

import "../../CommonItems"

Rectangle
{
    id: mainMenuPage

    width: 480
    height: 360

    color: "black"

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // self defined properties
    ///////////////////////////////////////////////////////////////////////////////////////////////

    property int currentEkxMainstate: mainStatemachine.ekxMainstate

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // signal handlers
    ///////////////////////////////////////////////////////////////////////////////////////////////

    onCurrentEkxMainstateChanged:
    {
        if( currentEkxMainstate !== EkxMainstateEnum.EKX_SETTINGS_MODE )
        {
            modeList.positionViewAtBeginning();
        }
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // items
    ///////////////////////////////////////////////////////////////////////////////////////////////

    ObjectModel
    {
        id: mainMenuEntries
        MainMenuEntry
        {
            iconImage : "qrc:/Icons/Service.png";
            notificationImage: "";
            menuText: "SERVICE";
            linkedState: SettingsStateEnum.SETTING_SERVICE;
        }
        MainMenuEntry
        {
            iconImage :  "qrc:/Icons/ICON-Strokes_Settings-112px_white.png";
            notificationImage: "";
            menuText: "SYSTEM";
            linkedState: SettingsStateEnum.SETTING_SYSTEM;
        }
        MainMenuEntry
        {
            iconImage : "qrc:/Icons/ICON-Strokes_Customize-112px_white.png";
            notificationImage: "";
            menuText: "CUSTOMIZE";
            linkedState: SettingsStateEnum.SETTING_CUSTOMIZE;
        }
        MainMenuEntry
        {
            iconImage :"qrc:/Icons/ICON-Strokes_User-MGMT-112px_white.png";
            notificationImage: "";
            menuText: "USER MGMT";
            linkedState: SettingsStateEnum.SETTING_USER;
        }
        MainMenuEntry
        {
            iconImage : "qrc:/Icons/ICON-Strokes_Notifications-112px_white.png";
            notificationImage : notificationCenter.menuImage;
            menuText: "INFO";
            linkedState: SettingsStateEnum.SETTING_INFO;
        }
    }

    ListView
    {
        id: modeList

        width: parent.width;
        height: parent.height;

        anchors.horizontalCenter: parent.horizontalCenter;

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

        highlightFollowsCurrentItem: true;
        preferredHighlightBegin: width/2-160/2; // 160 equals item width (MainMenuEntry)
        preferredHighlightEnd: width/2+160/2; // 160 equals item width (MainMenuEntry)

        highlightRangeMode:  ListView.StrictlyEnforceRange;
        boundsMovement: Flickable.FollowBoundsBehavior
        boundsBehavior: Flickable.DragAndOvershootBounds;
        snapMode: ListView.SnapToItem;
    }

    ShadowGroup
    {
        z: 1;
        shadowRightSize: 150;
        shadowLeftSize:  150;
    }
}

