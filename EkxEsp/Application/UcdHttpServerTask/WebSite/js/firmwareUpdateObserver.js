/********************************************************************************/

var stateIntervalId = null;
var stateTimeoutId = null;
var stateTimeout = 60;
var lastState = "";

/********************************************************************************/

function firmwareUpdateObserver()
{
	var jsonCmd = 
	{
		msgType : "RequestSoftwareUpdateStatus",
	};
	restApiCmd( false, "Get FW update state", jsonCmd, parseFirmwareUpdateState );
}

/********************************************************************************/

function firmwareUpdateFailed( info )
{
	console.log( "firmwareUpdateFailed()" );

	showNormalLog( "<font color='red'>" + info + "</font>" );
	hideProgressBar();
	enablePopupOk();

	firmwareUpdateAbort();
}

/********************************************************************************/

function firmwareUpdateSucceeded( info )
{
	console.log( "firmwareUpdateSucceeded()" );

	clearInterval(stateIntervalId);
	clearInterval(stateTimeoutId);
	showNormalLog( "<font color='green'>" + info + "</font>" );
	hideProgressBar();
	enablePopupOk();
}

/********************************************************************************/

function firmwareUpdateAbort()
{
	console.log( "firmwareUpdateAbort() send JSON" );
	
	clearInterval(stateIntervalId);
	clearInterval(stateTimeoutId);

	var jsonCmd = 
	{
		msgType : "ExecuteSysCommands",
		sysCommands : [ "sudo killall -q -9 swupdate" ]
	};

	restApiCmd( false, "Kill pending processes", jsonCmd );
}

/********************************************************************************/

function firmwareUpdateTimeout()
{
	firmwareUpdateFailed( "Update aborted by timeout! Please try to repeat!<br>"
							+ "<ul>"
							+ "<li>check WiFi quality to improve data transmission reliability" 
							+ "<li>locate grinder closer to your local router if grinder is in station mode" 
							+ "<li>locate your device closer to the grinder if your grinder is in access point mode" 
							+ "</ul>"
						);
}

/********************************************************************************/

