// This file is generated from EkxSqliteMaster.db
// Don't modify it manually. Call script create_include.py instead.

#include "EkxSqliteTypes.h"
#include <QQmlEngine>

namespace EkxSqliteTypes
{

const SqliteNotificationDefinition m_theNotifications[] =
{
    {
        SqliteNotificationType_UNKNOWN,
        "UNKNOWN",
        "UNKNOWN",
        "UNKNOWN",
        "UNKNOWN",
        false,
        SqliteNotificationClass::None
    },
    {
        SqliteNotificationType_GRINDER_CHAMBER_OPENED,
        "GRINDER_CHAMBER_OPENED",
        "The grinder chamber is opened. Motor cannot be actuated. No grind action possible.",
        "Open Grinder Chamber",
        "Grinder chamber\nopened!",
        false,
        SqliteNotificationClass::Error
    },
    {
        SqliteNotificationType_HOPPER_DISMOUNTED,
        "HOPPER_DISMOUNTED",
        "The bean hopper has been dismounted. Motor cannot be actuated. No grind action possible.",
        "No Bean Hopper",
        "Bean hopper\ndismounted!",
        false,
        SqliteNotificationClass::Error
    },
    {
        SqliteNotificationType_ESP_INTERFACE_FAILED,
        "ESP_INTERFACE_FAILED",
        "Communication between HMI and driver board is interrupted. No grind actions possible. WebServer is not available.",
        "Driver Comm Failed",
        "Driver board\ncommunication\nfailed!",
        true,
        SqliteNotificationClass::Error
    },
    {
        SqliteNotificationType_MCU_INTERFACE_FAILED,
        "MCU_INTERFACE_FAILED",
        "Communication between HMI board and motor control unit is interrupted. No grind actions possible. ",
        "MCU Comm Failed",
        "MCU board\ncommunication\nfailed!",
        true,
        SqliteNotificationClass::Error
    },
    {
        SqliteNotificationType_DDD1_FAILED,
        "DDD1_FAILED",
        "DDD 1 (coarse) sensor is not available or sensor data invalid. DDD display is invalid and AGSA deactivated.",
        "DDD 1 Failed!",
        "DDD 1 sensor\nfailed!",
        false,
        SqliteNotificationClass::Error
    },
    {
        SqliteNotificationType_AGSA_FAILED_BLOCKAGE,
        "AGSA_FAILED_BLOCKAGE",
        "AGSA motor drive has been stopped because of a blockage within drive system. Try to repeat action.",
        "AGSA Blockage",
        "AGSA driver\nfailed!\nBlockage!",
        false,
        SqliteNotificationClass::Warning
    },
    {
        SqliteNotificationType_CHANGE_DISCS_INFO,
        "CHANGE_DISCS_INFO",
        "Grinding discs life time has been exceeded. Exchange grinding discs and reset counter.",
        "Grinding Discs",
        "Change grinding\ndiscs!",
        false,
        SqliteNotificationClass::Info
    },
    {
        SqliteNotificationType_START_GRIND_FAILED,
        "START_GRIND_FAILED",
        "Grinding motor hasn't started although commanded to do so. Maybe grinding motor is blocked or communication disturbed?",
        "Start Grind Failed",
        "Start grind\nfailed!",
        false,
        SqliteNotificationClass::Error
    },
    {
        SqliteNotificationType_STOP_GRIND_FAILED,
        "STOP_GRIND_FAILED",
        "Grinding motor hasn't stopped although commanded to do so. Maybe communication to motor control unit disturbed?",
        "Stop Grind Failed",
        "Stop grind\nfailed!",
        false,
        SqliteNotificationClass::Error
    },
    {
        SqliteNotificationType_SYSTEM_TIME_LOST,
        "SYSTEM_TIME_LOST",
        "System clock has been resetted due to a long period power off. Please set the current date and time within system settings menu. The system provides a clock powering of max. 2 weeks.",
        "System Time Lost",
        "System time\nlost, please\nset manually!",
        true,
        SqliteNotificationClass::Warning
    },
    {
        SqliteNotificationType_DDD2_FAILED,
        "DDD2_FAILED",
        "DDD 2 (fine) sensor is not available or sensor data invalid. DDD display is invalid and AGSA deactivated.",
        "DDD 2 Failed!",
        "DDD 2 sensor\nfailed!",
        false,
        SqliteNotificationClass::Error
    },
    {
        SqliteNotificationType_AGSA_FAILED_TIMEOUT,
        "AGSA_FAILED_TIMEOUT",
        "AGSA motor drive has been stopped by timeout, because a complete AGSA move reaches timeout. Try to repeat action.",
        "AGSA Timeout",
        "AGSA driver\nfailed!\nTimeout!",
        false,
        SqliteNotificationClass::Warning
    },
    {
        SqliteNotificationType_SW_UPDATE_UPLOAD_FAILED,
        "SW_UPDATE_UPLOAD_FAILED",
        "Upload of software update package failed. Try to repeat SW update.",
        "SW Update Failed",
        "Upload of update\npackage failed!",
        false,
        SqliteNotificationClass::Warning
    },
    {
        SqliteNotificationType_SW_UPDATE_PACKAGE_FAILED,
        "SW_UPDATE_PACKAGE_FAILED",
        "Consitency check of SW update package failed. Try to repeat SW update or validate package source.",
        "SW Update Failed",
        "File is no valid\nupdate package!",
        false,
        SqliteNotificationClass::Error
    },
    {
        SqliteNotificationType_SW_UPDATE_PACKAGE_NO_UPGRADE,
        "SW_UPDATE_PACKAGE_NO_UPGRADE",
        "Software update package rejected. Package contains no upgrade version.",
        "SW Update Failed",
        "Update package\nis no upgrade!",
        false,
        SqliteNotificationClass::Warning
    },
    {
        SqliteNotificationType_SW_UPDATE_UNPACK_ROOTFS_FAILED,
        "SW_UPDATE_UNPACK_ROOTFS_FAILED",
        "Software update failed. Unpacking image #1 of 2 failed. Try to repeat SW update.",
        "SW Update Failed",
        "Unpacking image\n#1 failed!",
        false,
        SqliteNotificationClass::Error
    },
    {
        SqliteNotificationType_SW_UPDATE_UNPACK_APPLICATION_FAILED,
        "SW_UPDATE_UNPACK_APPLICATION_FAILED",
        "Software update failed. Unpacking image #2 of 2 failed. Try to repeat SW update.",
        "SW Update Failed",
        "Unpacking image\n#2 failed!",
        false,
        SqliteNotificationClass::Error
    },
    {
        SqliteNotificationType_STARTUP_ROOTFS_VERSION_FAILED,
        "STARTUP_ROOTFS_VERSION_FAILED",
        "Check of core system version information failed. Repeat SW update to recover.",
        "Startup Failed",
        "Startup check of\nimage #1 failed!",
        false,
        SqliteNotificationClass::Error
    },
    {
        SqliteNotificationType_LOAD_SETTINGS_FAILED,
        "LOAD_SETTINGS_FAILED",
        "Loading of configuration data failed. Try to reconfigure the grinder.",
        "Load Settings Failed",
        "Load settings\nfailed!",
        false,
        SqliteNotificationClass::Error
    },
    {
        SqliteNotificationType_SW_UPDATE_MCU_UPDATE_FAILED,
        "SW_UPDATE_MCU_UPDATE_FAILED",
        "Software update of MCU failed. System will be rolled back to previous state. Try to repeat SW update.",
        "SW Update Failed",
        "Update of MCU\nboard failed!",
        false,
        SqliteNotificationClass::Error
    },
    {
        SqliteNotificationType_SW_UPDATE_ESP_UPDATE_FAILED,
        "SW_UPDATE_ESP_UPDATE_FAILED",
        "Software update of driver board failed. System will be rolled back to previous state. Try to repeat SW update.",
        "SW Update Failed",
        "Update of driver\nboard failed!",
        false,
        SqliteNotificationClass::Error
    },
    {
        SqliteNotificationType_SW_UPDATE_UPLOAD_ABORTED,
        "SW_UPDATE_UPLOAD_ABORTED",
        "Upload of software update package has been aborted by user. Try to repeat SW update.",
        "SW Update Failed",
        "Upload aborted\nby user!",
        false,
        SqliteNotificationClass::Warning
    },
    {
        SqliteNotificationType_SW_UPDATE_ROLLED_BACK,
        "SW_UPDATE_ROLLED_BACK",
        "System has been rolled back to previous state after failed SW update. Try to repeat SW update.",
        "Rollback",
        "Perform rollback\nafter failed\nSW update!",
        false,
        SqliteNotificationClass::Info
    },
    {
        SqliteNotificationType_SW_UPDATE_ROLLBACK_APP_FAILED,
        "SW_UPDATE_ROLLBACK_APP_FAILED",
        "Rollback of image #2 of 2 to previous state failed after failed SW update. System is probably inconsistent. Try to repeat SW update.",
        "Rollback Failed",
        "Rollback of\nimage #2 failed!",
        false,
        SqliteNotificationClass::Error
    },
    {
        SqliteNotificationType_SW_UPDATE_ROLLBACK_ROOTFS_FAILED,
        "SW_UPDATE_ROLLBACK_ROOTFS_FAILED",
        "Rollback of image #1 of 2 to previous state failed after failed SW update. System is probably inconsistent. Try to repeat SW update.",
        "Rollback Failed",
        "Rollback of\nimage #1 failed!",
        false,
        SqliteNotificationClass::Error
    },
    {
        SqliteNotificationType_SW_UPDATE_COMPLETED,
        "SW_UPDATE_COMPLETED",
        "Software update has been successfully completed.",
        "SW Update Completed",
        "Software update\nsuccessfully\ncompleted!",
        false,
        SqliteNotificationClass::Info
    },
    {
        SqliteNotificationType_SW_UPDATE_ROLLBACK_FAILED,
        "SW_UPDATE_ROLLBACK_FAILED",
        "Rollback to previous system state failed after failed SW update. Try to repeat SW update.",
        "Rollback Failed",
        "Rollback after\nSW update failed!",
        false,
        SqliteNotificationClass::Error
    },
    {
        SqliteNotificationType_STARTUP_ESP_CHECK_VERSION_TIMEOUT,
        "STARTUP_ESP_CHECK_VERSION_TIMEOUT",
        "Read of driver board version information timed out.",
        "Startup Failed",
        "Driver board SW\nversion check failed!\nTimeout!",
        false,
        SqliteNotificationClass::Error
    },
    {
        SqliteNotificationType_STARTUP_MCU_CHECK_VERSION_TIMEOUT,
        "STARTUP_MCU_CHECK_VERSION_TIMEOUT",
        "Read of MCU version information timed out.",
        "Startup Failed",
        "MCU board SW\nversion check failed!\nTimeout!",
        false,
        SqliteNotificationClass::Error
    },
    {
        SqliteNotificationType_SW_UPDATE_ESP_UPDATE_TIMEOUT,
        "SW_UPDATE_ESP_UPDATE_TIMEOUT",
        "Software update of driver board timed out. System will be rolled back to previous state. Try to repeat SW update.",
        "SW Update Failed",
        "Timeout while\nupdating driver board!",
        false,
        SqliteNotificationClass::Error
    },
    {
        SqliteNotificationType_SW_UPDATE_MCU_UPDATE_TIMEOUT,
        "SW_UPDATE_MCU_UPDATE_TIMEOUT",
        "Software update of MCU timed out. System will be rolled back to previous state. Try to repeat SW update.",
        "SW Update Failed",
        "Timeout while\nupdating MCU board!",
        false,
        SqliteNotificationClass::Error
    },
    {
        SqliteNotificationType_SW_UPDATE_ESP_VALIDATE_VERSION_TIMEOUT,
        "SW_UPDATE_ESP_VALIDATE_VERSION_TIMEOUT",
        "Validation of driver board version timed out after software update. System will be rolled back to previous state. Try to repeat SW update.",
        "SW Update Failed",
        "Timeout while\nvalidating driver board\nSW version!",
        false,
        SqliteNotificationClass::Error
    },
    {
        SqliteNotificationType_SW_UPDATE_MCU_VALIDATE_VERSION_TIMEOUT,
        "SW_UPDATE_MCU_VALIDATE_VERSION_TIMEOUT",
        "Validation of MCU version timed out after software update. System will be rolled back to previous state. Try to repeat SW update.",
        "SW Update Failed",
        "Timeout while\nvalidating MCU board\nSW version!",
        false,
        SqliteNotificationClass::Error
    },
    {
        SqliteNotificationType_AGSA_FAILED_NO_DDD,
        "AGSA_FAILED_NO_DDD",
        "AGSA motor drive has been stopped because at least one DDD sensor is failed.",
        "AGSA No DDD",
        "AGSA driver\nfailed!\nNo DDD!",
        false,
        SqliteNotificationClass::Error
    },
    {
        SqliteNotificationType_ROLLBACK_STARTUP_FAILED,
        "ROLLBACK_STARTUP_FAILED",
        "Rollback to previous system state failed after a failed SW update. Try to repeat SW update.",
        "Startup Failed",
        "Rollback startup\nfailed!",
        false,
        SqliteNotificationClass::Error
    },
    {
        SqliteNotificationType_SW_UPDATE_STARTUP_FAILED,
        "SW_UPDATE_STARTUP_FAILED",
        "System startup failed due to previous error. Maybe restart grinder.",
        "Startup Failed",
        "System startup\nfailed due to\nprevious errors!",
        false,
        SqliteNotificationClass::Info
    },
    {
        SqliteNotificationType_SW_UPDATE_MCU_VALIDATE_VERSION_FAILED,
        "SW_UPDATE_MCU_VALIDATE_VERSION_FAILED",
        "Validation of MCU version failed after software update. System will be rolled back to previous state. Try to repeat SW update.",
        "SW Update Failed",
        "MCU board SW\nversion validation\nfailed!",
        false,
        SqliteNotificationClass::Error
    },
    {
        SqliteNotificationType_SW_UPDATE_ESP_VALIDATE_VERSION_FAILED,
        "SW_UPDATE_ESP_VALIDATE_VERSION_FAILED",
        "Validation of driver board version failed after software update. System will be rolled back to previous state. Try to repeat SW update.",
        "SW Update Failed",
        "Driver board SW\nversion validation\nfailed!",
        false,
        SqliteNotificationClass::Error
    },
    {
        SqliteNotificationType_DB_UPDATE_FAILED,
        "DB_UPDATE_FAILED",
        "Internal database error. Database update failed.",
        "DB Update Failed",
        "Database update\nfailed!",
        false,
        SqliteNotificationClass::Error
    },
    {
        SqliteNotificationType_SAVE_SETTINGS_FAILED,
        "SAVE_SETTINGS_FAILED",
        "Save of configuration file failed. Changes are lost.",
        "Save Settings Failed",
        "Save settings\nfailed!",
        true,
        SqliteNotificationClass::Error
    },
    {
        SqliteNotificationType_DB_QUERY_STATISTICS_RESULT,
        "DB_QUERY_STATISTICS_RESULT",
        "Internal database error. Query of statistics database result failed.",
        "DB Query Stat. Failed",
        "Database access\nquery statistics\nresult failed!",
        false,
        SqliteNotificationClass::Warning
    },
    {
        SqliteNotificationType_DB_CHANGE_RECIPE,
        "DB_CHANGE_RECIPE",
        "Internal database error. Inserting a recipe change failed. ",
        "DB Chg.Recipe Failed",
        "Database access\nchange recipe\nfailed!",
        false,
        SqliteNotificationClass::Warning
    },
    {
        SqliteNotificationType_DB_INSERT_RECIPE,
        "DB_INSERT_RECIPE",
        "Internal database error. Insertion of a new recipe failed.",
        "DB Ins.Recipe Failed",
        "Database access\ninsert recipe\nfailed!",
        false,
        SqliteNotificationClass::Warning
    },
    {
        SqliteNotificationType_DB_ADD_GRIND,
        "DB_ADD_GRIND",
        "Internal database error. Adding a grind result into database failed.",
        "DB Add Grind Failed",
        "Database access\nadd grind\nfailed!",
        false,
        SqliteNotificationClass::Warning
    },
    {
        SqliteNotificationType_DB_ADD_DELTA_TO_COUNTER,
        "DB_ADD_DELTA_TO_COUNTER",
        "Internal database error. Incrementing of a counter has been failed.",
        "DB Delta.Cnt. Failed",
        "Database access\ndelta to counter\nfailed!",
        false,
        SqliteNotificationClass::Warning
    },
    {
        SqliteNotificationType_DB_RESET_COUNTER,
        "DB_RESET_COUNTER",
        "Internal database error. Reset of counters failed.",
        "DB Reset.Cnt. Failed",
        "Database access\nreset counter\nfailed!",
        false,
        SqliteNotificationClass::Warning
    },
    {
        SqliteNotificationType_DB_QUERY_COUNTER,
        "DB_QUERY_COUNTER",
        "Internal database error. Database request for a counter failed.",
        "DB Delta.Cnt. Failed",
        "Database access\nquery counter\nfailed!",
        false,
        SqliteNotificationClass::Warning
    },
    {
        SqliteNotificationType_DB_CREATE_NEW_SCHEME,
        "DB_CREATE_NEW_SCHEME",
        "A new database scheme has been successfully activated. ",
        "Create New DB",
        "New database\ncreated!",
        false,
        SqliteNotificationClass::Info
    },
    {
        SqliteNotificationType_DB_CREATE_NEW_SCHEME_FAILED,
        "DB_CREATE_NEW_SCHEME_FAILED",
        "Internal database error. Buildup of a new database failed, the scheme couldn't be applied.",
        "Create New DB Failed",
        "Create new\ndatabase\nfailed!",
        false,
        SqliteNotificationClass::Error
    },
    {
        SqliteNotificationType_MAIN_STATE_FILE_CORRUPTED,
        "MAIN_STATE_FILE_CORRUPTED",
        "Internal state file corrupted. Maybe inconsistent grinder behaviour.",
        "State File Corrupt",
        "Main state file\ncorrupted!",
        false,
        SqliteNotificationClass::Error
    },
    {
        SqliteNotificationType_SW_UPDATE_STATE_FILE_CORRUPTED,
        "SW_UPDATE_STATE_FILE_CORRUPTED",
        "Internal SW update state file corrupted. System state reset to normal.",
        "State File Corrupt",
        "Software update\nstate file\ncorrupted!",
        false,
        SqliteNotificationClass::Error
    },
    {
        SqliteNotificationType_MAIN_STATE_FILE_READ_FAILED,
        "MAIN_STATE_FILE_READ_FAILED",
        "Internal state file couldn't be read and will be resetted. Maybe inconsistent grinder behaviour.",
        "State File Read",
        "Main state file\nread failed!",
        false,
        SqliteNotificationClass::Error
    },
    {
        SqliteNotificationType_MAIN_STATE_FILE_WRITE_FAILED,
        "MAIN_STATE_FILE_WRITE_FAILED",
        "Internal state file couldn't be written and will be resetted. Maybe inconsistent grinder behaviour.",
        "State File Write",
        "Main state file\nwrite failed!",
        false,
        SqliteNotificationClass::Error
    },
    {
        SqliteNotificationType_SW_UPDATE_STATE_FILE_WRITE,
        "SW_UPDATE_STATE_FILE_WRITE",
        "Internal SW update state file couldn't be written. System state reset to normal.",
        "State File Write",
        "Software update\nstate file\nwrite failed!",
        false,
        SqliteNotificationClass::Error
    },
    {
        SqliteNotificationType_PDU_INSTALLED_AFTER_ABSENCE,
        "PDU_INSTALLED_AFTER_ABSENCE",
        "PDU has been automatically detected as installed after being absent.",
        "PDU Detected",
        "PDU detected\nafter being absent!",
        false,
        SqliteNotificationClass::Info
    },
    {
        SqliteNotificationType_PDU_UNINSTALLED_AFTER_PRESENCE,
        "PDU_UNINSTALLED_AFTER_PRESENCE",
        "PDU has been automatically detected as not installed after being present.",
        "PDU Not Detected",
        "PDU not detected\nafter being present!",
        false,
        SqliteNotificationClass::Info
    },
    {
        SqliteNotificationType_AGSA_FAULT_PIN_ACTIVE,
        "AGSA_FAULT_PIN_ACTIVE",
        "AGSA stepper motor driver chip indicates pin fault. Retry AGSA action or contact service if still present.",
        "AGSA Fault Pin",
        "AGSA stepper\nmotor driver pin\nindicates fault!",
        false,
        SqliteNotificationClass::Error
    },
    {
        SqliteNotificationType_PDU_FAULT_PIN_ACTIVE,
        "PDU_FAULT_PIN_ACTIVE",
        "PDU motor driver chip indicates pin fault. Retry PDU action or contact service if still present.",
        "PDU Fault Pin",
        "PDU motor\ndriver pin\nindicates fault!",
        false,
        SqliteNotificationClass::Error
    },
};


} // end namespace

