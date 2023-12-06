/*!*****************************************************************************
*
* @file ApplicationHelper.cpp
*
* @brief Implementation file of class ApplicationHelper.
*
* @author Ultratronik GmbH
*         Dornierstr. 9
*         D-82205 Gilching
*         www.ultratronik.de
*
* @author Gerd Esser, Forschung & Entwicklung
*
* @created 04.05.2023
*
* @copyright Copyright (C) Bircher Smart Access, CH-8222 Beringen, 2021
*            All rights reserved.
*            None of this file or parts of it may be
*            copied, redistributed or used in any other way
*            without written approval of Bircher BSA.
*
*******************************************************************************/

#include "ApplicationHelper.h"

#include <cstdio>                   // NOSONAR required for printf
#include <cJSON.h>
#include <esp_idf_version.h>

#if not( defined UNITTEST )
#   include <esp_log.h>
#endif // not UNITTEST

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

namespace ApplicationGlobals
{

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

const char ApplicationHelper::m_cBase64EncodingTable[ApplicationHelper::Base64EncodingTableSize] =
{
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
    'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
    'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
    'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
    'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
    'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
    'w', 'x', 'y', 'z', '0', '1', '2', '3',
    '4', '5', '6', '7', '8', '9', '+', '/'
};

const int ApplicationHelper::m_nBase64ModTable[ApplicationHelper::Base64ModTableSize] = { 0, 2, 1 };

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

ApplicationHelper::~ApplicationHelper()
{
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

// static
void ApplicationHelper::dumpHexData( const char * const    pszPrompt,
                                     const uint8_t * const pData,
                                     const size_t          u32DataSize )
{

//	#if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(5, 0, 0)
//#if not (defined UNITTEST)
//    printf( "(%d) %s: size %zd", esp_log_timestamp(), pszPrompt, u32DataSize );    // NOSONAR  printf used for uncolored debug output
//#else
//    printf( "%s: size %zd", pszPrompt, u32DataSize );    // NOSONAR  printf used for uncolored debug output
//#endif // not (defined UNITTEST)
//#else
//#if not (defined UNITTEST)
//    printf( "(%ld) %s: size %zd", esp_log_timestamp(), pszPrompt, u32DataSize );    // NOSONAR  printf used for uncolored debug output
//#else
//    printf( "%s: size %zd", pszPrompt, u32DataSize );    // NOSONAR  printf used for uncolored debug output
//#endif // not (defined UNITTEST)
//#endif

    if ( u32DataSize < c_nMaxFullDumpSize )
    {
        for ( size_t i = 0; i < u32DataSize; i++ )
        {
            if ( ( i % c_nHexNumbersPerLine ) == 0 )
            {
                printf( "\n    %5zd: ", i );                         // NOSONAR  printf used for uncolored debug output
            }

            printf( "%02x ", pData[i] );                    // NOSONAR  printf used for uncolored debug output
        }
    }
    else
    {
        for ( size_t i = 0; i < c_nMinDumpLinesNum * c_nHexNumbersPerLine; i++ )
        {
            if ( ( i % c_nHexNumbersPerLine ) == 0 )
            {
                printf( "\n    %5zd: ", i );                         // NOSONAR  printf used for uncolored debug output
            }

            printf( "%02x ", pData[i] );                    // NOSONAR  printf used for uncolored debug output
        }
        printf( "\n       ..." );                          // NOSONAR  printf used for uncolored debug output
        for ( size_t i = ( ( u32DataSize / c_nHexNumbersPerLine ) - c_nMinDumpLinesNum ) * c_nHexNumbersPerLine;
              i < u32DataSize;
              i++ )
        {
            if ( ( i % c_nHexNumbersPerLine ) == 0 )
            {
                printf( "\n    %5zd: ", i );                         // NOSONAR  printf used for uncolored debug output
            }

            printf( "%02x ", pData[i] );                    // NOSONAR  printf used for uncolored debug output
        }
    }
    printf( "\n" );                                     // NOSONAR  printf used for uncolored debug output
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

// static
stringlist ApplicationHelper::splitString( const std::string & strSubject,
                                           const std::string & strDelimiter )
{
    stringlist listResult;

    size_t i = 0;
    while ( true )
    {
        const size_t u32Found = strSubject.find( strDelimiter, i );

        if ( u32Found == std::string::npos )
        {
            listResult.push_back( strSubject.substr( i ) );
            break;
        }

        listResult.push_back( strSubject.substr( i, u32Found - i ) );
        i = u32Found + strDelimiter.size();
    }

    return listResult;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

// static
void ApplicationHelper::cJSON_PrintToStdString( const cJSON * const pJsonObj,
                                                std::string &       strDest )
{
    char * const pszJson = cJSON_Print( pJsonObj );
    strDest = pszJson;
    free( pszJson );        // NOSONAR cJSON_Print creates an object on the heap which has to be freed
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

// static
ApplicationHelper::Base64EncodeError ApplicationHelper::safeBase64Encode( const uint8_t * const pu8InputData,
                                                                          const size_t          u32InputDataLength,
                                                                          char * const          pszOutputData,
                                                                          const size_t          u32OutputBufferLength,
                                                                          size_t * const        pu32OutputDataLength )
{
    Base64EncodeError nReturn = Base64EncodeError::Ok;

    if ( pu8InputData == nullptr )
    {
        nReturn = Base64EncodeError::InputBufferNull;
    }
    else if ( pszOutputData == nullptr )
    {
        nReturn = Base64EncodeError::OutputBufferNull;
    }

    else
    {
        pszOutputData[0] = '\0';

        static const uint32_t u32SizeOf32Bit = 4;
        static const uint32_t u32Oversize    = 2;
        static const uint32_t u32Divider     = 3;

        *pu32OutputDataLength = u32SizeOf32Bit * ( ( u32InputDataLength + u32Oversize ) / u32Divider );

        if ( *pu32OutputDataLength >= u32OutputBufferLength )
        {
            nReturn = Base64EncodeError::OutputBufferTooSmall;
        }
        else
        {
            char *                pszOutputDataTemp = pszOutputData;
            const uint8_t *       pu8InputDataTemp  = pu8InputData;
            const uint8_t * const pu8InputDataEnd   = pu8InputData + u32InputDataLength;
            while ( pu8InputDataTemp < pu8InputDataEnd )
            {
                uint32_t u32OctetA = 0;
                if ( pu8InputDataTemp < pu8InputDataEnd )
                {
                    u32OctetA = *pu8InputDataTemp++;
                }
                uint32_t u32OctetB = 0;
                if ( pu8InputDataTemp < pu8InputDataEnd )
                {
                    u32OctetB = *pu8InputDataTemp++;
                }
                uint32_t u32OctetC = 0;
                if ( pu8InputDataTemp < pu8InputDataEnd )
                {
                    u32OctetC = *pu8InputDataTemp++;
                }

                const uint32_t u32Triple = ( u32OctetA << 0x10 ) | ( u32OctetB << 0x08 ) | u32OctetC;

                static const uint32_t u32FourthByte = 3;
                static const uint32_t u32ThirdByte  = 2;
                static const uint32_t u32SecondByte = 1;
                static const uint32_t u32FirstByte  = 0;
                static const uint32_t u32TripleSize = 6;
                static const uint32_t u32TripleMask = 0x3F;

                *pszOutputDataTemp++ = m_cBase64EncodingTable[ ( u32Triple >> ( u32FourthByte * u32TripleSize ) ) & u32TripleMask ];
                *pszOutputDataTemp++ = m_cBase64EncodingTable[ ( u32Triple >> ( u32ThirdByte  * u32TripleSize ) ) & u32TripleMask ];
                *pszOutputDataTemp++ = m_cBase64EncodingTable[ ( u32Triple >> ( u32SecondByte * u32TripleSize ) ) & u32TripleMask ];
                *pszOutputDataTemp++ = m_cBase64EncodingTable[ ( u32Triple >> ( u32FirstByte  * u32TripleSize ) ) & u32TripleMask ];
            }

            for ( int i = 0; i < m_nBase64ModTable[u32InputDataLength % Base64ModTableSize]; i++ )
            {
                pszOutputData[*pu32OutputDataLength - 1 - i] = '=';
            }

            pszOutputData[*pu32OutputDataLength] = '\0';
        }
    }

    return nReturn;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

} /* namespace ApplicationGlobals */

