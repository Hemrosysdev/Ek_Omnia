drop table if exists `events`;

CREATE TABLE IF NOT EXISTS "events" (
	"event_id"	INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
	"timestamp"	DATETIME NOT NULL,
	"event_type_id"	INTEGER NOT NULL,
	"recipe_id"	INTEGER,
	"notification_type_id"	INTEGER,
	"context"	VARCHAR(50),
	FOREIGN KEY("event_type_id") REFERENCES "event_types"("event_type_id")
);

REPLACE INTO `settings` (`key`, `value`) VALUES ( 'Version', '9' );
