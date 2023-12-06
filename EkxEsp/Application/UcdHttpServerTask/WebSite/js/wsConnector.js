var logConsole = false;
var websocket = null;
var wsServerUri = "";
var pingTrials = 10;
var wsServerConnected = false;
var aliveState = false;
var msgId = 0;

/********************************************************************************/
window.addEventListener( "load", () => { setInterval( sendPing, 500 ) } );
window.addEventListener( "load", () => { setInterval( tryWsServerReconnect, 5000 ) } );
/********************************************************************************/

/********************************************************************************/

function pollDriverState()
{
	setInterval( requestDriverState, 500 );
}

/********************************************************************************/

function tryWsServerReconnect()
{
	if ( websocket && wsServerConnected )
	{
		// do nothing
	}
	else if ( wsServerUri != "" )
	{
		openWebSocket( wsServerUri );
	}
}

/********************************************************************************/

function sendPing()
{
	pingTrials++;

	if ( websocket == null )
	{
	}
	else
	{
		var json = 
		{
			ping : true
		};
		doSendJson( json );
	}

	validateAlive();
}

/********************************************************************************/

function requestDriverState()
{
	if ( aliveState )
	{
		var json = 
		{
			requestType : "DriverState"
		};
		doSendJson( json );
	}
}

/********************************************************************************/

function requestDriverConfig()
{
	if ( aliveState )
	{
		var json = 
		{
			requestType : "DriverConfig"
		};
		doSendJson( json );
	}
}

/********************************************************************************/

function requestAdcConfig()
{
	if ( aliveState )
	{
		var json = 
		{
			requestType : "AdcConfig"
		};
		doSendJson( json );
	}
}

/********************************************************************************/

function validateAlive()
{
	if ( ( ( pingTrials < 8 ) && wsServerConnected ) != aliveState )
	{
		aliveState = ( ( pingTrials < 8 ) && wsServerConnected );

		writeToScreen( "Alive state: " + aliveState );
		
		if ( aliveState )
		{
			document.dispatchEvent( new CustomEvent( "wsServerConnected" ) );
		}
		else
		{
			document.dispatchEvent( new CustomEvent( "wsServerDisconnected" ) );
		}
	}
}

/********************************************************************************/

function initWsServer( uri, log )
{
	logConsole = log;
	wsServerUri = uri;

	writeToScreen( "Init: " + uri );
	
	tryWsServerReconnect();
}

/********************************************************************************/

function openWebSocket( uri )
{
	if ( websocket )
	{
		websocket.close();
		websocket = null;
	}

	writeToScreen( "Try connect to " + uri );
	websocket           = new WebSocket( uri );
	websocket.onopen    = function(evt) { onOpen(evt) };
	websocket.onclose   = function(evt) { onClose(evt) };
	websocket.onmessage = function(evt) { onMessage(evt) };
	websocket.onerror   = function(evt) { onError(evt) };
	
	sendPing();
}

/********************************************************************************/

function onOpen(evt)
{
	writeToScreen( "Event: CONNECTED" );
	wsServerConnected = true;
	validateAlive();
}

/********************************************************************************/

function onClose(evt)
{
	writeToScreen( "Event: DISCONNECTED" );
	wsServerConnected = false;
	validateAlive();
}

/********************************************************************************/

function onMessage(evt)
{
	writeToScreen( 'Event: MESSAGE RECEIVED: ' + evt.data );
	
	if ( evt.data.length < 2 )
	{
		writeToScreen( "Event data must be invalid" );
	}
	else
	{
		var json = JSON.parse( evt.data );

		if ( json == null )
		{
			writeToScreen( "Event data is no JSON object" );
		}
		else if ( json.pong != null )
		{
			pingTrials = 0;
			validateAlive();
		}
		else
		{
			document.dispatchEvent( new CustomEvent( "jsonReceipt", { detail : { data : json } } ) );
		}
	}
}

/********************************************************************************/

function onError(evt)
{
	writeToScreen( 'Event: ERROR: ' + evt.data );
}

/********************************************************************************/

function doSend( message )
{
	if ( wsServerConnected )
	{
		writeToScreen( "SEND: " + message );
		websocket.send(message);
	}
}

/********************************************************************************/

function doSendJson( json )
{
	json.msgId = 0;
	
	if ( wsServerConnected )
	{
		json.msgId = ++msgId;
		doSend( JSON.stringify( json, undefined, 4 ) );
	}
	
	return json.msgId;
}

/********************************************************************************/

function writeToScreen( message )
{
	if ( logConsole )
	{
		console.log( message );
	}
}

/********************************************************************************/
