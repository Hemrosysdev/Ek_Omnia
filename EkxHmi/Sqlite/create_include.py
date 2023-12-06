#! /usr/bin/python

import sqlite3
from sqlite3 import Error

def create_connection(db_file):
    """ create a database connection to the SQLite database
        specified by the db_file
    :param db_file: database file
    :return: Connection object or None
    """
    conn = None
    try:
        conn = sqlite3.connect(db_file)
    except Error as e:
        print(e)

    return conn


def query_event_types(conn,inc_file):
    """
    Query all rows in the tasks table
    :param conn: the Connection object
    :return:
    """
    cur = conn.cursor()
    cur.execute( "SELECT event_name, event_type_id FROM event_types order by event_name;")

    rows = cur.fetchall()

    inc_file.write( "enum SqliteEventTypes\n" )
    inc_file.write( "{\n" )
    inc_file.write( f"    SqliteEventType_UNKNOWN = 0,\n" )

    last = 0
    for row in rows:
        inc_file.write( f"    SqliteEventType_{row[0]} = {row[1]},\n" )
        last = max( last, row[1] )

    last = last + 1
    inc_file.write( f"    SqliteEventType_LAST = {last}\n" )
    inc_file.write( "};\n" )
    inc_file.write( "Q_ENUM_NS( SqliteEventTypes )\n" )
    inc_file.write( "\n" )

def query_version(conn,inc_file):
    """
    Query all rows in the tasks table
    :param conn: the Connection object
    :return:
    """
    cur = conn.cursor()
    cur.execute( "SELECT key, value FROM settings where key='Version';")

    rows = cur.fetchall()

    inc_file.write( "enum SqliteVersion\n" )
    inc_file.write( "{\n" )

    for row in rows:
        inc_file.write( f"    SqliteVersionIndex = {row[1]},\n" )

    inc_file.write( "};\n" )
    inc_file.write( "\n" )

def query_notification_types(conn,inc_file):
    """
    Query all rows in the tasks table
    :param conn: the Connection object
    :return:
    """
    cur = conn.cursor()
    cur.execute( "SELECT notification_name, notification_type_id FROM notification_types order by notification_name;")

    rows = cur.fetchall()

    inc_file.write( "enum SqliteNotificationType\n" )
    inc_file.write( "{\n" )
    inc_file.write( f"    SqliteNotificationType_UNKNOWN = 0,\n" )

    last = 0
    for row in rows:
        inc_file.write( f"    SqliteNotificationType_{row[0]} = {row[1]},\n" )
        last = max( last, row[1] )

    last = last + 1
    inc_file.write( f"    SqliteNotificationType_LAST = {last}\n" )
    inc_file.write( "};\n" )
    inc_file.write( "Q_ENUM_NS( SqliteNotificationType )\n" )
    inc_file.write( "\n" )

def query_notifications_impl(conn,inc_file):
    """
    Query all rows in the tasks table
    :param conn: the Connection object
    :return:
    """
    cur = conn.cursor()
    cur.execute( "SELECT notification_type_id, notification_name, notification_class, description, short_info, long_info, recover_action FROM notification_types, notification_classes where notification_types.notification_class_id = notification_classes.notification_class_id order by notification_type_id;")

    rows = cur.fetchall()

    inc_file.write( "const SqliteNotificationDefinition m_theNotifications[] =\n" )
    inc_file.write( "{\n" )
    inc_file.write( "    {\n" )
    inc_file.write( f"        SqliteNotificationType_UNKNOWN,\n" )
    inc_file.write( f"        \"UNKNOWN\",\n" )
    inc_file.write( f"        \"UNKNOWN\",\n" )
    inc_file.write( f"        \"UNKNOWN\",\n" )
    inc_file.write( f"        \"UNKNOWN\",\n" )
    inc_file.write( f"        false,\n" )
    inc_file.write( f"        SqliteNotificationClass::None\n" )
    inc_file.write( "    },\n" )

    for row in rows:
        inc_file.write( "    {\n" )
        inc_file.write( f"        SqliteNotificationType_{row[1]},\n" )
        inc_file.write( f"        \"{row[1]}\",\n" )
        inc_file.write( f"        \"{row[3]}\",\n" )
        inc_file.write( f"        \"{row[4]}\",\n" )
        inc_file.write( f"        \"{row[5]}\",\n" )
        if row[6] == 1:
            inc_file.write( f"        true,\n" )
        else:
            inc_file.write( f"        false,\n" )
        inc_file.write( f"        SqliteNotificationClass::{row[2]}\n" )
        inc_file.write( "    },\n" )

    inc_file.write( "};\n" )
    inc_file.write( "\n" )

def query_notification_classes(conn,inc_file):
    """
    Query all rows in the tasks table
    :param conn: the Connection object
    :return:
    """
    cur = conn.cursor()
    cur.execute( "SELECT notification_class_id, notification_class FROM notification_classes order by notification_class_id;")

    rows = cur.fetchall()

    inc_file.write( "enum SqliteNotificationClass\n" )
    inc_file.write( "{\n" )

    last = 0
    for row in rows:
        inc_file.write( f"    {row[1]} = {row[0]},\n" )
        last = max( last, row[0] )

    last = last + 1
    inc_file.write( f"    Last = {last}\n" )
    inc_file.write( "};\n" )
    inc_file.write( "Q_ENUM_NS( SqliteNotificationClass )\n" )
    inc_file.write( "\n" )