function parseFirmwareUpdateState( jsonResult )
{
	var jsonObj = null;

	try
	{
		jsonObj = JSON.parse( jsonResult );
	}
	catch ( e )
	{
		firmwareUpdateFailed( "Internal communication to grinder failed!<br>Please observe grinder display for current state." );
		return;
	}

	if ( jsonObj.msgType != "Reply" || jsonObj.refMsgType != "RequestSoftwareUpdateStatus" || jsonObj.success != true )
	{
		console.log( "invalid JSON object" );
		return;
	}
	
	//console.log( jsonObj.data.State );

	if ( lastState != jsonObj.data.State )
	{
		lastState = jsonObj.data.State;

		//console.log( "state change " + lastState );

		var timeout = stateTimeout;
		if ( parseInt( jsonObj["timeout"] ) > 0 )
		{
			//console.log( "set timeout " + jsonObj["timeout"] );
			timeout = parseInt( jsonObj["timeout"] );
		}

		clearTimeout( stateTimeoutId );
		stateTimeoutId = setTimeout( firmwareUpdateTimeout, timeout * 1000 );

		if ( jsonObj.data.State == "UPLOAD_COMPLETED" )
		{
			showProgressInfo( "Upload completed ..." );
			showProgressBar( "5%" );
		}
		else if ( jsonObj.data.State == "UPLOAD_FAILED" )
		{
			firmwareUpdateFailed( "Upload of .swu file failed!</br>Possibe reason: " + jsonObj.data.ErrorReason + "</br>Please restart the grinder and retry." );
		}
		else if ( jsonObj.data.State == "UPLOAD_ABORTED" )
		{
			firmwareUpdateFailed( "Upload of file aborted by user!" );
		}
		else if ( jsonObj.data.State == "EVALUATE_PACKAGE" )
		{
			showProgressInfo( "Evaluate upload package ..." );
			showProgressBar( "5%" );
		}
		else if ( jsonObj.data.State == "PACKAGE_FAILED" )
		{
			firmwareUpdateFailed( "Uploaded package check failed!</br>Maybe the uploaded file is corrupted or is no valid .swu file.</br>Please restart the grinder and retry with a valid .swu file." );
		}
		else if ( jsonObj.data.State == "PACKAGE_NO_UPGRADE" )
		{
			firmwareUpdateFailed( "Uploaded package is no upgrade related to the current firmware version!</br>Please retry with a valid upgrade .swu file." );
		}
		else if ( jsonObj.data.State == "UNPACK_ROOTFS" )
		{
			showProgressInfo( "Unpacking image #1 of 2 ..." );
			showProgressBar( "20%" );
		}
		else if ( jsonObj.data.State == "UNPACK_ROOTFS_FAILED" )
		{
			firmwareUpdateFailed( "Validation of image #1 failed!</br>Please retry, if still failed please contact technical support." );
		}
		else if ( jsonObj.data.State == "UNPACK_APPLICATION" )
		{
			showProgressInfo( "Unpacking image #2 of 2 ..." );
			showProgressBar( "40%" );
		}
		else if ( jsonObj.data.State == "UNPACK_APPLICATION_FAILED" )
		{
			firmwareUpdateFailed( "Validation of image #2 failed!</br>Please retry, if still failed please contact technical support." );
		}
		else if ( jsonObj.data.State == "REBOOT" )
		{
			showProgressInfo( "Waiting for restart of system ..." );
			showProgressBar( "50%" );
		}
		else if ( jsonObj.data.State == "STARTUP_CHECK" )
		{
			showProgressInfo( "Startup check ..." );
			showProgressBar( "55%" );
		}
		else if ( jsonObj.data.State == "CHECK_ROOTFS" )
		{
			showProgressInfo( "Checking image #1 consistency ..." );
			showProgressBar( "60%" );
		}
		else if ( jsonObj.data.State == "ROOTFS_FAILED" )
		{
			showProgressInfo( "Consistency check of image #1 failed!</br>Going into rollback ..." );
			showProgressBar( "65%" );
		}
		else if ( jsonObj.data.State == "ESP_CHECK_INTERFACE" )
		{
			showProgressInfo( "Interface check of WiFi board ..." );
			showProgressBar( "65%" );
		}
		else if ( jsonObj.data.State == "ESP_INTERFACE_FAILED" )
		{
			showProgressInfo( "Interface check of WiFi board failed!</br>Going into rollback ..." );
			showProgressBar( "70%" );
		}
		else if ( jsonObj.data.State == "ESP_CHECK_VERSION" )
		{
			showProgressInfo( "Check of WiFi board firmware version ..." );
			showProgressBar( "70%" );
		}
		else if ( jsonObj.data.State == "ESP_NOT_AVAILABLE" )
		{
			showProgressInfo( "WiFi board not available!</br>Going into rollback ..." );
			showProgressBar( "75%" );
		}
		else if ( jsonObj.data.State == "ESP_UPDATE_IN_PROGRESS" )
		{
			showProgressInfo( "WiFi board firmware update in progress ..." );
			showProgressBar( "75%" );
		}
		else if ( jsonObj.data.State == "ESP_UPDATE_FAILED" )
		{
			showProgressInfo( "WiFi board firmware update failed!</br>Going into rollback ..." );
			showProgressBar( "80%" );
		}
		else if ( jsonObj.data.State == "ESP_VALIDATE_VERSION" )
		{
			showProgressInfo( "Validation of WiFi board firmware version ..." );
			showProgressBar( "80%" );
		}
		else if ( jsonObj.data.State == "MCU_CHECK_INTERFACE" )
		{
			showProgressInfo( "Interface check of MCU board ..." );
			showProgressBar( "85%" );
		}
		else if ( jsonObj.data.State == "MCU_INTERFACE_FAILED" )
		{
			showProgressInfo( "Interface check of MCU board failed!</br>Going into rollback ..." );
			showProgressBar( "90%" );
		}
		else if ( jsonObj.data.State == "MCU_CHECK_VERSION" )
		{
			showProgressInfo( "Check of MCU board firmware version ..." );
			showProgressBar( "90%" );
		}
		else if ( jsonObj.data.State == "MCU_UPDATE_IN_PROGRESS" )
		{
			showProgressInfo( "MCU board firmware update in progress ..." );
			showProgressBar( "95%" );
		}
		else if ( jsonObj.data.State == "MCU_UPDATE_FAILED" )
		{
			showProgressInfo( "MCU board firmware update failed!</br>Going into rollback ..." );
			showProgressBar( "100%" );
		}
		else if ( jsonObj.data.State == "MCU_VALIDATE_VERSION" )
		{
			showProgressInfo( "Validation of MCU board firmware version ..." );
			showProgressBar( "100%" );
		}
		else if ( jsonObj.data.State == "STARTUP_COMPLETED" )
		{
			firmwareUpdateSucceeded( "Startup successfully finished!" );
		}
		else if ( jsonObj.data.State == "STARTUP_FAILED" )
		{
			firmwareUpdateFailed( "Startup failed!</br>Possible reason: " + jsonObj.data.ErrorReason );
		}
		else if ( jsonObj.data.State == "UPDATE_COMPLETED" )
		{
			firmwareUpdateSucceeded( "Firmware update successfully finished!" );
		}
		else if ( jsonObj.data.State == "UPDATE_FAILED" )
		{
			showProgressInfo( "Firmware update failed!</br>Going into rollback ..." );
			showProgressBar( "100%" );
		}
		else if ( jsonObj.data.State == "TIMEOUT" )
		{
			firmwareUpdateFailed( "Firmware update failed with timeout!" );
		}
		else if ( jsonObj.data.State == "IDLE" )
		{
			firmwareUpdateSucceeded( "Firmware update successfully finished! (IDLE)" );
		}
		else
		{
			firmwareUpdateFailed( "Firmware update finished with unknown state! (" + jsonObj.data.State + ")"  );
		}
	}
}

/********************************************************************************/
