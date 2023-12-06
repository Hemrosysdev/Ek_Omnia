/*
 * JsonHelper.cpp
 *
 *  Created on: 04.11.2019
 *      Author: fsonntag
 */

#include "JsonHelper.h"

#include <cstdio>
#include <cstring>

#include "esp_log.h"


/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

JsonHelper::JsonHelper()
{

}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

// static
int JsonHelper::getChildCountFromJson( cJSON * const obj )
{
    int count = 0;

    cJSON* child = obj->child;

    if( child )
    {
        while( child )
        {
            child = child->next;
            count++;
        }
    }

    return count;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

// static
char* JsonHelper::getStringFromJson( cJSON * const pObj,
                                     const char*         pszItemName )
{
    char* pszString = NULL;

    cJSON* pJCommand = cJSON_GetObjectItem( pObj,
                                            pszItemName );
    if ( pJCommand != NULL )
    {
        pszString = cJSON_GetStringValue( pJCommand );
    }

    return pszString;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

// static
void JsonHelper::setStringInJson( cJSON * const obj,
                                  const char * itemName,
                                  const char * pszString )
{
    cJSON * pJson = cJSON_CreateString( pszString );

    if ( pJson )
    {
        cJSON_ReplaceItemInObject( obj, itemName, pJson );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

// static
int JsonHelper::getIntegerFromJson( cJSON * const obj,
                                    const char *itemName )
{
    int32_t ret = 0u;

    cJSON *integerObj = cJSON_GetObjectItem( obj, itemName );

    if ( integerObj != NULL )
    {
        ret = integerObj->valueint;
    }

    return ret;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

// static
void JsonHelper::setIntegerInJson( cJSON * const obj,
                                   const char *itemName,
                                   const int val )
{
    cJSON * pJson = cJSON_GetObjectItem( obj, itemName );

    if ( pJson )
    {
        cJSON_SetIntValue( pJson, val );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

// static
double JsonHelper::getNumberFromJson( cJSON * const obj,
                                      const char * itemName )
{
    cJSON *numberObj = cJSON_GetObjectItem( obj, itemName );

    double ret = 0.0;

    if( numberObj != NULL )
    {
        ret =  numberObj->valuedouble;
    }
    return ret;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

// static
void JsonHelper::setNumberInJson( cJSON * const obj,
                                  const char * itemName,
                                  const double val )
{
    cJSON * pJson = cJSON_GetObjectItem( obj, itemName );

    if ( pJson )
    {
        cJSON_SetNumberValue( pJson, val );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

// static
int JsonHelper::getBoolFromJson( cJSON * const obj,
                                 const char * itemName )
{

    int ret = 2;

    cJSON *boolObj = cJSON_GetObjectItem( obj, itemName );
    if ( boolObj )
    {
        if ( cJSON_True == boolObj->type )
        {
            ret = 1;
        }
        else if ( cJSON_False == boolObj->type )
        {
            ret = 0;
        }
        else
        {
            ESP_LOGE( __FUNCTION__, "neither True nor False" );
        }
    }

    return ret;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

// static
void JsonHelper::setBoolInJson( cJSON * const obj,
                                const char *itemName,
                                const int val )
{
    if ( 0 == val )
    {
        cJSON * pJson = cJSON_CreateFalse();

        if ( pJson )
        {
            cJSON_ReplaceItemInObject( obj, itemName, pJson );
        }
    }
    else
    {
        cJSON * pJson = cJSON_CreateTrue();

        if ( pJson )
        {
            cJSON_ReplaceItemInObject( obj, itemName, pJson );
        }
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

// static
void JsonHelper::createAndSetBoolInJson( cJSON* const   pObj,
                                         const char*    pszItemName,
                                         const bool     bValue )
{
    cJSON * pJson = cJSON_CreateBool( bValue );
    if ( pJson )
    {
        cJSON_AddItemToObject( pObj,
                               pszItemName,
                               pJson );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

// static
void JsonHelper::createAndSetNumberInJson( cJSON* const   pObj,
                                           const char*          pszItemName,
                                           const uint16_t       val )
{
    cJSON* pcJNuber = cJSON_CreateNumber( val );
    if ( pcJNuber )
    {
        cJSON_AddItemToObject( pObj,
                               pszItemName,
                               pcJNuber );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

// static
void JsonHelper::createAndSetStringInJson( cJSON* const pObj,
                                           const char*        pszItemName,
                                           const char*        pszString )
{
    cJSON* pcJString  = cJSON_CreateString( pszString );

    if ( pcJString )
    {
        cJSON_AddItemToObject( pObj,
                               pszItemName,
                               pcJString );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

// static
bool JsonHelper::getIfFoundByteFromJson( cJSON* const pObj,
                                         const char*        pszItemName,
                                         uint8_t*     pByte )
{
    int valueint;

    bool bProperByteValueFound = getIfFoundIntFromJson( pObj,
                                                        pszItemName,
                                                        &valueint );

    if ( bProperByteValueFound )
    {
        if ( ( valueint <= 255 ) &&
                        ( valueint >= 0 ) )
        {
            *pByte = (uint8_t) valueint;
        }
        else
        {
            bProperByteValueFound = false;
        }
    }

    return bProperByteValueFound;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

// static
bool JsonHelper::getIfFoundIntFromJson( cJSON* const pObj,
                                        const  char* pszItemName,
                                        int*         pInt )
{
    bool bProperByteValueFound = false;

    cJSON* pByteObj = cJSON_GetObjectItem( pObj,
                                           pszItemName );

    if ( pByteObj != NULL )
    {
        *pInt = pByteObj->valueint;
        bProperByteValueFound = true;
    }

    return bProperByteValueFound;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

// static
bool JsonHelper::getIfFoundStringFromJson( cJSON* const  pObj,
                                           const  char*  pszItemName,
                                           char**        ppszString )
{
    bool bProperStringValueFound = false;

    cJSON* pJsString = cJSON_GetObjectItem( pObj,
                                            pszItemName );

    if ( pJsString != NULL )
    {
        char *pszString = cJSON_GetStringValue( pJsString );

        if ( pszString != NULL )
        {
            bProperStringValueFound = true;
            *ppszString = pszString;
        }
    }

    return bProperStringValueFound;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

// static
bool JsonHelper::getIfFoundBoolFromJson( cJSON* const pObj,
                                         const char*        pszItemName,
                                         bool*        pBool )
{
    bool bProperBoolValueFound;

    const int res = getBoolFromJson( pObj,
                                     pszItemName );

    switch ( res )
    {
        case 1:
            *pBool = true;
            bProperBoolValueFound = true;
            break;
        case 0:
            *pBool = false;
            bProperBoolValueFound = true;
            break;
        default:
            bProperBoolValueFound = false;
            break;
    }


    return bProperBoolValueFound;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void JsonHelper::dumpJson( cJSON * const pJson )
{
    if ( !pJson )
    {
        printf( "JsonHelper::dumpJson: object is null\n" );
    }
    else
    {
        char * pszJson = cJSON_Print( pJson );

        if ( !pszJson )
        {
            printf( "JsonHelper::dumpJson: cJSON_PrintFormatted returns null\n" );
        }
        else
        {
            printf( "JsonHelper::dumpJson:\n%s\n", pszJson );
            free( pszJson );
        }
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/
