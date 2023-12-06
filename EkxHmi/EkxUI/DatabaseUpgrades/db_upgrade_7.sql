drop table if exists `error_types`;
drop table if exists `events`;

CREATE TABLE IF NOT EXISTS "notification_types" (
	"notification_type_id"	INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
	"notification_name"	VARCHAR(30) NOT NULL UNIQUE,
	"description"	VARCHAR(255),
	"notification_class_id"	INTEGER
);
CREATE TABLE IF NOT EXISTS "notification_classes" (
	"notification_class_id"	INTEGER NOT NULL,
	"notification_class"	VARCHAR(10) NOT NULL,
	PRIMARY KEY("notification_class_id")
);
CREATE TABLE IF NOT EXISTS "events" (
	"event_id"	INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
	"timestamp"	DATETIME NOT NULL,
	"event_type_id"	INTEGER NOT NULL,
	"recipe_id"	INTEGER,
	"notification_type_id"	INTEGER,
	"context"	VARCHAR(50),
	FOREIGN KEY("event_type_id") REFERENCES "event_types"("event_type_id"),
	FOREIGN KEY("notification_type_id") REFERENCES "notification_types"("notification_type_id"),
	FOREIGN KEY("recipe_id") REFERENCES "recipes"("recipe_id")
);

