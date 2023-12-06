<?php
session_start();
?>
<?php
	$message = '';
	$log = "";
	$state_file_json = "/storage/SoftwareUpdateState.json";
	$rdev = "";
	
	extract( $_REQUEST, EXTR_PREFIX_ALL | EXTR_REFS, 'imp' );

	$hmi_time = ( new DateTime() )->format( "d.m.Y H:i:s" );

	if ( isset( $imp_dev ) )
	{
		$_SESSION["dev"] = "dev";
	}
	else if ( isset( $imp_nodev ) )
	{
		unset( $_SESSION["dev"] );
	}

	if ( isset( $imp_mcu ) )
	{
		$_SESSION["mcu"] = "mcu";
	}
	else if ( isset( $imp_nomcu ) )
	{
		unset( $_SESSION["mcu"] );
	}

	$serialNo = file_get_contents( "/proc/device-tree/hde/sernr" );
	$serialNo = str_replace( ":", "", $serialNo );
	$serialNo = str_replace( "\0", "", $serialNo );

	// request active rfs
	$output = null;
	exec( "rdev", $output );
	$rdev = implode( ";", $output );

	$output = null;
	exec( "df | grep /usr/local | cut -f 1 -d \\ ", $output );
	$appimage = implode( "\n", $output );
	$output = null;
	
	$file = "";
	if ( file_exists( $state_file_json ) )
	{
		exec( "sudo chmod a+w $state_file_json" );
		$file = file_get_contents( $state_file_json );
	}
	$json = json_decode( $file, true );

	$allow_update = true;
	
	if ( !empty( $json ) )
	{
		if ( $json["State"] != "IDLE" )
		{
			$allow_update = false;
			$message = "<font color='orange'>Update in progress!</font>";
		}
	}

	if ( isset( $imp_downloadAgsaLdtLogs ) )
	{
		$files = glob( "/storage/HemroEkxAgsaLdtLogs/*.zip" );
		
		$popupCaption = "AGSA test logs download failed";
		if ( count( $files ) == 0 )
		{
			$popupMessage = "No log files found!";
		}
		else
		{
			$popupMessage  = "Please execute following commands in console to download all log files:</br>";
			$popupMessage .= "Maybe replace correct IP address if using other networks than grinder access point!</br>";
			$popupMessage .= "For Windows replace wget by wget.exe (just for Flo)!</br></br>";
			
			foreach ( $files as &$file ) 
			{
				$file = substr( $file, 29 );
				$popupMessage .= "wget --content-disposition http://192.168.4.1/ekx_rest_worker.php?download_agsa_log=$file</br>";
			}
		}
	}

	else if ( isset( $imp_downloadDatabase ) )
	{
		$dbFile = "/db_storage/EkxSqlite.db";
		$targetDbFile = "EkxSqlite_$serialNo.db";

		exec( "sudo chmod a+r $dbFile" );

		if ( !file_exists( $dbFile ) ) 
		{
			$popupCaption = "DB file download failed";
			$popupMessage = "No DB file existing!";
		}
		else
		{
			header( 'Content-Description: File Transfer' );
			header( 'Content-Type: application/octet-stream' );
			header( 'Content-Disposition: attachment; filename="' . $targetDbFile . '"' );
			header( 'Expires: 0' );
			header( 'Cache-Control: must-revalidate' );
			header( 'Pragma: public' );
			header( 'Content-Length: ' . filesize( $dbFile ) );
			readfile( $dbFile );
			exit;
		}
	}
	
	$displaySplashScreen = "none";
	if ( file_exists( "/config/custom-splash-screen" ) )
	{
		$displaySplashScreen = "block";
	}
?>
<!DOCTYPE html>

