BEGIN TRANSACTION;
CREATE TABLE IF NOT EXISTS "events" (
	"event_id"	INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
	"timestamp"	DATETIME NOT NULL,
	"event_type_id"	INTEGER NOT NULL,
	"recipe_id"	INTEGER,
	"notification_type_id"	INTEGER,
	"context"	VARCHAR(50),
	FOREIGN KEY("notification_type_id") REFERENCES "notification_types"("notification_type_id"),
	FOREIGN KEY("event_type_id") REFERENCES "event_types"("event_type_id")
);
CREATE TABLE IF NOT EXISTS "notification_types" (
	"notification_type_id"	INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
	"notification_name"	VARCHAR(30) NOT NULL UNIQUE,
	"description"	VARCHAR(255),
	"notification_class_id"	INTEGER NOT NULL,
	"recover_action"	INTEGER,
	"short_info"	VARCHAR(50),
	"long_info"	VARCHAR(255)
);
CREATE TABLE IF NOT EXISTS "notification_classes" (
	"notification_class_id"	INTEGER NOT NULL,
	"notification_class"	varchar(10) NOT NULL,
	PRIMARY KEY("notification_class_id")
);
CREATE TABLE IF NOT EXISTS "event_types" (
	"event_type_id"	INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
	"event_name"	VARCHAR(20) NOT NULL UNIQUE,
	"description"	VARCHAR(255)
);
CREATE TABLE IF NOT EXISTS "units" (
	"unit_id"	INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
	"unit_name"	VARCHAR(20) NOT NULL UNIQUE,
	"label"	VARCHAR(20) NOT NULL
);
CREATE TABLE IF NOT EXISTS "recipes" (
	"recipe_id"	INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
	"recipe_uuid"	VARCHAR(37) NOT NULL UNIQUE,
	"recipe_name"	VARCHAR(30) NOT NULL,
	"recipe_mode_id"	INTEGER(1) NOT NULL,
	"porta_filter_index"	INTEGER(1),
	FOREIGN KEY("recipe_mode_id") REFERENCES "recipe_modes"("recipe_mode_id")
);
CREATE TABLE IF NOT EXISTS "recipe_modes" (
	"recipe_mode_id"	INTEGER NOT NULL,
	"recipe_mode_name"	VARCHAR(20) NOT NULL UNIQUE,
	"description"	VARCHAR(30),
	PRIMARY KEY("recipe_mode_id")
);
CREATE TABLE IF NOT EXISTS "counters" (
	"counter_id"	INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
	"counter_name"	VARCHAR(20) NOT NULL UNIQUE,
	"unit_id"	INTEGER NOT NULL,
	"value"	BIGINT NOT NULL,
	FOREIGN KEY("unit_id") REFERENCES "units"("unit_id")
);
CREATE TABLE IF NOT EXISTS "grinds" (
	"grind_id"	INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
	"recipe_id"	INTEGER NOT NULL,
	"timestamp"	DATETIME NOT NULL,
	"grind_time_100ths"	BIGINT NOT NULL,
	FOREIGN KEY("recipe_id") REFERENCES "recipes"
);
CREATE TABLE IF NOT EXISTS "settings" (
	"key"	varchar(30) NOT NULL UNIQUE,
	"value"	varchar(30),
	PRIMARY KEY("key")
);
INSERT INTO "notification_types" ("notification_type_id","notification_name","description","notification_class_id","recover_action","short_info","long_info") VALUES (1,'GRINDER_CHAMBER_OPENED','The grinder chamber is opened. Motor cannot be actuated. No grind action possible.',3,0,'Open Grinder Chamber','Grinder chamber\nopened!'),
 (2,'HOPPER_DISMOUNTED','The bean hopper has been dismounted. Motor cannot be actuated. No grind action possible.',3,0,'No Bean Hopper','Bean hopper\ndismounted!'),
 (3,'ESP_INTERFACE_FAILED','Communication between HMI and driver board is interrupted. No grind actions possible. WebServer is not available.',3,1,'Driver Comm Failed','Driver board\ncommunication\nfailed!'),
 (4,'MCU_INTERFACE_FAILED','Communication between HMI board and motor control unit is interrupted. No grind actions possible. ',3,1,'MCU Comm Failed','MCU board\ncommunication\nfailed!'),
 (5,'DDD1_FAILED','DDD 1 (coarse) sensor is not available or sensor data invalid. DDD display is invalid and AGSA deactivated.',3,0,'DDD 1 Failed!','DDD 1 sensor\nfailed!'),
 (6,'AGSA_FAILED_BLOCKAGE','AGSA motor drive has been stopped because of a blockage within drive system. Try to repeat action.',2,0,'AGSA Blockage','AGSA driver\nfailed!\nBlockage!'),
 (7,'CHANGE_DISCS_INFO','Grinding discs life time has been exceeded. Exchange grinding discs and reset counter.',1,0,'Grinding Discs','Change grinding\ndiscs!'),
 (8,'START_GRIND_FAILED','Grinding motor hasn''t started although commanded to do so. Maybe grinding motor is blocked or communication disturbed?',3,0,'Start Grind Failed','Start grind\nfailed!'),
 (9,'STOP_GRIND_FAILED','Grinding motor hasn''t stopped although commanded to do so. Maybe communication to motor control unit disturbed?',3,0,'Stop Grind Failed','Stop grind\nfailed!'),
 (10,'SYSTEM_TIME_LOST','System clock has been resetted due to a long period power off. Please set the current date and time within system settings menu. The system provides a clock powering of max. 2 weeks.',2,1,'System Time Lost','System time\nlost, please\nset manually!'),
 (11,'DDD2_FAILED','DDD 2 (fine) sensor is not available or sensor data invalid. DDD display is invalid and AGSA deactivated.',3,0,'DDD 2 Failed!','DDD 2 sensor\nfailed!'),
 (12,'AGSA_FAILED_TIMEOUT','AGSA motor drive has been stopped by timeout, because a complete AGSA move reaches timeout. Try to repeat action.',2,0,'AGSA Timeout','AGSA driver\nfailed!\nTimeout!'),
 (13,'SW_UPDATE_UPLOAD_FAILED','Upload of software update package failed. Try to repeat SW update.',2,0,'SW Update Failed','Upload of update\npackage failed!'),
 (14,'SW_UPDATE_PACKAGE_FAILED','Consitency check of SW update package failed. Try to repeat SW update or validate package source.',3,0,'SW Update Failed','File is no valid\nupdate package!'),
 (15,'SW_UPDATE_PACKAGE_NO_UPGRADE','Software update package rejected. Package contains no upgrade version.',2,0,'SW Update Failed','Update package\nis no upgrade!'),
 (16,'SW_UPDATE_UNPACK_ROOTFS_FAILED','Software update failed. Unpacking image #1 of 2 failed. Try to repeat SW update.',3,0,'SW Update Failed','Unpacking image\n#1 failed!'),
 (17,'SW_UPDATE_UNPACK_APPLICATION_FAILED','Software update failed. Unpacking image #2 of 2 failed. Try to repeat SW update.',3,0,'SW Update Failed','Unpacking image\n#2 failed!'),
 (18,'STARTUP_ROOTFS_VERSION_FAILED','Check of core system version information failed. Repeat SW update to recover.',3,0,'Startup Failed','Startup check of\nimage #1 failed!'),
 (19,'LOAD_SETTINGS_FAILED','Loading of configuration data failed. Try to reconfigure the grinder.',3,0,'Load Settings Failed','Load settings\nfailed!'),
 (20,'SW_UPDATE_MCU_UPDATE_FAILED','Software update of MCU failed. System will be rolled back to previous state. Try to repeat SW update.',3,0,'SW Update Failed','Update of MCU\nboard failed!'),
 (21,'SW_UPDATE_ESP_UPDATE_FAILED','Software update of driver board failed. System will be rolled back to previous state. Try to repeat SW update.',3,0,'SW Update Failed','Update of driver\nboard failed!'),
 (22,'SW_UPDATE_UPLOAD_ABORTED','Upload of software update package has been aborted by user. Try to repeat SW update.',2,0,'SW Update Failed','Upload aborted\nby user!'),
 (23,'SW_UPDATE_ROLLED_BACK','System has been rolled back to previous state after failed SW update. Try to repeat SW update.',1,0,'Rollback','Perform rollback\nafter failed\nSW update!'),
 (24,'SW_UPDATE_ROLLBACK_APP_FAILED','Rollback of image #2 of 2 to previous state failed after failed SW update. System is probably inconsistent. Try to repeat SW update.',3,0,'Rollback Failed','Rollback of\nimage #2 failed!'),
 (25,'SW_UPDATE_ROLLBACK_ROOTFS_FAILED','Rollback of image #1 of 2 to previous state failed after failed SW update. System is probably inconsistent. Try to repeat SW update.',3,0,'Rollback Failed','Rollback of\nimage #1 failed!'),
 (26,'SW_UPDATE_COMPLETED','Software update has been successfully completed.',1,0,'SW Update Completed','Software update\nsuccessfully\ncompleted!'),
 (27,'SW_UPDATE_ROLLBACK_FAILED','Rollback to previous system state failed after failed SW update. Try to repeat SW update.',3,0,'Rollback Failed','Rollback after\nSW update failed!'),
 (28,'STARTUP_ESP_CHECK_VERSION_TIMEOUT','Read of driver board version information timed out.',3,0,'Startup Failed','Driver board SW\nversion check failed!\nTimeout!'),
 (29,'STARTUP_MCU_CHECK_VERSION_TIMEOUT','Read of MCU version information timed out.',3,0,'Startup Failed','MCU board SW\nversion check failed!\nTimeout!'),
 (30,'SW_UPDATE_ESP_UPDATE_TIMEOUT','Software update of driver board timed out. System will be rolled back to previous state. Try to repeat SW update.',3,0,'SW Update Failed','Timeout while\nupdating driver board!'),
 (31,'SW_UPDATE_MCU_UPDATE_TIMEOUT','Software update of MCU timed out. System will be rolled back to previous state. Try to repeat SW update.',3,0,'SW Update Failed','Timeout while\nupdating MCU board!'),
 (32,'SW_UPDATE_ESP_VALIDATE_VERSION_TIMEOUT','Validation of driver board version timed out after software update. System will be rolled back to previous state. Try to repeat SW update.',3,0,'SW Update Failed','Timeout while\nvalidating driver board\nSW version!'),
 (33,'SW_UPDATE_MCU_VALIDATE_VERSION_TIMEOUT','Validation of MCU version timed out after software update. System will be rolled back to previous state. Try to repeat SW update.',3,0,'SW Update Failed','Timeout while\nvalidating MCU board\nSW version!'),
 (34,'AGSA_FAILED_NO_DDD','AGSA motor drive has been stopped because at least one DDD sensor is failed.',3,0,'AGSA No DDD','AGSA driver\nfailed!\nNo DDD!'),
 (35,'ROLLBACK_STARTUP_FAILED','Rollback to previous system state failed after a failed SW update. Try to repeat SW update.',3,0,'Startup Failed','Rollback startup\nfailed!'),
 (36,'SW_UPDATE_STARTUP_FAILED','System startup failed due to previous error. Maybe restart grinder.',1,0,'Startup Failed','System startup\nfailed due to\nprevious errors!'),
 (37,'SW_UPDATE_MCU_VALIDATE_VERSION_FAILED','Validation of MCU version failed after software update. System will be rolled back to previous state. Try to repeat SW update.',3,0,'SW Update Failed','MCU board SW\nversion validation\nfailed!'),
 (38,'SW_UPDATE_ESP_VALIDATE_VERSION_FAILED','Validation of driver board version failed after software update. System will be rolled back to previous state. Try to repeat SW update.',3,0,'SW Update Failed','Driver board SW\nversion validation\nfailed!'),
 (39,'DB_UPDATE_FAILED','Internal database error. Database update failed.',3,0,'DB Update Failed','Database update\nfailed!'),
 (40,'SAVE_SETTINGS_FAILED','Save of configuration file failed. Changes are lost.',3,1,'Save Settings Failed','Save settings\nfailed!'),
 (41,'DB_QUERY_STATISTICS_RESULT','Internal database error. Query of statistics database result failed.',2,0,'DB Query Stat. Failed','Database access\nquery statistics\nresult failed!'),
 (42,'DB_CHANGE_RECIPE','Internal database error. Inserting a recipe change failed. ',2,0,'DB Chg.Recipe Failed','Database access\nchange recipe\nfailed!'),
 (43,'DB_INSERT_RECIPE','Internal database error. Insertion of a new recipe failed.',2,0,'DB Ins.Recipe Failed','Database access\ninsert recipe\nfailed!'),
 (44,'DB_ADD_GRIND','Internal database error. Adding a grind result into database failed.',2,0,'DB Add Grind Failed','Database access\nadd grind\nfailed!'),
 (45,'DB_ADD_DELTA_TO_COUNTER','Internal database error. Incrementing of a counter has been failed.',2,0,'DB Delta.Cnt. Failed','Database access\ndelta to counter\nfailed!'),
 (46,'DB_RESET_COUNTER','Internal database error. Reset of counters failed.',2,0,'DB Reset.Cnt. Failed','Database access\nreset counter\nfailed!'),
 (47,'DB_QUERY_COUNTER','Internal database error. Database request for a counter failed.',2,0,'DB Delta.Cnt. Failed','Database access\nquery counter\nfailed!'),
 (48,'DB_CREATE_NEW_SCHEME','A new database scheme has been successfully activated. ',1,0,'Create New DB','New database\ncreated!'),
 (49,'DB_CREATE_NEW_SCHEME_FAILED','Internal database error. Buildup of a new database failed, the scheme couldn''t be applied.',3,0,'Create New DB Failed','Create new\ndatabase\nfailed!'),
 (50,'MAIN_STATE_FILE_CORRUPTED','Internal state file corrupted. Maybe inconsistent grinder behaviour.',3,0,'State File Corrupt','Main state file\ncorrupted!'),
 (51,'SW_UPDATE_STATE_FILE_CORRUPTED','Internal SW update state file corrupted. System state reset to normal.',3,0,'State File Corrupt','Software update\nstate file\ncorrupted!'),
 (52,'MAIN_STATE_FILE_READ_FAILED','Internal state file couldn''t be read and will be resetted. Maybe inconsistent grinder behaviour.',3,0,'State File Read','Main state file\nread failed!'),
 (53,'MAIN_STATE_FILE_WRITE_FAILED','Internal state file couldn''t be written and will be resetted. Maybe inconsistent grinder behaviour.',3,0,'State File Write','Main state file\nwrite failed!'),
 (54,'SW_UPDATE_STATE_FILE_WRITE','Internal SW update state file couldn''t be written. System state reset to normal.',3,0,'State File Write','Software update\nstate file\nwrite failed!'),
 (55,'PDU_INSTALLED_AFTER_ABSENCE','PDU has been automatically detected as installed after being absent.',1,0,'PDU Detected','PDU detected\nafter being absent!'),
 (56,'PDU_UNINSTALLED_AFTER_PRESENCE','PDU has been automatically detected as not installed after being present.',1,0,'PDU Not Detected','PDU not detected\nafter being present!'),
 (57,'AGSA_FAULT_PIN_ACTIVE','AGSA stepper motor driver chip indicates pin fault. Retry AGSA action or contact service if still present.',3,0,'AGSA Fault Pin','AGSA stepper\nmotor driver pin\nindicates fault!'),
 (58,'PDU_FAULT_PIN_ACTIVE','PDU motor driver chip indicates pin fault. Retry PDU action or contact service if still present.',3,0,'PDU Fault Pin','PDU motor\ndriver pin\nindicates fault!');
