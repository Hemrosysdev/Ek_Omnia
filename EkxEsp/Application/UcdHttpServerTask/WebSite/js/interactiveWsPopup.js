var interactivePopupMsgId = 0;

/********************************************************************************/

document.addEventListener( 'jsonReceipt', (evt) => {
		if ( evt.detail.data.type == "GrinderActionResult" )
		{
			if ( evt.detail.data.refId == interactivePopupMsgId )
			{
				enablePopupOk();
				enableLoader( false );
				showPreLog( "Success: " + evt.detail.data.data.success + "<br>Done!" );
				interactivePopupMsgId = 0;
			}
		}
	} );

/********************************************************************************/

function processInteractiveWsJson( caption, json )
{
	openPopup( caption, true );
	
	interactivePopupMsgId = doSendJson( json );

	if ( interactivePopupMsgId == 0 )
	{
		enablePopupCancel();
		enableLoader( false );
		showPreLog( "Success: false<br>Done!" );
	}
}

/********************************************************************************/
