///////////////////////////////////////////////////////////////////////////////
///
/// @file EspNvsValue.h
///
/// This file was developed as part of UX Extended Eco System Testframework (UX-EES-TSFW)
///
/// @brief Header file of class EspNvsValue.
///
/// @author Ultratronik GmbH
///         Dornierstr. 9
///         D-82205 Gilching
///         http://www.ultratronik.de
///
/// @author written by Gerd Esser, Research & Development, gesser@ultratronik.de
///
/// @date 29.10.2019
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

#ifndef EspNvsValue_h
#define EspNvsValue_h

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#include <cstdint>
#include <string>
#include <esp_err.h>
#include <nvs.h>

#include "EspLog.h"

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

namespace UxEspCppLibrary
{

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class EspNvsValue : public EspLog
{

public:

    EspNvsValue();
    explicit EspNvsValue( const std::string & strKey );

    ~EspNvsValue() override;

    static void setNvsLabel( const std::string & strNvsLabel );

    void setKey( const std::string & strKey );
    const std::string & key( void ) const;

    bool  valueBool( const bool bDefault = false );
    int8_t valueI8( const int8_t i8Default = 0 );
    int16_t valueI16( const int16_t i16Default = 0 );
    int32_t valueI32( const int32_t i32Default = 0 );
    uint8_t valueU8( const uint8_t u8Default = 0 );
    uint16_t valueU16( const uint16_t u16Default = 0 );
    uint32_t valueU32( const uint32_t u32Default = 0 );
    std::string valueString( const std::string & strDefault = "" );

    bool  valueBool( const std::string & strKey,
                     const bool bDefault = false );
    int8_t valueI8( const std::string & strKey,
                    const int8_t i8Default = 0 );
    int16_t valueI16( const std::string & strKey,
                      const int16_t i16Default = 0 );
    int32_t valueI32( const std::string & strKey,
                      const int32_t i32Default = 0 );
    uint8_t valueU8( const std::string & strKey,
                     const uint8_t u8Default = 0 );
    uint16_t valueU16( const std::string & strKey,
                       const uint16_t u16Default = 0 );
    uint32_t valueU32( const std::string & strKey,
                       const uint32_t u32Default = 0 );
    std::string valueKeyString( const std::string & strKey,
                                const std::string & strDefault = "" );

    esp_err_t setValueBool( const bool bValue );
    esp_err_t setValueI8( const int8_t i8Value );
    esp_err_t setValueI16( const int16_t i16Value );
    esp_err_t setValueI32( const int32_t i32Value );
    esp_err_t setValueU8( const uint8_t u8Value );
    esp_err_t setValueU16( const uint16_t u16Value );
    esp_err_t setValueU32( const uint32_t u32Value );
    esp_err_t setValueString( const std::string & strValue );

    esp_err_t setValueBool( const std::string & strKey,
                            const bool bValue );
    esp_err_t setValueI8( const std::string & strKey,
                          const int8_t i8Value );
    esp_err_t setValueI16( const std::string & strKey,
                           const int16_t i16Value );
    esp_err_t setValueI32( const std::string & strKey,
                           const int32_t i32Value );
    esp_err_t setValueU8( const std::string & strKey,
                          const uint8_t u8Value );
    esp_err_t setValueU16( const std::string & strKey,
                           const uint16_t u16Value );
    esp_err_t setValueU32( const std::string & strKey,
                           const uint32_t u32Value );
    esp_err_t setValueString( const std::string & strKey,
                              const std::string & strValue );

private:

    ////////////////////////////////////////
    // private methods
    ////////////////////////////////////////

    esp_err_t openNvs( void );

    void closeNvs( void );

    ////////////////////////////////////////
    // private member variables
    ////////////////////////////////////////

    nvs_handle  m_hNvs;

    std::string m_strKey;

    static std::string m_strNvsLabel;
};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

} // namespace UxEspCppLibrary

#endif /* EspNvsValue_h */
