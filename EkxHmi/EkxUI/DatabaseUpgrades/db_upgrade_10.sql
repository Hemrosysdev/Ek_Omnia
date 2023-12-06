
REPLACE INTO "notification_types" ("notification_type_id","notification_name","description","notification_class_id","recover_action","short_info","long_info") VALUES
 (57,'AGSA_FAULT_PIN_ACTIVE','AGSA stepper motor driver chip indicates pin fault. Retry AGSA action or contact service if still present.',3,0,'AGSA Fault Pin','AGSA stepper\nmotor driver pin\nindicates fault!'),
 (58,'PDU_FAULT_PIN_ACTIVE','PDU motor driver chip indicates pin fault. Retry PDU action or contact service if still present.',3,0,'PDU Fault Pin','PDU motor\ndriver pin\nindicates fault!');


REPLACE INTO `settings` (`key`, `value`) VALUES ( 'Version', '10' );
