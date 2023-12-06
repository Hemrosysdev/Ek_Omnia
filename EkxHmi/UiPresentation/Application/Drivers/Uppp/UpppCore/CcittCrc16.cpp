///////////////////////////////////////////////////////////////////////////////
///
/// @file CcittCrc16.cpp
///
/// @brief Implementation file of class CcittCrc16.
///
/// @author Ultratronik GmbH \n
///         Dornierstr. 9 \n
///         D-82205 Gilching \n
///         Germany
///         http://www.ultratronik.de
///
/// @author written by Gerd Esser, Forschung & Entwicklung, gesser@ultratronik.de
///
/// @date 17.08.2020
///
/// @copyright Copyright 2020 by Hemro International AG \n
///            Hemro International AG \n
///            Länggenstrasse 34 \n
///            CH 8184 Bachenbülach \n
///            Switzerland \n
///            Homepage: www.hemrogroup.com
///
///////////////////////////////////////////////////////////////////////////////

#include "CcittCrc16.h"
#include <QByteArray>
#include <QDebug>

bool CcittCrc16::m_bInitDone = false;
quint16 CcittCrc16::m_u16CrcTable[256];

#define	CRC_POLY_16             0xA001
#define	CRC_POLY_32             0xEDB88320ul
#define	CRC_POLY_64             0x42F0E1EBA9EA3693ull
#define	CRC_POLY_CCITT          0x1021
#define	CRC_POLY_DNP            0xA6BC
#define	CRC_POLY_KERMIT         0x8408
#define	CRC_POLY_SICK           0x8005

#define	CRC_START_8             0x00
#define	CRC_START_16            0x0000
#define	CRC_START_MODBUS        0xFFFF
#define	CRC_START_XMODEM        0x0000
#define	CRC_START_CCITT_1D0F    0x1D0F
#define	CRC_START_CCITT_FFFF    0xFFFF
#define	CRC_START_KERMIT        0x0000
#define	CRC_START_SICK          0x0000
#define	CRC_START_DNP           0x0000
#define	CRC_START_32            0xFFFFFFFFul
#define	CRC_START_64_ECMA       0x0000000000000000ull
#define	CRC_START_64_WE         0xFFFFFFFFFFFFFFFFull

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

CcittCrc16::CcittCrc16()
    : m_u16Value( 0 )
{
    if ( !m_bInitDone )
    {
        initTable();
    }

    init();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

CcittCrc16::~CcittCrc16()
{

}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void CcittCrc16::init()
{
    m_u16Value = 0xFFFF;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

CcittCrc16 &CcittCrc16::operator<<(const quint8 u8Byte)
{
    m_u16Value = ( static_cast<quint16>( m_u16Value << 8 )
                   ^ m_u16CrcTable[ ( ( m_u16Value >> 8 )
                                      ^ static_cast<uint16_t>( u8Byte ) ) & 0xff ] );

    return *this;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

CcittCrc16 &CcittCrc16::operator<<(const int nByte)
{
    return operator<<( static_cast<quint8>( nByte ) );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

CcittCrc16 &CcittCrc16::operator<<(const QByteArray &data)
{
    for ( int i = 0; i < data.size(); i++ )
    {
        *this << static_cast<quint8>( data[i] );
    }

    return *this;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

quint16 CcittCrc16::value() const
{
    return m_u16Value;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

// static
void CcittCrc16::initTable( void )
{
    for ( quint16 i = 0; i < 256; i++ )
    {
        quint16 u16Crc = static_cast<quint16>( i << 8 );

        for ( quint16 j = 0; j < 8; j++ )
        {
            if ( ( u16Crc & 0x8000 ) > 0 )
            {
                u16Crc = ( static_cast<quint16>( u16Crc << 1 ) ^ CRC_POLY_CCITT );
            }
            else
            {
                u16Crc = static_cast<quint16>( u16Crc << 1 );
            }
        }

        m_u16CrcTable[i] = u16Crc;
    }

    //    for ( int k = 0; k < 256; )
    //    {
    //        for ( int l = 0; l < 8; l++ )
    //        {
    //            printf( "0x%04x ", m_u16CrcTable[k++] );
    //        }
    //        printf( "\n" );
    //    }

    m_bInitDone = true;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

