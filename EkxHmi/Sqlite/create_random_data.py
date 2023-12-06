#! /usr/bin/python

import sqlite3
from sqlite3 import Error
from random import random
from random import seed
from random import randint

def create_connection(db_file):
    conn = None
    try:
        conn = sqlite3.connect(db_file)
    except Error as e:
        print(e)

    return conn

def query_insert_events(conn,entryNums):
    cur = conn.cursor()

    for i in range( 1, entryNums ):
        event_type_id = randint( 1, 20 )
        day = randint( 0, 2 * 365 )
        cur.execute( f"insert into events ( event_type_id, timestamp ) values ( {event_type_id}, date( 'now', '-{day} day' ) );")

def query_insert_grinds(conn,entryNums):
    cur = conn.cursor()

    lifetime_grind_time = 0
    for i in range( 1, entryNums ):
        recipe_id = randint( 1, 11 )
        grind_time = randint( 200, 8000 )
        lifetime_grind_time += grind_time
        seconds = randint( 0, 2 * 365 * 24 * 3600 )
        cur.execute( f"insert into grinds ( recipe_id, timestamp, grind_time_100ths ) values ( {recipe_id}, datetime( 'now', '-{seconds} seconds' ), {grind_time} );")

    cur.execute( f"update counters set value={lifetime_grind_time} where counter_name='TOTAL_MOTOR_ON_TIME';")
    cur.execute( f"update counters set value={lifetime_grind_time} where counter_name='DISC_USAGE_TIME';")
    cur.execute( f"update counters set value={lifetime_grind_time} where counter_name='TOTAL_GRIND_TIME';")
    cur.execute( f"update counters set value={entryNums} where counter_name='TOTAL_GRIND_SHOTS';")

def query_empty_db(conn):
    cur = conn.cursor()

    cur.execute( "delete from events;")
    cur.execute( "delete from grinds;")

def main():
    database = r"/home/gesser/EkxData/storage/EkxDatabase/EkxSqlite.db"
    
    #data will be generated within 2 years

    seed()

    # create a database connection
    conn = create_connection(database)
    with conn:
        query_empty_db(conn)
        query_insert_grinds( conn, round( 2 * 365 * 10000 / 7 ) )
        #query_insert_grinds( conn, round( 2 * 365 * 10000 ) )
        query_insert_events(conn, 2 * 365 * 100 )
        #query_insert_errors(conn, 2 * 365 * 5 )

if __name__ == '__main__':
    main()