REPLACE INTO "notification_types" ("notification_type_id","notification_name","description","notification_class_id") VALUES
 (1,'GRINDER_CHAMBER_OPENED','The grinder chamber is opened. Motor cannot be actuated. No grind action possible.',3),
 (2,'HOPPER_DISMOUNTED','The grinder chamber is opened. Motor cannot be actuated. No grind action possible.',3),
 (3,'ESP_INTERFACE_FAILED','Communication between HMI and WiFi board is interrupted. No grind actions possible. WebServer is not available.',3),
 (4,'MCU_INTERFACE_FAILED','Communication between HMI board and motor control unit is interrupted. No grind actions possible. ',3),
 (5,'DDD1_FAILED','DDD 1 (coarse) sensor is not available or sensor data invalid. DDD display is invalid and AGSA deactivated.',3),
 (6,'AGSA_FAILED_BLOCKAGE','AGSA motor drive has been stopped because of a blockage within drive system. Try to repeat action.',2),
 (7,'CHANGE_DISCS_INFO','Grinding discs life time has been exceeded. Exchange grinding discs and reset counter.',1),
 (8,'START_GRIND_FAILED','Grinding motor hasn''t started although commanded to do so. Maybe grinding motor is blocked or communication disturbed?',3),
 (9,'STOP_GRIND_FAILED','Grinding motor hasn''t stopped although commanded to do so. Maybe communication to motor control unit disturbed?',3),
 (10,'SYSTEM_TIME_LOST','System clock has been resetted due to a long period power off. Please set the current date and time within system settings menu. The system provides a clock powering of max. 2 weeks.',2),
 (11,'DDD2_FAILED','DDD 2 (fine) sensor is not available or sensor data invalid. DDD display is invalid and AGSA deactivated.',3),
 (12,'AGSA_FAILED_TIMEOUT','AGSA motor drive has been stopped by timeout, because a complete AGSA move reaches timeout. Try to repeat action.',2),
 (13,'SW_UPDATE_UPLOAD_FAILED','Upload of software update package failed. Try to repeat SW update.',2),
 (14,'SW_UPDATE_PACKAGE_FAILED','Consitency check of SW update package failed. Try to repeat SW update or validate package source.',3),
 (15,'SW_UPDATE_PACKAGE_NO_UPGRADE','Software update package rejected. Package contains no upgrade version.',2),
 (16,'SW_UPDATE_UNPACK_ROOTFS_FAILED','Software update failed. Unpacking image #1 of 2 failed. Try to repeat SW update.',3),
 (17,'SW_UPDATE_UNPACK_APPLICATION_FAILED','Software update failed. Unpacking image #2 of 2 failed. Try to repeat SW update.',3),
 (18,'STARTUP_ROOTFS_VERSION_FAILED','Check of core system version information failed. Repeat SW update to recover.',3),
 (19,'LOAD_SETTINGS_FAILED','Loading of configuration data failed. Try to reconfigure the grinder.',3),
 (20,'SW_UPDATE_MCU_UPDATE_FAILED','Software update of MCU failed. System will be rolled back to previous state. Try to repeat SW update.',3),
 (21,'SW_UPDATE_ESP_UPDATE_FAILED','Software update of WiFi board failed. System will be rolled back to previous state. Try to repeat SW update.',3),
 (22,'SW_UPDATE_UPLOAD_ABORTED','Upload of software update package has been aborted by user. Try to repeat SW update.',2),
 (23,'SW_UPDATE_ROLLED_BACK','System has been rolled back to previous state after failed SW update. Try to repeat SW update.',1),
 (24,'SW_UPDATE_ROLLBACK_APP_FAILED','Rollback of image #2 of 2 to previous state failed after failed SW update. System is probably inconsistent. Try to repeat SW update.',3),
 (25,'SW_UPDATE_ROLLBACK_ROOTFS_FAILED','Rollback of image #1 of 2 to previous state failed after failed SW update. System is probably inconsistent. Try to repeat SW update.',3),
 (26,'SW_UPDATE_COMPLETED','Software update has been successfully completed.',1),
 (27,'SW_UPDATE_ROLLBACK_FAILED','Rollback to previous system state failed after failed SW update. Try to repeat SW update.',3),
 (28,'STARTUP_ESP_CHECK_VERSION_TIMEOUT','Read of WiFi board version information timed out.',3),
 (29,'STARTUP_MCU_CHECK_VERSION_TIMEOUT','Read of MCU version information timed out.',3),
 (30,'SW_UPDATE_ESP_UPDATE_TIMEOUT','Software update of WiFi board timed out. System will be rolled back to previous state. Try to repeat SW update.',3),
 (31,'SW_UPDATE_MCU_UPDATE_TIMEOUT','Software update of MCU timed out. System will be rolled back to previous state. Try to repeat SW update.',3),
 (32,'SW_UPDATE_ESP_VALIDATE_VERSION_TIMEOUT','Validation of WiFi board version timed out after software update. System will be rolled back to previous state. Try to repeat SW update.',3),
 (33,'SW_UPDATE_MCU_VALIDATE_VERSION_TIMEOUT','Validation of MCU version timed out after software update. System will be rolled back to previous state. Try to repeat SW update.',3),
 (34,'AGSA_FAILED_NO_DDD','AGSA motor drive has been stopped because at least one DDD sensor is failed.',3),
 (35,'ROLLBACK_STARTUP_FAILED','Rollback to previous system state failed after a failed SW update. Try to repeat SW update.',3),
 (36,'SW_UPDATE_STARTUP_FAILED','System startup failed due to previous error. Maybe restart grinder.',1),
 (37,'SW_UPDATE_MCU_VALIDATE_VERSION_FAILED','Validation of MCU version failed after software update. System will be rolled back to previous state. Try to repeat SW update.',3),
 (38,'SW_UPDATE_ESP_VALIDATE_VERSION_FAILED','Validation of WiFi board version failed after software update. System will be rolled back to previous state. Try to repeat SW update.',3),
 (39,'DB_UPDATE_FAILED','Internal database error. Database update failed.',3),
 (40,'SAVE_SETTINGS_FAILED','Save of configuration file failed. Changes are lost.',3),
 (41,'DB_QUERY_STATISTICS_RESULT','Internal database error. Query of statistics database result failed.',2),
 (42,'DB_CHANGE_RECIPE','Internal database error. Inserting a recipe change failed. ',2),
 (43,'DB_INSERT_RECIPE','Internal database error. Insertion of a new recipe failed.',2),
 (44,'DB_ADD_GRIND','Internal database error. Adding a grind result into database failed.',2),
 (45,'DB_ADD_DELTA_TO_COUNTER','Internal database error. Incrementing of a counter has been failed.',2),
 (46,'DB_RESET_COUNTER','Internal database error. Reset of counters failed.',2),
 (47,'DB_QUERY_COUNTER','Internal database error. Database request for a counter failed.',2),
 (48,'DB_CREATE_NEW_SCHEME','A new database scheme has been successfully activated. ',1),
 (49,'DB_CREATE_NEW_SCHEME_FAILED','Internal database error. Buildup of a new database failed, the scheme couldn''t be applied.',3),
 (50,'MAIN_STATE_FILE_CORRUPTED','Internal state file corrupted. Maybe inconsistent grinder behaviour.',3),
 (51,'SW_UPDATE_STATE_FILE_CORRUPTED','Internal SW update state file corrupted. System state reset to normal.',3),
 (52,'MAIN_STATE_FILE_READ_FAILED','Internal state file couldn''t be read and will be resetted. Maybe inconsistent grinder behaviour.',3),
 (53,'MAIN_STATE_FILE_WRITE_FAILED','Internal state file couldn''t be written and will be resetted. Maybe inconsistent grinder behaviour.',3),
 (54,'SW_UPDATE_STATE_FILE_WRITE','Internal SW update state file couldn''t be written. System state reset to normal.',3),
 (55,'PDU_INSTALLED_AFTER_ABSENCE','PDU has been automatically detected as installed after being absent.',1),
 (56,'PDU_UNINSTALLED_AFTER_PRESENCE','PDU has been automatically detected as not installed after being present.',1);

REPLACE INTO "notification_classes" ("notification_class_id","notification_class") VALUES
 (0,'None'),
 (1,'Info'),
 (2,'Warning'),
 (3,'Error');

REPLACE INTO `settings` (`key`, `value`) VALUES ( 'Version', '7' );
