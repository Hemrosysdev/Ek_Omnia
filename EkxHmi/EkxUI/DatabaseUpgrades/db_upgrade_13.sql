
REPLACE INTO "event_types" ("event_type_id","event_name","description") VALUES
 (42,'SYSTEM_SETUP_CONFIRMED',NULL),
 (43,'CHANGE_ENABLE_AGSA',NULL),
 (44,'CHANGE_SHOW_DOSING_TIME',NULL);

REPLACE INTO `settings` (`key`, `value`) VALUES ( 'Version', '13' );
