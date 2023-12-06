<?php
session_start();
?>
<?php
	$message = '';
	$log = "";
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

	$strSerialNo = file_get_contents( "/proc/device-tree/hde/sernr" );
	$strSerialNo = str_replace( ":", "", $strSerialNo );
	$strSerialNo = str_replace( "\0", "", $strSerialNo );
	
	if ( empty( $imp_periodStart ) )
	{
		$imp_periodStart = date( "Y-m-d" );
	}
	if ( empty( $imp_periodEnd ) )
	{
		$imp_periodEnd = date( "Y-m-d" );
	}
	
	if ( isset( $imp_downloadCounters ) )
	{
		$timestamp = date( "Ymd_His" );
		$zipFile = "/storage/EkxHmiCounters_$strSerialNo_$timestamp.zip";
		$csvFile = "/storage/EkxHmiCounters_$strSerialNo_$timestamp.csv";

		$output = null;
		exec( "sudo rm -f /storage/*.zip /storage/*.csv /storage/*.swu", $output, $returnCode );
		
		$query = "select counter_name, value, unit_name from counters, units where counters.unit_id = units.unit_id order by counter_name;";
		$output = null;
		exec( "sudo /usr/bin/sqlite3 /db_storage/EkxSqlite.db -csv -header -separator \";\" \"$query\" > $csvFile", $output, $returnCode );
		exec( "sync" );

		if ( $returnCode != 0 )
		{
			$popupCaption = "Download counters";
			$popupMessage = "Execution of query failed!<br>" . implode( "<br>", $output );
		}
		else if ( !file_exists( $csvFile ) ) 
		{
			$popupCaption = "Download counters";
			$popupMessage = "Error, no csv file generated!";
		}
		else
		{
			$output = null;
			exec( "cd /storage/ && sudo zip -r $zipFile " . basename( $csvFile ), $output, $returnCode );

			if ( $returnCode != 0 )
			{
				$popupCaption = "Download counters";
				$popupMessage = "Generation of zip file failed!<br>" . implode( "<br>", $output );
			}
			else if ( !file_exists( $csvFile ) ) 
			{
				$popupCaption = "Download counters";
				$popupMessage = "Error, no zip file generated!";
			}
			else
			{
				header( 'Content-Description: File Transfer' );
				header( 'Content-Type: application/x-tar' );
				header( 'Content-Disposition: attachment; filename="' . basename( $zipFile ) . '"' );
				header( 'Expires: 0' );
				header( 'Cache-Control: must-revalidate' );
				header( 'Pragma: public' );
				header( 'Content-Length: ' . filesize( $zipFile ) );
				readfile( $zipFile );
				exit;
			}
		}
	}
	else if ( isset( $imp_downloadEvents ) )
	{
		$timestamp = date( "Ymd_His" );
		$zipFile = "/storage/EkxHmiEvents_$strSerialNo_$timestamp.zip";
		$csvFile = "/storage/EkxHmiEvents_$strSerialNo_$timestamp.csv";
		$sqlFile = "/storage/EkxHmiEvents_$strSerialNo_$timestamp.sql";

		$output = null;
		exec( "sudo rm -f /storage/*.zip /storage/*.csv /storage/*.sql /storage/*.swu", $output, $returnCode );

		$query = "";
		
		if ( $imp_eventGroupSelection == "single" )
		{
			$query .= "select events.timestamp, event_types.event_name, notification_types.notification_name, recipes.recipe_uuid, recipes.recipe_name, events.context ";
		}
		else if ( $imp_eventGroupSelection == "group" )
		{
			$query .= "select count(*) as amount, event_types.event_name, notification_types.notification_name ";
		}
		$query .= "from events "
		          . "left join event_types on events.event_type_id = event_types.event_type_id "
		          . "left join notification_types on events.notification_type_id = notification_types.notification_type_id "
		          . "left join recipes on recipes.recipe_id = events.recipe_id ";
		          
		$query .= "where date( timestamp ) <= '$imp_periodEnd' and date( timestamp ) >= '$imp_periodStart' ";
		         
		if ( $imp_eventNotificationExclusionSelection == "all" )
		{
			// no added filter
		}
		else if ( $imp_eventNotificationExclusionSelection == "onlyNotifications" )
		{
			$query .= " and events.notification_type_id > 0 ";
		}
		else if ( $imp_eventNotificationExclusionSelection == "withoutNotifications" )
		{
			$query .= " and events.notification_type_id == 0 ";
		}

		if ( $imp_eventTypeSelection == "all" )
		{
			// no added filter
		}
		else
		{
			$query .= " and events.event_type_id == '$imp_eventTypeSelection' ";
		}
		
		if ( $imp_notificationTypeSelection == "all" )
		{
			// no added filter
		}
		else
		{
			$query .= " and events.notification_type_id == '$imp_notificationTypeSelection' ";
		}
		
		if ( $imp_eventGroupSelection == "single" )
		{
			$query .= "order by timestamp ";
		}
		else if ( $imp_eventGroupSelection == "group" )
		{
			$query .= "group by event_types.event_name, notification_types.notification_name ";
		}
		$query .= ";";
		
		$output = null;
		exec( "sudo /usr/bin/sqlite3 /db_storage/EkxSqlite.db -csv -header -separator \";\" \"$query\" > $csvFile", $output, $returnCode );
		file_put_contents( $sqlFile, $query );
		exec( "sync" );

		if ( $returnCode != 0 )
		{
			$popupCaption = "Download event statistics";
			$popupMessage = "Execution of query failed!<br>" . implode( "<br>", $output );
		}
		else if ( !file_exists( $csvFile ) ) 
		{
			$popupCaption = "Download event statistics";
			$popupMessage = "Error, no csv file generated!";
		}
		else
		{
			$output = null;
			exec( "cd /storage/ && sudo zip -r $zipFile " . basename( $csvFile ) . " " . basename( $sqlFile ), $output, $returnCode );
		
			if ( $returnCode != 0 )
			{
				$popupCaption = "Download event statistics";
				$popupMessage = "Generation of zip file failed!<br>" . implode( "<br>", $output );
			}
			else if ( !file_exists( $csvFile ) ) 
			{
				$popupCaption = "Download event statistics";
				$popupMessage = "Error, no zip file generated!";
			}
			else
			{
				header( 'Content-Description: File Transfer' );
				header( 'Content-Type: application/x-tar' );
				header( 'Content-Disposition: attachment; filename="' . basename( $zipFile ) . '"' );
				header( 'Expires: 0' );
				header( 'Cache-Control: must-revalidate' );
				header( 'Pragma: public' );
				header( 'Content-Length: ' . filesize( $zipFile ) );
				readfile( $zipFile );
				exit;
			}
		}
	}
	else if ( isset( $imp_downloadGrinds ) )
	{
		$timestamp = date( "Ymd_His" );
		$zipFile = "/storage/EkxHmiGrinds_$strSerialNo_$timestamp.zip";
		$csvFile = "/storage/EkxHmiGrinds_$strSerialNo_$timestamp.csv";
		$sqlFile = "/storage/EkxHmiGrinds_$strSerialNo_$timestamp.sql";

		$output = null;
		exec( "sudo rm -f /storage/*.zip /storage/*.csv /storage/*.sql /storage/*.swu", $output, $returnCode );
		
		$query = "";
		
		if ( $imp_grindGroupSelection == "single" )
		{
			$query .= "select grinds.timestamp, recipes.recipe_uuid, recipes.recipe_name, recipe_modes.recipe_mode_name, grinds.grind_time_100ths ";
		}
		else if ( $imp_grindGroupSelection == "group" )
		{
			$query .= "select count(1) as Amount, recipes.recipe_uuid, recipes.recipe_name, recipe_modes.recipe_mode_name, sum( grinds.grind_time_100ths ) ";
		}
		$query .= "from grinds "
		          . "left join recipes on recipes.recipe_id = grinds.recipe_id "
		          . "left join recipe_modes on recipes.recipe_mode_id = recipe_modes.recipe_mode_id ";

		$query .= "where date( timestamp ) <= '$imp_periodEnd' and date( timestamp ) >= '$imp_periodStart' ";
		         
		if ( $imp_recipeModeSelection == "all" )
		{
			// no added filter
		}
		else 
		{
			$query .= " and recipe_modes.recipe_mode_id == '$imp_recipeModeSelection' ";
		}

		if ( $imp_recipeNameSelection == "all" )
		{
			// no added filter
		}
		else 
		{
			$query .= " and recipes.recipe_id == '$imp_recipeNameSelection' ";
		}

		if ( $imp_grindGroupSelection == "single" )
		{
			$query .= "order by timestamp ";
		}
		else if ( $imp_grindGroupSelection == "group" )
		{
			$query .= "group by recipes.recipe_uuid, recipes.recipe_name, recipe_modes.recipe_mode_name ";
		}
		$query .= ";";
		
		$output = null;
		exec( "sudo /usr/bin/sqlite3 /db_storage/EkxSqlite.db -csv -header -separator \";\" \"$query\" > $csvFile", $output, $returnCode );
		file_put_contents( $sqlFile, $query );
		exec( "sync" );

		if ( $returnCode != 0 )
		{
			$popupCaption = "Download grind statistics";
			$popupMessage = "Execution of query failed!<br>" . implode( "<br>", $output );
		}
		else if ( !file_exists( $csvFile ) ) 
		{
			$popupCaption = "Download grind statistics";
			$popupMessage = "Error, no csv file generated!";
		}
		else
		{
			$output = null;
			exec( "cd /storage/ && sudo zip -r $zipFile " . basename( $csvFile ) . " " . basename( $sqlFile ), $output, $returnCode );
		
			if ( $returnCode != 0 )
			{
				$popupCaption = "Download grind statistics";
				$popupMessage = "Generation of zip file failed!<br>" . implode( "<br>", $output );
			}
			else if ( !file_exists( $csvFile ) ) 
			{
				$popupCaption = "Download grind statistics";
				$popupMessage = "Error, no zip file generated!";
			}
			else
			{
				header( 'Content-Description: File Transfer' );
				header( 'Content-Type: application/x-tar' );
				header( 'Content-Disposition: attachment; filename="' . basename( $zipFile ) . '"' );
				header( 'Expires: 0' );
				header( 'Cache-Control: must-revalidate' );
				header( 'Pragma: public' );
				header( 'Content-Length: ' . filesize( $zipFile ) );
				readfile( $zipFile );
				exit;
			}
		}
	}

