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
    SqliteVersionIndex = 13,
};

enum SqliteEventTypes
{
    SqliteEventType_UNKNOWN = 0,
    SqliteEventType_CHANGE_AGSA_DELAY = 36,
    SqliteEventType_CHANGE_AUTO_LOGOUT = 32,
    SqliteEventType_CHANGE_BRIGHTNESS = 19,
    SqliteEventType_CHANGE_DATE = 28,
    SqliteEventType_CHANGE_ENABLE_AGSA = 43,
    SqliteEventType_CHANGE_GRINDER_NAME = 25,
    SqliteEventType_CHANGE_MCU_ACCELERATION_TIME = 40,
    SqliteEventType_CHANGE_MCU_DECELERATION_TIME = 41,
    SqliteEventType_CHANGE_MCU_MAX_MOTOR_SPEED = 38,
    SqliteEventType_CHANGE_MCU_NOMINAL_MOTOR_SPEED = 39,
    SqliteEventType_CHANGE_SETTINGS_MODE_EXIT_TIME = 37,
    SqliteEventType_CHANGE_SHOW_DDD = 22,
    SqliteEventType_CHANGE_SHOW_DOSING_TIME = 44,
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
    SqliteEventType_SYSTEM_SETUP_CONFIRMED = 42,
    SqliteEventType_TECHNICIAN_LOGIN = 6,
    SqliteEventType_TECHNICIAN_LOGOUT = 11,
    SqliteEventType_TECHNICIAN_PIN_CHANGE = 8,
    SqliteEventType_LAST = 45
};
Q_ENUM_NS( SqliteEventTypes )

enum SqliteNotificationType
{
    SqliteNotificationType_UNKNOWN = 0,
    SqliteNotificationType_AGSA_FAILED_BLOCKAGE = 6,
    SqliteNotificationType_AGSA_FAILED_NO_DDD = 34,
    SqliteNotificationType_AGSA_FAILED_TIMEOUT = 12,
    SqliteNotificationType_AGSA_FAULT_PIN_ACTIVE = 57,
    SqliteNotificationType_CHANGE_DISCS_INFO = 7,
    SqliteNotificationType_DB_ADD_DELTA_TO_COUNTER = 45,
    SqliteNotificationType_DB_ADD_GRIND = 44,
    SqliteNotificationType_DB_CHANGE_RECIPE = 42,
    SqliteNotificationType_DB_CREATE_NEW_SCHEME = 48,
    SqliteNotificationType_DB_CREATE_NEW_SCHEME_FAILED = 49,
    SqliteNotificationType_DB_INSERT_RECIPE = 43,
    SqliteNotificationType_DB_QUERY_COUNTER = 47,
    SqliteNotificationType_DB_QUERY_STATISTICS_RESULT = 41,
    SqliteNotificationType_DB_RESET_COUNTER = 46,
    SqliteNotificationType_DB_UPDATE_FAILED = 39,
    SqliteNotificationType_DDD1_FAILED = 5,
    SqliteNotificationType_DDD2_FAILED = 11,
    SqliteNotificationType_ESP_INTERFACE_FAILED = 3,
    SqliteNotificationType_GRINDER_CHAMBER_OPENED = 1,
    SqliteNotificationType_HOPPER_DISMOUNTED = 2,
    SqliteNotificationType_LOAD_SETTINGS_FAILED = 19,
    SqliteNotificationType_MAIN_STATE_FILE_CORRUPTED = 50,
    SqliteNotificationType_MAIN_STATE_FILE_READ_FAILED = 52,
    SqliteNotificationType_MAIN_STATE_FILE_WRITE_FAILED = 53,
    SqliteNotificationType_MCU_INTERFACE_FAILED = 4,
    SqliteNotificationType_PDU_FAULT_PIN_ACTIVE = 58,
    SqliteNotificationType_PDU_INSTALLED_AFTER_ABSENCE = 55,
    SqliteNotificationType_PDU_UNINSTALLED_AFTER_PRESENCE = 56,
    SqliteNotificationType_ROLLBACK_STARTUP_FAILED = 35,
    SqliteNotificationType_SAVE_SETTINGS_FAILED = 40,
    SqliteNotificationType_STARTUP_ESP_CHECK_VERSION_TIMEOUT = 28,
    SqliteNotificationType_STARTUP_MCU_CHECK_VERSION_TIMEOUT = 29,
    SqliteNotificationType_STARTUP_ROOTFS_VERSION_FAILED = 18,
    SqliteNotificationType_START_GRIND_FAILED = 8,
    SqliteNotificationType_STOP_GRIND_FAILED = 9,
    SqliteNotificationType_SW_UPDATE_COMPLETED = 26,
    SqliteNotificationType_SW_UPDATE_ESP_UPDATE_FAILED = 21,
    SqliteNotificationType_SW_UPDATE_ESP_UPDATE_TIMEOUT = 30,
    SqliteNotificationType_SW_UPDATE_ESP_VALIDATE_VERSION_FAILED = 38,
    SqliteNotificationType_SW_UPDATE_ESP_VALIDATE_VERSION_TIMEOUT = 32,
    SqliteNotificationType_SW_UPDATE_MCU_UPDATE_FAILED = 20,
    SqliteNotificationType_SW_UPDATE_MCU_UPDATE_TIMEOUT = 31,
    SqliteNotificationType_SW_UPDATE_MCU_VALIDATE_VERSION_FAILED = 37,
    SqliteNotificationType_SW_UPDATE_MCU_VALIDATE_VERSION_TIMEOUT = 33,
    SqliteNotificationType_SW_UPDATE_PACKAGE_FAILED = 14,
    SqliteNotificationType_SW_UPDATE_PACKAGE_NO_UPGRADE = 15,
    SqliteNotificationType_SW_UPDATE_ROLLBACK_APP_FAILED = 24,
    SqliteNotificationType_SW_UPDATE_ROLLBACK_FAILED = 27,
    SqliteNotificationType_SW_UPDATE_ROLLBACK_ROOTFS_FAILED = 25,
    SqliteNotificationType_SW_UPDATE_ROLLED_BACK = 23,
    SqliteNotificationType_SW_UPDATE_STARTUP_FAILED = 36,
    SqliteNotificationType_SW_UPDATE_STATE_FILE_CORRUPTED = 51,
    SqliteNotificationType_SW_UPDATE_STATE_FILE_WRITE = 54,
    SqliteNotificationType_SW_UPDATE_UNPACK_APPLICATION_FAILED = 17,
    SqliteNotificationType_SW_UPDATE_UNPACK_ROOTFS_FAILED = 16,
    SqliteNotificationType_SW_UPDATE_UPLOAD_ABORTED = 22,
    SqliteNotificationType_SW_UPDATE_UPLOAD_FAILED = 13,
    SqliteNotificationType_SYSTEM_TIME_LOST = 10,
    SqliteNotificationType_LAST = 59
};
Q_ENUM_NS( SqliteNotificationType )

