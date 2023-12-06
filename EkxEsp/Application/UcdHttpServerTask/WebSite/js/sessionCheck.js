/********************************************************************************/

function processSessionCheck()
{
	const url='/rest_api/check_session';
	$.ajax({
		url:url,
		type:"POST",
		success:
			function(result)
			{
				if ( result.trim() != "session_valid" )
				{
					window.location.href = "/login.php";
				}
			}
	});
}

/********************************************************************************/

window.addEventListener( 	"load", 
							function() 
							{
								setInterval( processSessionCheck, 15000 );
							},
							false );

/********************************************************************************/