<html lang="en">
	<head>
		<meta charset="utf-8">
		
		<title>Hemro EKX Web Interface</title>

		<link rel="shortcut icon" type="image/x-icon" href="favicon.ico" />
		<link rel="stylesheet" type="text/css" href="style.css?ver=2" />
		
		<script src="zepto.min.js?ver=ekx"></script>
		<script src="firmwareWsClient.js?ver=ekx"></script>
		<script src="aliveCheck.js?ver=ekx"></script>
		<script src="popupManagement.js?ver=ekx"></script>

		<script type="text/javascript">

			var lastPercent = -10;
			var firmwareUpdateFile = null;
			var stateInterval = null;

			function ekxRestWorkerCmd( strCaption, strCmd )
			{
				openPopup( strCaption, true );
				
				const url='ekx_rest_worker.php?' + strCmd;
				$.ajax({
					url:url,
					type:"GET",
					success:
						function(result)
						{
							showPreLog(result);
						},
					complete: 
						function(result)
						{
							popupLoader.style.display = "none";
							enablePopupOk();
						}
				})
			}
			
			function processUploadResonse( json )
			{
				json = json.substring( 0, json.length - 2 );

				var obj = JSON.parse( json );

				var error = true;
				if ( obj != null )
				{
					logElement.innerHTML = obj.message;
					error                = obj.error;
				}
				
				return error;
			}

			function handleSplashScreenUpload()
 			{
 				openPopup( "Custom Splash-Screen upload", false );
				
				const uploadUrl='upload_splash_screen.php';
				var file = $('#splash_file_select').get(0).files[0];
				var dataForm = new FormData();
				dataForm.append( 'splashScreenFile', file );

				$.ajax({
					url:          uploadUrl,
					type:        'POST',
					dataType:    'binary',
					header:      { 'Transfer-Encoding' : 'chunked' },
					data:        dataForm,
					processData: false,
					contentType: false, //'application/json', //x-www-form-urlencoded',
					responseType: 'json',
					xhr:
						function()
						{
							var xhr = $.ajaxSettings.xhr();
							xhr.upload.onload = function()
												{
													logElement.innerHTML = 'Upload successfully done!';
												};
							return xhr;
						},
					beforeSend:
						function()
						{
 							logElement.innerHTML = "Start file upload ...<br>";
						},
					success:
						function(result)
						{
							popupProgressInfo.style.display = "none";
							//console.log(result.responseText);
						},
					error:
						function(result)
						{
 							popupProgressInfo.style.display = "none";
							//console.log(result.responseText);
						},
					complete:
						function(result)
						{
							popupProgressInfo.style.display = "none";
							if ( processUploadResonse( result.responseText ) )
							{
								document.getElementById( "show_splash_screen" ).style.display = "none";
							}
							enablePopupOk();
							var timestamp = new Date().getTime();
							document.getElementById( "image_splash_screen" ).src = "custom-splash-screen?t=" + timestamp; 
						}
				});
			}

			function handleFirmwareUpload()
			{
				openPopup( "Firmware Update", false );

				document.addEventListener( 'startFirmwareUpload', (evt) => { 
							showNormalLog( "Start file upload ...<br>" );
							showProgressBar( "0%" );
							enablePopupCancel();
							showProgressInfo( "Upload new firmware file</br>" + evt.detail.file );
							//sendRequestProgress( 0 );
					} );
				document.addEventListener( 'updateFirmwareUpload', (evt) => { 
							showNormalLog( "" );
							showProgressBar( evt.detail.progress );
							popupEta.innerHTML = "Time to go: " +  Math.floor( evt.detail.eta / 60 ) + "min " + evt.detail.eta % 60 + "sec";
							//sendRequestProgress( parseInt( evt.detail.progress ) );
					} );
				document.addEventListener( 'errorFirmwareUpload', (evt) => { 
							popupEta.innerHTML = "";
							disablePopupCancel();
							hideProgressBar();
							showNormalLog( "Upload has been aborted by error: " + evt.detail.reason );
							enablePopupOk();
							//sendRequestAborted();
					} );
				document.addEventListener( 'finishFirmwareUpload', (evt) => { 
							document.removeEventListener( 'errorFirmwareUpload', this.handleFileEmail, false);
							showProgressBar( "0%" );
							popupEta.innerHTML = "";
							disablePopupCancel();
							showProgressInfo( "Upload successfully completed ..." );
							stateInterval = window.setInterval( swUpdateObserver, 500 );
					} );

				setAliveInterval( 60000 );
				startFirmwareUpload( firmwareUpdateFile );
			}

			function swUpdateObserver()
			{
				const url='ekx_rest_worker.php?get_sw_update_state';
				$.ajax({
					url:url,
					type:"GET",
					success:
						function(result)
						{
							parseSwUpdateState( result );
						},
					complete: 
						function(result)
						{
						}
				})
			}

			function firmwareUpdateFailed( info )
			{
				setAliveInterval( 5000 );
				window.clearInterval(stateInterval);
				showNormalLog( "<font color='red'>" + info + "</font>" );
				hideProgressBar();
				enablePopupOk();
			}
			
			function firmwareUpdateSucceeded( info )
			{
				setAliveInterval( 5000 );
				window.clearInterval(stateInterval);
				showNormalLog( "<font color='green'>" + info + "</font>" );
				hideProgressBar();
				enablePopupOk();
			}

			function parseSwUpdateState( jsonResult )
			{
				var jsonObj = null;
				
				try
				{
					jsonObj = JSON.parse( jsonResult );
				}
				catch ( e )
				{
					firmwareUpdateFailed( "Internal communication to grinder failed!</br>Please observe grinder display for current state." );
					return;
				}

				if ( jsonObj["State"] == "UPLOAD_COMPLETED" )
				{
					window.clearInterval( stateInterval );
					const url='ekx_rest_worker.php?start_fw_update=' + firmwareUpdateFile.name;
					$.ajax({
						url:url,
						type:"GET",
						success:
							function(result)
							{
							},
						complete: 
							function(result)
							{
								stateInterval = window.setInterval( swUpdateObserver, 500 );
							}
					})
				}
				else if ( jsonObj["State"] == "UPLOAD_FAILED" )
				{
					firmwareUpdateFailed( "Upload of .swu file failed!</br>Possibe reason: " + jsonObj["ErrorReason"] + "</br>Please restart the grinder and retry." );
				}
				else if ( jsonObj["State"] == "UPLOAD_ABORTED" )
				{
					firmwareUpdateFailed( "Upload of file aborted by user!" );
				}
				else if ( jsonObj["State"] == "EVALUATE_PACKAGE" )
				{
					showProgressInfo( "Evaluate upload package ..." );
					showProgressBar( "5%" );
				}
				else if ( jsonObj["State"] == "PACKAGE_FAILED" )
				{
					firmwareUpdateFailed( "Uploaded package check failed!</br>Maybe the uploaded file is corrupted or is no valid .swu file.</br>Please restart the grinder and retry with a valid .swu file." );
				}
				else if ( jsonObj["State"] == "PACKAGE_NO_UPGRADE" )
				{
					firmwareUpdateFailed( "Uploaded package is no upgrade related to the current firmware version!</br>Please retry with a valid upgrade .swu file." );
				}
				else if ( jsonObj["State"] == "UNPACK_ROOTFS" )
				{
					showProgressInfo( "Unpacking image #1 of 2 ..." );
					showProgressBar( "20%" );
				}
				else if ( jsonObj["State"] == "UNPACK_ROOTFS_FAILED" )
				{
					firmwareUpdateFailed( "Validation of image #1 failed!</br>Please retry, if still failed please contact technical support." );
				}
				else if ( jsonObj["State"] == "UNPACK_APPLICATION" )
				{
					showProgressInfo( "Unpacking image #2 of 2 ..." );
					showProgressBar( "40%" );
				}
				else if ( jsonObj["State"] == "UNPACK_APPLICATION_FAILED" )
				{
					firmwareUpdateFailed( "Validation of image #2 failed!</br>Please retry, if still failed please contact technical support." );
				}
				else if ( jsonObj["State"] == "REBOOT" )
				{
					showProgressInfo( "Waiting for restart of system ..." );
					showProgressBar( "50%" );
				}
				else if ( jsonObj["State"] == "STARTUP_CHECK" )
				{
					showProgressInfo( "Startup check ..." );
					showProgressBar( "55%" );
				}
				else if ( jsonObj["State"] == "CHECK_ROOTFS" )
				{
					showProgressInfo( "Checking image #1 consistency ..." );
					showProgressBar( "60%" );
				}
				else if ( jsonObj["State"] == "ROOTFS_FAILED" )
				{
					showProgressInfo( "Consistency check of image #1 failed!</br>Going into rollback ..." );
					showProgressBar( "65%" );
				}
				else if ( jsonObj["State"] == "ESP_CHECK_INTERFACE" )
				{
					showProgressInfo( "Interface check of WiFi board ..." );
					showProgressBar( "65%" );
				}
				else if ( jsonObj["State"] == "ESP_INTERFACE_FAILED" )
				{
					showProgressInfo( "Interface check of WiFi board failed!</br>Going into rollback ..." );
					showProgressBar( "70%" );
				}
				else if ( jsonObj["State"] == "ESP_CHECK_VERSION" )
				{
					showProgressInfo( "Check of WiFi board firmware version ..." );
					showProgressBar( "70%" );
				}
				else if ( jsonObj["State"] == "ESP_NOT_AVAILABLE" )
				{
					showProgressInfo( "WiFi board not available!</br>Going into rollback ..." );
					showProgressBar( "75%" );
				}
				else if ( jsonObj["State"] == "ESP_UPDATE_IN_PROGRESS" )
				{
					showProgressInfo( "WiFi board firmware update in progress ..." );
					showProgressBar( "75%" );
				}
				else if ( jsonObj["State"] == "ESP_UPDATE_FAILED" )
				{
					showProgressInfo( "WiFi board firmware update failed!</br>Going into rollback ..." );
					showProgressBar( "80%" );
				}
				else if ( jsonObj["State"] == "ESP_VALIDATE_VERSION" )
				{
					showProgressInfo( "Validation of WiFi board firmware version ..." );
					showProgressBar( "80%" );
				}
				else if ( jsonObj["State"] == "MCU_CHECK_INTERFACE" )
				{
					showProgressInfo( "Interface check of MCU board ..." );
					showProgressBar( "85%" );
				}
				else if ( jsonObj["State"] == "MCU_INTERFACE_FAILED" )
				{
					showProgressInfo( "Interface check of MCU board failed!</br>Going into rollback ..." );
					showProgressBar( "90%" );
				}
				else if ( jsonObj["State"] == "MCU_CHECK_VERSION" )
				{
					showProgressInfo( "Check of MCU board firmware version ..." );
					showProgressBar( "90%" );
				}
				else if ( jsonObj["State"] == "MCU_UPDATE_IN_PROGRESS" )
				{
					showProgressInfo( "MCU board firmware update in progress ..." );
					showProgressBar( "95%" );
				}
				else if ( jsonObj["State"] == "MCU_UPDATE_FAILED" )
				{
					showProgressInfo( "MCU board firmware update failed!</br>Going into rollback ..." );
					showProgressBar( "100%" );
				}
				else if ( jsonObj["State"] == "MCU_VALIDATE_VERSION" )
				{
					showProgressInfo( "Validation of MCU board firmware version ..." );
					showProgressBar( "100%" );
				}
				else if ( jsonObj["State"] == "STARTUP_COMPLETED" )
				{
					firmwareUpdateSucceeded( "Startup successfully finished!" );
				}
				else if ( jsonObj["State"] == "STARTUP_FAILED" )
				{
					firmwareUpdateFailed( "Startup failed!</br>Possible reason: " + jsonObj["ErrorReason"] );
				}
				else if ( jsonObj["State"] == "UPDATE_COMPLETED" )
				{
					firmwareUpdateSucceeded( "Firmware update successfully finished!" );
				}
				else if ( jsonObj["State"] == "UPDATE_FAILED" )
				{
					showProgressInfo( "Firmware update failed!</br>Going into rollback ..." );
					showProgressBar( "100%" );
				}
				else if ( jsonObj["State"] == "TIMEOUT" )
				{
					firmwareUpdateFailed( "Firmware update failed with timeout!" );
				}
				else if ( jsonObj["State"] == "IDLE" )
				{
					firmwareUpdateSucceeded( "Firmware update successfully finished! (IDLE)" );
				}
				else
				{
					firmwareUpdateFailed( "Firmware update finished with unknown state! (" + jsonObj["State"] + ")"  );
				}
			}
			
			
			window.addEventListener( 	"load", 
										function() {
														initFirmwareWsClient(	location.host,
																				"FilePos",
																				"ChunkSize",
																				"Progress",
																				"EffTransferRate",
																				"RealTransferRate",
																				"EtaSec",
																				"Success",
																				"Reason" ); 
										},
										false );

		</script>
	</head>

	<body>

		<div style="width:100%;padding:0;">
			<div id="headerBackground" style="width:100%;height:240px;margin:auto;color:black;background-color:white;position:relative;">
				<div id="imageCenterBar" style="position:relative;top:50%;transform:translateY(-50%);">
					<img id="mahlkoenig_logo" alt="Mahlk&ouml;nig Logo" 
						style="position:relative;height:120px;left:50%;transform:translateX(-50%);" 
						src="mahlkoenig_logo_210x120.png" />
				</div>
			</div>
			
			<div id="grinderBackground" style="width:100%;height:960px;color:white;background-color:black;padding:0;">
				<table style="margin:auto;position:relative;top:50%;transform:translateY(-50%);">
					<tr>
						<td>
							<img alt="EKX Grinder" style="height:800px;" src="ekx.jpg" />
						</td>
						<td style="text-align:left;padding:100px;">
							<p>
								<div style="font-size:50px;font-weight: bold;">
									EK Omnia
								</div>
							</p>
							<p>
								<div style="font-size:20px;font-weight: 500;text-transform: uppercase;">
									SN
								</div>
								<div style="font-size:25px;font-weight: bold;">
									<?= $serialNo?>
								</div>
							</p>

							<p>&nbsp;</p>
							<p>
								
							</p>
						</td>
					</tr>
				</table>
			</div>
			
			<noscript>
				<div style="color: red;font-size: 20px;">
					Please activate Javascript to use EKX Web Interface!
				</div>
			</noscript>
		

			<div id="infoBackground" style="width:100%;color:black;background-color:#f7f7f7;margin:auto;float:left;">
				
				<p>&nbsp;</p>

				<div class="infoRegion" >
					<table class="infoTable">
						<tr>
							<td class="tableDescriptorBold" width="308px">System statistics</td>
							<td width="240">
								<button class="redbutton" id="Configure_statistics">Configure</button>
								<script>
									$( "#Configure_statistics" ).on( "click", function()
																	{
																		window.open( "statistics.php", "_self" );
																	});
								</script>
							</td>
							<td width="160">
							</td>
						</tr>

						<tr>
							<td class="tableDescriptorBold">
								Database
							</td>
							<td>
								<form method="POST" action="<?=$_SERVER['PHP_SELF']?>">
									<button class="redbutton" name="downloadDatabase" id="downloadDatabase">Download</button>
								</form>
							</td>
							<td>
							</td>
						</tr>

				<?php
					//if ( isset( $_SESSION["mcu"] ) ) 
					{ 
				?>
						<tr>
							<td class="tableDescriptorBold">Start MCU update</td>
							<td>
								<button class="redbutton" id="setMcuUpdateState">Start</button>
								<script>
									$( "#setMcuUpdateState" ).on( "click", function()
																		{
																			ekxRestWorkerCmd( "Start MCU Update", "set_mcu_update_state" );
																		});
								</script>
							</td>
							<td>
								&nbsp;
							</td>
						</tr>
				<?php 
					} 
				?>
				
						<tr>
							<td class="tableDescriptorBold">Start ESP update</td>
							<td>
								<button class="redbutton" id="setEspUpdateState">Start</button>
								<script>
									$( "#setEspUpdateState" ).on( "click", function()
																		{
																			ekxRestWorkerCmd( "Start ESP Update", "set_esp_update_state" );
																		});
								</script>
							</td>
							<td>
								&nbsp;
							</td>
						</tr>
					</table>
				</div>
				
				<p>&nbsp;</p>

				<?php if ( isset( $_SESSION["dev"] ) ) { ?>
				<div class="infoRegion" >
					<table class="infoTable">
						<tr>
							<td class="tableDescriptorBold" width="308px">Development</td>
							<td width="240">&nbsp;</td>
							<td width="160">&nbsp;</td>
						</tr>

						<tr>
							<td class="tableDescriptorBold">
								HMI AGSA test logs</br>
								<?php
									$output = null;
									exec( "du -h /log/HemroEkxAgsaLdtLogs | awk '{print $1}'", $output, $returnCode );
									echo "($output[0] used, $freeLogSpace[0] free)";
								?>
							</td>
							<td>
								<form method="POST" action="<?=$_SERVER['PHP_SELF']?>">
									<button class="redbutton" name="downloadAgsaLdtLogs" id="downloadAgsaLdtLogs">Download</button>
								</form>
							</td>
							<td>
								<button class="blackbutton" id="clearAgsaLogs">Clear</button>
								<script>
									$( "#clearAgsaLogs" ).on( "click", function()
																		{
																			ekxRestWorkerCmd( "Clear AGSA test logs", "clear_agsa_logs" );
																		});
								</script>
							</td>
						</tr>

						<tr>
							<td class="tableDescriptorBold">HMI reboot</td>
							<td>
								<button class="redbutton" id="doRebootBtn">Reboot</button>
								<script>
									$( "#doRebootBtn" ).on( "click", function()
																		{
																			ekxRestWorkerCmd( "HMI Reboot", "reboot" );
																		});
								</script>
							</td>
							<td>
							</td>
						</tr>

						<tr>
							<td class="tableDescriptorBold">Start WS Server</td>
							<td>
								<button class="redbutton" id="startWsServer">Start</button>
								<script>
									$( "#startWsServer" ).on( "click", function()
																		{
																			ekxRestWorkerCmd( "Start WS Server", "start_ws_server" );
																		});
								</script>
							</td>
							<td>
							</td>
						</tr>

						<tr>
							<td class="tableDescriptorBold">Start/Stop applications</td>
							<td>
								<button class="redbutton" id="stopall">Stop all</button>
								<script>
									$( "#stopall" ).on( "click", function()
																		{
																			ekxRestWorkerCmd( "Start/Stop Applications", "stopall" );
																		});
								</script>
							</td>
							<td>
								<button class="blackbutton" id="startall">Start all</button>
								<script>
									$( "#startall" ).on( "click", function()
																		{
																			ekxRestWorkerCmd( "Start/Stop Applications", "startall" );
																		});
								</script>
							</td>
						</tr>

						<tr>
							<td class="tableDescriptorBold">SW update state</td>
							<td>
								<button class="redbutton" id="setSwUpdateState">Set state</button>
								<script>
									$( "#setSwUpdateState" ).on( "click", function()
																		{
																			ekxRestWorkerCmd( "SW Update State", "set_sw_update_state" );
																		});
								</script>
							</td>
							<td>
								<button class="blackbutton" id="resetSwUpdateState">Reset state</button>
								<script>
									$( "#resetSwUpdateState" ).on( "click", function()
																		{
																			ekxRestWorkerCmd( "SW Update State", "reset_sw_update_state" );
																		});
								</script>
							</td>
						</tr>

						<tr>
							<td class="tableDescriptorBold">Enable SSH access</td>
							<td>
								<button class="redbutton" id="enableSshBtn">Enable</button>
								<script>
									$( "#enableSshBtn" ).on( "click", function()
																		{
																			ekxRestWorkerCmd( "Enable SSH", "enable_ssh" );
																		});
								</script>
							</td>
							<td>
								<button class="blackbutton" id="disableSshBtn">Disable</button>
								<script>
									$( "#disableSshBtn" ).on( "click", function()
																		{
																			ekxRestWorkerCmd( "Disable SSH", "disable_ssh" );
																		});
								</script>
							</td>
						</tr>

						<tr>
							<td class="tableDescriptorBold">Active RFS partition</td>
							<td>
								<?=$rdev?>
							</td>
							<td>
							</td>
						</tr>

						<tr>
							<td class="tableDescriptorBold">Toggle RFS partition</td>
							<td>
								<button class="redbutton" id="setrfsimage3">Part #3</button>
								<script>
									$( "#setrfsimage3" ).on( "click", function()
																		{
																			ekxRestWorkerCmd( "Toggle RFS Partition", "set_rfs_partition_3" );
																		});
								</script>
							</td>
							<td>
								<button class="blackbutton" id="setrfsimage4">Part #4</button>
								<script>
									$( "#setrfsimage4" ).on( "click", function()
																		{
																			ekxRestWorkerCmd( "Toggle RFS Partition", "set_rfs_partition_4" );
																		});
								</script>
							</td>
						</tr>

						<tr>
							<td class="tableDescriptorBold">Active app partition</td>
							<td>
								<?=$appimage?>
							</td>
							<td>
							</td>
						</tr>

						<tr>
							<td class="tableDescriptorBold">Toggle app partition</td>
							<td>
								<button class="redbutton" id="setappimage5">Part #5</button>
								<script>
									$( "#setappimage5" ).on( "click", function()
																		{
																			ekxRestWorkerCmd( "Toggle App Partition", "set_app_partition_5" );
																		});
								</script>
							</td>
							<td>
								<button class="blackbutton" id="setappimage6">Part #6</button>
								<script>
									$( "#setappimage6" ).on( "click", function()
																		{
																			ekxRestWorkerCmd( "Toggle App Partition", "set_app_partition_6" );
																		});
								</script>
							</td>
						</tr>

						<tr>
							<td class="tableDescriptorBold">Unlock partitions</td>
							<td>
								<button class="redbutton" id="unlockRfs">Root FS</button>
								<script>
									$( "#unlockRfs" ).on( "click", function()
																		{
																			ekxRestWorkerCmd( "Unlock Partitions", "unlockrfs" );
																		});
								</script>
							</td>
							<td>
								<button class="blackbutton" id="unlockApp">Application</button>
								<script>
									$( "#unlockApp" ).on( "click", function()
																		{
																			ekxRestWorkerCmd( "Unlock Partitions", "unlockapp" );
																		});
								</script>
							</td>
						</tr>

						<tr>
							<td class="tableDescriptorBold">MCU check</td>
							<td>
								<?= file_exists( "/config/mcu_ignored" ) ? "disabled" : "enabled" ?>
							</td>
							<td>
							</td>
						</tr>

						<tr>
							<td class="tableDescriptorBold"></td>
							<td>
								<button class="redbutton" id="disableMcuCheck">disable</button>
								<script>
									$( "#disableMcuCheck" ).on( "click", function()
																		{
																			ekxRestWorkerCmd( "Disable MCU Check", "disable_mcu_check" );
																			setTimeout(() => { location.reload(); }, 1000);
																		});
								</script>
							</td>
							<td>
								<button class="blackbutton" id="enableMcuCheck">enable</button>
								<script>
									$( "#enableMcuCheck" ).on( "click", function()
																		{
																			ekxRestWorkerCmd( "Enable MCU Check", "enable_mcu_check" );
																			setTimeout(() => { location.reload(); }, 1000);
																		});
								</script>
							</td>
						</tr>

						<tr>
							<td class="tableDescriptorBold">SQLite database</td>
							<td>
								<button class="redbutton" id="removeDb">Remove</button>
								<script>
									$( "#removeDb" ).on( "click", function()
																		{
																			ekxRestWorkerCmd( "Remove SQLite database", "removedb" );
																		});
								</script>
							</td>
							<td>
								&nbsp;
							</td>
						</tr>

						<tr>
							<td class="tableDescriptorBold">Time</td>
							<td>
								<button class="redbutton" id="setTime">Set browser time</button>
								<script>
									$( "#setTime" ).on( "click", function()
																		{
																			ekxRestWorkerCmd( "Set board time to local browser time", "set_time=" + document.getElementById("local_time").value );
																		});
								</script>
							</td>
							<td align="center" class="tableDescriptorNormal">
								Local browser time</br>
								<input type="text" id="local_time" name="local_time" class="tableDescriptorNormal" size="15" value="" readonly > </br>
								<script type="text/javascript">
									var timer = setInterval(updateClock, 1000);
									var localTime = document.getElementById("local_time");
									function updateClock()
									{
										const today = new Date();
										
										localTime.value = today.toLocaleString();
									};
								</script>
								HMI board time</br>
								<input type="text" id="hmi_time" name="hmi_time" class="tableDescriptorNormal" size="15" value="<?=$hmi_time?>" readonly > 
							</td>
						</tr>
						
					</table>
				</div>

				<p>&nbsp;</p>
				<?php } ?>

				<div class="infoRegion" >
					<table class="infoTable">
						<tr>
							<td class="tableDescriptorBold" width="308px">Firmware update</td>
							<td width="240">&nbsp;</td>
							<td width="160">&nbsp;</td>
						</tr>

						<tr>
							<td class="tableDescriptorNormal">1. Please select<br/>HEM-EKX-S01...swu<br/>firmware file</td>
							<td colspan="2">
								<input class="redbutton" id="file_select" name="File" type="file" size="50" accept=".swu,application/octet-stream" hidden/>
								<button class="redbutton" id="file_select_shadow">Select file</button>
								<script>
									$( "#file_select_shadow" ).on( "click", function()
																{
																	$( "#file_select" ).click();
																});
								</script>
							</td>
						</tr>

						<tr>
							<td class="tableDescriptorNormal">2. Chosen file</td>
							<td colspan="2" class="tableDescriptorSmallBold">
								<div id="fileChosen">No file chosen</div>
								<script>
									document.getElementById( "file_select" ).addEventListener( 'change', function()
																{
																	document.getElementById( "fileChosen" ).innerHTML = this.files[0].name;
																});
								</script>
							</td>
						</tr>
						<tr>
							<td class="tableDescriptorNormal">3. Perform upload &amp; install</td>
							<td colspan="2">
								<button class="redbutton" id="upload" disabled>Upload File</button></br>
								<script>
									document.getElementById( "file_select" ).addEventListener( 'change', function()
																{
																	document.getElementById( "upload" ).disabled = false;
																	firmwareUpdateFile = this.files[0];
																});
									$( "#upload" ).on( "click", function()
																{
																	handleFirmwareUpload();
																});
								</script>
								<div style="display:none;">
									From: <input type="text" id="FilePos" readonly /></br>
									Size: <input type="text" id="ChunkSize" readonly /></br>
									Progress: <input type="text" id="Progress" readonly /></br>
									Effective Transfer Rate: <input type="text" id="EffTransferRate" readonly /> bytes/sec</br>
									Real Transfer Rate: <input type="text" id="RealTransferRate" readonly /> bytes/sec</br>
									ETA: <input type="text" id="EtaSec" readonly /></br>
									Success: <input type="text" id="Success" readonly /></br>
									Reason: <input type="text" id="Reason" readonly /></br>
								</div>
							</td>
						</tr>

					</table>
				</div>
				
				<p>&nbsp;</p>

				<div class="infoRegion" >
					<table class="infoTable">
						<tr>
							<td class="tableDescriptorBold" width="308px">Custom Splash-Screen upload</td>
							<td width="400" colspan="2">
								<div id="show_splash_screen" style="display:<?=$displaySplashScreen?>;width:120px;height:90px;background-color: black;">
									<img id="image_splash_screen" src="custom-splash-screen" style="position:relative;top:50%;left:50%;transform: translate(-50%, -50%);width:25%;" />
								</div>
							</td>
						</tr>

						<tr>
							<td class="tableDescriptorNormal">1. Please select any image file</td>
							<td>
								<input class="redbutton" id="splash_file_select" name="File" type="file" size="50" accept=".jpeg .jpg .png .gif,image/*" hidden/>
								<button class="redbutton" id="splash_file_select_shadow">Select file</button>
								<script>
									$( "#splash_file_select_shadow" ).on( "click", function()
																{
																	$( "#splash_file_select" ).click();
																});
								</script>
							</td>
							<td class="tableDescriptorNormal">Size: max. 480x360px</br>Format: jpg/png/gif
							</td>
						</tr>

						<tr>
							<td class="tableDescriptorNormal">2. Chosen file</td>
							<td colspan="2" class="tableDescriptorSmallBold">
								<div id="splashFileChosen">No file chosen</div>
								<script>
									document.getElementById( "splash_file_select" ).addEventListener( 'change', function()
																{
																	document.getElementById( "splashFileChosen" ).innerHTML = this.files[0].name;
																});
								</script>
							</td>
						</tr>
						<tr>
							<td class="tableDescriptorNormal">3. Perform upload</td>
							<td colspan="2">
								<button class="redbutton" id="splash_upload" disabled>Upload File</button></br>
								<script>
									document.getElementById( "splash_file_select" ).addEventListener( 'change', function()
																{
																	document.getElementById( "splash_upload" ).disabled = false;
																	firmwareUpdateFile = this.files[0];
																});
									$( "#splash_upload" ).on( "click", function()
																{
																	handleSplashScreenUpload();
																	document.getElementById( "show_splash_screen" ).style.display = "block";
																});
								</script>
							</td>
						</tr>
						<tr>
							<td class="tableDescriptorBold">Remove custom splash screen</td>
							<td colspan="2">
								<button class="redbutton" id="removeSplashScreen">Remove</button>
								<script>
									$( "#removeSplashScreen" ).on( "click", function()
																		{
																			ekxRestWorkerCmd( "Remove custom splash-screen image", "remove_splash_screen" );
																			document.getElementById( "show_splash_screen" ).style.display = "none";
																		});
								</script>
							</td>
						</tr>

					</table>
				</div>
				
				<p>&nbsp;</p>

				<div class="infoRegion" >
					<table class="infoTable">
						<tr>
							<td class="tableDescriptorBold" width="308">Support</td>
							<td class="tableDescriptorNormal" width="430" style="white-space: nowrap;">
								<a href="http://www.mahlkoenig.de/pages/service" target="_blanc">www.mahlkoenig.de/pages/service</a>
								<img alt="E54 QR Code" align="middle" style="height:80px;" src="qrcode.png" />
							</td>
						</tr>
						<tr>
							<td colspan="2" class="tableDescriptorNormal" style="text-align:center;">
								<a href="/copyright.html" style="font-size:12px;">EK-Omnia Software Copyright</a>
							</td>
						</tr>
					</table>
				</div>

				<p>&nbsp;</p>
			
			</div>
		</div>

		<!-- popup definition -->
		<div id="popup" class="popup">
			<div class="popup_box" id="popup_box">
				<span id="popup_close" class="popup_close">&times;</span>
				<div id="popup_caption" class="popup_caption">
					<div id="popup_caption_text" class="popup_caption_text">
					</div>
				</div>
				<div class="popup_content" id="popup_content">
					<div id="popup_loader" class="popup_loader">
						<p>Executing ...</p>
						<p><div class="lds-ripple"><div></div><div></div></div></p>
					</div>
					<div id="popup_progress_info" class="popup_progress_info">
						Firmware update in progress - don't switch off the grinder!</br></br><div id="popup_upload_file"></div></br>
						<div id="popup_progress_bar" class="popup_progress_bar">
							<div id="popup_progress_bar_progress" class="popup_progress_bar_progress"></div>
							<div id="popup_progress_bar_value" class="popup_progress_bar_value">0%</div>
							<div id="popup_eta" style="transform:translateY(-70px);"></div>
						</div>
					</div>
					<div id="log_id"></div>
					<pre id="pre_id"></pre>
				</div>
				<div class="popup_base" id="popup_base">
					<button class="blackbutton" id="popup_cancel" style="display:none;margin-top:15px;width:240px;">Cancel</button>
					<button class="redbutton" id="popup_ok" style="display:none;margin-top:15px;">Ok</button>
				</div>
			</div>
		</div>

		<!-- confirm popup definition -->
		<div id="confirm_popup" class="confirm_popup">
			<div class="confirm_popup_box">
				<span id="confirm_popup_close" class="confirm_popup_close">&times;</span>
				<div id="confirm_popup_caption" class="confirm_popup_caption">
					<div id="confirm_popup_caption_text" class="confirm_popup_caption_text">
					</div>
				</div>
				<div class="confirm_popup_content" id="confirm_popup_content">
				</div>
				<div class="confirm_popup_base" id="confirm_popup_base">
					<table>
						<tr>
							<td><button class="blackbutton" style="width:240px;" id="confirm_popup_cancel">Cancel</button></td>
							<td width="50px">&nbsp;</td>
							<td><button class="redbutton" id="confirm_popup_ok">Ok</button></td>
						</tr>
					</table>
				</div>
			</div>
		</div>

		<script type="text/javascript">

			// Get the popup objects
			var popup                    = document.getElementById( "popup" );
			var popupEta                 = document.getElementById( "popup_eta" );
			var popupClose               = document.getElementById( "popup_close" );
			var popupOk                  = document.getElementById( "popup_ok" );
			var popupCancel              = document.getElementById( "popup_cancel" );
			var popupContent             = document.getElementById( "popup_content" );
			var popupUploadFile          = document.getElementById( "popup_upload_file" );
			var popupCaptionText         = document.getElementById( "popup_caption_text");
			var popupLoader              = document.getElementById( "popup_loader");
			var popupProgressInfo        = document.getElementById( "popup_progress_info");
			var popupProgressBar         = document.getElementById( "popup_progress_bar");
			var popupProgressBarValue    = document.getElementById( "popup_progress_bar_value");
			var popupProgressBarProgress = document.getElementById( "popup_progress_bar_progress" );
			var preElement               = document.getElementById( "pre_id");
			var logElement               = document.getElementById( "log_id");

			var confirmPopupClose        = document.getElementById( "confirm_popup_close" );
			var confirmPopupOk           = document.getElementById( "confirm_popup_ok" );
			var confirmPopupCancel       = document.getElementById( "confirm_popup_cancel" );

			// When the user clicks on <span> (x), close the popup
			popupClose.onclick = function() 
			{
				closePopup();
			}

			popupCancel.onclick = function() 
			{
				closePopup();
			}

			popupOk.onclick = function() 
			{
				closePopup();
			}

			document.getElementById( "confirm_popup_cancel" ).onclick = function()
																		{
																			closeConfirmPopup();
																		};
			document.getElementById( "confirm_popup_close" ).onclick = function()
																		{
																			closeConfirmPopup();
																		};
																		
			<?php
				if ( !empty( $popupMessage ) )
				{
					echo "openPopup( \"$popupCaption\", false );\n";
					echo "showNormalLog( \"$popupMessage\" );\n";
					echo "enablePopupOk();\n";
				}
			?>
		</script>

	</body>
</html>
