var pingErrorCnt = 0;
var aliveInterval = null;

/********************************************************************************/

function processAliveCheck()
{
	pingErrorCnt++;

	const url='ekx_rest_worker.php?ping';
	$.ajax({
		url:url,
		type:"GET",
		success:
			function(result)
			{
				if ( pingErrorCnt >= 3 )
				{
					closeConfirmPopup();
					location.reload();
				}
				pingErrorCnt = 0;
			}
	});

	if ( pingErrorCnt == 4 )
	{
		openConfirmPopup( 	"Connection watchdog", 
							"Connection to grinder lost!</br>The web interface tries to reconnect soon.</br></br>If connection cannot be established again,</br>please check your WiFi connection." );
		disableConfirmPopupCancel();
		disableConfirmPopupClose();
		disableConfirmPopupOk();
	}
}

/********************************************************************************/

function setAliveInterval( intervalMs )
{
	clearInterval( aliveInterval );
	aliveInterval = setInterval( processAliveCheck, intervalMs );
}

/********************************************************************************/

window.addEventListener( 	"load", 
							function() 
							{
								setAliveInterval( 5000 );
							},
							false );

/********************************************************************************/
