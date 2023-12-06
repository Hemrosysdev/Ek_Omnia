///////////////////////////////////////////////////////////////////////////////
///
/// @file EspApplication.cpp
///
/// This file was developed as part of UX Extended Eco System Testframework (UX-EES-TSFW)
///
/// @brief Implementation file of class EspApplication.
///
/// @author Ultratronik GmbH
///         Dornierstr. 9
///         D-82205 Gilching
///         http://www.ultratronik.de
///
/// @author written by Gerd Esser, Research & Development, gesser@ultratronik.de
///
/// @date 13.12.2019
///
/// @copyright Copyright 2021 by Ultratronik GmbH.
///
/// This file and/or parts of it are subject to Ultratronik´s software license terms (SoLiT, Version 1.16.2).
/// With the use of this software you accept the SoLiT. Without written approval of Ultratronik GmbH this
/// software may not be copied, redistributed or used in any other way than stated in the conditions of the
/// SoLiT.
///
///////////////////////////////////////////////////////////////////////////////

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#include "EspApplication.h"

#include <esp_chip_info.h>
#include <esp_event.h>
#include <esp_flash.h>
#include <esp_ota_ops.h>
#include <nvs_flash.h>
#include <esp_flash_partitions.h>
#include <esp_partition.h>
#include <esp_vfs.h>
#include <esp_vfs_fat.h>
#include <esp_spiffs.h>
#include <esp_system.h>
#include <esp_netif.h>


#include "FreeRtosQueueTask.h"
#include "FreeRtosStartupCtrlTask.h"
#include "FreeRtosStartupCtrlQueue.h"
#include "UxLibCommon.h"

// Handle of the wear leveling library instance
static wl_handle_t wl_handle = WL_INVALID_HANDLE;

#define HASH_LEN                32 /* SHA-256 digest length */
#define HASH_HEX_DIGIT_SIZE     2

