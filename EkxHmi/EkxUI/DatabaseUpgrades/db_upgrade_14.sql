
REPLACE INTO "notification_types" ("notification_type_id","notification_name","description","notification_class_id","recover_action","short_info","long_info") VALUES
(55,'PDU_INSTALLED_AFTER_ABSENCE','PDU has been automatically detected as installed after being absent.',1,0,'PDU Detected','PDU detected\nafter being absent!'),
(56,'PDU_UNINSTALLED_AFTER_PRESENCE','PDU has been automatically detected as not installed after being present.',1,0,'PDU Not Detected','PDU not detected\nafter being present!');

REPLACE INTO `settings` (`key`, `value`) VALUES ( 'Version', '14' );