INSERT INTO "notification_classes" ("notification_class_id","notification_class") VALUES (0,'None'),
 (1,'Info'),
 (2,'Warning'),
 (3,'Error');
INSERT INTO "event_types" ("event_type_id","event_name","description") VALUES (1,'SYSTEM_RESTART','cold restart of grinder'),
 (2,'ERROR_RAISE','event when a certain error is raised: timestamp and error type'),
 (3,'ERROR_GONE','event when a certain error has gone: timestamp and error type'),
 (4,'SW_UPDATE_PACKAGE','software update successfully completed, old package version, new package version'),
 (5,'RECIPE_CHANGE',NULL),
 (6,'TECHNICIAN_LOGIN',NULL),
 (7,'OWNER_PIN_CHANGE',NULL),
 (8,'TECHNICIAN_PIN_CHANGE',NULL),
 (9,'OWNER_LOGOUT',NULL),
 (10,'OWNER_LOGIN',NULL),
 (11,'TECHNICIAN_LOGOUT',NULL),
 (12,'DISC_USAGE_TIME_RESET',NULL),
 (13,'DDD_CALIBRATE',NULL),
 (14,'RECIPE_DELETE',NULL),
 (15,'RECIPE_ADD',NULL),
 (16,'FACTORY_RESET',NULL),
 (17,'CHANGE_TEMP_WARNING',NULL),
 (18,'CHANGE_SHOW_TEMP',NULL),
 (19,'CHANGE_BRIGHTNESS',NULL),
 (20,'CHANGE_STANDBY_TIME',NULL),
 (21,'CHANGE_SHOW_SPOTLIGHT',NULL),
 (22,'CHANGE_SHOW_DDD',NULL),
 (23,'CHANGE_SHOW_LIBRARY_MODE',NULL),
 (24,'CHANGE_SHOW_TIME_MODE',NULL),
 (25,'CHANGE_GRINDER_NAME',NULL),
 (26,'CHANGE_TEMP_UNIT',NULL),
 (27,'CHANGE_TIME',NULL),
 (28,'CHANGE_DATE',NULL),
 (29,'CHANGE_WIFI_SETTINGS',NULL),
 (30,'CHANGE_TEMP_THRESHOLD',NULL),
 (31,'CHANGE_WIFI_ON',NULL),
 (32,'CHANGE_AUTO_LOGOUT',NULL),
 (33,'CHANGE_START_OF_WEEK',NULL),
 (34,'SW_UPDATE_ESP',NULL),
 (35,'SW_UPDATE_MCU',NULL),
 (36,'CHANGE_AGSA_DELAY',NULL),
 (37,'CHANGE_SETTINGS_MODE_EXIT_TIME',NULL),
 (38,'CHANGE_MCU_MAX_MOTOR_SPEED',NULL),
 (39,'CHANGE_MCU_NOMINAL_MOTOR_SPEED',NULL),
 (40,'CHANGE_MCU_ACCELERATION_TIME',NULL),
 (41,'CHANGE_MCU_DECELERATION_TIME',NULL),
 (42,'SYSTEM_SETUP_CONFIRMED',NULL),
 (43,'CHANGE_ENABLE_AGSA',NULL),
 (44,'CHANGE_SHOW_DOSING_TIME',NULL);
