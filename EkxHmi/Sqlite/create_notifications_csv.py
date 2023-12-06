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


def query_notification_types(conn,inc_file):
    """
    Query all rows in the tasks table
    :param conn: the Connection object
    :return:
    """
    cur = conn.cursor()
    cur.execute( "SELECT notification_type_id, notification_name, notification_class, recover_action, short_info, long_info, description FROM notification_types, notification_classes where notification_types.notification_class_id = notification_classes.notification_class_id order by notification_name;")

    rows = cur.fetchall()

    for row in rows:
        inc_file.write( f"{row[0]};{row[1]};{row[2]};{row[3]};{row[4]};{row[5]};{row[6]}\n" )


def main():
    database = r"EkxSqliteMaster.db"
    out_file = r"EkxNotifications.csv"
    
    inc_file = open( out_file, "w")
    inc_file.write( "notification_type_id;notification_name;notification_class;short_info;long_info;description\n" )

    # create a database connection
    conn = create_connection(database)
    with conn:
        query_notification_types(conn, inc_file)

    inc_file.close()

if __name__ == '__main__':
    main()
