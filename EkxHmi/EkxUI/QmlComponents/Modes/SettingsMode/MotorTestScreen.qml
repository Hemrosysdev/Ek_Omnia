import QtQuick 2.12
import QtQml 2.12

import "../../CommonItems"

Item
{
    id: idRoot;

    width: 480;
    height: 360;

    property bool visualDebug: false;
    property bool simulateMotorDebug: false

    property string titleBackup: "";
    property bool sliderVisible: false;
    property string sliderTopic: "UNDEFINED"
    property bool sectionsEnabled: false;

    property bool pduDcHallMotorValid: pduDcHallMotorDriver.valid
    property bool pduDcHallMotorRunning: pduDcHallMotorDriver.running
    property int  pduDcHallMotorRunTime: pduDcHallMotorDriver.runTime

    property bool agsaStepperMotorValid: agsaStepperMotorDriver.valid
    property bool agsaStepperMotorRunning: agsaStepperMotorDriver.running

    //# to be replaced with driver interface properties:
    property real dummyDcStepsMin:   100;
    property real dummyDcStepsMax: 20000;
    property real dummyDcStepsInc:   100;
    property real dummyDcStepsVal:   200;

    property real dummyDcPwmDutyMin:   1;
    property real dummyDcPwmDutyMax: 100;
    property real dummyDcPwmDutyInc:   1;
    property real dummyDcPwmDutyVal:  50;

    property real dummyDcPwmFreqMin:   500;
    property real dummyDcPwmFreqMax: 10000;
    property real dummyDcPwmFreqInc:   100;
    property real dummyDcPwmFreqVal:  5000;

    property real dummyTargetDddMin:     0;
    property real dummyTargetDddMax:   800;
    property real dummyTargetDddInc:     1;
    property real dummyTargetDddVal:     0;

    property real dummyStpStepsMin: -10000;
    property real dummyStpStepsMax:  10000;
    property real dummyStpStepsInc:    100;
    property real dummyStpStepsVal:    200;

    property real dummyStpStepsPerSecMin:   100;
    property real dummyStpStepsPerSecMax: 15000;
    property real dummyStpStepsPerSecInc:   100;
    property real dummyStpStepsPerSecVal:   600;

    property real dummyStpLowStepsPerSecMin:   10;
    property real dummyStpLowStepsPerSecMax: 1000;
    property real dummyStpLowStepsPerSecInc:   10;
    property real dummyStpLowStepsPerSecVal:  150;

    property real dummyStpLdtCyclesNumMin:     0;
    property real dummyStpLdtCyclesNumMax: 10000;
    property real dummyStpLdtCyclesNumInc:   100;
    property real dummyStpLdtCyclesNumVal:  5000;

    property real dummyStpLdtStartDddMin:     0;
    property real dummyStpLdtStartDddMax:   800;
    property real dummyStpLdtStartDddInc:     1;
    property real dummyStpLdtStartDddVal:    10;

    property real dummyStpLdtStopDddMin:     0;
    property real dummyStpLdtStopDddMax:   800;
    property real dummyStpLdtStopDddInc:     1;
    property real dummyStpLdtStopDddVal:   790;

    property real dummyStpLdtTestStepsMin:    100;
    property real dummyStpLdtTestStepsMax:  50000;
    property real dummyStpLdtTestStepsInc:    100;
    property real dummyStpLdtTestStepsVal:   10000;

    Component.onCompleted:
    {
        idSlider.currentValueX100 = 50*100;

        idRoot.dummyDcStepsVal         = pduDcHallMotorDriver.steps
        idRoot.dummyDcPwmDutyVal       = pduDcHallMotorDriver.pwmDuty10th / 10
        idRoot.dummyDcPwmFreqVal       = pduDcHallMotorDriver.pwmFrequency

        idRoot.dummyStpStepsVal           = agsaControl.steps
        idRoot.dummyStpStepsPerSecVal     = agsaControl.runFrequency
        idRoot.dummyStpLowStepsPerSecVal  = agsaControl.approachFrequency
        idRoot.dummyTargetDddVal          = dddCouple.dddValue

        idRoot.dummyStpLdtStartDddVal  = agsaLongDurationTest.manualStartDddValue
        idRoot.dummyStpLdtStopDddVal   = agsaLongDurationTest.manualStopDddValue
        idRoot.dummyStpLdtCyclesNumVal = agsaLongDurationTest.manualCyclesNum

        idRoot.dummyStpLdtTestStepsVal    = agsaLongDurationTest.testSteps

        agsaControl.testMode = true;
    }

    Component.onDestruction:
    {
        agsaControl.testMode = false;
    }

    onSliderVisibleChanged:
    {
        if(sliderVisible)
        {
            hideMenuNavigation(true);
        }
        else
        {
            idSliderDddValue.visible = false;
        }
    }

    function hideMenuNavigation(enable)
    {
        if(enable)
        {
            settingsMenuContentFactory.buttonPreviousVisible = false;
            settingsMenuContentFactory.navAvailable = false;
            titleBackup = settingsMenuContentFactory.menuTitle;
            settingsMenuContentFactory.menuTitle = "";
        }
        else
        {
            settingsMenuContentFactory.buttonPreviousVisible = true;
            settingsMenuContentFactory.navAvailable = true;
            settingsMenuContentFactory.menuTitle = titleBackup;
        }
    }

    function updateSliderValueText()
    {
        if (idRoot.sliderTopic==='dcSteps')
        {
            idSlider.currentValueText = "" + (idSlider.currentValueX100Rounded()/100.0) + "";
        }
        else if (idRoot.sliderTopic==='dcPwmDuty')
        {
            idSlider.currentValueText = "" + (idSlider.currentValueX100Rounded()/100.0) + "%";
        }
        else if (idRoot.sliderTopic==='dcPwmFrq')
        {
            idSlider.currentValueText = "" + (idSlider.currentValueX100Rounded()/100.0) + "Hz";
        }
        else if (idRoot.sliderTopic==='agsaNumSteps')
        {
            idSlider.currentValueText = "" + (idSlider.currentValueX100Rounded()/100.0) + "";
        }
        else if (idRoot.sliderTopic==='agsaStepsPerSec')
        {
            idSlider.currentValueText = "" + (idSlider.currentValueX100Rounded()/100.0) + "Hz";
        }
        else if (idRoot.sliderTopic==='agsaLowStepsPerSec')
        {
            idSlider.currentValueText = "" + (idSlider.currentValueX100Rounded()/100.0) + "Hz";
        }
        else if (idRoot.sliderTopic==='agsaMoveToDdd')
        {
            idSlider.currentValueText = (idSlider.currentValueX100Rounded()/100.0);
        }


        else if (idRoot.sliderTopic==='agsaLdtStartDdd')
        {
            idSlider.currentValueText = (idSlider.currentValueX100Rounded()/100.0);
        }
        else if (idRoot.sliderTopic==='agsaLdtStopDdd')
        {
            idSlider.currentValueText = (idSlider.currentValueX100Rounded()/100.0);
        }
        else if (idRoot.sliderTopic==='agsaLdtCyclesNum')
        {
            idSlider.currentValueText = (idSlider.currentValueX100Rounded()/100.0);
        }


        else if (idRoot.sliderTopic==='agsaLdtTestSteps')
        {
            idSlider.currentValueText = (idSlider.currentValueX100Rounded()/100.0);
        }


        else
        {
            idSlider.currentValueText = "UNKNOWN PARAM";
        }
    }

    function onSliderReleased()
    {
        if (idRoot.sliderTopic==='dcSteps')
        {
            idSlider.currentValueText = "" + (idSlider.currentValueX100Rounded()/100.0) + "";
        }
        else if (idRoot.sliderTopic==='dcPwmDuty')
        {
            idSlider.currentValueText = "" + (idSlider.currentValueX100Rounded()/100.0) + "%";
        }
        else if (idRoot.sliderTopic==='dcPwmFrq')
        {
            idSlider.currentValueText = "" + (idSlider.currentValueX100Rounded()/100.0) + "Hz";
        }


        else if (idRoot.sliderTopic==='agsaNumSteps')
        {
            idSlider.currentValueText = "" + (idSlider.currentValueX100Rounded()/100.0) + "";
        }
        else if (idRoot.sliderTopic==='agsaStepsPerSec')
        {
            idSlider.currentValueText = "" + (idSlider.currentValueX100Rounded()/100.0) + "Hz";
        }
        else if (idRoot.sliderTopic==='agsaLowStepsPerSec')
        {
            idSlider.currentValueText = "" + (idSlider.currentValueX100Rounded()/100.0) + "Hz";
        }
        else if (idRoot.sliderTopic==='agsaMoveToDdd')
        {
            idSlider.currentValueText = ( idSlider.currentValueX100Rounded() / 100.0 );

            if ( agsaControl.testLiveMode )
            {
                agsaControl.moveToDddValue( idSlider.currentValueX100Rounded() / 100.0 );
            }
        }


        else if (idRoot.sliderTopic==='agsaLdtStartDdd')
        {
            idSlider.currentValueText = "" + (idSlider.currentValueX100Rounded()/100.0) + "";
        }
        else if (idRoot.sliderTopic==='agsaLdtStopDdd')
        {
            idSlider.currentValueText = "" + (idSlider.currentValueX100Rounded()/100.0) + "";
        }
        else if (idRoot.sliderTopic==='agsaLdtCyclesNum')
        {
            idSlider.currentValueText = "" + (idSlider.currentValueX100Rounded()/100.0) + "";
        }



        else if (idRoot.sliderTopic==='agsaLdtTestSteps')
        {
            idSlider.currentValueText = "" + (idSlider.currentValueX100Rounded()/100.0) + "";
        }



        else
        {
            idSlider.currentValueText = "UNKNOWN PARAM";
        }
    }

    function processSliderValueAccepted()
    {
        if (idRoot.sliderTopic==='dcSteps')
        {
            idRoot.dummyDcStepsVal = (idSlider.currentValueX100Rounded()/100.0)
            pduDcHallMotorDriver.steps = idRoot.dummyDcStepsVal
            settingsSerializer.saveSettings()
        }
        else if (idRoot.sliderTopic==='dcPwmDuty')
        {
            idRoot.dummyDcPwmDutyVal = (idSlider.currentValueX100Rounded()/100.0);
            pduDcHallMotorDriver.pwmDuty10th = idRoot.dummyDcPwmDutyVal * 10;
            settingsSerializer.saveSettings()
        }
        else if (idRoot.sliderTopic==='dcPwmFrq')
        {
            let freq_hz = Math.round(idSlider.currentValueX100/100.0);
            idRoot.dummyDcPwmFreqVal = Math.round(freq_hz/idRoot.dummyDcPwmFreqInc)*idRoot.dummyDcPwmFreqInc
            pduDcHallMotorDriver.pwmFrequency = idRoot.dummyDcPwmFreqVal
            settingsSerializer.saveSettings()
        }


        else if (idRoot.sliderTopic==='agsaNumSteps')
        {
            idRoot.dummyStpStepsVal = (idSlider.currentValueX100Rounded()/100.0);
            agsaControl.steps = idRoot.dummyStpStepsVal
            settingsSerializer.saveSettings()
        }
        else if (idRoot.sliderTopic==='agsaStepsPerSec')
        {
            idRoot.dummyStpStepsPerSecVal = (idSlider.currentValueX100Rounded()/100.0);
            agsaControl.runFrequency   = idRoot.dummyStpStepsPerSecVal
            settingsSerializer.saveSettings()
        }
        else if (idRoot.sliderTopic==='agsaLowStepsPerSec')
        {
            idRoot.dummyStpLowStepsPerSecVal = (idSlider.currentValueX100Rounded()/100.0);
            agsaControl.approachFrequency   = idRoot.dummyStpLowStepsPerSecVal
            settingsSerializer.saveSettings()
        }
        else if (idRoot.sliderTopic==='agsaMoveToDdd')
        {
            idRoot.dummyTargetDddVal = (idSlider.currentValueX100Rounded()/100.0);
            if ( !agsaControl.testLiveMode )
            {
                agsaControl.moveStepsToDddValue( idSlider.currentValueX100Rounded()/100.0 );
            }
        }


        else if (idRoot.sliderTopic==='agsaLdtStartDdd')
        {
            idRoot.dummyStpLdtStartDddVal = (idSlider.currentValueX100Rounded()/100.0);
            agsaLongDurationTest.startDddValue = idRoot.dummyStpLdtStartDddVal
            settingsSerializer.saveSettings()
        }
        else if (idRoot.sliderTopic==='agsaLdtStopDdd')
        {
            idRoot.dummyStpLdtStopDddVal = (idSlider.currentValueX100Rounded()/100.0);
            agsaLongDurationTest.stopDddValue = idRoot.dummyStpLdtStopDddVal
            settingsSerializer.saveSettings()
        }
        else if (idRoot.sliderTopic==='agsaLdtCyclesNum')
        {
            idRoot.dummyStpLdtCyclesNumVal = (idSlider.currentValueX100Rounded()/100.0);
            agsaLongDurationTest.cyclesNum = idRoot.dummyStpLdtCyclesNumVal
            settingsSerializer.saveSettings()
        }


        else if (idRoot.sliderTopic==='agsaLdtTestSteps')
        {
            idRoot.dummyStpLdtTestStepsVal = (idSlider.currentValueX100Rounded()/100.0);
            agsaLongDurationTest.testSteps = idRoot.dummyStpLdtTestStepsVal
            settingsSerializer.saveSettings()
        }
    }

    function listEntryToggleVisible(xname)
    {
        if(xname==='agsaLiveMode')
        {
            return true;
        }
        else if(xname==='agsaBlockageDetect')
        {
            return true;
        }

        return false;
    }

    function listEntryToggleActive(xname)
    {
        if(xname==='agsaLiveMode')
        {
            return agsaControl.testLiveMode;
        }
        else if(xname==='agsaBlockageDetect')
        {
            return agsaControl.testBlockageDetect;
        }

        return false;
    }

    function processListToggleChanged(xname,active)
    {
        console.log("processListToggleChanged("+xname+","+active+")");

        if(xname==='agsaLiveMode')
        {
            agsaControl.testLiveMode = active;
            settingsSerializer.saveSettings()
        }
        else if(xname==='agsaBlockageDetect')
        {
            agsaControl.testBlockageDetect = active;
            settingsSerializer.saveSettings()
        }
    }

    function processListEntryClicked(xname)
    {
        console.log("processListEntryClicked("+xname+")");

        if(xname==='dcMenu')
        {
            idListView.model = idlistModelDcMotor;
            idButtonClose.visible = true;
            idButtonBackToMotorSelection.visible = true;
            hideMenuNavigation(true);
            idTitle.titleText = "PDU MOTOR TEST"
            idTitle.visible = true;
        }
        else if(xname==='agsaMenu')
        {
            idListView.model = idlistModelStpMotor;
            idButtonClose.visible = true;
            idButtonBackToMotorSelection.visible = true;
            hideMenuNavigation(true);
            idTitle.titleText = "AGSA MOTOR TEST"
            idTitle.visible = true;
        }
        else if(xname==='agsaLdtMenu')
        {
            idListView.model = idlistModelStpMotorLongTest;
            idButtonClose.visible = true;
            idButtonBackToMotorSelection.visible = true;
            hideMenuNavigation(true);
            idTitle.titleText = "AGSA LONG TEST"
            idTitle.visible = true;
        }
        else if(xname==='agsaLdtStressMenu')
        {
            idListView.model = idlistModelStpMotorStressTest;
            idButtonClose.visible = true;
            idButtonBackToMotorSelection.visible = true;
            hideMenuNavigation(true);
            idTitle.titleText = "AGSA STRESS TEST"
            idTitle.visible = true;
        }
        else if(xname==='agsaLdtStepsMenu')
        {
            idListView.model = idlistModelStpMotorStepsTest;
            idButtonClose.visible = true;
            idButtonBackToMotorSelection.visible = true;
            hideMenuNavigation(true);
            idTitle.titleText = "AGSA STEPS TEST"
            idTitle.visible = true;
        }
        else if(xname==='dcTest')
        {
            pduDcHallMotorDriver.testMotor()
        }
        else if(xname==='dcStart')
        {
            pduDcHallMotorDriver.steps = idRoot.dummyDcStepsVal
            pduDcHallMotorDriver.pwmFrequency = idRoot.dummyDcPwmFreqVal
            pduDcHallMotorDriver.pwmDuty10th = idRoot.dummyDcPwmDutyVal * 10

            pduDcHallMotorDriver.startMotorSteps()
        }
        else if (xname==='dcStop')
        {
            pduDcHallMotorDriver.stopMotor()
        }
        else if (xname==='dcSteps')
        {
            idRoot.sliderTopic = xname;
            idSliderTitle.titleText = "Num. steps";
            idSlider.minValue  = idRoot.dummyDcStepsMin;
            idSlider.maxValue  = idRoot.dummyDcStepsMax;
            idSlider.increment = idRoot.dummyDcStepsInc;
            idSlider.currentValueX100 = idRoot.dummyDcStepsVal*100;
            updateSliderValueText();
            idRoot.sliderVisible = true;
        }
        else if (xname==='dcPwmDuty')
        {
            idRoot.sliderTopic = xname;
            idSliderTitle.titleText = "PWM duty-cycle";
            idSlider.minValue  = idRoot.dummyDcPwmDutyMin;
            idSlider.maxValue  = idRoot.dummyDcPwmDutyMax;
            idSlider.increment = idRoot.dummyDcPwmDutyInc;
            idSlider.currentValueX100 = idRoot.dummyDcPwmDutyVal*100;
            updateSliderValueText();
            idRoot.sliderVisible = true;
        }
        else if (xname==='dcPwmFrq')
        {
            idRoot.sliderTopic = xname;
            idSliderTitle.titleText = "PWM frequency";
            idSlider.minValue  = idRoot.dummyDcPwmFreqMin;
            idSlider.maxValue  = idRoot.dummyDcPwmFreqMax;
            idSlider.increment = idRoot.dummyDcPwmFreqInc;
            idSlider.currentValueX100 = idRoot.dummyDcPwmFreqVal*100;
            updateSliderValueText();
            idRoot.sliderVisible = true;
        }


        else if (xname==='agsaMoveToDdd')
        {
            idRoot.sliderTopic = xname;
            idSliderTitle.titleText = "Move to DDD";
            idSlider.minValue  = idRoot.dummyTargetDddMin;
            idSlider.maxValue  = idRoot.dummyTargetDddMax;
            idSlider.increment = idRoot.dummyTargetDddInc;
            idSlider.currentValueX100 = dddCouple.dddValue*100;
            idSliderDddValue.visible = true;
            updateSliderValueText();
            idRoot.sliderVisible = true;
        }
        else if (xname==='agsaMoveSteps')
        {
            agsaControl.steps = idRoot.dummyStpStepsVal
            agsaControl.runFrequency = idRoot.dummyStpStepsPerSecVal
            agsaControl.approachFrequency = idRoot.dummyStpLowStepsPerSecVal
            agsaControl.startFrequency = idRoot.dummyStpStepsPerSecVal
            agsaControl.rampFreqPerStep = 5

            agsaControl.moveSteps( agsaControl.steps, agsaControl.runFrequency, true, false )
        }
        else if (xname==='agsaStop')
        {
            agsaControl.stopAgsaControl()
        }
        else if (xname==='agsaNumSteps')
        {
            idRoot.sliderTopic = xname;
            idSliderTitle.titleText = "Move Steps";
            idSlider.minValue  = idRoot.dummyStpStepsMin;
            idSlider.maxValue  = idRoot.dummyStpStepsMax;
            idSlider.increment = idRoot.dummyStpStepsInc;
            idSlider.currentValueX100 = idRoot.dummyStpStepsVal*100;
            updateSliderValueText();
            idRoot.sliderVisible = true;
        }
        else if (xname==='agsaStepsPerSec')
        {
            idRoot.sliderTopic = xname;
            idSliderTitle.titleText = "Frequency";
            idSlider.minValue  = idRoot.dummyStpStepsPerSecMin;
            idSlider.maxValue  = idRoot.dummyStpStepsPerSecMax;
            idSlider.increment = idRoot.dummyStpStepsPerSecInc;
            idSlider.currentValueX100 = idRoot.dummyStpStepsPerSecVal*100;
            updateSliderValueText();
            idRoot.sliderVisible = true;
        }
        else if (xname==='agsaLowStepsPerSec')
        {
            idRoot.sliderTopic = xname;
            idSliderTitle.titleText = "Appr. Freq.";
            idSlider.minValue  = idRoot.dummyStpLowStepsPerSecMin;
            idSlider.maxValue  = idRoot.dummyStpLowStepsPerSecMax;
            idSlider.increment = idRoot.dummyStpLowStepsPerSecInc;
            idSlider.currentValueX100 = idRoot.dummyStpLowStepsPerSecVal*100;
            updateSliderValueText();
            idRoot.sliderVisible = true;
        }


        else if (xname==='agsaLdtStart')
        {
            agsaLongDurationTest.loggingEnabled = true;
            agsaLongDurationTest.startManualTest()
        }
        else if (xname==='agsaLdtStop')
        {
            agsaLongDurationTest.userStopTest()
        }
        else if (xname==='agsaLdtStartDdd')
        {
            idRoot.sliderTopic = xname;
            idSliderTitle.titleText = "Start DDD";
            idSlider.minValue  = idRoot.dummyStpLdtStartDddMin;
            idSlider.maxValue  = idRoot.dummyStpLdtStartDddMax;
            idSlider.increment = idRoot.dummyStpLdtStartDddInc;
            idSlider.currentValueX100 = idRoot.dummyStpLdtStartDddVal*100;
            updateSliderValueText();
            idRoot.sliderVisible = true;
        }
        else if (xname==='agsaLdtStopDdd')
        {
            idRoot.sliderTopic = xname;
            idSliderTitle.titleText = "Stop DDD";
            idSlider.minValue  = idRoot.dummyStpLdtStopDddMin;
            idSlider.maxValue  = idRoot.dummyStpLdtStopDddMax;
            idSlider.increment = idRoot.dummyStpLdtStopDddInc;
            idSlider.currentValueX100 = idRoot.dummyStpLdtStopDddVal*100;
            updateSliderValueText();
            idRoot.sliderVisible = true;
        }
        else if (xname==='agsaLdtCyclesNum')
        {
            idRoot.sliderTopic = xname;
            idSliderTitle.titleText = "Cycles Number";
            idSlider.minValue  = idRoot.dummyStpLdtCyclesNumMin;
            idSlider.maxValue  = idRoot.dummyStpLdtCyclesNumMax;
            idSlider.increment = idRoot.dummyStpLdtCyclesNumInc;
            idSlider.currentValueX100 = idRoot.dummyStpLdtCyclesNumVal*100;
            updateSliderValueText();
            idRoot.sliderVisible = true;
        }

        else if (xname==='agsaLdtStressStart')
        {
            agsaLongDurationTest.loggingEnabled = true;
            agsaLongDurationTest.startStressTest()
        }
        else if (xname==='agsaLdtStressStop')
        {
            agsaLongDurationTest.userStopTest()
        }

        else if (xname==='agsaLdtStepsStart')
        {
            agsaLongDurationTest.loggingEnabled = true;
            agsaLongDurationTest.startStepsTest()
        }
        else if (xname==='agsaLdtStepsStop')
        {
            agsaLongDurationTest.userStopTest()
        }
        else if (xname==='agsaLdtTestSteps')
        {
            idRoot.sliderTopic = xname;
            idSliderTitle.titleText = "Test Steps";
            idSlider.minValue  = idRoot.dummyStpLdtTestStepsMin;
            idSlider.maxValue  = idRoot.dummyStpLdtTestStepsMax;
            idSlider.increment = idRoot.dummyStpLdtTestStepsInc;
            idSlider.currentValueX100 = idRoot.dummyStpLdtTestStepsVal * 100;
            updateSliderValueText();
            idRoot.sliderVisible = true;
        }
    }


    function listEntryValueText(xname)
    {
        if (xname==='dcSteps')
        {
            return("" + idRoot.dummyDcStepsVal + "");
        }
        else if (xname==='dcRunTime')
        {
            return("" + pduDcHallMotorDriver.runTime + "ms");
        }
        else if (xname==='dcOvershootSteps')
        {
            return("" + pduDcHallMotorDriver.overshootSteps + "");
        }
        else if (xname==='dcPwmDuty')
        {
            return("" + idRoot.dummyDcPwmDutyVal + "%");
        }
        else if (xname==='dcPwmFrq')
        {
            return("" + idRoot.dummyDcPwmFreqVal / 1000 + "kHz");
        }
        else if (xname==='dcTest')
        {
            return( pduDcHallMotorDriver.motorTestOk ? "OK" : "Failed" );
        }



        else if (xname==='agsaNumSteps')
        {
            return( idRoot.dummyStpStepsVal );
        }
        else if (xname==='agsaStepsPerSec')
        {
            return("" + idRoot.dummyStpStepsPerSecVal / 1000 + "kHz");
        }
        else if (xname==='agsaLowStepsPerSec')
        {
            return("" + idRoot.dummyStpLowStepsPerSecVal / 1000 + "kHz");
        }
        else if (xname==='agsaMoveToDdd')
        {
            return( idRoot.dummyTargetDddVal );
        }
        else if (xname==='agsaStatus')
        {
            return(agsaControl.failed ? "FAILED" : "OK" );
        }
        else if (xname==='agsaDdd')
        {
            return( dddCouple.dddValue );
        }



        else if (xname==='agsaLdtStartDdd')
        {
            return agsaLongDurationTest.manualStartDddValue;
        }
        else if (xname==='agsaLdtStopDdd')
        {
            return agsaLongDurationTest.manualStopDddValue;
        }
        else if (xname==='agsaLdtCyclesNum')
        {
            return agsaLongDurationTest.manualCyclesNum;
        }
        else if (xname==='agsaLdtProgress')
        {
            if ( agsaLongDurationTest.manualCyclesNum )
                return Math.ceil( agsaLongDurationTest.stepCycle * 100 / agsaLongDurationTest.manualCyclesNum ) + "%";
            else
                return "0%";
        }



        else if (xname==='agsaLdtStressStep0to200Cycle')
        {
            return agsaLongDurationTest.stressStep0to200Cycle;
        }
        else if (xname==='agsaLdtStressStep200to400Cycle')
        {
            return agsaLongDurationTest.stressStep200to400Cycle;
        }
        else if (xname==='agsaLdtStressStep400to600Cycle')
        {
            return agsaLongDurationTest.stressStep400to600Cycle;
        }
        else if (xname==='agsaLdtStressStep600to800Cycle')
        {
            return agsaLongDurationTest.stressStep600to800Cycle;
        }
        else if (xname==='agsaLdtTotalCycle')
        {
            return agsaLongDurationTest.totalCycle;
        }



        else if (xname==='agsaLdtTestSteps')
        {
            return( idRoot.dummyStpLdtTestStepsVal );
        }



        else
        {
            return("");
        }
    }

    function listEntryEnabled( xname )
    {
        let isenabled = false;

        if (xname==='dcMenu')
        {
            isenabled = true
        }
        else if (xname==='agsaMenu')
        {
            isenabled = !agsaLongDurationTest.running
        }
        else if (xname==='agsaLdtMenu')
        {
            isenabled = true
        }
        else if (xname==='agsaLdtStressMenu')
        {
            isenabled = true
        }
        else if (xname==='agsaLdtStepsMenu')
        {
            isenabled = true
        }
        else if (xname==='dcTest')
        {
            isenabled = ( !pduDcHallMotorRunning && pduDcHallMotorValid )
        }
        else if (xname==='dcStart')
        {
            isenabled = ( !pduDcHallMotorRunning && pduDcHallMotorValid )
        }
        else if (xname==='dcStop')
        {
            isenabled = ( pduDcHallMotorRunning && pduDcHallMotorValid )
        }
        else if (xname==='dcRunTime')
        {
            isenabled = false
        }
        else if (xname==='dcOvershootSteps')
        {
            isenabled = false
        }
        else if ( xname.startsWith( "dc" ) )
        {
            isenabled = !pduDcHallMotorRunning
        }


        else if (xname==='agsaMoveToDdd')
        {
            isenabled = true;//( !agsaStepperMotorRunning && agsaStepperMotorValid )
        }
        else if (xname==='agsaStatus')
        {
            isenabled = true
        }
        else if (xname==='agsaDdd')
        {
            isenabled = true
        }
        else if (xname==='agsaLiveMode')
        {
            isenabled = true
        }
        else if (xname==='agsaBlockageDetect')
        {
            isenabled = true
        }
        else if (xname==='agsaMoveSteps')
        {
            isenabled = ( !agsaStepperMotorRunning && agsaStepperMotorValid )
        }
        else if (xname==='agsaStop')
        {
            isenabled = ( agsaStepperMotorRunning && agsaStepperMotorValid )
        }


        else if (xname==='agsaLdtStart')
        {
            isenabled = ( !agsaLongDurationTest.running )
        }
        else if (xname==='agsaLdtStop')
        {
            isenabled = ( agsaLongDurationTest.running )
        }
        else if (xname==='agsaLdtStartDdd')
        {
            isenabled = true
        }
        else if (xname==='agsaLdtStopDdd')
        {
            isenabled = true
        }
        else if (xname==='agsaLdtCyclesNum')
        {
            isenabled = true
        }
        else if (xname==='agsaLdtProgress')
        {
            isenabled = true
        }


        else if (xname==='agsaLdtStressStart')
        {
            isenabled = ( !agsaLongDurationTest.running )
        }
        else if (xname==='agsaLdtStressStop')
        {
            isenabled = ( agsaLongDurationTest.running )
        }
        else if (xname==='agsaLdtTotalCycle')
        {
            isenabled = false
        }
        else if (xname==='agsaLdtStressStep0to200Cycle')
        {
            isenabled = false
        }
        else if (xname==='agsaLdtStressStep200to400Cycle')
        {
            isenabled = false
        }
        else if (xname==='agsaLdtStressStep400to600Cycle')
        {
            isenabled = false
        }
        else if (xname==='agsaLdtStressStep600to800Cycle')
        {
            isenabled = false
        }


        else if (xname==='agsaLdtStepsStart')
        {
            isenabled = ( !agsaLongDurationTest.running )
        }
        else if (xname==='agsaLdtStepsStop')
        {
            isenabled = ( agsaLongDurationTest.running )
        }
        else if (xname==='agsaLdtTestSteps')
        {
            isenabled = ( !agsaLongDurationTest.running )
        }


        else if ( xname.startsWith( "agsa" ) )
        {
            isenabled = !agsaStepperMotorRunning
        }

        return isenabled
    }

    ListModel
    {
        id: idlistModel;

        ListElement {
            motor: "PDU MOTOR TEST";
            xname: "dcTest";
            xtext: "Test"
        }
        ListElement {
            motor: "PDU MOTOR TEST";
            xname: "dcStart";
            xtext: "Start"
        }
        ListElement {
            motor: "PDU MOTOR TEST";
            xname: "dcStop"
            xtext: "Stop"
        }
        ListElement {
            motor: "PDU MOTOR TEST";
            xname: "dcSteps"
            xtext: "Num. steps"
        }
        ListElement {
            motor: "PDU MOTOR TEST";
            xname: "dcOvershootSteps"
            xtext: "Overshoot"
        }
        ListElement {
            motor: "PDU MOTOR TEST";
            xname: "dcRunTime"
            xtext: "Run time"
        }
        ListElement {
            motor: "PDU MOTOR TEST";
            xname: "dcPwmDuty"
            xtext: "PWM duty"
        }
        ListElement {
            motor: "PDU MOTOR TEST";
            xname: "dcPwmFrq"
            xtext: "PWM freq."
        }



        ListElement {
            motor: "AGSA MOTOR TEST"
            xname: "agsaDdd";
            xtext: "DDD Value"
        }
        ListElement {
            motor: "AGSA MOTOR TEST"
            xname: "agsaStatus";
            xtext: "Status"
        }
        ListElement {
            motor: "AGSA MOTOR TEST"
            xname: "agsaMoveToDdd";
            xtext: "Move to DDD"
        }
        ListElement {
            motor: "AGSA MOTOR TEST";
            xname: "agsaMoveSteps";
            xtext: "Move Steps"
        }
        ListElement {
            motor: "AGSA MOTOR TEST";
            xname: "agsaStop"
            xtext: "Stop"
        }
        ListElement {
            motor: "AGSA MOTOR TEST";
            xname: "agsaNumSteps"
            xtext: "Num. Steps"
        }
        ListElement {
            motor: "AGSA MOTOR TEST"
            xname: "agsaLiveMode";
            xtext: "Live Mode"
        }
        ListElement {
            motor: "AGSA MOTOR TEST"
            xname: "agsaBlockageDetect";
            xtext: "Blockage Detect"
        }
        ListElement {
            motor: "AGSA MOTOR TEST";
            xname: "agsaStepsPerSec"
            xtext: "Frequency"
        }
        ListElement {
            motor: "AGSA MOTOR TEST";
            xname: "agsaLowStepsPerSec"
            xtext: "Appr. Freq."
        }



        ListElement {
            motor: "AGSA LONG TEST"
            xname: "agsaDdd";
            xtext: "DDD Value"
        }
        ListElement {
            motor: "AGSA LONG TEST"
            xname: "agsaLdtStart";
            xtext: "Start"
        }
        ListElement {
            motor: "AGSA LONG TEST"
            xname: "agsaLdtStop";
            xtext: "Stop"
        }
        ListElement {
            motor: "AGSA LONG TEST"
            xname: "agsaLdtProgress"
            xtext: "Progress"
        }
        ListElement {
            motor: "AGSA LONG TEST"
            xname: "agsaLdtStartDdd";
            xtext: "Start DDD Value"
        }
        ListElement {
            motor: "AGSA LONG TEST"
            xname: "agsaLdtStopDdd";
            xtext: "Stop DDD Value"
        }



        ListElement {
            motor: "AGSA STRESS TEST"
            xname: "agsaLdtStressStart";
            xtext: "Start"
        }
        ListElement {
            motor: "AGSA STRESS TEST"
            xname: "agsaLdtStressStop";
            xtext: "Stop"
        }
        ListElement {
            motor: "AGSA STRESS TEST"
            xname: "agsaLdtTotalCycle"
            xtext: "Total Cycle"
        }
        ListElement {
            motor: "AGSA STRESS TEST"
            xname: "agsaLdtStressStep0to200Cycle"
            xtext: "0-210 Cycle"
        }
        ListElement {
            motor: "AGSA STRESS TEST"
            xname: "agsaLdtStressStep200to400Cycle"
            xtext: "190-410 Cycle"
        }
        ListElement {
            motor: "AGSA STRESS TEST"
            xname: "agsaLdtStressStep400to600Cycle"
            xtext: "390-610 Cycle"
        }
        ListElement {
            motor: "AGSA STRESS TEST"
            xname: "agsaLdtStressStep600to800Cycle"
            xtext: "590-800 Cycle"
        }


        ListElement {
            motor: "AGSA STEPS TEST"
            xname: "agsaLdtStepsStart";
            xtext: "Start"
        }
        ListElement {
            motor: "AGSA STEPS TEST"
            xname: "agsaLdtStepsStop";
            xtext: "Stop"
        }
        ListElement {
            motor: "AGSA STEPS TEST"
            xname: "agsaLdtTotalCycle"
            xtext: "Total Cycle"
        }
        ListElement {
            motor: "AGSA STEPS TEST"
            xname: "agsaLdtTestSteps"
            xtext: "Steps"
        }

    }

    ListModel
    {
        id: idlistModelMotorSelection;

        ListElement {
            motor: "PDU MOTOR TEST";
            xname: "dcMenu";
            xtext: "PDU Motor"
        }
        ListElement {
            motor: "AGSA MOTOR TEST"
            xname: "agsaMenu"
            xtext: "AGSA Stepper"
        }
        ListElement {
            motor: "AGSA LONG TEST"
            xname: "agsaLdtMenu"
            xtext: "AGSA Long Test"
        }
        ListElement {
            motor: "AGSA STRESS TEST"
            xname: "agsaLdtStressMenu"
            xtext: "AGSA Stress Test"
        }
        ListElement {
            motor: "AGSA STEPS TEST"
            xname: "agsaLdtStepsMenu"
            xtext: "AGSA Steps Test"
        }
    }

    ListModel
    {
        id: idlistModelDcMotor;

        ListElement {
            motor: "PDU MOTOR TEST";
            xname: "dcTest";
            xtext: "Test"
        }
        ListElement {
            motor: "PDU MOTOR TEST";
            xname: "dcStart";
            xtext: "Start"
        }
        ListElement {
            motor: "PDU MOTOR TEST";
            xname: "dcStop"
            xtext: "Stop"
        }
        ListElement {
            motor: "PDU MOTOR TEST";
            xname: "dcOvershootSteps"
            xtext: "Overshoot"
        }
        ListElement {
            motor: "PDU MOTOR TEST";
            xname: "dcRunTime"
            xtext: "Run time"
        }
        ListElement {
            motor: "PDU MOTOR TEST";
            xname: "dcSteps"
            xtext: "Num. steps"
        }
        ListElement {
            motor: "PDU MOTOR TEST";
            xname: "dcPwmDuty"
            xtext: "PWM duty"
        }
        ListElement {
            motor: "PDU MOTOR TEST";
            xname: "dcPwmFrq"
            xtext: "PWM freq."
        }
    }

    ListModel
    {
        id: idlistModelStpMotor;

        ListElement {
            motor: "AGSA MOTOR TEST"
            xname: "agsaDdd";
            xtext: "DDD Value"
        }
        ListElement {
            motor: "AGSA MOTOR TEST"
            xname: "agsaStatus";
            xtext: "Status"
        }
        ListElement {
            motor: "AGSA MOTOR TEST"
            xname: "agsaMoveToDdd";
            xtext: "Move to DDD"
        }
        ListElement {
            motor: "AGSA MOTOR TEST"
            xname: "agsaMoveSteps";
            xtext: "Move Steps"
        }
        ListElement {
            motor: "AGSA MOTOR TEST"
            xname: "agsaStop"
            xtext: "Stop"
        }
        ListElement {
            motor: "AGSA MOTOR TEST"
            xname: "agsaNumSteps"
            xtext: "Num. Steps"
        }
        ListElement {
            motor: "AGSA MOTOR TEST"
            xname: "agsaLiveMode";
            xtext: "Live Mode"
        }
        ListElement {
            motor: "AGSA MOTOR TEST"
            xname: "agsaBlockageDetect";
            xtext: "Blockage Detect"
        }
        ListElement {
            motor: "AGSA MOTOR TEST"
            xname: "agsaStepsPerSec"
            xtext: "Frequency"
        }
        ListElement {
            motor: "AGSA MOTOR TEST"
            xname: "agsaLowStepsPerSec"
            xtext: "Appr. Freq."
        }
    }

    ListModel
    {
        id: idlistModelStpMotorLongTest;

        ListElement {
            motor: "AGSA LONG TEST"
            xname: "agsaDdd";
            xtext: "DDD Value"
        }
        ListElement {
            motor: "AGSA LONG TEST"
            xname: "agsaLdtStart";
            xtext: "Start"
        }
        ListElement {
            motor: "AGSA LONG TEST"
            xname: "agsaLdtStop";
            xtext: "Stop"
        }
        ListElement {
            motor: "AGSA LONG TEST"
            xname: "agsaLdtProgress"
            xtext: "Progress"
        }
        ListElement {
            motor: "AGSA LONG TEST"
            xname: "agsaLdtStartDdd";
            xtext: "Start DDD Value"
        }
        ListElement {
            motor: "AGSA LONG TEST"
            xname: "agsaLdtStopDdd";
            xtext: "Stop DDD Value"
        }
        ListElement {
            motor: "AGSA MOTOR TEST"
            xname: "agsaLdtCyclesNum"
            xtext: "Cycles Number"
        }
        ListElement {
            motor: "AGSA MOTOR TEST"
            xname: "agsaStepsPerSec"
            xtext: "Frequency"
        }
        ListElement {
            motor: "AGSA MOTOR TEST"
            xname: "agsaLowStepsPerSec"
            xtext: "Appr. Freq."
        }
    }

    ListModel
    {
        id: idlistModelStpMotorStressTest;

        ListElement {
            motor: "AGSA STRESS TEST"
            xname: "agsaLdtStressStart";
            xtext: "Start"
        }
        ListElement {
            motor: "AGSA STRESS TEST"
            xname: "agsaLdtStressStop";
            xtext: "Stop"
        }
        ListElement {
            motor: "AGSA STRESS TEST"
            xname: "agsaLdtTotalCycle"
            xtext: "Total Cycle"
        }
        ListElement {
            motor: "AGSA STRESS TEST"
            xname: "agsaLdtStressStep0to200Cycle"
            xtext: "0-210 Cycle"
        }
        ListElement {
            motor: "AGSA STRESS TEST"
            xname: "agsaLdtStressStep200to400Cycle"
            xtext: "190-410 Cycle"
        }
        ListElement {
            motor: "AGSA STRESS TEST"
            xname: "agsaLdtStressStep400to600Cycle"
            xtext: "390-610 Cycle"
        }
        ListElement {
            motor: "AGSA STRESS TEST"
            xname: "agsaLdtStressStep600to800Cycle"
            xtext: "590-800 Cycle"
        }
    }

    ListModel
    {
        id: idlistModelStpMotorStepsTest;

        ListElement {
            motor: "AGSA STEPS TEST"
            xname: "agsaLdtStepsStart";
            xtext: "Start"
        }
        ListElement {
            motor: "AGSA STEPS TEST"
            xname: "agsaLdtStepsStop";
            xtext: "Stop"
        }
        ListElement {
            motor: "AGSA STEPS TEST"
            xname: "agsaLdtTotalCycle"
            xtext: "Total Cycle"
        }
        ListElement {
            motor: "AGSA STEPS TEST"
            xname: "agsaLdtTestSteps"
            xtext: "Steps"
        }
    }

    ListView
    {
        id: idListView;

        anchors.fill: parent
        // For a display height of 226.
        topMargin: 80
        bottomMargin: 54

        model: idlistModelMotorSelection;

        delegate: Rectangle
        {
            id: idListEntry;
            width: 480;
            height: 96;
            color: "black";

            property bool xactive: listEntryEnabled( xname )

            Text
            {
                id: idListEntryText;
                anchors.fill: parent;
                font.pixelSize: 58
                font.letterSpacing: 0.23
                font.bold: false
                font.family: "D-DIN Condensed HEMRO"
                horizontalAlignment: Text.AlignHCenter;
                verticalAlignment:   Text.AlignVCenter;
                color: xactive ? idMain.colorEkxTextWhite : idMain.colorEkxTextGrey;
                text: xtext;
            }

            Text
            {
                anchors.right: parent.right;
                anchors.rightMargin: 10;
                anchors.verticalCenter: parent.verticalCenter;
                width: 80
                height: 80
                font.pixelSize: 40
                font.letterSpacing: 2
                font.family: "D-DIN Condensed HEMRO"
                font.bold: true
                horizontalAlignment: Text.AlignRight;
                verticalAlignment: Text.AlignVCenter;
                color: idMain.colorEkxTextGrey
                text: listEntryValueText(xname);
            }

            ActivateToggle
            {
                id: idActivateToggle

                visible: listEntryToggleVisible( xname )
                width: 72
                height: 72
                z:2

                anchors.right: parent.right

                active: listEntryToggleActive( xname )

                onToggleChanged:
                {
                    processListToggleChanged(xname,active);
                }
            }

            MouseArea
            {
                enabled: xactive
                anchors.fill: parent;
                onClicked:
                {
                    processListEntryClicked(xname);
                }
            }
        }
        section.property: idRoot.sectionsEnabled ? "motor" : ""
        section.criteria: ViewSection.FullString
        section.delegate: Rectangle
        {
            id: idListSection;
            width: 480;
            height: 96;
            color: "black";

            Text
            {
                id: idListSectionText
                anchors.fill: parent;
                font.pixelSize: 58
                font.letterSpacing: 0.23
                font.bold: true
                font.family: "D-DIN Condensed HEMRO"
                horizontalAlignment: Text.AlignHCenter;
                verticalAlignment:   Text.AlignVCenter;
                color: false ? idMain.colorEkxTextWhite : idMain.colorEkxTextGrey
                text: section;
            }
        }
    }

    EkxButton
    {
        id: idButtonClose;
        z:2
        visible: false;
        buttonType: EkxButton.ButtonType.CLOSE;
        onClicked:
        {
            idListView.model = idlistModelMotorSelection;
            idButtonClose.visible = false;
            idButtonBackToMotorSelection.visible = false;
            idTitle.visible = false;
            hideMenuNavigation(false);

            settingsStateMachine.backToMainMenu();
        }
    }

    EkxButton
    {
        id: idButtonBackToMotorSelection;
        z: 2;
        visible: false;
        //# back to motor selection menu.
        buttonType: EkxButton.ButtonType.PREVIOUS;
        onClicked:
        {
            idListView.model = idlistModelMotorSelection;
            idButtonClose.visible = false;
            idButtonBackToMotorSelection.visible = false;
            idTitle.visible = false;
            hideMenuNavigation(false);
        }
    }

    EkxTitle
    {
        id: idTitle;
        visible: false;
        titleText: "";
    }


    Rectangle
    {
        id: idPageSlider;
        z: 5;
        visible: idRoot.sliderVisible;
        width: 480;
        height: 360;
        color: "black";

        EkxTitle
        {
            id: idSliderTitle;
            titleText: "Dummy";
        }

        EkxSlider
        {
            id: idSlider

            currentValueX100: -1 // must be monitored by onCurrentValueX100Changed handler
            showLimits: false
            minValue: 0
            maxValue: 100
            increment: 1

            onCurrentValueX100Changed:
            {
                updateSliderValueText()
            }

            onReleased:
            {
                onSliderReleased()
            }
        }

        EkxButton
        {
            buttonType: EkxButton.ButtonType.PREVIOUS;
            onClicked:
            {
                idRoot.sliderVisible = false;
            }
        }

        EkxButton
        {
            buttonType: EkxButton.ButtonType.OK;
            onClicked:
            {
                idRoot.sliderVisible = false;
                processSliderValueAccepted();
            }
        }

        Item
        {
            id: idSliderDddValue
            visible: false
            anchors.fill: parent

            Text
            {
                visible: !agsaControl.failed
                id: idSliderDddValLabel
                color: "white"
                font.pixelSize: 60
                font.letterSpacing: 0.33
                font.family: "D-DIN Condensed HEMRO"
                anchors.left: parent.left
                anchors.bottom: parent.bottom
                width: parent.width / 2 - 10
                height: 70
                text: "DDD"
                verticalAlignment: Text.AlignTop
                horizontalAlignment: Text.AlignRight
            }

            Text
            {
                visible: !agsaControl.failed
                id: idSliderDddValValue
                color: "white"
                font.pixelSize: 60
                font.letterSpacing: 0.33
                font.family: "D-DIN Condensed HEMRO"
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                width: parent.width / 2 - 10
                height: idSliderDddValLabel.height

                text: dddCouple.dddValue
                verticalAlignment: Text.AlignTop
                horizontalAlignment: Text.AlignLeft
            }

            Text
            {
                visible: agsaControl.failed
                font.pixelSize: 60
                font.letterSpacing: 0.33
                font.family: "D-DIN Condensed HEMRO"
                anchors.bottom: parent.bottom
                anchors.left: parent.left
                height: idSliderDddValLabel.height
                width: parent.width

                color: agsaControl.failed ? "red" : "white"
                text: agsaControl.failed ? "FAILED" : "OK"

                verticalAlignment: Text.AlignTop
                horizontalAlignment: Text.AlignHCenter
            }
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

    // --- Debugging helpers: ------------------------------
    Rectangle
    {
        visible: simulateMotorDebug;
        z:7
        x: 0;
        y: 100+25*0;
        width: 200;
        height: 25;
        border.width: 1;
        border.color: "white"
        color: "blue"
        Text
        {
            anchors.fill: parent;
            text: "pduDcHallMotorValid: "+pduDcHallMotorValid
        }
        MouseArea
        {
            anchors.fill: parent;
            onClicked:
            {
                pduDcHallMotorValid = pduDcHallMotorValid?false:true;
            }
        }
    }

    Rectangle
    {
        visible: simulateMotorDebug;
        z:7
        x: 0;
        y: 100+25*1;
        width: 200;
        height: 25;
        border.width: 1;
        border.color: "white"
        color: "blue"
        Text
        {
            anchors.fill: parent;
            text: "pduDcHallMotorRunning: "+pduDcHallMotorRunning
        }
        MouseArea
        {
            anchors.fill: parent;
            onClicked:
            {
                pduDcHallMotorRunning = pduDcHallMotorRunning?false:true;
            }
        }
    }

    Rectangle
    {
        visible: simulateMotorDebug;
        z:7
        x: 0;
        y: 100+25*2;
        width: 200;
        height: 25;
        border.width: 1;
        border.color: "white"
        color: "green"
        Text
        {
            anchors.fill: parent;
            text: "agsaStepperMotorValid: "+agsaStepperMotorValid
        }
        MouseArea
        {
            anchors.fill: parent;
            onClicked:
            {
                agsaStepperMotorValid = agsaStepperMotorValid?false:true;
            }
        }
    }

    Rectangle
    {
        visible: simulateMotorDebug;
        z:7
        x: 0;
        y: 100+25*3;
        width: 200;
        height: 25;
        border.width: 1;
        border.color: "white"
        color: "green"
        Text
        {
            anchors.fill: parent;
            text: "agsaStepperMotorRunning: "+agsaStepperMotorRunning
        }
        MouseArea
        {
            anchors.fill: parent;
            onClicked:
            {
                agsaStepperMotorRunning = agsaStepperMotorRunning?false:true;
            }
        }
    }

}