INSERT INTO "units" ("unit_id","unit_name","label") VALUES (1,'MILLISECONDS','[ms]'),
 (2,'SECONDS','[s]'),
 (3,'10TH_SECSONDS','[1/10s]'),
 (4,'100TH_SECONDS','[1/100s]'),
 (5,'AMOUNT','[x]');
INSERT INTO "recipes" ("recipe_id","recipe_uuid","recipe_name","recipe_mode_id","porta_filter_index") VALUES (1,'52659fdc-7f2e-4ec1-a23c-d82a0d9fbd1f','Classic',0,NULL),
 (2,'1efe7671-b365-4a3c-8cbb-90f0cf6a7baf','Recipe 1',1,0),
 (3,'a58a9991-49c8-4efa-afb8-3b75f85d4f1f','Recipe 2',1,1),
 (4,'d99006ac-a8f4-415a-96f0-7067373fd8d8','Recipe 3',1,2),
 (5,'d03abbec-1b89-4661-af39-cfaf1aab8e4d','Recipe 4',1,NULL),
 (6,'d812779f-431b-44c1-ac2f-82ca9471ea40','Recipe 5',1,NULL),
 (7,'6db9c354-eb34-4e5d-8220-a152cb8c5e0c','Recipe 6',1,NULL),
 (8,'32fac243-8ed7-47ba-bf57-a9cba8f0e3cf','El Puente',2,NULL),
 (9,'4e379882-c2f0-4b36-9cd1-cdc67d768b1b','Burundi Bwayi',2,NULL),
 (10,'816dba71-dcbb-46fc-878b-f14fd826d2bc','Mischgold',2,NULL),
 (11,'b57afa23-b02e-4d38-99a1-63a2cc109c51','La Dalia',2,NULL);
INSERT INTO "recipe_modes" ("recipe_mode_id","recipe_mode_name","description") VALUES (0,'CLASSIC','CLASSIC MODE'),
 (1,'TIME','TIME MODE'),
 (2,'LIBRARY','LIBRARY MODE');
INSERT INTO "counters" ("counter_id","counter_name","unit_id","value") VALUES (1,'TOTAL_MOTOR_ON_TIME',4,0),
 (2,'DISC_USAGE_TIME',4,0),
 (3,'STANDBY_TIME',2,0),
 (4,'SYSTEM_ON_TIME',2,0),
 (5,'TOTAL_GRIND_SHOTS',5,0),
 (6,'TOTAL_GRIND_TIME',4,0);
INSERT INTO "settings" ("key","value") VALUES ('Version','13');
COMMIT;
