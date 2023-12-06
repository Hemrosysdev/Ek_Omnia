///////////////////////////////////////////////////////////////////////////////
///
/// @file DeviceMemoryIo.h
///
/// @brief Header file of class DeviceMemoryIo.
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

#ifndef DeviceMemoryIo_h
#define DeviceMemoryIo_h

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#include <QtGlobal>
#include <QMap>
#include <QString>
#include <functional>

namespace SystemIo
{

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

class DeviceMemoryIo
{
public:

    static const QString c_strCpuStm32Mp157;

private:

    struct ioRegister_t
    {
        QString m_strOffsetName;
        quint64 m_u64OffsetAddress;
        QString m_strRegisterName;
        quint64 m_u64RegisterAddress;
    };

    using RegisterNameMap    = QMap<QString, ioRegister_t>;
    using RegisterAddressMap = QMap<quint32, ioRegister_t>;

public:

    DeviceMemoryIo();
    virtual ~DeviceMemoryIo();

    bool loadRegisterMap( const QString & strCpuName );

    static quint32 read32bit( const quint64 u64Address );

    quint32 read32bit( const QString & strOffsetName,
                       const QString & strRegisterName ) const;

    static quint32 write32bitOr( const quint64 u64Address,
                                 const quint32 u32OrValue );

    static quint32 write32bit( const quint64 u64Address,
                               const quint32 u32Value );

    quint32 write32bit( const QString & strOffsetName,
                        const QString & strRegisterName,
                        const quint32   u32Value );

    void dumpRegisterAddressMap() const;

    void dumpRegisterAddressMap( std::function<void (QString, quint64, QString, quint64, quint32)> dumpFunc ) const;

    QString registerAddressMapToCsv() const;

    static QString u32ToHexString( const quint32 u32Value );

private:

    void loadRegister( const QString & strOffsetName,
                       const quint64   u64OffsetAddress,
                       const QString & strRegisterName,
                       const quint64   u64RegisterAddress );

private:

    QString            m_strCpuName;

    RegisterNameMap    m_loadedRegisterNameMap;

    RegisterAddressMap m_loadedRegisterAddressMap;

};

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

} // namespace SystemIo

#endif // DeviceMemoryIo_h
