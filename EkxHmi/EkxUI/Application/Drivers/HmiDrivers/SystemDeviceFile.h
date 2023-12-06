///////////////////////////////////////////////////////////////////////////////
///
/// @file SystemDeviceFile.h
///
/// @brief Header file of class SystemDeviceFile.
///
/// @author Ultratronik GmbH
///         Dornierstr. 9
///         D-82205 Gilching
///         Germany
///         http://www.ultratronik.de
///
/// @author written by Gerd Esser, Forschung & Entwicklung, gesser@ultratronik.de
///
/// @date 20.09.2022
///
/// @copyright Copyright 2022 by Hemro International AG
///            Hemro International AG
///            Länggenstrasse 34
///            CH 8184 Bachenbülach
///            Switzerland
///            Homepage: www.hemrogroup.com
///
///////////////////////////////////////////////////////////////////////////////

#ifndef SystemDeviceFile_h
#define SystemDeviceFile_h

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#include <QFile>

namespace SystemIo
{

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

class SystemDeviceFile : public QFile
{
    Q_OBJECT

public:

    explicit SystemDeviceFile( QObject * pParent = nullptr );
    explicit SystemDeviceFile( const QString & strFile,
                               QObject *       pParent = nullptr );
    ~SystemDeviceFile() override;

    bool writeFile( const QString & strContent );

    bool writeFile( const double dValue );

    bool writeFile( const int nValue );

    bool writeFile( const quint32 u32Value );

    bool readFile( QString & strContent );

    bool readFile( double & dValue );

    bool readFile( int & nValue );

};

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

} // namespace SystemIo

#endif // SystemDeviceFile_h
