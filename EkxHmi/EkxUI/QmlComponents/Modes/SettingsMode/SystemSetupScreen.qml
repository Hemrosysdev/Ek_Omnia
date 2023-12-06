import QtQuick 2.12
import QtQuick.Layouts 1.1
import QtQml.Models 2.1

import "../../CommonItems" as Common

import SettingsSerializerEnum 1.0
import com.hemrogroup.drivers 1.0 as Drivers

Rectangle
{
    id: idRoot;

    width: 480;
    height: 360;
    color: "black";

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // self-defined properties
    ///////////////////////////////////////////////////////////////////////////////////////////////

    // How many degrees NTC temp can be offset from board temp to be considered plausible.
    readonly property int ntcTemperatureDeltaValid: 10 // °C

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // signals
    ///////////////////////////////////////////////////////////////////////////////////////////////

    signal finished

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // functions
    ///////////////////////////////////////////////////////////////////////////////////////////////

    function reset() {
        mainStatemachine.systemSetupOpen = false;

        if (agsaLongDurationTest.running) {
            agsaLongDurationTest.userStopTest();
        }

        mainStatemachine.grindRunning = false;

        if (pduDcHallMotorDriver.running) {
            pduDcHallMotorDriver.startMotor();
        }

        startStopLightDriver.startStopDutyCycle = undefined;
        spotLightDriver.spotLightDutyCycle = false;
    }

    // TODO Move this into a shared utils helper (with TemperatureView).
    function formatAsTemperature(celsiusTemp : int) {
        if (settingsSerializer.tempUnit === SettingsSerializerEnum.TEMPUNIT_CELSIUS) {
            return celsiusTemp + " °C";
        } else {
            const fahrenheit = Math.round( ( ( celsiusTemp * 9.0 ) / 5.0 ) + 32.0 );
            return fahrenheit + " °F";
        }
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // signal handlers
    ///////////////////////////////////////////////////////////////////////////////////////////////

    Component.onCompleted: {
        mainStatemachine.systemSetupOpen = true;
    }

    onVisibleChanged: {
        if (visible) {
            mainStatemachine.systemSetupOpen = true;
        } else {
            reset();
        }
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // components
    ///////////////////////////////////////////////////////////////////////////////////////////////

    component ActionButton : Rectangle {
        id: button

        property alias text: label.text
        property int horizontalPadding: 10
        property int verticalPadding: 4
        property bool checked: false

        signal clicked

        implicitWidth: label.implicitWidth + horizontalPadding * 2
        implicitHeight: label.implicitHeight + verticalPadding * 2
        color: buttonArea.pressed || checked ? idMain.colorEkxTextWhite : idMain.colorEkxTextGrey
        opacity: enabled ? 1 : 0.5
        radius: 4

        Common.Label {
            id: label
            anchors.fill: parent
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            color: buttonArea.pressed || button.checked ? "black" : idMain.colorEkxTextWhite
        }

        MouseArea {
            id: buttonArea
            anchors {
                fill: parent
                margins: -10 // Increase hit area.
            }
            onClicked: {
                button.clicked()
            }
        }
    }

    component LabelDelegate : SystemSetupDelegate {
        id: labelDelegate
        property alias text: label.text
        // string = text, true = OK, false = fault/NOK, null = N/A.
        property var value: undefined

        Common.Label {
            id: label
            Layout.fillWidth: true
            textColor: Common.Label.TextColor.Grey
        }

        Common.Label {
            id: valueLabel
            horizontalAlignment: Text.AlignRight
            textColor: {
                if (labelDelegate.value === true) {
                    return Common.Label.TextColor.ConfirmGreen;
                } else if (labelDelegate.value === false) {
                    return Common.Label.TextColor.ConfirmRed;
                } else {
                    return Common.Label.TextColor.White;
                }
            }
            text: {
                if (labelDelegate.value === undefined) {
                    // Mostly to catch programmer errors of not assigning a value.
                    return "– – –";
                } else if (labelDelegate.value === null) {
                    return "N/A";
                } else if (labelDelegate.value === true) {
                    return "OK";
                } else if (labelDelegate.value === false) {
                    return "Fault";
                } else {
                    return labelDelegate.value;
                }
            }
        }
    }

    component ActionDelegate : SystemSetupDelegate {
        id: idActionDelegate
        property alias text: label.text
        property alias actionText: button.text
        property bool checked: false

        signal clicked

        Common.Label {
            id: label
            Layout.fillWidth: true
            textColor: Common.Label.TextColor.Grey
        }

        ActionButton {
            id: button
            checked: idActionDelegate.checked
            onClicked: {
                idActionDelegate.clicked()
            }
        }
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // items
    ///////////////////////////////////////////////////////////////////////////////////////////////

    ObjectModel
    {
        id: setupEntriesModel

        // Section DDD:
        ActionDelegate {
            sectionTitle: "DDD"
            text: "DDD Calibration"
            actionText: confirmCalibrationTimer.running ? "Calibrated" : "Calibrate"
            checked: confirmCalibrationTimer.running
            onClicked: {
                dddCouple.calibrateDdd();
                confirmCalibrationTimer.restart();
            }

            Timer {
                id: confirmCalibrationTimer
                interval: 2000
                repeat: false
            }
        }

        LabelDelegate {
            sectionTitle: "DDD"
            text: "Current Value"
            value: {
                if (dddCouple.failed) {
                    return false;
                } else {
                    return dddCouple.dddValue + " µm";
                }
            }
        }

        // Section AGSA:
        ActionDelegate {
            sectionTitle: "AGSA"
            text: "AGSA"
            actionText: agsaLongDurationTest.running ? "Stop Test" : "Start Test"
            onClicked: {
                if (agsaLongDurationTest.running) {
                    agsaLongDurationTest.userStopTest();
                } else {
                    agsaLongDurationTest.loggingEnabled = false;
                    agsaLongDurationTest.manualStartDddValue = 0;
                    agsaLongDurationTest.manualStopDddValue = 800;
                    agsaLongDurationTest.manualCyclesNum = 1;
                    agsaLongDurationTest.startManualTest();
                }
            }
        }

        // Section Interlocks:
        LabelDelegate {
            sectionTitle: "Interlocks"
            text: "Hopper"
            value: mcuDriver.mcuStatusValid ? ( mcuDriver.systemStatusHopperDismounted ? "Dismounted" : "Mounted" ) : null
        }

        LabelDelegate {
            sectionTitle: "Interlocks"
            text: "Grinding Chamber"
            value: mcuDriver.mcuStatusValid ? ( mcuDriver.systemStatusGrinderChamberOpen ? "Open" : "Closed" ) : null
        }

        // Section MCU:
        ActionDelegate {
            sectionTitle: "MCU"
            text: "Motor Test"
            actionText: mainStatemachine.grindRunning ? "Stop" : "Start"
            enabled: mcuDriver.mcuStatusValid
            onClicked: {
                mainStatemachine.grindRunning  = !mainStatemachine.grindRunning;
            }
        }

        LabelDelegate {
            sectionTitle: "MCU"
            text: "Motor Running"
            // TODO or just mainStatemachine.grindRunning?
            value: mcuDriver.mcuStatusValid ? ( mcuDriver.systemStatusMotorRunning ? "Running" : "Not Running" ) : null
        }

        LabelDelegate {
            sectionTitle: "MCU"
            text: "Motor RPM"
            value: mcuDriver.motorInfoValid ? ( mcuDriver.motorSpeed + " rpm" ) : null
        }

        LabelDelegate {
            sectionTitle: "MCU"
            text: "Motor Current"
            value: mcuDriver.motorInfoValid ? ( mcuDriver.motorCurrent + " mA" ) : null
        }

        LabelDelegate {
            sectionTitle: "MCU"
            text: "Motor Temperature"
            value: mcuDriver.motorTempValid ? formatAsTemperature( mcuDriver.motorTemp ) : null
        }

        LabelDelegate {
            sectionTitle: "MCU"
            text: "DC Bus Voltage"
            value: mcuDriver.motorDcBusVoltageValid ? ( mcuDriver.motorDcBusVoltage + " V" ) : null
        }

        LabelDelegate {
            sectionTitle: "MCU"
            text: "Board Temperature"
            value: mcuDriver.mcuBoardTempValid ? formatAsTemperature( mcuDriver.mcuBoardTemp ) : null
        }

        /*ActionDelegate {
            sectionTitle: "MCU"
            text: "Test Fan"
            actionText: mcuDriver.systemStatusFanRunning ? "Stop" : "Start"
            enabled: mcuDriver.mcuStatusValid
            onClicked: {
                // TODO
            }
        }*/

        LabelDelegate {
            sectionTitle: "MCU"
            text: "Fan Running"
            value: mcuDriver.mcuStatusValid ? ( mcuDriver.systemStatusFanRunning ? "Running" : "Not Running" ) : null
        }

        // Section MCU Faults:
        LabelDelegate {
            sectionTitle: "MCU Faults"
            text: "Board Overtemp."
            value: mcuDriver.mcuStatusValid ? mcuDriver.faultStatusBoardOverTemp : null
        }

        LabelDelegate {
            sectionTitle: "MCU Faults"
            text: "DC Overcurrent"
            value: mcuDriver.mcuStatusValid ? mcuDriver.faultStatusDcOverCurrent : null
        }

        LabelDelegate {
            sectionTitle: "MCU Faults"
            text: "AC Overvoltage"
            value: mcuDriver.mcuStatusValid ? mcuDriver.faultStatusAcOverVoltage : null
        }

        LabelDelegate {
            sectionTitle: "MCU Faults"
            text: "AC Undervoltage"
            value: mcuDriver.mcuStatusValid ? mcuDriver.faultStatusAcUnderVoltage : null
        }

        LabelDelegate {
            sectionTitle: "MCU Faults"
            text: "Hall Sensor"
            value: mcuDriver.mcuStatusValid ? mcuDriver.faultStatusHallSensor : null
        }

        LabelDelegate {
            sectionTitle: "MCU Faults"
            text: "Current Sensor"
            value: mcuDriver.mcuStatusValid ? mcuDriver.faultStatusCurrentSensor : null
        }

        LabelDelegate {
            sectionTitle: "MCU Faults"
            text: "Rotor Locked"
            value: mcuDriver.mcuStatusValid ? mcuDriver.faultStatusRotorLocked : null
        }

        // Section PDU:
        LabelDelegate {
            sectionTitle: "PDU"
            text: "PDU Present"
            // TODO indicate faultPinActive?
            value: pduDcHallMotorDriver.motorTestOk ? "Present" : "Not Present"
        }

        ActionDelegate {
            sectionTitle: "PDU"
            text: "Test PDU"
            actionText: pduDcHallMotorDriver.running ? "Stop" : "Start"
            onClicked: {
                if (pduDcHallMotorDriver.running) {
                    pduDcHallMotorDriver.startMotor();
                } else {
                    pduDcHallMotorDriver.stopMotor();
                }
            }
        }

        LabelDelegate {
            sectionTitle: "PDU"
            text: "PDU Running"
            value: {
                if (pduDcHallMotorDriver.motorTestOk) {
                    if (pduDcHallMotorDriver.running) {
                        return "Running";
                    } else {
                        return "Not Running";
                    }
                }
                return null;
            }
        }

        // Section Illumination:
        LabelDelegate {
            sectionTitle: "Illumination"
            text: "Start/Stop LED"
            value: {
                if (startStopLightDriver.startStopDutyCycle > 0) {
                    return "On";
                } else if (startStopLightDriver.startStopDutyCycle === 0) {
                    return "Off";
                } else {
                    return false;
                }
            }
        }

        ActionDelegate {
            sectionTitle: "Illumination"
            text: "Test Start/Stop LED"
            actionText: startStopLightDriver.startStopDutyCycle > 0 ? "Turn Off" : "Turn On"
            onClicked: {
                if (startStopLightDriver.startStopDutyCycle > 0) {
                    startStopLightDriver.startStopDutyCycle = 0;
                } else {
                    startStopLightDriver.startStopDutyCycle = Drivers.StartStopLightDriver.StartStopDutyIdle;
                }
            }
        }

        LabelDelegate {
            sectionTitle: "Illumination"
            text: "Nozzle Light"
            value: {
                if (spotLightDriver.spotLightDutyCycle > 0) {
                    return "On";
                } else if (spotLightDriver.spotLightDutyCycle === 0) {
                    return "Off";
                } else {
                    return false;
                }
            }
        }

        ActionDelegate {
            sectionTitle: "Illumination"
            text: "Test Nozzle Light"
            actionText: spotLightDriver.spotLightDutyCycle > 0 ? "Turn Off" : "Turn On"
            onClicked: {
                if (spotLightDriver.spotLightDutyCycle > 0) {
                    spotLightDriver.spotLightDutyCycle = 0;
                } else {
                    spotLightDriver.spotLightDutyCycle = Drivers.SpotLightDriver.SpotLightDutyGrinding;
                }
            }
        }

        // Section NTC:
        LabelDelegate {
            sectionTitle: "NTC"
            text: "NTC Temperature"
            value: ntcTempDriver.currentTempValid ? idRoot.formatAsTemperature(ntcTempDriver.currentTemp) : null
        }

        LabelDelegate {
            sectionTitle: "NTC"
            text: "Temp. Plausible"
            value: {
                if (ntcTempDriver.currentTempValid && mcuDriver.mcuBoardTempValid) {
                    return Math.abs( mcuDriver.mcuBoardTemp - ntcTempDriver.currentTemp ) <= 10;
                }
                return null;
            }
        }
    }

    Component {
        id: confirmSetupComponent

        ColumnLayout {
            property string sectionTitle: "Finish"

            width: ListView.view ? ListView.view.width : implicitWidth
            spacing: 20

            Common.Label {
                Layout.fillWidth: true
                Layout.topMargin: 40
                horizontalAlignment: Text.AlignHCenter
                text: "Confirm system\nsetup completed."
            }

            ActionButton {
                Layout.alignment: Qt.AlignHCenter
                text: "Confirm"
                layer.enabled: confirmLongPressAnimation.layerEnabled
                layer.samplerName: "maskSource"
                layer.effect: Common.LongPressShaderEffect {
                    longPressAnimation: confirmLongPressAnimation
                }

                Common.LongPressAnimation {
                    id: confirmLongPressAnimation
                    anchors.margins: -10 // Increase hit area.
                    longPressOnly: true
                    fillColor: idMain.colorEkxConfirmGreen
                    onLongPressed: {
                        settingsSerializer.confirmSystemSetup();

                        idRoot.finished();
                    }
                }
            }
        }
    }

    ListView
    {
        id: idView

        anchors.fill: parent;
        model: setupEntriesModel
        orientation: Qt.Vertical;
        bottomMargin: 80

        // # Sections:
        section.property: "sectionTitle";
        section.criteria: ViewSection.FullString
        section.labelPositioning: ViewSection.InlineLabels;
        section.delegate: Item
        {
            width: 480;
            height: 72;

            Common.Label
            {
                anchors.centerIn: parent;
                x: 16;
                y: 0;
                width: 480-2*16;
                height: 72
                opacity: 1;
                horizontalAlignment: Text.AlignLeft;
                verticalAlignment:   Text.AlignVCenter;
                pixelSize: 40
                font.letterSpacing: 2
                bold: true
                textColor: Common.Label.TextColor.Grey
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

            Common.Label
            {
                anchors.centerIn: parent;
                width: 480
                height: 72;
                opacity: 1;
                horizontalAlignment: Text.AlignHCenter;
                verticalAlignment:   Text.AlignVCenter;
                pixelSize: 40;
                font.letterSpacing: 2;
                bold: true
                textColor: Common.Label.TextColor.Grey
                text: settingsMenuContentFactory.menuTitle.toUpperCase();
            }
        }

        Component.onCompleted: {
            // DO NOT use a binding here!
            // When we set systemSetupDone to true in the LongPressAnimation above
            // this would cause the footer to be destroyed during the signal handler
            // where the ShaderEffect is still active and it would get utterly
            // confused and crash when the LongPressAnimation gets destroyed.
            if (!settingsSerializer.systemSetupDone) {
                footer = confirmSetupComponent;
            }
        }
    }

    Common.ShadowGroup
    {
        //visible:  ( menuList.length > 3 && !mainMenu) ? true : false
        z:1;

        shadowTopSize: Common.ShadowGroup.ShadowTopSizes.StandardSize;
        shadowTopThirdGradientStopEnable:   true;
        shadowTopThirdGradientStopPosition: 0.5;
        shadowTopThirdGradientStopOpacity:  0.85;

        shadowBottomSize: 75;

        //visualDebug: true;
    }

}
