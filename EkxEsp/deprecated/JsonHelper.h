/*
 * JsonHelper.h
 *
 *  Created on: 04.11.2019
 *      Author: fsonntag
 */

#ifndef JSONHELPER_H
#define JSONHELPER_H

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#include <cJSON.h>
#include <stdint.h>

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class JsonHelper
{

public:

    JsonHelper();

    static bool constructRecipeListObject( cJSON * const rootObj );

    static bool constructWifiConfigObject( cJSON * const rootObj );

    static bool constructStatisticsObject( cJSON * const rootObj );

    static bool constructTemperatureObject( cJSON * const rootObj );

    static bool constructReponseObject( cJSON * const rootObj );

    static bool constructTimeStampObject( cJSON * const rootObj );

    static int getChildCountFromJson( cJSON * const obj );

    static char* getStringFromJson( cJSON * const pObj,
                                    const char*         pszItemName );


    static void setStringInJson( cJSON * const obj,
                                 const char * itemName,
                                 const char * pszString );


    static int getIntegerFromJson( cJSON * const obj,
                                   const char * itemName );


    static void setIntegerInJson( cJSON * const obj,
                                  const char * itemName,
                                  const int val );


    static double getNumberFromJson( cJSON * const obj,
                                     const char * itemName );


    static void setNumberInJson( cJSON * const obj,
                                 const char *itemName,
                                 const double val );

    static int getBoolFromJson( cJSON * const obj,
                                const char * itemName );

    static void setBoolInJson( cJSON * const obj,
                               const char * itemName,
                               const int val );

    static void createAndSetBoolInJson( cJSON* const   pObj,
                                        const char*    pszItemName,
                                        const bool     bVal );

    static void createAndSetNumberInJson( cJSON* const   pObj,
                                          const char*    pszItemName,
                                          const uint16_t val );

    static void createAndSetStringInJson( cJSON* const pObj,
                                          const char*  pszItemName,
                                          const char*  pszString );

    static bool getIfFoundByteFromJson( cJSON* const pObj,
                                        const char*  pszItemName,
                                        uint8_t*     pByte );

    static bool getIfFoundIntFromJson(  cJSON* const pObj,
                                        const char*  pszItemName,
                                        int*         pInt );

    static bool getIfFoundStringFromJson( cJSON* const pObj,
                                          const  char* pszItemName,
                                          char**       ppszString );

    static bool getIfFoundBoolFromJson( cJSON* const pObj,
                                        const char*  pszItemName,
                                        bool*        pBool );

    static void dumpJson( cJSON* const pObj );
};



#endif /* JSONHELPER_H */
