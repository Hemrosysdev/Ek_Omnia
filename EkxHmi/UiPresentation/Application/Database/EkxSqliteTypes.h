// This file is generated from EkxSqliteMaster.db
// Don't modify it manually. Call script create_include.py instead.

#ifndef EkxSqliteTypes_h
#define EkxSqliteTypes_h

#include <QObject>

namespace EkxSqliteTypes
{
Q_NAMESPACE

enum SqliteVersion
{
    SqliteVersionIndex = 2,
};

enum SqliteEventTypes
{
    SqliteEventType_UNKNOWN = 0,
    SqliteEventType_CHANGE_AUTO_LOGOUT = 32,
    SqliteEventType_CHANGE_BRIGHTNESS = 19,
    SqliteEventType_CHANGE_DATE = 28,
    SqliteEventType_CHANGE_GRINDER_NAME = 25,
    SqliteEventType_CHANGE_SHOW_DDD = 22,
    SqliteEventType_CHANGE_SHOW_LIBRARY_MODE = 23,
    SqliteEventType_CHANGE_SHOW_SPOTLIGHT = 21,
    SqliteEventType_CHANGE_SHOW_TEMP = 18,
    SqliteEventType_CHANGE_SHOW_TIME_MODE = 24,
    SqliteEventType_CHANGE_STANDBY_TIME = 20,
    SqliteEventType_CHANGE_START_OF_WEEK = 33,
    SqliteEventType_CHANGE_TEMP_THRESHOLD = 30,
    SqliteEventType_CHANGE_TEMP_UNIT = 26,
    SqliteEventType_CHANGE_TEMP_WARNING = 17,
    SqliteEventType_CHANGE_TIME = 27,
    SqliteEventType_CHANGE_WIFI_ON = 31,
    SqliteEventType_CHANGE_WIFI_SETTINGS = 29,
    SqliteEventType_DDD_CALIBRATE = 13,
    SqliteEventType_DISC_USAGE_TIME_RESET = 12,
    SqliteEventType_ERROR_GONE = 3,
    SqliteEventType_ERROR_RAISE = 2,
    SqliteEventType_FACTORY_RESET = 16,
    SqliteEventType_OWNER_LOGIN = 10,
    SqliteEventType_OWNER_LOGOUT = 9,
    SqliteEventType_OWNER_PIN_CHANGE = 7,
    SqliteEventType_RECIPE_ADD = 15,
    SqliteEventType_RECIPE_CHANGE = 5,
    SqliteEventType_RECIPE_DELETE = 14,
    SqliteEventType_SW_UPDATE_ESP = 34,
    SqliteEventType_SW_UPDATE_MCU = 35,
    SqliteEventType_SW_UPDATE_PACKAGE = 4,
    SqliteEventType_SYSTEM_RESTART = 1,
    SqliteEventType_TECHNICIAN_LOGIN = 6,
    SqliteEventType_TECHNICIAN_LOGOUT = 11,
    SqliteEventType_TECHNICIAN_PIN_CHANGE = 8,
};
Q_ENUM_NS( SqliteEventTypes )

enum SqliteErrorTypes
{
    SqliteErrorType_UNKNOWN = 0,
    SqliteErrorType_ESP_COM = 3,
    SqliteErrorType_GRINDER_CHAMBER_OPENED = 1,
    SqliteErrorType_HOPPER_DISMOUNTED = 2,
    SqliteErrorType_MCU_COM = 4,
};
Q_ENUM_NS( SqliteErrorTypes )

enum SqliteRecipeModes
{
    SqliteRecipeMode_CLASSIC = 0,
    SqliteRecipeMode_LIBRARY = 2,
    SqliteRecipeMode_TIME = 1,
};
Q_ENUM_NS( SqliteRecipeModes )

enum SqliteCounters
{
    SqliteCounter_UNKNOWN = 0,
    SqliteCounter_DISC_USAGE_TIME = 2,   // unit [1/100s]
    SqliteCounter_STANDBY_TIME = 3,   // unit [s]
    SqliteCounter_SYSTEM_ON_TIME = 4,   // unit [s]
    SqliteCounter_TOTAL_GRIND_SHOTS = 5,   // unit [x]
    SqliteCounter_TOTAL_GRIND_TIME = 6,   // unit [1/100s]
    SqliteCounter_TOTAL_MOTOR_ON_TIME = 1,   // unit [1/100s]
};
Q_ENUM_NS( SqliteCounters )

#define RECIPE_UUID_CLASSIC  "52659fdc-7f2e-4ec1-a23c-d82a0d9fbd1f"
#define RECIPE_UUID_RECIPE1  "1efe7671-b365-4a3c-8cbb-90f0cf6a7baf"
#define RECIPE_UUID_RECIPE2  "a58a9991-49c8-4efa-afb8-3b75f85d4f1f"
#define RECIPE_UUID_RECIPE3  "d99006ac-a8f4-415a-96f0-7067373fd8d8"
#define RECIPE_UUID_RECIPE4  "d03abbec-1b89-4661-af39-cfaf1aab8e4d"
#define RECIPE_UUID_RECIPE5  "d812779f-431b-44c1-ac2f-82ca9471ea40"
#define RECIPE_UUID_RECIPE6  "6db9c354-eb34-4e5d-8220-a152cb8c5e0c"
#define RECIPE_UUID_ELPUENTE  "32fac243-8ed7-47ba-bf57-a9cba8f0e3cf"
#define RECIPE_UUID_BURUNDIBWAYI  "4e379882-c2f0-4b36-9cd1-cdc67d768b1b"
#define RECIPE_UUID_MISCHGOLD  "816dba71-dcbb-46fc-878b-f14fd826d2bc"
#define RECIPE_UUID_LADALIA  "b57afa23-b02e-4d38-99a1-63a2cc109c51"

} // end namespace

#endif // EkxSqliteTypes_h
