var websocket = null;
var firmwareFile = null;
var firmwareFileReader = new FileReader();
var fileContent = null;
var wsServerUri = null;
var firmwareUploadRunning = false;
var lastRequestChunkPos = 0;
var lastStalledRequestChunkPos = 0;
var errorCount = 0;
var stalledCounter = 0;
var debug = false;

/********************************************************************************/

firmwareFileReader.addEventListener('load', (event) => {
	fileContent = event.target.result;
	
	if ( websocket )
	{
		if ( !fileContent )
		{
			websocket.close();
			websocket = null;
		}
		else
		{
			//for ( var i = 0; i < 30; i++)
			//{
			//	console.log( parseInt(fileContent[i], 2).toString(16).toUpperCase() );
			//}
			//console.log( fileContent.slice( 0, 100 ) );
			sendStartFirmwareUpload();
		}
	}
});

/********************************************************************************/

function sendStartFirmwareUpload()
{
	json = {};
	json.UploadFile = {}
	json.UploadFile.Name = firmwareFile.name;
	json.UploadFile.Size = firmwareFile.size;
	
	doSendFirmwareJson( json, debug );
}

/********************************************************************************/

function initFirmwareWsClient( wsServerIp,
								idFilePos,
								idChunkSize,
								idProgress,
								idEffTransferRate,
								idRealTransferRate,
								idEtaSec,
								idSuccess,
								idReason )
{
	wsServerUri = "ws://" + wsServerIp + ":9988";
	
	docIdFilePos          = idFilePos;
	docIdChunkSize        = idChunkSize;
	docIdProgress         = idProgress;
	docIdEffTransferRate  = idEffTransferRate;
	docIdRealTransferRate = idRealTransferRate;
	docIdEtaSec           = idEtaSec;

	docIdSuccess          = idSuccess;
	docIdReason           = idReason;
}

/********************************************************************************/

function openWebSocket()
{
	if ( websocket )
	{
		websocket.close();
		websocket = null;
	}
	
	debugFirmware( "Try connect to " + wsServerUri );
	
	websocket = new WebSocket( wsServerUri );
	
	if ( websocket )
	{
		websocket.binaryType = "arraybuffer";
		websocket.onopen     = function(evt) { onFirmwareOpen(evt) };
		websocket.onclose    = function(evt) { onFirmwareClose(evt) };
		websocket.onmessage  = function(evt) { onFirmwareMessage(evt) };
		websocket.onerror    = function(evt) { onFirmwareError(evt) };
	}
	
	if ( !websocket )
	{
		onFirmwareError( new Event( "bla", { data : "Software upload server not available" } ) );
	}
}

/********************************************************************************/

function retryStalledUpload()
{
	debugFirmware( "retryStalledUpload" );
	openWebSocket();
}

/********************************************************************************/

function onFirmwareOpen(evt)
{
	debugFirmware( "CONNECTED" );

	if ( websocket && fileContent )
	{
		sendStartFirmwareUpload();
	}
}

/********************************************************************************/

function onFirmwareClose(evt)
{
	debugFirmware( "DISCONNECTED" );

	websocket = null;
}

/********************************************************************************/

function onFirmwareMessage(evt)
{
	var json = JSON.parse( evt.data );

	if ( json.RequestChunk != null )
	{
		document.getElementById( docIdFilePos ).value          = json.RequestChunk.Pos + json.RequestChunk.Size;
		document.getElementById( docIdChunkSize ).value        = json.RequestChunk.Size;
		document.getElementById( docIdProgress ).value         = json.RequestChunk.Progress;
		document.getElementById( docIdEffTransferRate ).value  = json.RequestChunk.EffTransferRate;
		document.getElementById( docIdRealTransferRate ).value = json.RequestChunk.RealTransferRate;
		
		lastRequestChunkPos = json.RequestChunk.Pos;
		
		debugFirmware( 'RequestChunk: ' + json.RequestChunk.Pos );

		if ( parseInt( json.RequestChunk.EtaSec ) > 0 )
		{
			document.getElementById( docIdEtaSec ).value = Math.floor( json.RequestChunk.EtaSec / 60 ) + "min" + json.RequestChunk.EtaSec % 60 + "sec";
		}
		else
		{
			document.getElementById( docIdEtaSec ).value = json.RequestChunk.EtaSec;
		}

		document.dispatchEvent( new CustomEvent( "updateFirmwareUpload", { detail : { progress : json.RequestChunk.Progress, eta : json.RequestChunk.EtaSec } } ) );

		if ( json.RequestChunk.Pos >= firmwareFile.size )
		{
			jsonResponse = {};
			jsonResponse.UploadStatus = {};
			jsonResponse.UploadStatus.Error = true;
			jsonResponse.UploadStatus.Reason = "reaching eof";
			
			doSendFirmwareJson( jsonResponse, debug );
		}
		else
		{
			var chunkSize = Math.min( firmwareFile.size - json.RequestChunk.Pos, json.RequestChunk.Size );

			var chunk = new Uint8Array( ( Math.floor( chunkSize / 4 ) + 2 ) * 4 );

			var posBuffer = new ArrayBuffer( 4 );
			var posView32 = new Int32Array( posBuffer );
			posView32[0] = json.RequestChunk.Pos;

			var posView8 = new Uint8Array( posBuffer );
			chunk.set( posView8, 0, 4 );

			var fileView = new Uint8Array( fileContent );
			chunk.set( fileView.slice( json.RequestChunk.Pos, json.RequestChunk.Pos + chunkSize ), 4 );

			doSendFirmwareChunk( chunk.slice( 0, chunkSize + 4 ), debug );
		}
	}
	else
	{ 
		debugFirmware( 'MESSAGE RECEIVED:\n' + evt.data );

		if ( json.StopDownload != null )
		{
			if ( json.StopDownload.Success )
			{
				document.dispatchEvent( new CustomEvent( "finishFirmwareUpload" ) );
			}
			else
			{
				document.dispatchEvent( new CustomEvent( "errorFirmwareUpload", { detail : { reason : json.StopDownload.Reason } } ) );
			}
			
			firmwareUploadRunning = false;

			document.getElementById( docIdSuccess ).value  = json.StopDownload.Success;
			document.getElementById( docIdReason ).value   = json.StopDownload.Reason;
			fileContent = null;

			if ( websocket )
			{
				websocket.close();
				websocket = null;
			}
		}
		else if ( json.Statistic != null )
		{
		}
	}
}

