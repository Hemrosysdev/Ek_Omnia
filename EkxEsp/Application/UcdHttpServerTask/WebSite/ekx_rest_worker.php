<?php
	session_start();

	$state_file_json = "/storage/SoftwareUpdateState.json";
	$state_file_lock = "/tmp/SoftwareUpdateState.lock";

	extract( $_REQUEST, EXTR_PREFIX_ALL | EXTR_REFS, 'imp' );


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
