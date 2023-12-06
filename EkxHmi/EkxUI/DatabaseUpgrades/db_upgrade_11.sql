
REPLACE INTO "notification_types" ("notification_type_id","notification_name","description","notification_class_id","recover_action","short_info","long_info") VALUES
 (3,'ESP_INTERFACE_FAILED','Communication between HMI and driver board is interrupted. No grind actions possible. WebServer is not available.',3,1,'Driver Comm Failed','Driver board\ncommunication\nfailed!'),
 (21,'SW_UPDATE_ESP_UPDATE_FAILED','Software update of driver board failed. System will be rolled back to previous state. Try to repeat SW update.',3,0,'SW Update Failed','Update of driver\nboard failed!'),
 (28,'STARTUP_ESP_CHECK_VERSION_TIMEOUT','Read of driver board version information timed out.',3,0,'Startup Failed','Driver board SW\nversion check failed!\nTimeout!'),
 (30,'SW_UPDATE_ESP_UPDATE_TIMEOUT','Software update of driver board timed out. System will be rolled back to previous state. Try to repeat SW update.',3,0,'SW Update Failed','Timeout while\nupdating driver board!'),
 (32,'SW_UPDATE_ESP_VALIDATE_VERSION_TIMEOUT','Validation of driver board version timed out after software update. System will be rolled back to previous state. Try to repeat SW update.',3,0,'SW Update Failed','Timeout while\nvalidating driver board\nSW version!'),
 (38,'SW_UPDATE_ESP_VALIDATE_VERSION_FAILED','Validation of driver board version failed after software update. System will be rolled back to previous state. Try to repeat SW update.',3,0,'SW Update Failed','Driver board SW\nversion validation\nfailed!');


REPLACE INTO `settings` (`key`, `value`) VALUES ( 'Version', '11' );
