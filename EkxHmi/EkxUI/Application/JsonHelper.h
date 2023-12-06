///////////////////////////////////////////////////////////////////////////////
///
/// @file JsonHelper.h
///
/// @brief Header file of class JsonHelper.
///
/// @author Ultratronik GmbH
///         Dornierstr. 9
///         D-82205 Gilching
///         Germany
///         http://www.ultratronik.de
///
/// @author written by Gerd Esser, Forschung & Entwicklung, gesser@ultratronik.de
///
/// @date 18.11.2022
///
/// @copyright Copyright 2022 by Hemro International AG
///            Hemro International AG
///            Länggenstrasse 34
///            CH 8184 Bachenbülach
///            Switzerland
///            Homepage: www.hemrogroup.com
///
///////////////////////////////////////////////////////////////////////////////

#ifndef JsonHelper_h
#define JsonHelper_h

#include <QString>

class QJsonDocument;
class QJsonObject;

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

class JsonHelper
{

public:

    JsonHelper();
    virtual ~JsonHelper();

    static bool readJsonFile( const QString & strFile,
                              QJsonDocument & jsonDoc );

    static bool readJsonFile( const QString & strFile,
                              QJsonObject &   jsonObj );

    static bool writeJsonFile( const QString &       strFile,
                               const QJsonDocument & jsonDoc );

    static bool writeJsonFile( const QString &     strFile,
                               const QJsonObject & jsonObj );

    static double read( const QJsonObject & obj,
                        const QString &     strKey,
                        const double        dDefault = 0.0,
                        bool *              pError = nullptr );

    static int read( const QJsonObject & obj,
                     const QString &     strKey,
                     const int           nDefault = 0,
                     bool *              pError   = nullptr );

    static bool read( const QJsonObject & obj,
                      const QString &     strKey,
                      const bool          bDefault = false,
                      bool *              pError = nullptr );

    static QString read( const QJsonObject & obj,
                         const QString &     strKey,
                         const QString &     strDefault = "",
                         bool *              pError = nullptr );

    static QString read( const QJsonObject & obj,
                         const QString &     strKey,
                         const char *        pszDefault = nullptr,
                         bool *              pError     = nullptr );

};

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#endif // JsonHelper_h
