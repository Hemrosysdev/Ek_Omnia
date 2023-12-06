/********************************************************************************/

function startModal()
{
	// When the modal is shown, we want a fixed body
	const scrollY = document.documentElement.scrollTop;
	document.body.style.position = 'fixed';
	document.body.style.top = '-' + scrollY + 'px';
}

/********************************************************************************/

function stopModal()
{
	const scrollY = document.body.style.top;
	document.body.style.position = '';
	document.body.style.top = '';
	document.documentElement.scrollTop = -parseInt( scrollY );
}

/********************************************************************************/

function closeConfirmPopup()
{
	stopModal();
	document.getElementById( "confirm_popup" ).style.display = "none";
	$( "#confirm_popup_ok" ).unbind( "click" );
}

/********************************************************************************/

function openConfirmPopup( strCaption, strPrompt )
{
	startModal();

	var confirmPopupElement = document.getElementById( "confirm_popup" );
	var confirmPopupCaptionTextElement = document.getElementById( "confirm_popup_caption_text" );
	var confirmPopupContentElement = document.getElementById( "confirm_popup_content" );
	
	confirmPopupElement.style.display = "block";
	confirmPopupCaptionTextElement.innerHTML = strCaption;
	confirmPopupContentElement.innerHTML = strPrompt;

	$( "#confirm_popup_ok" ).unbind( "click" );
}

/********************************************************************************/

function disableConfirmPopupOk()
{
	confirmPopupOk.style.display = "none";
}

/********************************************************************************/

function enableConfirmPopupCancel()
{
	confirmPopupCancel.style.display = "block";
}

/********************************************************************************/

function disableConfirmPopupCancel()
{
	confirmPopupCancel.style.display = "none";
}

/********************************************************************************/

function enableConfirmPopupClose()
{
	confirmPopupClose.style.display = "block";
}

/********************************************************************************/

function disableConfirmPopupClose()
{
	confirmPopupClose.style.display = "none";
}

/********************************************************************************/

function closePopup()
{
	stopModal();
	popup.style.display = "none";
	abortUpload();
	window.clearInterval( stateInterval );
}

/********************************************************************************/

function enablePopupOk()
{
	popupOk.style.display = "block";
	popupCancel.style.display = "none";
	popupContent.style.height = "390px";
}

/********************************************************************************/

function disablePopupOk()
{
	popupOk.style.display = "none";
}

/********************************************************************************/

function enablePopupCancel()
{
	popupCancel.style.display = "block";
}

/********************************************************************************/

function disablePopupCancel()
{
	popupCancel.style.display = "none";
}

/********************************************************************************/

function enablePopupClose()
{
	popupClose.style.display = "block";
}

/********************************************************************************/

function disablePopupClose()
{
	popupClose.style.display = "none";
}

/********************************************************************************/

function openPopup( strCaption, bStartLoader )
{
	startModal();

	popupCaptionText.innerHTML = strCaption;
	popup.style.display = "block";
	preElement.innerHTML = "";
	logElement.innerHTML = "";
	
	if ( bStartLoader )
	{
		popupLoader.style.display = "block";
	}
	popupProgressInfo.style.display = "none";
	popupOk.style.display = "none";
	popupCancel.style.display = "none";
	popupContent.style.height = "490px";
}

/********************************************************************************/

function showNormalLog( result )
{
	preElement.innerHTML = "";
	logElement.innerHTML = result;
}

/********************************************************************************/

function showProgressInfo( info )
{
	popupUploadFile.innerHTML = info;
}

/********************************************************************************/

function showPreLog( result )
{
	preElement.innerHTML = "";
	logElement.innerHTML = "";
	var res = result.split("\\n");
	for (let i of res) 
	{
		$("#pre_id").append(i);
	}
}

/********************************************************************************/

function showProgressBar( value )
{
	popupProgressInfo.style.display="block";
	popupProgressBarProgress.style.width = value;
	popupProgressBarValue.innerHTML = value;
}

/********************************************************************************/

function hideProgressBar()
{
	popupProgressInfo.style.display="none";
}

/********************************************************************************/
