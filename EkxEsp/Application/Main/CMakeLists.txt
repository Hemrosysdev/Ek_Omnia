set(COMPONENT_SRCDIRS Sources )
set(COMPONENT_ADD_INCLUDEDIRS Interfaces )

set( COMPONENT_REQUIRES 
	UxEspCppLibrary 
	Common 
	PluginTaskAt24c16
	PluginTaskWifiConnector
    UcdDcHallMotorTask
	UcdDiscreteInTask
	UcdEepromTask 
	UcdPwmOutTask
	UcdSpotLightTask
	UcdStartStopBtnTask 
	UcdStartStopLightTask
    UcdStepperMotorTask
	UcdSwUpdateTask 
	UcdTempSensorTask
	UcdMcuUartTask
	UcdWifiConnectorTask
	CtrlUartTask
	UcdHttpServerTask 
	)

register_component()
