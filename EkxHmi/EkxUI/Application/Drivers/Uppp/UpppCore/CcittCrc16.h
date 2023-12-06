///////////////////////////////////////////////////////////////////////////////
///
/// @file CcittCrc16.h
///
/// @brief Header file of class CcittCrc16.
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

#ifndef CcittCrc16_H
#define CcittCrc16_H

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#include <QtGlobal>

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

class CcittCrc16
{

public:

    CcittCrc16();

    virtual ~CcittCrc16();

    void init( void );

    CcittCrc16 & operator<<( const quint8 u8Byte );

    CcittCrc16 & operator<<( const int nByte );

    CcittCrc16 & operator<<( const QByteArray & data );

    quint16 value( void ) const;

    static void initTable( void );

private:

    quint16          m_u16Value = 0;

    static bool      m_bInitDone;

    static const int c_nCrcTableSize = 256;

    static quint16   m_u16CrcTable[c_nCrcTableSize];

};

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#endif // CcittCrc16_H