namespace UxEspCppLibrary
{

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

EspApplication::EspApplication( const std::string & strName )
    : EspLog( strName )
    , m_theStartupCtrl( this )
{
    vlogInfo( "EspApplication: free heap %d", esp_get_free_heap_size() );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

EspApplication::~EspApplication()
{
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void EspApplication::printSha256( const uint8_t * const pu8ImageHash,
                                  const char * const    pszLabel )
{
    char szHashBuf[HASH_LEN * HASH_HEX_DIGIT_SIZE + 1];
    szHashBuf[HASH_LEN * HASH_HEX_DIGIT_SIZE] = 0;

    for ( int i = 0; i < HASH_LEN; ++i )
    {
        snprintf( &szHashBuf[i * HASH_HEX_DIGIT_SIZE], sizeof( szHashBuf ), "%02x", pu8ImageHash[i] );
    }

    vlogInfo( "%s: %s", pszLabel, szHashBuf );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

esp_err_t EspApplication::mountFatFs( const std::string & strFatFsBasePath,
                                      const std::string & strFatFsLabel )
{
    esp_err_t nEspError = ESP_FAIL;

    vlogInfo( "Mounting FATFS ... please wait" );

    // To mount device we need name of device partition, define base_path
    // and allow format partition in case if it is new one and was not formated before
    const esp_vfs_fat_mount_config_t mountConfig =
    {
        .format_if_mount_failed = true,
        .max_files              = 4,
        .allocation_unit_size   = CONFIG_WL_SECTOR_SIZE,
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL( 5, 0, 0 )
        .disk_status_check_enable = false
#endif
    };

#if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL( 5, 0, 0 )
    nEspError = esp_vfs_fat_spiflash_mount( strFatFsBasePath.c_str(),
                                            strFatFsLabel.c_str(),
                                            &mountConfig,
                                            &wl_handle );
#else
    nEspError = esp_vfs_fat_spiflash_mount_rw_wl( strFatFsBasePath.c_str(),
                                            strFatFsLabel.c_str(),
                                            &mountConfig,
                                            &wl_handle );
#endif

    if ( nEspError != ESP_OK )
    {
        if ( nEspError == ESP_FAIL )
        {
            vlogError( "Failed to mount or format file system" );
        }
        else if ( nEspError == ESP_ERR_NOT_FOUND )
        {
            vlogError( "Failed to find SPI flash partition" );
        }
        else
        {
            vlogError( "Failed to initialize FATFS (%s)", esp_err_to_name( nEspError ) );
        }
    }
    else
    {
        vlogInfo( "FATFS successfully mounted" );
    }

    return nEspError;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void EspApplication::unmountFatFs( const std::string & strFatFsBasePath )
{
    // Unmount FATFS
#if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL( 5, 0, 0 )
    esp_vfs_fat_spiflash_unmount( strFatFsBasePath.c_str(), wl_handle );
#else
    esp_vfs_fat_spiflash_unmount_rw_wl( strFatFsBasePath.c_str(), wl_handle );
#endif

    vlogInfo( "FATFS unmounted" );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

esp_err_t EspApplication::mountSpiFfs( const std::string & strBasePath,
                                       const std::string & strPartitionLabel )
{
    esp_err_t nEspError = ESP_FAIL;

    vlogInfo( "Mounting SPIFFS ... please wait" );

    // To mount device we need name of device partition, define base_path
    // and allow format partition in case if it is new one and was not formated before
    const esp_vfs_spiffs_conf_t mountConfig =
    {
        .base_path              = strBasePath.c_str(),
        .partition_label        = strPartitionLabel.c_str(),
        .max_files              = 4,
        .format_if_mount_failed = true,
    };

    nEspError = esp_vfs_spiffs_register( &mountConfig );

    if ( nEspError != ESP_OK )
    {
        vlogError( "Failed to register SPIFFS, %s", esp_err_to_name( nEspError ) );
    }
    else
    {
        vlogInfo( "SPIFFS successfully registered" );
    }

    size_t u32Total = 0;
    size_t u32Used  = 0;
    nEspError = esp_spiffs_info( mountConfig.partition_label, &u32Total, &u32Used );
    if ( nEspError != ESP_OK )
    {
        vlogError( "Failed to get SPIFFS partition information (%s)", esp_err_to_name( nEspError ) );
    }
    else
    {
        vlogInfo( "SPIFFS Partition size: total: %d, used: %d", u32Total, u32Used );
    }

    return nEspError;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void EspApplication::unmountSpiFfs( const std::string & strPartitionLabel )
{
    // Unmount FATFS
    esp_vfs_spiffs_unregister( strPartitionLabel.c_str() );

    vlogInfo( "FATFS unmounted" );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void EspApplication::processOtaRollback( void )
{
    uint8_t         pszSha256[HASH_LEN] = { 0 };
    esp_partition_t partition;
    esp_err_t       nEspError;

    // get sha256 digest for the partition table
    partition.address = ESP_PARTITION_TABLE_OFFSET;
    partition.size    = ESP_PARTITION_TABLE_MAX_LEN;
    partition.type    = ESP_PARTITION_TYPE_DATA;
    nEspError         = esp_partition_get_sha256( &partition, pszSha256 );
    printSha256( pszSha256, "SHA-256 for the partition table" );
    if ( nEspError != ESP_OK )
    {
        vlogError( "Partition table SHA wrong - maybe data corruption!" );
    }

    // get sha256 digest for bootloader
    partition.address = ESP_BOOTLOADER_OFFSET;
    partition.size    = ESP_PARTITION_TABLE_OFFSET;
    partition.type    = ESP_PARTITION_TYPE_APP;
    nEspError         = esp_partition_get_sha256( &partition, pszSha256 );
    printSha256( pszSha256, "SHA-256 for bootloader         " );
    if ( nEspError != ESP_OK )
    {
        vlogError( "Boot partition SHA wrong - maybe data corruption!" );
    }

    const esp_partition_t * const pRunningPartition = esp_ota_get_running_partition();

    // get sha256 digest for running partition
    nEspError = esp_partition_get_sha256( pRunningPartition, pszSha256 );
    printSha256( pszSha256, "SHA-256 for current firmware   " );
    if ( nEspError != ESP_OK )
    {
        vlogError( "Firmware partition SHA wrong - maybe data corruption!" );
    }

    esp_ota_img_states_t nOtaState;
    nEspError = esp_ota_get_state_partition( pRunningPartition, &nOtaState );

    if ( nEspError == ESP_OK )
    {
        if ( nOtaState == ESP_OTA_IMG_PENDING_VERIFY )
        {
            // run diagnostic function ...
            const bool diagnostic_is_ok = true;
            if ( diagnostic_is_ok )
            {
                vlogInfo( "Initialization successfully completed after SW update, no rollback required! Continuing execution!" );
                esp_ota_mark_app_valid_cancel_rollback();
            }
            else
            {
                vlogError( "Diagnostics failed! Start rollback to the previous version ..." );
                esp_ota_mark_app_invalid_rollback_and_reboot();
            }
        }
        else
        {
            vlogInfo( "No OTA rollback action detected, OTA state %d", nOtaState );
        }
    }
    else
    {
        vlogWarning( "No OTA state partition, vlogError (%s)", esp_err_to_name( nEspError ) );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void EspApplication::initTcpIp( void )
{
    vlogInfo( "init TCP/IP stack (netif)" );
    esp_netif_init();
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

esp_err_t EspApplication::initNvs( void )
{
    //Initialize NVS
    vlogInfo( "NVS flash init" );

    esp_err_t nEspError = nvs_flash_init();
    if ( ( nEspError == ESP_ERR_NVS_NO_FREE_PAGES )
         || ( nEspError == ESP_ERR_NVS_NEW_VERSION_FOUND ) )
    {
        vlogInfo( "NVS flash erase" );
        ESP_ERROR_CHECK( nvs_flash_erase() );
        nEspError = nvs_flash_init();
    }

    return nEspError;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void EspApplication::printChipInfo( void )
{
    /* Print chip vlogInformation */
    esp_chip_info_t chip_info;
    esp_chip_info( &chip_info );
    uint32_t size_flash_chip;
    printf( "=====================================================\n"
            "This is ESP32 chip with %d CPU cores, WiFi%s%s, ",
            chip_info.cores,
            ( ( chip_info.features & CHIP_FEATURE_BT ) ? "/BT" : "" ),
            ( ( chip_info.features & CHIP_FEATURE_BLE ) ? "/BLE" : "" ) );

    printf( "silicon revision %d, ",
            chip_info.revision );

    printf( "%dMB %s flash\n"
            "=====================================================\n\n",
            esp_flash_get_size( NULL, &size_flash_chip ) / ( 1024 * 1024 ),
            ( ( chip_info.features & CHIP_FEATURE_EMB_FLASH ) ? "embedded" : "external" ) );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void EspApplication::appMain()
{
    printChipInfo();

    ESP_ERROR_CHECK( initNvs() );

    //Initialize TCP/IP stack
    initTcpIp();

    // Initialize event loop
    vlogInfo( "create event loop" );
    ESP_ERROR_CHECK( esp_event_loop_create_default() );

    createApp();

    createTasks();

    printf( "\n"
            "=====================================================\n"
            "Application create done\n"
            "=====================================================\n" );

    m_theStartupCtrl.freeRtosStartupCtrlQueue()->sendBeginStartup();
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void EspApplication::registerTask( FreeRtosQueueTask * const pTask )
{
    m_theTasks.push_back( pTask );
    startupCtrl().registerStartupTask( pTask );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void EspApplication::createTasks( void )
{
    m_theStartupCtrl.createTask();

    for ( int i = 0; i < m_theTasks.size(); i++ )
    {
        m_theTasks[i]->createTask();
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

FreeRtosStartupCtrlTask & EspApplication::startupCtrl( void )
{
    return m_theStartupCtrl;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void EspApplication::createApp( void )
{
    // special stuff done in overloaded function
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

} // namespace UxEspCppLibrary

