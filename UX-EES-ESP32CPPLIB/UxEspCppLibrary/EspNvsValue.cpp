///////////////////////////////////////////////////////////////////////////////
///
/// @file EspNvsValue.cpp
///
/// This file was developed as part of UX Extended Eco System Testframework (UX-EES-TSFW)
///
/// @brief Implementation file of class EspNvsValue.
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

#include "EspNvsValue.h"

#include <cstring>

#define MAX_NVS_KEY_LEN  15

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

namespace UxEspCppLibrary
{

std::string EspNvsValue::m_strNvsLabel = "storage";

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

EspNvsValue::EspNvsValue()
: EspLog( "EspNvsValue" )
, m_hNvs( 0 )
, m_strKey("")
{
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

EspNvsValue::EspNvsValue( const std::string& strKey )
: EspLog( "EspNvsValue" )
, m_hNvs( 0 )
, m_strKey( strKey )
{
    if ( m_strKey.size() > MAX_NVS_KEY_LEN )
    {
        vlogError( "key length too big for key %s, may %d chars allowed", strKey.c_str(), MAX_NVS_KEY_LEN );
        ESP_ERROR_CHECK( ESP_FAIL );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

EspNvsValue::~EspNvsValue()
{
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

// static
void EspNvsValue::setNvsLabel( const std::string& strNvsLabel )
{
    m_strNvsLabel = strNvsLabel;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

bool EspNvsValue::valueBool( const bool bDefault )
{
    return valueBool( m_strKey, bDefault );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

int8_t EspNvsValue::valueI8( const int8_t i8Default )
{
    return valueI8( m_strKey, i8Default );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

int16_t EspNvsValue::valueI16( const int16_t i16Default )
{
    return valueI16( m_strKey, i16Default );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

int32_t EspNvsValue::valueI32( const int32_t i32Default )
{
    return valueI32( m_strKey, i32Default );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

uint8_t EspNvsValue::valueU8( const uint8_t u8Default )
{
    return valueU8( m_strKey, u8Default );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

uint16_t EspNvsValue::valueU16( const uint16_t u16Default )
{
    return valueU16( m_strKey, u16Default );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

uint32_t EspNvsValue::valueU32( const uint32_t u32Default )
{
    return valueU32( m_strKey, u32Default );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

bool EspNvsValue::valueBool( const std::string& strKey,
                             const bool bDefault )
{
    const uint8_t u8Default = ( bDefault ? 1 : 0 );

    bool bReturn = false;

    if ( valueU8( strKey, u8Default ) > 0 )
    {
        bReturn = true;
    }

    return bReturn;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

int8_t EspNvsValue::valueI8( const std::string& strKey, const int8_t i8Default )
{
    int8_t i8Value = i8Default;

    esp_err_t nEspError = openNvs();

    if ( nEspError == ESP_OK )
    {
        nEspError = nvs_get_i8( m_hNvs, strKey.c_str(), &i8Value );

        if ( nEspError == ESP_ERR_NVS_NOT_FOUND )
        {
            i8Value = i8Default;
        }
        else if ( nEspError != ESP_OK )
        {
            vlogError( "valueI8: error while getting value %s", strKey.c_str() );
        }
        else
        {
            // ESP_OK, nothing to be done
        }

        closeNvs();
    }

    return i8Value;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

int16_t EspNvsValue::valueI16( const std::string& strKey, const int16_t i16Default )
{
    int16_t i16Value = i16Default;

    esp_err_t nEspError = openNvs();

    if ( nEspError == ESP_OK )
    {
        nEspError = nvs_get_i16( m_hNvs, strKey.c_str(), &i16Value );

        if ( nEspError == ESP_ERR_NVS_NOT_FOUND )
        {
            i16Value = i16Default;
        }
        else if ( nEspError != ESP_OK )
        {
            vlogError( "valueI168: error while getting value %s", strKey.c_str() );
        }
        else
        {
            // ESP_OK, nothing to be done
        }

        closeNvs();
    }

    return i16Value;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

int32_t EspNvsValue::valueI32( const std::string& strKey, const int32_t i32Default )
{
    int32_t i32Value = i32Default;

    esp_err_t nEspError = openNvs();

    if ( nEspError == ESP_OK )
    {
        nEspError = nvs_get_i32( m_hNvs, strKey.c_str(), &i32Value );

        if ( nEspError == ESP_ERR_NVS_NOT_FOUND )
        {
            i32Value = i32Default;
        }
        else if ( nEspError != ESP_OK )
        {
            vlogError( "valueI32: error while getting value %s", strKey.c_str() );
        }
        else
        {
            // ESP_OK, nothing to be done
        }

        closeNvs();
    }

    return i32Value;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

uint8_t EspNvsValue::valueU8( const std::string& strKey, const uint8_t u8Default )
{
    uint8_t u8Value = u8Default;

    esp_err_t nEspError = openNvs();

    if ( nEspError == ESP_OK )
    {
        nEspError = nvs_get_u8( m_hNvs, strKey.c_str(), &u8Value );

        if ( nEspError == ESP_ERR_NVS_NOT_FOUND )
        {
            u8Value = u8Default;
        }
        else if ( nEspError != ESP_OK )
        {
            vlogError( "valueU8: error while getting value %s", strKey.c_str() );
        }
        else
        {
            // ESP_OK, nothing to be done
        }

        closeNvs();
    }

    return u8Value;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

uint16_t EspNvsValue::valueU16( const std::string& strKey,
                                const uint16_t u16Default )
{
    uint16_t u16Value = u16Default;

    esp_err_t nEspError = openNvs();

    if ( nEspError == ESP_OK )
    {
        nEspError = nvs_get_u16( m_hNvs, strKey.c_str(), &u16Value );

        if ( nEspError == ESP_ERR_NVS_NOT_FOUND )
        {
            u16Value = u16Default;
        }
        else if ( nEspError != ESP_OK )
        {
            vlogError( "valueU16: error while getting value %s", strKey.c_str() );
        }
        else
        {
            // ESP_OK, nothing to be done
        }

        closeNvs();
    }

    return u16Value;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

uint32_t EspNvsValue::valueU32( const std::string& strKey,
                                const uint32_t u32Default )
{
    uint32_t u32Value = u32Default;

    esp_err_t nEspError = openNvs();

    if ( nEspError == ESP_OK )
    {
        nEspError = nvs_get_u32( m_hNvs, strKey.c_str(), &u32Value );

        if ( nEspError == ESP_ERR_NVS_NOT_FOUND )
        {
            u32Value = u32Default;
        }
        else if ( nEspError != ESP_OK )
        {
            vlogError( "valueU32: error while getting value %s", strKey.c_str() );
        }
        else
        {
            // ESP_OK, nothing to be done
        }

        closeNvs();
    }

    return u32Value;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

esp_err_t EspNvsValue::setValueBool( const bool bValue )
{
    return setValueBool( m_strKey, bValue );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

esp_err_t EspNvsValue::setValueI8( const int8_t i8Value )
{
    return setValueI8( m_strKey, i8Value );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

esp_err_t EspNvsValue::setValueI16( const int16_t i16Value )
{
    return setValueI16( m_strKey, i16Value );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

esp_err_t EspNvsValue::setValueI32( const int32_t i32Value )
{
    return setValueI32( m_strKey, i32Value );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

esp_err_t EspNvsValue::setValueU8( const uint8_t u8Value )
{
    return setValueU8( m_strKey, u8Value );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

esp_err_t EspNvsValue::setValueU16( const uint16_t u16Value )
{
    return setValueU16( m_strKey, u16Value );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

esp_err_t EspNvsValue::setValueU32( const uint32_t u32Value )
{
    return setValueU32( m_strKey, u32Value );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

esp_err_t EspNvsValue::setValueBool( const std::string& strKey,
                                     const bool bValue )
{
    return setValueU8( strKey, bValue ? 1 : 0 );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

esp_err_t EspNvsValue::setValueI8( const std::string& strKey,
                                   const int8_t i8Value )
{
    esp_err_t nEspError = openNvs();

    if ( nEspError == ESP_OK )
    {
        nEspError = nvs_set_i8( m_hNvs, strKey.c_str(), i8Value );

        if ( nEspError != ESP_OK )
        {
            vlogError( "setValueI8: error while setting value %d", i8Value );
        }

        closeNvs();
    }

    return nEspError;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

esp_err_t EspNvsValue::setValueI16( const std::string& strKey,
                                    const int16_t i16Value )
{
    esp_err_t nEspError = openNvs();

    if ( nEspError == ESP_OK )
    {
        nEspError = nvs_set_i16( m_hNvs, strKey.c_str(), i16Value );

        if ( nEspError != ESP_OK )
        {
            vlogError( "setValueI16: error while setting value %d", i16Value );
        }

        closeNvs();
    }

    return nEspError;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

esp_err_t EspNvsValue::setValueI32( const std::string& strKey,
                                    const int32_t i32Value )
{
    esp_err_t nEspError = openNvs();

    if ( nEspError == ESP_OK )
    {
        nEspError = nvs_set_i32( m_hNvs, strKey.c_str(), i32Value );

        if ( nEspError != ESP_OK )
        {
            vlogError( "setValueI32: error while setting value %d", i32Value );
        }

        closeNvs();
    }

    return nEspError;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

esp_err_t EspNvsValue::setValueU8( const std::string& strKey,
                                   const uint8_t u8Value )
{
    esp_err_t nEspError = openNvs();

    if ( nEspError == ESP_OK )
    {
        nEspError = nvs_set_u8( m_hNvs, strKey.c_str(), u8Value );

        if ( nEspError != ESP_OK )
        {
            vlogError( "setValueU8: error while setting value %s-%u, (%s)", strKey.c_str(), u8Value, esp_err_to_name( nEspError ) );
        }

        closeNvs();
    }
    else
    {
        vlogError( "setValueU8: error while openNvs for value %s-%u, (%s)", strKey.c_str(), u8Value, esp_err_to_name( nEspError ) );
    }

    return nEspError;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

esp_err_t EspNvsValue::setValueU16( const std::string& strKey,
                                    const uint16_t u16Value )
{
    esp_err_t nEspError = openNvs();

    if ( nEspError == ESP_OK )
    {
        nEspError = nvs_set_u16( m_hNvs, strKey.c_str(), u16Value );

        if ( nEspError != ESP_OK )
        {
            vlogError( "setValueU16: error while setting value %u", u16Value );
        }

        closeNvs();
    }

    return nEspError;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

esp_err_t EspNvsValue::setValueU32( const std::string& strKey,
                                    const uint32_t u32Value )
{
    esp_err_t nEspError = openNvs();

    if ( nEspError == ESP_OK )
    {
        nEspError = nvs_set_u32( m_hNvs, strKey.c_str(), u32Value );

        if ( nEspError != ESP_OK )
        {
            vlogError( "setValueU32: error while setting value %u", u32Value );
        }

        closeNvs();
    }

    return nEspError;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

esp_err_t EspNvsValue::openNvs( void )
{
    esp_err_t nEspError = ESP_FAIL;

    m_hNvs = 0;

    if ( !m_strNvsLabel.empty() )
    {
        nEspError = nvs_open( m_strNvsLabel.c_str(), NVS_READWRITE, &m_hNvs );

        if ( nEspError != ESP_OK )
        {
            vlogError( "openNvs: Can't NVS label %s", m_strNvsLabel.c_str() );
        }
    }
    else
    {
        vlogError( "openNvs: No storage label defined" );
    }

    return nEspError;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

std::string EspNvsValue::valueString( const std::string& strDefault )
{
    return valueKeyString( m_strKey, strDefault );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

std::string EspNvsValue::valueKeyString( const std::string& strKey,
                                         const std::string& strDefault )
{
    std::string strValue;

    esp_err_t nEspError = openNvs();

    if ( nEspError == ESP_OK )
    {
        size_t u32Length = 0;
        nEspError = nvs_get_str( m_hNvs, strKey.c_str(), nullptr, &u32Length );

        if ( nEspError == ESP_ERR_NVS_NOT_FOUND )
        {
            strValue = strDefault;
        }
        else if ( nEspError != ESP_OK )
        {
            vlogError( "valueKeyString: error while getting value %s", strKey.c_str() );
        }
        else if ( u32Length >= 1 )
        {
            strValue.resize( u32Length - 1 );  // skip last NULL character
            nEspError = nvs_get_str( m_hNvs,
                                     strKey.c_str(),
                                     const_cast<char*>( strValue.data() ),   // NOSONAR   accepted pattern to access internal, well resized buffer
                                     &u32Length );
        }
        else
        {
            // ESP_OK, nothing to be done, string shall be empty
        }

        closeNvs();
    }

    return strValue;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

esp_err_t EspNvsValue::setValueString( const std::string& strValue )
{
    return setValueString( m_strKey, strValue );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

esp_err_t EspNvsValue::setValueString( const std::string& strKey,
                                       const std::string& strValue )
{
    esp_err_t nEspError = openNvs();

    if ( nEspError == ESP_OK )
    {
        nEspError = nvs_set_str( m_hNvs, strKey.c_str(), strValue.c_str() );

        if ( nEspError != ESP_OK )
        {
            vlogError( "setValueString: error while setting value %s", strKey.c_str() );
        }

        closeNvs();
    }

    return nEspError;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void EspNvsValue::setKey( const std::string& strKey )
{
    m_strKey = strKey;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

const std::string& EspNvsValue::key( void ) const
{
    return m_strKey;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void EspNvsValue::closeNvs( void )
{
    if ( m_hNvs != 0 )
    {
        nvs_close( m_hNvs );
        m_hNvs = 0;
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

} // namespace UxEspCppLibrary