?>
<!DOCTYPE html>

<html lang="en">
	<head>
		<meta charset="utf-8">
		
		<title>Hemro EKX Web Interface - Statistics</title>

		<link rel="shortcut icon" type="image/x-icon" href="favicon.ico" />
		<link rel="stylesheet" type="text/css" href="style.css?ver=2" />
		
		<script src="minimalJs.js"></script>
		<script src="aliveCheck.js"></script>
		<script src="popupManagement.js"></script>

		<script type="text/javascript">

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
									EK43
								</div>
							</p>
							<p>
								<div style="font-size:20px;font-weight: 500;text-transform: uppercase;">
									SN
								</div>
								<div style="font-size:25px;font-weight: bold;">
									<?= $strSerialNo?>
								</div>
							</p>
							<p>
								<div style="font-size:20px;font-weight: 500;text-transform: uppercase;">
									MODEL
								</div>
								<div style="font-size:25px;font-weight: bold;">
									EK43 new generation
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
				
				<form method="POST">

					<p>&nbsp;</p>

					<div class="infoRegion" >
						<table class="infoTable">
							<tr>
								<td class="tableDescriptorBold" colspan="2"><a href="index.php"><img src="home.png" alt="Home" /></a></td>
							</tr>
						</table>
					</div>

					<p>&nbsp;</p>

					<div class="infoRegion" >
						<table class="infoTable">
							<tr>
								<td class="tableDescriptorBold" colspan="2">Configure statistics period</td>
							</tr>

							<tr>
								<td width="300px" class="tableDescriptorBold">Quick period selections</td>
								<td width="400px">
									<select name="periodSelection" id="periodSelection" class="redbutton" style="width:400px;">
										<option value="thisWeek" selected="selected">This week</option>
										<option value="lastWeek">Last week</option>
										<option value="thisMonth">This month</option>
										<option value="lastMonth">Last month</option>
										<option value="thisYear">This year</option>
										<option value="lastYear">Last year</option>
										<option value="all">Don't care</option>
									</select>
									<script>
										$( "#periodSelection" ).change( function()
																			{
																				var currentSelection = this.value;
																				
																				var endDate = new Date();
																				var startDate = new Date();

																				if ( currentSelection == "thisWeek" )
																				{
																					startDate.setTime( startDate.getTime() - startDate.getDay() * 24 * 60 * 60 * 1000 );
																					// endDate unchanged
																				}
																				else if ( currentSelection == "thisMonth" )
																				{
																					startDate = new Date( startDate.getFullYear(), startDate.getMonth(), 1 );
																					// endDate unchanged
																				}
																				else if ( currentSelection == "thisYear" )
																				{
																					startDate = new Date( startDate.getFullYear(), 0, 1 );
																					// endDate unchanged
																				}
																				else if ( currentSelection == "all" )
																				{
																					startDate = new Date( 1970, 0, 1 );
																					// endDate unchanged
																				}
																				else if ( currentSelection == "lastWeek" )
																				{
																					endDate.setTime( endDate.getTime() - ( endDate.getDay() + 1 ) * 24 * 60 * 60 * 1000 );
																					startDate.setTime( endDate.getTime() - endDate.getDay() * 24 * 60 * 60 * 1000 );
																				}
																				else if ( currentSelection == "lastMonth" )
																				{
																					if ( endDate.getMonth() > 0 )
																						endDate = new Date( endDate.getFullYear(), endDate.getMonth(), 0 );
																					else
																						endDate = new Date( endDate.getFullYear() - 1, 11, 31 );
																					startDate = new Date( endDate.getFullYear(), endDate.getMonth(), 1 );
																				}
																				else if ( currentSelection == "lastYear" )
																				{
																					startDate = new Date( startDate.getFullYear() - 1, 0, 1 );
																					endDate = new Date( endDate.getFullYear() - 1, 11, 31 );
																				}
																				
																				var dd = String( endDate.getDate() ).padStart( 2, '0' );
																				var mm = String( endDate.getMonth() + 1 ).padStart( 2, '0' );
																				var yyyy = endDate.getFullYear();
																				var periodEnd = yyyy + "-" + mm + "-" + dd;
																				$( "#periodEnd" ).val( periodEnd );

																				var dd = String( startDate.getDate() ).padStart( 2, '0' );
																				var mm = String( startDate.getMonth() + 1 ).padStart( 2, '0' );
																				var yyyy = startDate.getFullYear();
																				var periodStart = yyyy + "-" + mm + "-" + dd;
																				$( "#periodStart" ).val( periodStart );
																			});
									</script>
								</td>
							</tr>

							<tr>
								<td class="tableDescriptorBold">Period start</td>
								<td>
									<input type="date" id="periodStart" name="periodStart" class="tableDescriptorNormal" size="20" maxlength="20" value="<?=$imp_periodStart?>">
								</td>
							</tr>

							<tr>
								<td class="tableDescriptorBold">Period end</td>
								<td>
									<input type="date" id="periodEnd" name="periodEnd" class="tableDescriptorNormal" size="20" maxlength="20" value="<?=$imp_periodEnd?>">
								</td>
							</tr>

						</table>
					</div>
					
					<p>&nbsp;</p>

					<div class="infoRegion" >
						<table class="infoTable">
							<tr>
								<td class="tableDescriptorBold" colspan="2">Configure event statistics</td>
							</tr>

							<tr>
								<td width="300px" class="tableDescriptorBold">Event group</td>
								<td width="400px">
									<select name="eventNotificationExclusionSelection" id="eventNotificationExclusionSelection" class="redbutton" style="width:400px;">
										<option value="all" selected="selected">Don't care</option>
										<option value="onlyNotifications">Only notifications</option>
										<option value="withoutNotifications">Without notifications</option>
									</select>
								</td>
							</tr>

							<tr>
								<td class="tableDescriptorBold">Event type</td>
								<td>
									<select name="eventTypeSelection" id="eventTypeSelection" class="redbutton" style="width:400px;">
										<option value="all" selected="selected">Don't care</option>
										<?php
											$query = "select event_type_id, event_name from event_types order by event_name;";
											exec( "sudo /usr/bin/sqlite3 /db_storage/EkxSqlite.db -csv -separator \";\" \"$query\" > /tmp/event_types;sync", $output, $returnCode );

											$result = file_get_contents( "/tmp/event_types" );
											$result_list = explode( "\n", $result );
											
											for ( $i = 0; $i < count( $result_list ); $i++ )
											{
												if ( !empty( $result_list[$i] ) )
												{
													$line_result = explode( ";", $result_list[$i] );
													echo "<option value=\"$line_result[0]\">$line_result[1]</option>\n";
												}
											}
										?>
									</select>
								</td>
							</tr>

							<tr>
								<td class="tableDescriptorBold">Notification type</td>
								<td>
									<select name="notificationTypeSelection" id="notificationTypeSelection" class="redbutton" style="width:400px;">
										<option value="all" selected="selected">Don't care</option>
										<?php
											$query = "select notification_type_id, notification_name from notification_types order by notification_name;";
											exec( "sudo /usr/bin/sqlite3 /db_storage/EkxSqlite.db -csv -separator \";\" \"$query\" > /tmp/notification_types;sync", $output, $returnCode );

											$result = file_get_contents( "/tmp/notification_types" );
											$result_list = explode( "\n", $result );
											
											for ( $i = 0; $i < count( $result_list ); $i++ )
											{
												if ( !empty( $result_list[$i] ) )
												{
													$line_result = explode( ";", $result_list[$i] );
													echo "<option value=\"$line_result[0]\">$line_result[1]</option>\n";
												}
											}
										?>
									</select>
								</td>
							</tr>

							<tr>
								<td class="tableDescriptorBold">Grouping</td>
								<td>
									<select name="eventGroupSelection" id="eventGroupSelection" class="redbutton" style="width:400px;">
										<option value="single" selected="selected">List each single entry</option>
										<option value="group">Count same entries together</option>
									</select>
								</td>
							</tr>

							<tr>
								<td class="tableDescriptorBold">Process request</td>
								<td>
									<button class="blackbutton" id="downloadEvents" name="downloadEvents" style="width:400px;">Download .csv</button>
									<script>
										$( "#downloadEvents" ).on( "click", function()
																			{
																				if ( $( "#periodStart" ).val() > $( "#periodEnd" ).val() )
																				{
																					openPopup( "Download event statistics",
																								"Error!<br>Start period date lays behind end period date.<br>Please correct period dates!" );
																					return false;
																				}
																				
																				return true;
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
								<td class="tableDescriptorBold" colspan="2">Configure grind statistics</td>
							</tr>

							<tr>
								<td width="300px" class="tableDescriptorBold">Recipe mode</td>
								<td width="400px">
									<select name="recipeModeSelection" id="recipeModeSelection" class="redbutton" style="width:400px;">
										<option value="all" selected="selected">Don't care</option>
										<?php
											$query = "select recipe_mode_id, recipe_mode_name from recipe_modes order by recipe_mode_name;";
											exec( "sudo /usr/bin/sqlite3 /db_storage/EkxSqlite.db -csv -separator \";\" \"$query\" > /tmp/recipe_modes;sync", $output, $returnCode );

											$result = file_get_contents( "/tmp/recipe_modes" );
											$result_list = explode( "\n", $result );
											
											for ( $i = 0; $i < count( $result_list ); $i++ )
											{
												if ( !empty( $result_list[$i] ) )
												{
													$line_result = explode( ";", $result_list[$i] );
													echo "<option value=\"$line_result[0]\">$line_result[1]</option>\n";
												}
											}
										?>
									</select>
								</td>
							</tr>

							<tr>
								<td class="tableDescriptorBold">Recipe name</td>
								<td>
									<select name="recipeNameSelection" id="recipeNameSelection" class="redbutton" style="width:400px;">
										<option value="all" selected="selected">Don't care</option>
										<?php
											$query = "select recipe_id, recipe_name from recipes order by recipe_name;";
											exec( "sudo /usr/bin/sqlite3 /db_storage/EkxSqlite.db -csv -separator \";\" \"$query\" > /tmp/recipe_names;sync", $output, $returnCode );

											$result = file_get_contents( "/tmp/recipe_names" );
											$result_list = explode( "\n", $result );

											for ( $i = 0; $i < count( $result_list ); $i++ )
											{
												if ( !empty( $result_list[$i] ) )
												{
													$line_result = explode( ";", $result_list[$i] );
													echo "<option value=\"$line_result[0]\">$line_result[1]</option>\n";
												}
											}
										?>
									</select>
								</td>
							</tr>

							<tr>
								<td class="tableDescriptorBold">Grouping</td>
								<td>
									<select name="grindGroupSelection" id="grindGroupSelection" class="redbutton" style="width:400px;">
										<option value="single" selected="selected">List each single entry</option>
										<option value="group">Count same entries together</option>
									</select>
								</td>
							</tr>

							<tr>
								<td class="tableDescriptorBold">Process request</td>
								<td>
									<button class="blackbutton" id="downloadGrinds" name="downloadGrinds" style="width:400px;">Download .csv</button>
									<script>
										$( "#downloadGrinds" ).on( "click", function()
																			{
																				if ( $( "#periodStart" ).val() > $( "#periodEnd" ).val() )
																				{
																					openPopup( "Download grind statistics",
																								"Error!<br>Start period date lays behind end period date.<br>Please correct period dates!" );
																					return false;
																				}
																				
																				return true;
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
								<td class="tableDescriptorBold" colspan="2">Counters statistics</td>
							</tr>

							<tr>
								<td width="300px" class="tableDescriptorBold">Process request</td>
								<td width="400px">
									<button class="blackbutton" id="downloadCounters" name="downloadCounters" style="width:400px;">Download .csv</button>
									<script>
										$( "#downloadCounters" ).on( "click", function()
																			{
																				return true;
																			});
									</script>
								</td>
							</tr>

						</table>
					</div>
				
				</form>

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
