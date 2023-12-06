/*
 * PluginTaskAt24c16QueueTypes.h
 *
 *  Created on: 27.04.2020
 *      Author: gesser
 */

#ifndef PluginTaskAt24c16QueueTypes_H
#define PluginTaskAt24c16QueueTypes_H

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#include <stdint.h>

#include "PluginTaskAt24c16Types.h"

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

namespace UxEspCppLibrary
{

namespace PluginTaskAt24c16QueueTypes
{

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

enum class messageType : uint32_t
{
    WRITE_PRODUCT_NO = 0,
    WRITE_SERIAL_NO,
    REQUEST_DATA
};

struct productNoPayload
{
    char m_pszProductNo[MAX_PROD_NO_LEN + 1];
};

struct serialNoPayload
{
    char m_pszSerialNo[MAX_SERIAL_NO_LEN + 1];
};

union messagePayload
{
    serialNoPayload  serialNoView;
    productNoPayload productNoView;
};

struct message
{
    messageType    u32Type;
    messagePayload payload;
};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

} // namespace PluginTaskAt24c16QueueTypes

namespace PluginTaskAt24c16ConsumerQueueTypes
{

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

const uint32_t u32PluginTaskAt24c16Offset = 0x12350000UL;

enum class messageType : uint32_t
{
    EEPROM_DATA = u32PluginTaskAt24c16Offset
};

struct eepromDataPayload
{
    bool m_bValid;
    char m_szProductNo[MAX_PROD_NO_LEN + 1];
    char m_szSerialNo[MAX_SERIAL_NO_LEN + 1];
};

union messagePayload
{
    eepromDataPayload eepromData;
};

struct message
{
    messageType    u32Type;
    messagePayload payload;
};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

} // namespace PluginTaskAt24c16ConsumerQueueTypes

} // namespace UxEspCppLibrary

#endif /* PluginTaskAt24c16QueueTypes_H */
