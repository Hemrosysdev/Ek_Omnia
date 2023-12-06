/*!*****************************************************************************
*
* @file ApplicationHelper.h
*
* @brief Header file for class ApplicationHelper.
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

#ifndef ApplicationHelper_h
#define ApplicationHelper_h

/*##***********************************************************************************************************************************************************
 *  Includes
 *************************************************************************************************************************************************************/

#include <cstdint>
#include <cstddef>
#include <vector>
#include <string>

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class cJSON;

namespace ApplicationGlobals
{

using stringlist = std::vector<std::string>;

/*!************************************************************************************************************************************************************
 *
 * @brief Class to provide system wide usable static helper functions
 *
 *************************************************************************************************************************************************************/

class ApplicationHelper
{

private:

    static const int c_nHexNumbersPerLine {   16 };
    static const int c_nMaxFullDumpSize   { 1024 };
    static const int c_nMinDumpLinesNum   {    5 };

public:

    ////////////////////////////////////////
    // public type definitions
    ////////////////////////////////////////

    enum Base64EncodeError
    {
        Ok = 0,
        InputBufferNull,
        OutputBufferNull,
        OutputBufferTooSmall,
    };

    ////////////////////////////////////////
    // public methods
    ////////////////////////////////////////

    /*!*****************************************************************************************************************************************
     *
     * @brief Standard destructor - as all functions are static, no constructor/destructor is required
     *
     ******************************************************************************************************************************************/
    ~ApplicationHelper();

    /*!*****************************************************************************************************************************************
     *
     * @brief Dumps debug data to console of a byte stream in hex format
     *
     * The dump will be preceded by the pszPrompt followed by a new line. Afterwards the byte stream will be output
     * with 16 bytes per line.
     * If the data stream is greater than 1024 bytes, the output will be shortcut by showing only a few head
     * and tail lines of the complete stream.
     *
     * @param [in] const char * const pszPrompt - preceding prompt line
     * @param [in] const uint8_t * const pData  - binary data stream to be dumped
     * @param [in] const size_t u32DataSize     - size of binary data stream
     *
     ******************************************************************************************************************************************/
    static void dumpHexData( const char * const    pszPrompt,
                             const uint8_t * const pData,
                             const size_t          u32DataSize );

    /*!*****************************************************************************************************************************************
     *
     * @brief Splits strSubject into a list of tokens delimited by strDelimiter
     *
     * @param [in] const std::string & strSubject   - string to be splitted by delimiter
     * @param [in] const std::string & strDelimiter - delimiter string which splits the strSubject
     *
     * @return std::vector list of strings without the delimiter
     *
     ******************************************************************************************************************************************/
    static stringlist splitString( const std::string & strSubject,
                                   const std::string & strDelimiter );

    /*!*****************************************************************************************************************************************
     *
     * @brief Converts a cJSON object into a std::string
     *
     * This function isn't provided by the cJSON API and inserted here for convenience and preventing
     * misuse of the heap based cJSON_Print() function.
     *
     * @param [in] const cJSON * const pJsonObj - cJSON object to be converted into std::string
     * @param [in] std::string & strDest        - target std::string where result is stored into
     *
     ******************************************************************************************************************************************/
    static void cJSON_PrintToStdString( const cJSON * const pJsonObj,
                                        std::string &       strDest );

    /*!*****************************************************************************************************************************************
     *
     * @brief Converts binary data into base64 encoded string considering buffer limits
     *
     * This is a extended version of the standard base64_encode method. The standard method
     * doesn't consider buffer overwrites. The Espressif mbed_tls_ methods are not reliable
     * according to performance.
     *
     * @param [in] const uint8_t * const pu8InputData  - binary data buffer to be encoded
     * @param [in] const size_t u32InputDataLength     - length of data in binary buffer
     * @param [in] char * const pszOutputData          - prepared buffer for base64 encoded output string
     * @param [in] const size_t u32OutputBufferLength  - max. size of output buffer
     * @param [in] size_t * const pu32OutputDataLength - size of base64 encoded output string
     *
     * @return Base64EncodeError - Base64EncodeError::Ok in case of no error, otherwise other error reasons
     *
     ******************************************************************************************************************************************/
    static Base64EncodeError safeBase64Encode( const uint8_t * const pu8InputData,
                                               const size_t          u32InputDataLength,
                                               char * const          pszOutputData,
                                               const size_t          u32OutputBufferLength,
                                               size_t * const        pu32OutputDataLength );

private:

    /*!*****************************************************************************************************************************************
     *
     * @brief Standard constructor - unused because class provides only static functions
     *
     ******************************************************************************************************************************************/
    ApplicationHelper() = delete;

    static const int  Base64EncodingTableSize = 64;
    static const int  Base64ModTableSize      = 3;

    static const char m_cBase64EncodingTable[Base64EncodingTableSize];

    static const int  m_nBase64ModTable[Base64ModTableSize];
};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

} /* namespace ApplicationGlobals */

#endif /* ApplicationHelper_h */
