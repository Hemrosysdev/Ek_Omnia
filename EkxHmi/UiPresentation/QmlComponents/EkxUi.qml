import QtQml 2.12
import QtQuick 2.12
import QtQuick.Window 2.12
import QtGraphicalEffects 1.12
import "Modes"
import "Modes/SwUpdate"
import "Modes/Standby"
import EkxMainstateEnum 1.0

Rectangle
{
    id: idMain

    visible: true
    width: 480
    height: 360
    color: "black"
    clip: true

    property color colorEkxTextWhite:    "white"
    property color colorEkxTextGrey:     Qt.rgba( 0.5, 0.5, 0.5, 1.0 )
    property color colorEkxConfirmGreen: "#00ffb3"
    property color colorEkxConfirmRed:   "#ff465f"
    property color colorEkxDarkGreen:    "#00805a"
    property color colorEkxWarning:      "#ff8b00"
    property color colorEkxError:        "#ff465f"

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // self defined properties
    ///////////////////////////////////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // signal handlers
    ///////////////////////////////////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // items
    ///////////////////////////////////////////////////////////////////////////////////////////////

    ModeSelector2
    {
    }

    SwUpdateInfo
    {
        z: 10
        visible: mainStatemachine.ekxMainstate === EkxMainstateEnum.EKX_SWUPDATE
    }

    StandbyInfo
    {
        visible: mainStatemachine.ekxMainstate === EkxMainstateEnum.EKX_STANDBY || standbyController.preStandbyDimActive
    }
}