def query_recipe_modes(conn,inc_file):
    """
    Query all rows in the tasks table
    :param conn: the Connection object
    :return:
    """
    cur = conn.cursor()
    cur.execute( "SELECT recipe_mode_name, recipe_mode_id FROM recipe_modes order by recipe_mode_name;")

    rows = cur.fetchall()

    inc_file.write( "enum SqliteRecipeModes\n" )
    inc_file.write( "{\n" )

    last = 0
    for row in rows:
        inc_file.write( f"    SqliteRecipeMode_{row[0]} = {row[1]},\n" )
        last = max( last, row[1] )

    last = last + 1
    inc_file.write( f"    SqliteRecipeMode_LAST = {last}\n" )
    inc_file.write( "};\n" )
    inc_file.write( "Q_ENUM_NS( SqliteRecipeModes )\n" )
    inc_file.write( "\n" )

def query_counters(conn,inc_file):
    """
    Query all rows in the tasks table
    :param conn: the Connection object
    :return:
    """
    cur = conn.cursor()
    cur.execute( "SELECT counter_name, counter_id, label FROM counters left join units on counters.unit_id = units.unit_id order by counter_name;")

    rows = cur.fetchall()

    inc_file.write( "enum SqliteCounters\n" )
    inc_file.write( "{\n" )
    inc_file.write( f"    SqliteCounter_UNKNOWN = 0,\n" )

    last = 0
    for row in rows:
        inc_file.write( f"    SqliteCounter_{row[0]} = {row[1]},   // unit {row[2]}\n" )
        last = max( last, row[1] )

    last = last + 1
    inc_file.write( f"    SqliteCounter_LAST = {last}\n" )
    inc_file.write( "};\n" )
    inc_file.write( "Q_ENUM_NS( SqliteCounters )\n" )
    inc_file.write( "\n" )

def query_uuids(conn,inc_file):
    """
    Query all rows in the tasks table
    :param conn: the Connection object
    :return:
    """
    cur = conn.cursor()
    cur.execute( "SELECT recipe_name, recipe_uuid FROM recipes;")

    rows = cur.fetchall()

    for row in rows:
        name = f"{row[0]}".replace( " ", "" ).upper()
        inc_file.write( f"#define RECIPE_UUID_{name}  \"{row[1]}\"\n" )

    inc_file.write( "\n" )

def main():
    database = r"EkxSqliteMaster.db"
    include_file = r"../EkxUI/Application/Database/EkxSqliteTypes.h"
    impl_file = r"../EkxUI/Application/Database/EkxSqliteTypes.cpp"
    
    # create a database connection
    conn = create_connection(database)

    inc_file = open( include_file, "w")
    inc_file.write( "// This file is generated from " + database + "\n" )
    inc_file.write( "// Don't modify it manually. Call script create_include.py instead.\n" )
    inc_file.write( "\n" )
    inc_file.write( "#ifndef EkxSqliteTypes_h\n" )
    inc_file.write( "#define EkxSqliteTypes_h\n" )
    inc_file.write( "\n" )
    inc_file.write( "#include <QObject>\n" )
    inc_file.write( "\n" )
    inc_file.write( "namespace EkxSqliteTypes\n" )
    inc_file.write( "{\n" )
    inc_file.write( "Q_NAMESPACE\n" )
    inc_file.write( "\n" )

    with conn:
        query_version(conn, inc_file)
        query_event_types(conn, inc_file)
        query_notification_types(conn, inc_file)
        query_notification_classes(conn, inc_file)
        query_recipe_modes(conn, inc_file)
        query_counters(conn, inc_file)
        query_uuids(conn, inc_file)

    inc_file.write( "struct SqliteNotificationDefinition\n" )
    inc_file.write( "{\n" )
    inc_file.write( "    SqliteNotificationType    m_nTypeId;\n" )
    inc_file.write( "    QString                   m_strName;\n" )
    inc_file.write( "    QString                   m_strDescription;\n" )
    inc_file.write( "    QString                   m_strShortInfo;\n" )
    inc_file.write( "    QString                   m_strLongInfo;\n" )
    inc_file.write( "    bool                      m_bRecoverAction;\n" )
    inc_file.write( "    SqliteNotificationClass   m_nClassId;\n" )
    inc_file.write( "};\n" )
    inc_file.write( "\n" )
    inc_file.write( "extern const SqliteNotificationDefinition m_theNotifications[];\n" )
    inc_file.write( "\n" )
    inc_file.write( "} // end namespace\n" )
    inc_file.write( "\n" )
    inc_file.write( "#endif // EkxSqliteTypes_h\n" )
    inc_file.close()

    inc_file = open( impl_file, "w")
    inc_file.write( "// This file is generated from " + database + "\n" )
    inc_file.write( "// Don't modify it manually. Call script create_include.py instead.\n" )
    inc_file.write( "\n" )
    inc_file.write( "#include \"EkxSqliteTypes.h\"\n" )
    inc_file.write( "#include <QQmlEngine>\n" )
    inc_file.write( "\n" )
    inc_file.write( "namespace EkxSqliteTypes\n" )
    inc_file.write( "{\n" )
    inc_file.write( "\n" )


    with conn:
        query_notifications_impl(conn, inc_file)

    inc_file.write( "\n" )
    inc_file.write( "} // end namespace\n" )
    inc_file.write( "\n" )
    inc_file.close()

if __name__ == '__main__':
    main()
