import QtQml 2.12
import QtQuick 2.0
import "../CommonItems"

Item
{
    id: dddAdjustment

    visible: settingsSerializer.showDddEnable

    width: 480
    height: 80

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // self defined properties
    ///////////////////////////////////////////////////////////////////////////////////////////////

    property real currentDddValue: 17.5;
    property real currentRecipeDdd;

    property bool showCurrentDdd;
    property bool timeModeEqual: false
    property bool longPressAction: false

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // signals
    ///////////////////////////////////////////////////////////////////////////////////////////////

    signal saveCurrentDdd();

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // functions
    ///////////////////////////////////////////////////////////////////////////////////////////////

    function handleDddChange()
    {
        if( currentDddValue > currentRecipeDdd)
        {
            idSensorValue.text = currentDddValue.toFixed(1).toString()
            idRecipeValue.text = currentRecipeDdd.toFixed(1).toString()
            idRecipeValue.smallerIsRecipeValue = true
            idSensorValue.greaterIsRecipeValue = false

        }

        if( currentDddValue < currentRecipeDdd)
        {
            idSensorValue.text = currentRecipeDdd.toFixed(1).toString()
            idRecipeValue.text = currentDddValue.toFixed(1).toString()

            idRecipeValue.smallerIsRecipeValue = false
            idSensorValue.greaterIsRecipeValue = true
        }

        if( currentDddValue === currentRecipeDdd )
        {
            if( showCurrentDdd )
            {
                timeModeEqual = true;
                if( idRecipeValue.smallerIsRecipeValue )
                {
                    idSensorValue.greaterIsRecipeValue = false;
                    idRecipeValue.text = currentRecipeDdd.toFixed(1).toString()

                }
                else
                {
                    idSensorValue.text = currentRecipeDdd.toFixed(1).toString()
                    idRecipeValue.smallerIsRecipeValue = false;
                }
            }
            else
            {
                if( idRecipeValue.smallerIsRecipeValue )
                {
                    idRecipeValue.text = currentRecipeDdd.toFixed(1).toString()
                }
                else
                {
                     idSensorValue.text = currentRecipeDdd.toFixed(1).toString()
                }
                idSensorValue.greaterIsRecipeValue = false;
                idRecipeValue.smallerIsRecipeValue = false;
            }
        }
        else
        {
            if( showCurrentDdd )
            {
                timeModeEqual = false;
            }
        }
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // signal handlers
    ///////////////////////////////////////////////////////////////////////////////////////////////

    onCurrentDddValueChanged:
    {
        handleDddChange();
    }

    onCurrentRecipeDddChanged:
    {
       handleDddChange();
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // items
    ///////////////////////////////////////////////////////////////////////////////////////////////

    Text
    {
        id: idSensorValue

        visible: greaterIsRecipeValue ? true : ( showCurrentDdd && !timeModeEqual )

        anchors.left:  parent.left
        anchors.leftMargin: timeModeEqual ? 216 : 115
        anchors.top: parent.top
        anchors.topMargin: 8

        text: currentDddValue.toFixed(1).toString()
        color: ( greaterIsRecipeValue && !timeModeEqual ) ?  idMain.colorEkxConfirmRed : idMain.colorEkxTextGrey

        font.pixelSize: 58
        font.letterSpacing: 0.23
        font.family: "D-DIN Condensed HEMRO _Mono_Numbers"

        layer.enabled: longPressAction
        layer.smooth: true;

        // This item should be used as the 'mask'
        layer.samplerName: "maskSource"
        layer.effect: LongPressShaderEffect
        {
            longPressAnimation: idSaveAnimation
        }

        property bool greaterIsRecipeValue: false;
    }

    Image
    {
        id: idArrow

        visible: !(currentDddValue === currentRecipeDdd)

        width: 72
        height:72

        anchors.verticalCenter: idSensorValue.verticalCenter
        anchors.left: idSensorValue.right
        anchors.leftMargin: 20

        source: "qrc:/Icons/redArrowlo.png"
        mirror: idRecipeValue.smallerIsRecipeValue
    }

    Text
    {
        id: idRecipeValue

        visible: smallerIsRecipeValue ? true : ( showCurrentDdd && !timeModeEqual )

        anchors.left: idArrow.right
        anchors.leftMargin: timeModeEqual ? (-192) : 20
        anchors.top: parent.top
        anchors.topMargin: 8

        text: currentRecipeDdd.toFixed(1).toString()
        color: ( smallerIsRecipeValue && !timeModeEqual ) ? idMain.colorEkxConfirmRed : idMain.colorEkxTextGrey

        font.pixelSize: 58
        font.letterSpacing: 0.23
        font.family: "D-DIN Condensed HEMRO _Mono_Numbers"

        layer.enabled: longPressAction
        layer.smooth: true;

        // This item should be used as the 'mask'
        layer.samplerName: "maskSource"
        layer.effect: LongPressShaderEffect
        {
            longPressAnimation: idSaveAnimation
        }

        property bool smallerIsRecipeValue: false;
    }

    Item
    {
        anchors.fill: parent

        LongPressAnimation
        {
            id: idSaveAnimation

            fillColor: idMain.colorEkxConfirmGreen

            onStarted:
            {
                longPressAction = true
            }

            onSimplePressed:
            {
                longPressAction = false
            }

            onLongPressed:
            {
                longPressAction = false
                saveCurrentDdd();
            }
        }
    }
}
