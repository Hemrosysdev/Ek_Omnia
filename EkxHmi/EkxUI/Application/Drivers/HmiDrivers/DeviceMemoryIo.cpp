///////////////////////////////////////////////////////////////////////////////
///
/// @file DeviceMemoryIo.cpp
///
/// @brief Implementation file of class DeviceMemoryIo.
///
/// @author Ultratronik GmbH
///         Dornierstr. 9
///         D-82205 Gilching
///         Germany
///         http://www.ultratronik.de
///
/// @author written by Gerd Esser, Forschung & Entwicklung, gesser@ultratronik.de
///
/// @date 22.09.2022
///
/// @copyright Copyright 2022 by Hemro International AG
///            Hemro International AG
///            Länggenstrasse 34
///            CH 8184 Bachenbülach
///            Switzerland
///            Homepage: www.hemrogroup.com
///
///////////////////////////////////////////////////////////////////////////////

#include "DeviceMemoryIo.h"

#include <QDebug>

#ifdef __linux__
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#endif

namespace SystemIo
{

const QString DeviceMemoryIo::c_strCpuStm32Mp157 = "STM32MP157";

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

DeviceMemoryIo::DeviceMemoryIo()
{
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

DeviceMemoryIo::~DeviceMemoryIo()
{

}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool DeviceMemoryIo::loadRegisterMap( const QString & strCpuName )
{
    qInfo() << "DeviceMemoryIo::loadRegisterMap()" << strCpuName;

    bool bSuccess = false;

    m_loadedRegisterNameMap.clear();
    m_strCpuName.clear();

    if ( strCpuName == c_strCpuStm32Mp157 )
    {
        m_loadedRegisterNameMap.clear();
        m_strCpuName = strCpuName;

        loadRegister( "ADC1", 0x48003000, "ADC_ISR", 0x00 );
        loadRegister( "ADC1", 0x48003000, "ADC_IER", 0x04 );
        loadRegister( "ADC1", 0x48003000, "ADC_CR", 0x08 );
        loadRegister( "ADC1", 0x48003000, "ADC_CFGR", 0x0C );
        loadRegister( "ADC1", 0x48003000, "ADC_CFGR2", 0x10 );
        loadRegister( "ADC1", 0x48003000, "ADC_SMPR1", 0x14 );
        loadRegister( "ADC1", 0x48003000, "ADC_SMPR2", 0x18 );
        loadRegister( "ADC1", 0x48003000, "ADC_PCSEL", 0x1C );
        loadRegister( "ADC1", 0x48003000, "ADC_LTR1", 0x20 );
        loadRegister( "ADC1", 0x48003000, "ADC_HTR1", 0x24 );
        loadRegister( "ADC1", 0x48003000, "ADC_SQR1", 0x30 );
        loadRegister( "ADC1", 0x48003000, "ADC_SQR2", 0x34 );
        loadRegister( "ADC1", 0x48003000, "ADC_SQR3", 0x38 );
        loadRegister( "ADC1", 0x48003000, "ADC_SQR4", 0x3C );
        loadRegister( "ADC1", 0x48003000, "ADC_DR", 0x40 );
        loadRegister( "ADC1", 0x48003000, "ADC_JSQR", 0x4C );
        loadRegister( "ADC1", 0x48003000, "ADC_OFR1", 0x60 );
        loadRegister( "ADC1", 0x48003000, "ADC_OFR2", 0x64 );
        loadRegister( "ADC1", 0x48003000, "ADC_OFR3", 0x68 );
        loadRegister( "ADC1", 0x48003000, "ADC_OFR4", 0x6C );
        loadRegister( "ADC1", 0x48003000, "ADC_JDR1", 0x80 );
        loadRegister( "ADC1", 0x48003000, "ADC_JDR2", 0x84 );
        loadRegister( "ADC1", 0x48003000, "ADC_JDR3", 0x88 );
        loadRegister( "ADC1", 0x48003000, "ADC_JDR4", 0x8C );
        loadRegister( "ADC1", 0x48003000, "ADC_AWD2CR", 0xA0 );
        loadRegister( "ADC1", 0x48003000, "ADC_AWD3CR", 0xA4 );
        loadRegister( "ADC1", 0x48003000, "ADC_LTR2", 0xB0 );
        loadRegister( "ADC1", 0x48003000, "ADC_HTR2", 0xB4 );
        loadRegister( "ADC1", 0x48003000, "ADC_LTR3", 0xB8 );
        loadRegister( "ADC1", 0x48003000, "ADC_HTR3", 0xBC );
        loadRegister( "ADC1", 0x48003000, "ADC_DIFSEL", 0xC0 );
        loadRegister( "ADC1", 0x48003000, "ADC_CALFACT", 0xC4 );
        loadRegister( "ADC1", 0x48003000, "ADC_CALFACT2", 0xC8 );
        loadRegister( "ADC1", 0x48003000, "ADC_OR", 0xD0 );
        loadRegister( "ADC1", 0x48003000, "ADC_OR", 0xD0 );

        loadRegister( "ADC2", 0x48003100, "ADC_ISR", 0x00 );
        loadRegister( "ADC2", 0x48003100, "ADC_IER", 0x04 );
        loadRegister( "ADC2", 0x48003100, "ADC_CR", 0x08 );
        loadRegister( "ADC2", 0x48003100, "ADC_CFGR", 0x0C );
        loadRegister( "ADC2", 0x48003100, "ADC_CFGR2", 0x10 );
        loadRegister( "ADC2", 0x48003100, "ADC_SMPR1", 0x14 );
        loadRegister( "ADC2", 0x48003100, "ADC_SMPR2", 0x18 );
        loadRegister( "ADC2", 0x48003100, "ADC_PCSEL", 0x1C );
        loadRegister( "ADC2", 0x48003100, "ADC_LTR1", 0x20 );
        loadRegister( "ADC2", 0x48003100, "ADC_HTR1", 0x24 );
        loadRegister( "ADC2", 0x48003100, "ADC_SQR1", 0x30 );
        loadRegister( "ADC2", 0x48003100, "ADC_SQR2", 0x34 );
        loadRegister( "ADC2", 0x48003100, "ADC_SQR3", 0x38 );
        loadRegister( "ADC2", 0x48003100, "ADC_SQR4", 0x3C );
        loadRegister( "ADC2", 0x48003100, "ADC_DR", 0x40 );
        loadRegister( "ADC2", 0x48003100, "ADC_JSQR", 0x4C );
        loadRegister( "ADC2", 0x48003100, "ADC_OFR1", 0x60 );
        loadRegister( "ADC2", 0x48003100, "ADC_OFR2", 0x64 );
        loadRegister( "ADC2", 0x48003100, "ADC_OFR3", 0x68 );
        loadRegister( "ADC2", 0x48003100, "ADC_OFR4", 0x6C );
        loadRegister( "ADC2", 0x48003100, "ADC_JDR1", 0x80 );
        loadRegister( "ADC2", 0x48003100, "ADC_JDR2", 0x84 );
        loadRegister( "ADC2", 0x48003100, "ADC_JDR3", 0x88 );
        loadRegister( "ADC2", 0x48003100, "ADC_JDR4", 0x8C );
        loadRegister( "ADC2", 0x48003100, "ADC_AWD2CR", 0xA0 );
        loadRegister( "ADC2", 0x48003100, "ADC_AWD3CR", 0xA4 );
        loadRegister( "ADC2", 0x48003100, "ADC_LTR2", 0xB0 );
        loadRegister( "ADC2", 0x48003100, "ADC_HTR2", 0xB4 );
        loadRegister( "ADC2", 0x48003100, "ADC_LTR3", 0xB8 );
        loadRegister( "ADC2", 0x48003100, "ADC_HTR3", 0xBC );
        loadRegister( "ADC2", 0x48003100, "ADC_DIFSEL", 0xC0 );
        loadRegister( "ADC2", 0x48003100, "ADC_CALFACT", 0xC4 );
        loadRegister( "ADC2", 0x48003100, "ADC_CALFACT2", 0xC8 );
        loadRegister( "ADC2", 0x48003100, "ADC_OR", 0xD0 );
        loadRegister( "ADC2", 0x48003100, "ADC_OR", 0xD0 );

        loadRegister( "ADC", 0x48003000, "ADC_CCR", 0x308 );
        loadRegister( "RCC", 0x50000000, "RCC_ADCCKSELR", 0x928 );

        loadRegister( "BSEC", 0x5c005000, "BSEC_OTP_CONTROL", 0x004 );
        loadRegister( "BSEC", 0x5c005000, "BSEC_OTP_DATA0", 0x200 );
        loadRegister( "BSEC", 0x5c005000, "BSEC_OTP_HW7", 0x200 + 23 * 4 );

        loadRegister( "VREFBUF", 0x50025000, "VREFBUF_CSR", 0x00 );

        bSuccess = true;
    }

    return bSuccess;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

// static
quint32 DeviceMemoryIo::read32bit( const quint64 u64Address )
{
    quint32 u32Value = -1;

#ifdef __linux__
    int fd = open( "/dev/mem", O_SYNC );
    if ( fd <= 0 )
    {
        qCritical() << "DeviceMemoryIo::readRegister() can't open /dev/mem";
    }
    else
    {
        // Truncate offset to a multiple of the page size, or mmap will fail.
        size_t          pagesize    = sysconf( _SC_PAGE_SIZE );
        off_t           page_base   = ( u64Address / pagesize ) * pagesize;
        off_t           page_offset = u64Address - page_base;
        int             len         = 4;
        unsigned char * mem         = reinterpret_cast<unsigned char *>( mmap( NULL, page_offset + len, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, page_base ) );
        if ( mem == MAP_FAILED )
        {
            qCritical( "DeviceMemoryIo::readRegister(): Can't map memory" );
            return -1;
        }
        close( fd );

        u32Value = *reinterpret_cast<volatile quint32 *>( &mem[page_offset] );
    }
#else
    Q_UNUSED( u64Address );
#endif

    return u32Value;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

quint32 DeviceMemoryIo::read32bit( const QString & strOffsetName,
                                   const QString & strRegisterName ) const
{
    quint32 u32Value = 0;

    QString strKey = strOffsetName + "_" + strRegisterName;

    RegisterNameMap::const_iterator it = m_loadedRegisterNameMap.find( strKey );

    if ( it == m_loadedRegisterNameMap.end() )
    {
        throw 0;
    }
    else
    {
        u32Value = read32bit( it.value().m_u64OffsetAddress + it.value().m_u64RegisterAddress );
    }

    return u32Value;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

quint32 DeviceMemoryIo::write32bitOr( const quint64 u64Address,
                                      const quint32 u32OrValue )
{
    quint32 u32Previous = -1;

#ifdef __linux__
    int fd = open( "/dev/mem", O_RDWR | O_SYNC );
    if ( fd <= 0 )
    {
        qCritical() << "DeviceMemoryIo::writeRegister() can't open /dev/mem";
    }
    else
    {
        // Truncate offset to a multiple of the page size, or mmap will fail.
        size_t          pagesize    = sysconf( _SC_PAGE_SIZE );
        off_t           page_base   = ( u64Address / pagesize ) * pagesize;
        off_t           page_offset = u64Address - page_base;
        int             len         = 4;
        unsigned char * mem         = reinterpret_cast<unsigned char *>( mmap( NULL, page_offset + len, PROT_READ | PROT_WRITE, MAP_SHARED, fd, page_base ) );
        if ( mem == MAP_FAILED )
        {
            qCritical( "DeviceMemoryIo::writeRegister(): Can't map memory" );
            return -1;
        }

        u32Previous                                                = *reinterpret_cast<volatile quint32 *>( &mem[page_offset] );
        *reinterpret_cast<volatile quint32 *>( &mem[page_offset] ) = ( u32Previous | u32OrValue );

        close( fd );
    }
#else
    Q_UNUSED( u64Address );
    Q_UNUSED( u32OrValue );
#endif

    return u32Previous;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

// static
quint32 DeviceMemoryIo::write32bit( const quint64 u64Address,
                                    const quint32 u32Value )
{
    quint32 u32Previous = -1;

#ifdef __linux__
    int fd = open( "/dev/mem", O_RDWR | O_SYNC );
    if ( fd <= 0 )
    {
        qCritical() << "DeviceMemoryIo::writeRegister() can't open /dev/mem";
    }
    else
    {
        // Truncate offset to a multiple of the page size, or mmap will fail.
        size_t          pagesize    = sysconf( _SC_PAGE_SIZE );
        off_t           page_base   = ( u64Address / pagesize ) * pagesize;
        off_t           page_offset = u64Address - page_base;
        int             len         = 4;
        unsigned char * mem         = reinterpret_cast<unsigned char *>( mmap( NULL, page_offset + len, PROT_READ | PROT_WRITE, MAP_SHARED, fd, page_base ) );
        if ( mem == MAP_FAILED )
        {
            qCritical( "DeviceMemoryIo::writeRegister(): Can't map memory" );
            return -1;
        }

        u32Previous                                                = *reinterpret_cast<volatile quint32 *>( &mem[page_offset] );
        *reinterpret_cast<volatile quint32 *>( &mem[page_offset] ) = u32Value;

        close( fd );
    }
#else
    Q_UNUSED( u64Address );
    Q_UNUSED( u32Value );
#endif

    return u32Previous;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

quint32 DeviceMemoryIo::write32bit( const QString & strOffsetName,
                                    const QString & strRegisterName,
                                    const quint32   u32Value )
{
    quint32 u32Previous = 0;

    QString strKey = strOffsetName + "_" + strRegisterName;

    RegisterNameMap::iterator it = m_loadedRegisterNameMap.find( strKey );

    if ( it == m_loadedRegisterNameMap.end() )
    {
        throw 0;
    }
    else
    {
        u32Previous = write32bit( it.value().m_u64OffsetAddress + it.value().m_u64RegisterAddress, u32Value );
    }

    return u32Previous;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void DeviceMemoryIo::dumpRegisterAddressMap() const
{
    qInfo() << "Dump register address map for " << m_strCpuName;

    RegisterAddressMap::const_iterator it;

    for ( it = m_loadedRegisterAddressMap.begin(); it != m_loadedRegisterAddressMap.end(); it++ )
    {
        quint64 u64Address = it.value().m_u64OffsetAddress + it.value().m_u64RegisterAddress;
        qInfo().nospace().noquote() << QString( "  %1 / %2 (%3) = %4" )
            .arg( it.value().m_strOffsetName, -5 )
            .arg( it.value().m_strRegisterName, -15 )
            .arg( u32ToHexString( u64Address ), u32ToHexString( read32bit( u64Address ) ) );
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void DeviceMemoryIo::dumpRegisterAddressMap( std::function<void (QString, quint64, QString, quint64, quint32)> dumpFunc ) const
{
    RegisterAddressMap::const_iterator it;

    for ( it = m_loadedRegisterAddressMap.begin(); it != m_loadedRegisterAddressMap.end(); it++ )
    {
        const quint64 u64Address       = it.value().m_u64OffsetAddress + it.value().m_u64RegisterAddress;
        const quint32 u32RegisterValue = read32bit( u64Address );
        dumpFunc( it.value().m_strOffsetName,
                  it.value().m_u64OffsetAddress,
                  it.value().m_strRegisterName,
                  it.value().m_u64RegisterAddress,
                  u32RegisterValue );
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

QString DeviceMemoryIo::registerAddressMapToCsv() const
{
    QString strCsv;

    RegisterAddressMap::const_iterator it;

    for ( it = m_loadedRegisterAddressMap.begin(); it != m_loadedRegisterAddressMap.end(); it++ )
    {
        quint64 u64Address = it.value().m_u64OffsetAddress + it.value().m_u64RegisterAddress;
        strCsv += QString( "%1;%2;%3;%4\n" ).arg( it.value().m_strOffsetName,
                                                  it.value().m_strRegisterName,
                                                  u32ToHexString( u64Address ),
                                                  u32ToHexString( read32bit( u64Address ) ) );
    }

    return strCsv;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

QString DeviceMemoryIo::u32ToHexString( const quint32 u32Value )
{
    return QString( "0x%1" ).arg( u32Value, 8, 16, QLatin1Char( '0' ) );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void DeviceMemoryIo::loadRegister( const QString & strOffsetName,
                                   const quint64   u64OffsetAddress,
                                   const QString & strRegisterName,
                                   const quint64   u64RegisterAddress )
{
    QString strKey = strOffsetName + "_" + strRegisterName;

    m_loadedRegisterNameMap.insert( strKey, { strOffsetName, u64OffsetAddress, strRegisterName, u64RegisterAddress } );
    m_loadedRegisterAddressMap.insert( u64OffsetAddress + u64RegisterAddress, { strOffsetName, u64OffsetAddress, strRegisterName, u64RegisterAddress } );

}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

} // namespace SystemIo