enum SqliteNotificationClass
{
    None = 0,
    Info = 1,
    Warning = 2,
    Error = 3,
    Last = 4
};
Q_ENUM_NS( SqliteNotificationClass )

enum SqliteRecipeModes
{
    SqliteRecipeMode_CLASSIC = 0,
    SqliteRecipeMode_LIBRARY = 2,
    SqliteRecipeMode_TIME = 1,
    SqliteRecipeMode_LAST = 3
};
Q_ENUM_NS( SqliteRecipeModes )

enum SqliteCounters
{
    SqliteCounter_UNKNOWN = 0,
    SqliteCounter_DISC_USAGE_TIME = 2,     // unit [1/100s]
    SqliteCounter_STANDBY_TIME = 3,        // unit [s]
    SqliteCounter_SYSTEM_ON_TIME = 4,      // unit [s]
    SqliteCounter_TOTAL_GRIND_SHOTS = 5,   // unit [x]
    SqliteCounter_TOTAL_GRIND_TIME = 6,    // unit [1/100s]
    SqliteCounter_TOTAL_MOTOR_ON_TIME = 1, // unit [1/100s]
    SqliteCounter_LAST = 7
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

struct SqliteNotificationDefinition
{
    SqliteNotificationType  m_nTypeId;
    QString                 m_strName;
    QString                 m_strDescription;
    QString                 m_strShortInfo;
    QString                 m_strLongInfo;
    bool                    m_bRecoverAction;
    SqliteNotificationClass m_nClassId;
};

extern const SqliteNotificationDefinition m_theNotifications[];

} // end namespace

#endif // EkxSqliteTypes_h