/********************************************************************************/

function onFirmwareError(evt)
{
	debugFirmware( 'ERROR: ' + evt.data );
	if ( websocket )
	{
		websocket.close();
		websocket = null;
	}
	
	errorCount++;

	if ( errorCount > 5 )
	{
		firmwareUploadRunning = false;
		document.dispatchEvent( new CustomEvent( "errorFirmwareUpload", { detail : { reason : "Server not available, stalled progress or " + evt.data } } ) );
	}
}

/********************************************************************************/

function doSendFirmwareMsg( message, debug )
{
	if ( websocket )
	{
		if ( debug )
		{
			console.log( "SEND: " + message );
		}
		websocket.send( message );
	}
}

/********************************************************************************/

function doSendFirmwareChunk( chunk, debug )
{
	if ( websocket )
	{
		if ( debug )
		{
			console.log( "SEND: chunk len " + chunk.length );
		}
		websocket.send( chunk );
	}
}

/********************************************************************************/

function abortUpload()
{
	if ( websocket )
	{
		debugFirmware( "ABORT UPLOAD" );
		json = {};
		json.AbortUpload = true;

		doSendFirmwareJson( json, debug );
		
		firmwareUploadRunning = false;
	}
}

/********************************************************************************/

function debugFirmware( msg )
{
	if ( debug )
	{
		console.log( msg );
	}
}

/********************************************************************************/

function doSendFirmwareJson( json, debug )
{
	doSendFirmwareMsg( JSON.stringify( json ), debug );//, undefined, 4 ) );
}

/********************************************************************************/

function checkStalledState()
{
	if ( firmwareUploadRunning )
	{
		if ( lastStalledRequestChunkPos == lastRequestChunkPos )
		{
			stalledCounter++;
		}
		else
		{
			stalledCounter = 0;
		}
		
		lastStalledRequestChunkPos = lastRequestChunkPos;
		setTimeout( () => checkStalledState(), 1000 );
		
		if ( stalledCounter > 5 )
		{
			retryStalledUpload();
			stalledCounter = 0;
		}
	}
}

/********************************************************************************/

function startFirmwareUpload( selectedFirmwareFile )
{
	errorCount = 0;
	firmwareFile = selectedFirmwareFile;
	fileContent = null;
	firmwareUploadRunning = true;
	lastRequestChunkPos = 0;
	stalledCounter = 0;
	
	document.getElementById( docIdFilePos ).value           = "";
	document.getElementById( docIdChunkSize ).value         = "";
	document.getElementById( docIdProgress ).value          = "0%";
	document.getElementById( docIdEffTransferRate ).value   = "";
	document.getElementById( docIdRealTransferRate ).value  = "";
	document.getElementById( docIdEtaSec ).value            = "";

	document.getElementById( docIdSuccess ).value   = "";
	document.getElementById( docIdReason ).value    = "";

	document.dispatchEvent( new CustomEvent( "startFirmwareUpload", { detail : { file: firmwareFile.name } } ) );

	firmwareFileReader.readAsArrayBuffer( firmwareFile );

	openWebSocket();
	
	setTimeout( () => checkStalledState(), 1000 );
}

/********************************************************************************/


/********************************************************************************/
