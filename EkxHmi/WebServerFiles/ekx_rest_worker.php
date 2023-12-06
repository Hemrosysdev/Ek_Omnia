<?php
	session_start();

	$state_file_json = "/storage/SoftwareUpdateState.json";
	$state_file_lock = "/tmp/SoftwareUpdateState.lock";

	extract( $_REQUEST, EXTR_PREFIX_ALL | EXTR_REFS, 'imp' );

	if ( isset( $imp_dev ) )
	{
		$_SESSION["dev"] = "dev";
	}
	else if ( isset( $imp_nodev ) )
	{
		unset( $_SESSION["dev"] );
	}

	$output = null;

	if ( isset( $imp_set_sw_update_state ) )
	{
		$json = "{ \"Process\" : \"Update\", \"State\" : \"UPLOAD_IN_PROGRESS\" }";
		file_put_contents( $state_file_json, $json );

		echo "Set state done!\n";
	}
	else if ( isset( $imp_flash_esp_eeprom ) )
	{
		exec( "sudo chmod 666 /tmp/esp_connector/output/At24c16Out.json" );
		$json = "{ \"Cmd\" : \"Flash\", \"ProductNo\" : \"$imp_product_no\", \"SerialNo\" : \"$imp_serial_no\" }";
		$result = file_put_contents( "/tmp/esp_connector/output/At24c16Out.json", $json );

		echo "Set flash info done! Result=$result\n";
	}
	else if ( isset( $imp_set_mcu_update_state ) )
	{
		$json = "{ \"Process\" : \"Normal\", \"State\" : \"MCU_UPDATE_IN_PROGRESS\" }";
		file_put_contents( $state_file_json, $json );

		echo "Set state done!\n";
	}
	else if ( isset( $imp_set_esp_update_state ) )
	{
		$json = "{ \"Process\" : \"Normal\", \"State\" : \"ESP_UPDATE_IN_PROGRESS\" }";
		file_put_contents( $state_file_json, $json );

		echo "Set state done!\n";
	}
	else if ( isset( $imp_reset_sw_update_state ) )
	{
		$json = "{ \"Process\" : \"Normal\", \"State\" : \"IDLE\" }";
		file_put_contents( $state_file_json, $json );

		echo "Set state done!\n";
	}
	else if ( isset( $imp_reset_sw_update_state ) )
	{
		$json = "{ \"Process\" : \"Normal\", \"State\" : \"IDLE\" }";
		file_put_contents( $state_file_json, $json );

		echo "Set state done!\n";
	}
	else if ( isset( $imp_get_sw_update_state ) )
	{
		passthru( "cat $state_file_json", $returnCode );
	}
	else if ( isset( $imp_enable_ssh ) )
	{
		passthru( "sudo unlockrfs", $returnCode );
		passthru( "echo -n 'o' > /tmp/dropbear", $returnCode );
		passthru( "cat /etc/default/dropbear >> /tmp/dropbear", $returnCode );
		passthru( "sudo cp /tmp/dropbear /etc/default/dropbear", $returnCode );
		passthru( "sudo /etc/init.d/S50dropbear restart", $returnCode );

		echo "Done!\n";
		echo "Return code " . $returnCode . "\n";
	}
	else if ( isset( $imp_disable_ssh ) )
	{
		passthru( "sudo unlockrfs", $returnCode );
		passthru( "echo -n 'DROPBEAR_ARGS=\"-w -s\"' > /tmp/dropbear", $returnCode );
		passthru( "sudo cp /tmp/dropbear /etc/default/dropbear", $returnCode );
		passthru( "sudo /etc/init.d/S50dropbear restart", $returnCode );

		echo "Done!\n";
		echo "Return code " . $returnCode . "\n";
	}
	else if ( isset( $imp_unlockrfs ) )
	{
		passthru( "sudo mount -o,remount -w /", $returnCode );

		echo "Done!\n";
		echo "Return code " . $returnCode . "\n";
	}
	else if ( isset( $imp_unlockapp ) )
	{
		passthru( "sudo mount -o,remount -w /usr/local/", $returnCode );

		echo "Done!\n";
		echo "Return code " . $returnCode . "\n";
	}
	else if ( isset( $imp_reboot ) )
	{
		passthru( "sudo reboot", $returnCode );

		echo "Done!\n";
		echo "Return code " . $returnCode . "\n";
	}
	else if ( isset( $imp_stopall ) )
	{
		passthru( "sudo /etc/init.d/S90app stop", $returnCode );

		echo "Done!\n";
		echo "Return code " . $returnCode . "\n";
	}
	else if ( isset( $imp_startall ) )
	{
		passthru( "sudo /etc/init.d/S90app start", $returnCode );

		echo "Done!\n";
		echo "Return code " . $returnCode . "\n";
	}
	else if ( isset( $imp_start_ws_server ) )
	{
		passthru( "sudo killall -9 EkxWsServer", $returnCode );
		passthru( "sudo nohup /usr/local/bin/EkxWsServer -d /storage &", $returnCode );

		echo "Done!\n";
		echo "Return code " . $returnCode . "\n";
	}
	else if ( isset( $imp_set_rfs_partition_3 ) )
	{
		passthru( "sudo fw_setenv part 3", $returnCode );

		echo "Done!\n";
		echo "Return code " . $returnCode . "\n";
	}
	else if ( isset( $imp_set_rfs_partition_4 ) )
	{
		exec( "sudo fw_setenv part 4", $output, $returnCode );

		echo "Done!\n";
		echo "Return code " . $returnCode . "\n";
	}
	else if ( isset( $imp_set_app_partition_5 ) )
	{
		passthru( "sudo fw_setenv app_partition 5", $returnCode );

		echo "Done!\n";
		echo "Return code " . $returnCode . "\n";
	}
	else if ( isset( $imp_set_app_partition_6 ) )
	{
		passthru( "sudo fw_setenv app_partition 6", $returnCode );

		echo "Done!\n";
		echo "Return code " . $returnCode . "\n";
	}
	else if ( isset( $imp_removedb ) )
	{
		passthru( "sudo rm -f /db_storage/EkxSqlite.db", $returnCode );

		echo "Done!\n";
		echo "Return code " . $returnCode . "\n";
	}
	else if ( isset( $imp_remove_splash_screen ) )
	{
		exec( "sudo rm -f /config/custom-splash-screen", $output, $returnCode );
		exec( "sudo /etc/init.d/S01splashscreen restart", $output, $returnCode );

		echo "Done!\n";
		echo "Return code " . $returnCode . "\n";
	}
	else if ( isset( $imp_reformatpartv2 ) )
	{
		passthru( "sudo nohup /usr/local/www/reformat.sh &", $returnCode );

		echo "Done!\n";
		echo "Return code " . $returnCode . "\n";
	}
	else if ( isset( $imp_disable_mcu_check ) )
	{
		passthru( "sudo touch /config/mcu_ignored &", $returnCode );

		echo "Done!\n";
		echo "Return code " . $returnCode . "\n";
	}
	else if ( isset( $imp_enable_mcu_check ) )
	{
		passthru( "sudo rm /config/mcu_ignored &", $returnCode );

		echo "Done!\n";
		echo "Return code " . $returnCode . "\n";
	}
	else if ( isset( $imp_start_fw_update ) )
	{
		$targetFile = "/storage/" . $imp_start_fw_update;
		$error = !file_exists( $targetFile );

		if ( !$error )
		{
			exec( "sudo rm -f /tmp/start_update.sh", $output, $returnCode );
			$output = null;
			exec( "sudo swupdate -i \"$targetFile\" -e tmp,startupdate -k /etc/keys/hem-swupdate-key.pub &> /log/swupdate_step1.log", $output, $returnCode );
			$log = implode( "\n", $output );
			$output = null;
			exec( "sync;" );
			
			if ( $returnCode == 0 )
			{
				if ( !file_exists( "/tmp/start_update.sh" ) )
				{
					$error = true;
					echo "Done!\n";
					echo "Error: Start script not available after extraction from package!\n";
					echo "Return code -100\n";
					$log = file_get_contents( "/log/swupdate_step1.log" );
					echo $log;
				}
			}
			else
			{
				$error = true;
				echo "Done!\n";
				echo "Error: swupdate cannot extract start script from package!\n";
				echo "Return code " . $returnCode . "\n";
				$log = file_get_contents( "/log/swupdate_step1.log" );
				echo $log;
			}
		}
		else
		{
			$error = true;
			echo "Done!\n";
			echo "Error: Update file not found.\n";
			echo "Return code -101\n";
		}
		
		if ( !$error )
		{
			$output = null;
			exec( "sudo /tmp/start_update.sh -p \"$targetFile\" > /log/start_update.log &", $output, $returnCode );

			echo "Done!\n";
			echo "Return code " . $returnCode . "\n";
		}
	}
	else if ( isset( $imp_query_db ) )
	{
		header( 'Content-Description: EKX SQLite export' );
		header( 'Content-Type: text/plain' );
		//header( 'Content-Type: text/csv' );
		//header( 'Content-Disposition: attachment; filename="ekx_sqlite_export.csv"' );
		header( 'Expires: 0' );
		header( 'Cache-Control: must-revalidate' );
		header( 'Pragma: public' );
		passthru( "sudo /usr/bin/sqlite3 /db_storage/EkxSqlite.db -csv -header -separator \";\" $imp_query_db", $returnCode );
	}
	else if ( isset( $imp_set_time ) )
	{
		echo "Set time to: $imp_set_time<br>";
		$timestamp = strtotime( $imp_set_time );
		passthru( "sudo date +%s -s @$timestamp" );
		passthru( "sudo hwclock --systohc" );

		echo "Done!\n";
	}
	else if ( isset( $imp_clear_agsa_logs ) )
	{
		passthru( "sudo rm -f /log/HemroEkxAgsaLdtLogs/*" );
		passthru( "sudo rm -f /storage/HemroEkxAgsaLdtLogs/*" );

		echo "Done!\n";
	}
	else if ( isset( $imp_ping ) )
	{
		echo "pong\n";
	}
	else if ( isset( $imp_download_agsa_log ) )
	{
		$zipFile = "/storage/HemroEkxAgsaLdtLogs/$imp_download_agsa_log";
		
		header( 'Content-Description: File Transfer' );
		header( 'Content-Type: application/zip' );
		header( 'Content-Disposition: attachment; filename="' . basename( $zipFile ) . '"' );
		header( 'Expires: 0' );
		header( 'Cache-Control: must-revalidate' );
		header( 'Pragma: public' );
		header( 'Content-Length: ' . filesize( $zipFile ) );
		readfile( $zipFile );
		exit;
	}
	else
	{
		echo "Error, unknown REST command!\n";
	}

?>
