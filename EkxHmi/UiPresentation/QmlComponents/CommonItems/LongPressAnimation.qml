import QtQuick 2.0
import QtQml 2.12

Item
{
    id: idRoot

    anchors.fill: parent

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // self defined properties
    ///////////////////////////////////////////////////////////////////////////////////////////////

    property bool  layerEnabled: true;
    property color fillColor: "cyan"
    property int   incrementTimeMs: 100
    property int   incrementCntStartAnimation: 2
    property int   incrementCntStopAnimation: 7

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // functions
    ///////////////////////////////////////////////////////////////////////////////////////////////

    function start()
    {
        //console.log("function start")
        incrementTimer.counter = 0
        incrementTimer.start();
        started()
    }

    function stop()
    {
        //console.log("function stop")
        incrementTimer.stop();
    }

    function abort()
    {
        //console.log("function abort")
        incrementTimer.counter = incrementCntStopAnimation
        incrementTimer.stop();
        fillerReset()
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // signals
    ///////////////////////////////////////////////////////////////////////////////////////////////

    signal started();
    signal simplePressed();
    signal longPressed();
    signal fillerIncremented();
    signal fillerReset();

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // items
    ///////////////////////////////////////////////////////////////////////////////////////////////

    Timer
    {
        id: incrementTimer

        interval: incrementTimeMs;
        running: false;
        repeat: true

        property int counter: 0

        onTriggered:
        {
            counter++;

            if ( counter > incrementCntStartAnimation )
            {
                fillerIncremented();
            }

            if ( counter >= incrementCntStopAnimation )
            {
                fillerReset();
                incrementTimer.stop();
                counter = 0;

                //console.log( "longPressed" );
                idRoot.longPressed();
            }
        }

        onRunningChanged:
        {
            if ( !running )
            {
                if ( counter < incrementCntStopAnimation )
                {
                    fillerReset();
                    counter = 0;

                    //console.log( "simplePressed" );
                    idRoot.simplePressed();
                }
            }
        }
    }

    MouseArea
    {
        anchors.fill: parent

        onPressed:
        {
            start()
        }

        onReleased:
        {
            //console.log("onReleased")
            stop();
        }
    }
}
