<?php
	session_start();

	$error = true;
	$message = "<font color='red'>Upload failed!<br/>Illegal command call!</font>";
	$target_file = "";

	if ( !empty( $_FILES ) )
	{
		$tmpName    = $_FILES['splashScreenFile']['tmp_name'];
		$fileName   = $_FILES['splashScreenFile']['name'];
		$fileSize   = $_FILES['splashScreenFile']['size'];
		$fileType   = $_FILES['splashScreenFile']['type'];
		$targetFile = "/config/custom-splash-screen";
		
		if ( $_FILES['splashScreenFile']['error'] != UPLOAD_ERR_OK )
		{
			$message = "<font color='red'>Error while uploading splash screen file:<br/>";

			switch ( $_FILES['splashScreenFile']['error'] )
			{
				case UPLOAD_ERR_OK:
					$message .= "Shouldn't happen!";
				break;
				case UPLOAD_ERR_INI_SIZE:
				case UPLOAD_ERR_FORM_SIZE:
					$message .= "File size too big!";
				break;
				case UPLOAD_ERR_PARTIAL:
					$message .= "File upload only partially!";
				break;
				case UPLOAD_ERR_NO_FILE:
					$message .= "No file uploaded!";
				break;
				case UPLOAD_ERR_NO_TMP_DIR:
					$message .= "Temporary folder missing!";
				break;
				case UPLOAD_ERR_CANT_WRITE:
					$message .= "Error while saving file!";
				break;
				case UPLOAD_ERR_EXTENSION:
					$message .= "PHP extension stops upload!";
				break;
			}
			$message .= "</font>";
		}
		else 
		{
			$output = null;
			exec( "sudo rm -f $targetFile", $output, $returnCode );
			
			$imageInfo = getimagesize( $tmpName );
			
			if ( !is_array( $imageInfo ) )
			{
				$message = "<font color='red'>Upload failed!<br/>File is no image!</font>";
			}
			else if ( $imageInfo[0] < 0 || $imageInfo[0] > 480
			          || $imageInfo[1] < 0 || $imageInfo[1] > 360 )
			{
				$message = "<font color='red'>Upload failed!<br/>Image size ($imageInfo[0]x$imageInfo[1]px) not compatible (max. 480x360px)!</font>";
			}
			else
			{
				$output = null;
				exec( "/usr/bin/fbv $tmpName", $output, $returnCode );
				
				if ( $returnCode == 0 )
				{
					$message = "<font color='red'>Upload failed!<br/>File could not be used by fbv application!</br>";
					$message .= "</font>";
				}
				else if ( !move_uploaded_file( $tmpName, $targetFile ) )
				{
					$message = "<font color='red'>Upload failed!<br/>Cannot move temporary file!</font>";
				}
				else
				{
					$message = "Upload successful!";
					$error = false;
				}
			}
			exec( "sudo /etc/init.d/S01splashscreen restart", $output, $returnCode );
		}
	}
	else
	{
		//exec( "rm -f /storage/*.swu" );
	}

	$json                 = null;
	$json['message']      = $message;
	$json['error']        = $error;
	$json['upload_done']  = true;
	$json['file']         = $target_file;

	echo json_encode( $json, JSON_PRETTY_PRINT );
?>
@
