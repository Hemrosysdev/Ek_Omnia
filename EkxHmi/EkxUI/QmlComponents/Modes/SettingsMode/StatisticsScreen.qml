import QtQuick 2.12
import QtQml 2.12
import "../../CommonItems"


// TODO
// (W) qrc:/QmlComponents/Modes/Settings/StatisticsScreen.qml:69:19: QML StatisticsScreenEntry: Binding loop detected for property "height"


Rectangle
{
    id: idRoot;

    property bool periodMenuActive: false;
    property string titleBackup: "";
    property string errorText: statisticsBoard.errorText;

    width: 480;
    height: 360;
    color: "black";

    onPeriodMenuActiveChanged:
    {
        if(periodMenuActive)
        {
            //settingsMenuContentFactory.buttonOkVisible = false;
            settingsMenuContentFactory.buttonPreviousVisible = false;
            settingsMenuContentFactory.navAvailable = false;
            titleBackup = settingsMenuContentFactory.menuTitle;
            settingsMenuContentFactory.menuTitle = "TIME PERIOD";
        }
        else
        {
            //settingsMenuContentFactory.buttonOkVisible = true;
            settingsMenuContentFactory.buttonPreviousVisible = true;
            settingsMenuContentFactory.navAvailable = true;
            settingsMenuContentFactory.menuTitle = titleBackup;
        }
    }

    onErrorTextChanged:
    {
        console.log( "errorText " + errorText)
    }

    Text
    {
        id: idErrorText

        visible: errorText!==""
        anchors.fill: parent

        text: errorText

        horizontalAlignment: Text.AlignHCenter;
        verticalAlignment:   Text.AlignVCenter;
        font.pixelSize: 40
        font.letterSpacing: 2
        font.bold: true
        font.family: "D-DIN Condensed HEMRO"
        color: idMain.colorEkxError;
    }

    ListView
    {
        id: idView

        visible: errorText === ""

        anchors.fill: parent;
        model: statisticsBoard.results;
        orientation: Qt.Vertical;

        delegate: StatisticsScreenEntry
        {
            key: model.key;
            value: model.value;
            isLoading: model.isLoading;
            hasBar: model.hasBar;
            barCurrentPercentage: model.hasBar?model.barCurrentPercentage:0;
            barCriticalPercentage: model.hasBar?model.barCriticalPercentage:0;
            iconIndex: model.iconIndex;
        }

        // # Sections:
        section.property: "sectionTitle";
        section.criteria: ViewSection.FullString
        section.labelPositioning: ViewSection.InlineLabels;
        section.delegate: Rectangle
        {
            width: 480;
            height: 72;
            color: "black";
            //border.width: 1;
            //border.color: "orange";

            Text
            {
                // # section title:
                anchors.centerIn: parent;
                x: 16;
                y: 0;
                width: 480-2*16;
                height: 72
                opacity: 1;
                horizontalAlignment: Text.AlignLeft;
                verticalAlignment:   Text.AlignVCenter;
                font.pixelSize: 40
                font.letterSpacing: 2
                font.bold: true
                font.family: "D-DIN Condensed HEMRO"
                color: idMain.colorEkxTextGrey;
                text: section;
            }

            Rectangle
            {
                // # horizontal rule:
                x: 16;
                y: 72 - 11;
                width: 480-2*16;
                height: 2
                color: idMain.colorEkxTextGrey;
            }
        }

        // # Header:
        headerPositioning: ListView.OverlayHeader;
        header: Rectangle
        {
            z: 3;
            x: 0;
            y: 0;
            width: 480;
            height: 72;
            color: "transparent";
            opacity: 1;

            Text
            {
                // # title:
                anchors.centerIn: parent;
                width: 480
                height: 72;
                opacity: 1;
                horizontalAlignment: Text.AlignHCenter;
                verticalAlignment:   Text.AlignVCenter;
                font.pixelSize: 40;
                font.letterSpacing: 2;
                font.bold: true;
                font.family: "D-DIN Condensed HEMRO";
                color: idMain.colorEkxTextGrey;
                text: settingsMenuContentFactory.menuTitle.toUpperCase();
            }
        }

    }

    ShadowGroup
    {
        //visible:  ( menuList.length > 3 && !mainMenu) ? true : false
        z:1;

        shadowTopSize: ShadowGroup.ShadowTopSizes.StandardSize;
        shadowTopThirdGradientStopEnable:   true;
        shadowTopThirdGradientStopPosition: 0.5;
        shadowTopThirdGradientStopOpacity:  0.85;

        shadowBottomSize: 75;

        //visualDebug: true;
    }

    Item
    {
        id: idPeriodMenuButton

        // This rect is made similar to menuPge title to vAlign correctly.
        z:2;
        visible: idRoot.periodMenuActive === false

        anchors.top: parent.top;
        anchors.right: parent.right;
        height: idPeriodImage.height;
        width: idPeriodImage.width + idPeriodText.width

        Image
        {
            // # time period menu button:
            id: idPeriodImage;

            anchors.right: parent.right;
            anchors.top: parent.top;
            source: "qrc:/Icons/PeriodSelector-72px_white.png";
        }

        Text
        {
            id: idPeriodText;

            anchors.right: idPeriodImage.left;
            anchors.verticalCenter: idPeriodImage.verticalCenter
            horizontalAlignment: Text.AlignRight;
            verticalAlignment:   Text.AlignVCenter;

            font.pixelSize: 40;
            font.letterSpacing: 2;
            font.bold: false;
            font.family: "D-DIN Condensed HEMRO";
            color: idMain.colorEkxTextWhite;
            text: statisticsBoard.periodChoices[statisticsBoard.periodChoicesIndex].key;
        }

        MouseArea
        {
            anchors.fill: parent;
            onClicked:
            {
                console.log("time-period button clicked!")
                if(idRoot.periodMenuActive)
                {
                    idRoot.periodMenuActive = false;
                }
                else
                {
                    idRoot.periodMenuActive = true;
                    idPeriodMenu.selectedIndex = statisticsBoard.periodChoicesIndex;
                }
            }
        }
    }

    Rectangle
    {
        id: idPeriodMenu;

        property bool visualDebug: false;
        property int selectedIndex: statisticsBoard.periodChoicesIndex;

        visible: periodMenuActive;
        z: 4;
        anchors.fill: parent;
        color: "black";
        opacity: 1.0;

        ListView
        {
            id: idListViewChoices;
            anchors.fill: parent
            // For a display height of 226.
            topMargin: 80
            bottomMargin: 54

            model: statisticsBoard.periodChoices;
            delegate: Rectangle
            {
                width: idListViewChoices.width;
                height: Math.max( 96, Math.max(idTextShortName.height, idTextPeriodName.height, idRadioButtonImage.height) + 20 )
                opacity: 1.0;
                color: "black";

                Text
                {
                    id: idTextShortName;
                    anchors.left: parent.left;
                    anchors.leftMargin: 20;
                    anchors.verticalCenter: parent.verticalCenter;

                    horizontalAlignment: Text.AlignLeft;
                    verticalAlignment:   Text.AlignVCenter;
                    font.pixelSize: 40;
                    //font.pixelSize: 58;
                    font.letterSpacing: 2;
                    font.bold: false;
                    font.family: "D-DIN Condensed HEMRO";
                    color: idMain.colorEkxTextGrey;
                    text: ""+model.key;
                    DebugFrame { visible: idPeriodMenu.visualDebug; }
                }

                Text
                {
                    id: idTextPeriodName;
                    anchors.horizontalCenter: parent.horizontalCenter;
                    anchors.verticalCenter: parent.verticalCenter;

                    horizontalAlignment: Text.AlignHCenter;
                    verticalAlignment:   Text.AlignVCenter;
                    //font.pixelSize: 40;
                    font.pixelSize: 58;
                    font.letterSpacing: 2;
                    font.bold: false;
                    font.family: "D-DIN Condensed HEMRO";
                    color: idMain.colorEkxTextWhite;
                    text: ""+model.value;
                    DebugFrame { visible: idPeriodMenu.visualDebug; }
                }

                Image
                {
                    id:idRadioButtonImage;
                    anchors.right: parent.right;
                    anchors.rightMargin: 20;
                    anchors.verticalCenter: parent.verticalCenter;
                    width:  72
                    height: 72
                    source: ( idPeriodMenu.selectedIndex === index ) ? "qrc:/Icons/ICON_Indicator-72px_active.png" : "qrc:/Icons/ICON_Indicator-72px_deactive.png"
                    //DebugFrame { visible: idPeriodMenu.visualDebug; }
                }

                MouseArea
                {
                    anchors.fill: parent;
                    onClicked:
                    {
                        idPeriodMenu.selectedIndex = index;
                    }
                }

                DebugFrame { visible: idPeriodMenu.visualDebug; }
            }
        }

        ShadowGroup
        {
            z:1;
            shadowTopSize: ShadowGroup.ShadowTopSizes.StandardSize;
            shadowTopThirdGradientStopEnable:   true;
            shadowTopThirdGradientStopPosition: 0.5;
            shadowTopThirdGradientStopOpacity:  0.85;

            shadowBottomSize: 75;

            //visualDebug: true;
        }

        EkxButton
        {
            // id: idButtonPrevious;
            z:1;
            buttonType: EkxButton.ButtonType.PREVIOUS;
            onClicked:
            {
                idRoot.periodMenuActive = false;
            }
        }

        EkxButton
        {
            // id: idButtonOk;
            z:1;
            buttonType: EkxButton.ButtonType.OK;
            onClicked:
            {
                idRoot.periodMenuActive = false;
                statisticsBoard.periodChoicesIndex = idPeriodMenu.selectedIndex;
                statisticsBoard.queryResults();

                settingsSerializer.statisticsPeriodIndex = idPeriodMenu.selectedIndex
                settingsSerializer.saveSettings()
            }
        }
    }
}
