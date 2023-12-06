import QtQml 2.12
import QtQuick 2.12

import "../../CommonItems"

Rectangle
{
    id: root

    width:  480
    height: 360

    color:  "black"

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // self defined properties
    ///////////////////////////////////////////////////////////////////////////////////////////////

    property var libraryRecipeList: recipeControl.installedLibraryList;

    property color currentLibraryIndexColor: idMain.colorEkxTextWhite
    property color otherColor: idMain.colorEkxTextGrey

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // signals
    ///////////////////////////////////////////////////////////////////////////////////////////////

    signal closeBeansListView();
    signal openLibraryListEdit(var recipeIndex);

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // signal handlers
    ///////////////////////////////////////////////////////////////////////////////////////////////

    onVisibleChanged:
    {
        if ( visible )
        {
            //console.log("Beanslist became visible! scrolling to idx="+recipeControl.currentLibraryRecipeIndex);
            //idBeansList.positionViewAtBeginning();
            idBeansList.positionViewAtIndex( recipeControl.currentLibraryRecipeIndex, ListView.Center );
            idJigglingStepperTimer.start();
        }
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // items
    ///////////////////////////////////////////////////////////////////////////////////////////////

    Timer
    {
        id: idJigglingStepperTimer

        property int counter: 0;

        interval: 400;
        running: false;
        repeat: true

        onTriggered:
        {
            var allowedLength = 4;
            if( libraryRecipeList.length < allowedLength)
            {
                allowedLength = libraryRecipeList.length;
            }

            if( counter < allowedLength  )
            {
                idBeansList.contentItem.children[counter].startJiggling();
                counter++;
            }
            else
            {
                counter = 0;
                idJigglingStepperTimer.stop();
            }
        }
    }

    Timer
    {
        id: idSelectCloseTimer

        interval: 700;
        running: false;
        repeat: false

        onTriggered:
        {
            root.closeBeansListView();
        }
    }

    ShadowGroup
    {
        z: 1;
        shadowLeftSize: 100;
        shadowRightSize: 100;
        shadowTopSize: ShadowGroup.ShadowTopSizes.StandardSize;
        shadowBottomSize: ShadowGroup.ShadowBottomSizes.StandardSize;
        shadowBottomEnabled: (idBeansList.count>3 && !idBeansList.atYEnd)?true:false;
        //visualDebug: true
    }

    ListView
    {
        id: idBeansList
        property bool beansListMoving: false;
        //anchors.top: parent.top
        //anchors.topMargin: 80
        anchors.verticalCenter: parent.verticalCenter
        width: root.width
        height: 226; // same bounds position like MenuPage.qml
        displayMarginBeginning: 500;
        displayMarginEnd: 500;
        preferredHighlightBegin:     0
        preferredHighlightEnd:       0
        highlightFollowsCurrentItem: true
        flickableDirection: Flickable.VerticalFlick
        orientation: ListView.Vertical
        interactive: count > 3
        clip: false

        model: libraryRecipeList
        delegate: BeansListEntryView
        {
            id: idBeansListEntryDelegate
            property bool beansListMoving: idBeansList.beansListMoving
            recipeIndex: index
            //currentBeanName: modelData.beanName

            onNewLibraryIndexChoosen:
            {
                idSelectCloseTimer.stop();
                idSelectCloseTimer.start();
            }

            onOpenLibraryListEntryEdit:
            {
                openLibraryListEdit( listLibraryIndex )
            }

            onBeansListMovingChanged:
            {
                idBeansListEntryDelegate.abortLongPress()
            }
        }

        onMovementStarted:
        {
            beansListMoving = true;
        }

        onMovementEnded:
        {
            beansListMoving = false;
        }
        //DebugFrame{visible: true}
    }

    EkxButton
    {
        // id: idButtonClose;
        z: 2;
        buttonType: EkxButton.ButtonType.CLOSE;
        onClicked:
        {
            root.closeBeansListView();
        }
    }

    EkxButton
    {
        // id: idButtonAdd;
        z: 3;
        anchors.right:parent.right
        anchors.bottom: parent.bottom
        buttonType: EkxButton.ButtonType.PLUS;
        onClicked:
        {
            recipeControl.addLibraryRecipe();
            recipeControl.currentLibraryRecipeIndex = libraryRecipeList.length - 1
            openLibraryListEdit( libraryRecipeList.length - 1 );
        }
    }
}
