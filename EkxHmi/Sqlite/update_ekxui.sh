#! /bin/bash

#sqlite3 EkxSqliteMaster.db .dump > EkxSqliteMaster.sql

cp EkxSqliteMaster.sql ../EkxUI/DatabaseUpgrades
./create_include.py

./create_notifications_csv.py

git diff EkxSqliteMaster.sql > /tmp/db_upgrade_.sql
geany /tmp/db_upgrade_.sql &
