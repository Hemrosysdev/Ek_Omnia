///////////////////////////////////////////////////////////////////////////////
///
/// @file JsonHelper.cpp
///
/// @brief Implementation file of class JsonHelper.
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

#include "JsonHelper.h"

#include <QDebug>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonParseError>

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

JsonHelper::JsonHelper()
{

}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

JsonHelper::~JsonHelper()
{

}

/////////////////////////////////////////////////////////////////
//
//
/////////////////////////////////////////////////////////////////

// static
bool JsonHelper::readJsonFile( const QString & strFile,
                               QJsonDocument & jsonDoc )
{
    bool  bSuccess = false;
    QFile file( strFile );
    jsonDoc = QJsonDocument();

    if ( file.open( QIODevice::ReadOnly ) )
    {
        QByteArray array = file.readAll();
        file.close();

        QJsonParseError jsonError;
        jsonDoc = QJsonDocument( QJsonDocument::fromJson( array, &jsonError ) );

        if ( jsonError.error != QJsonParseError::NoError )
        {
            qCritical() << "JsonHelper::readJsonFile() error while parsing, " << jsonError.errorString();
        }
        else if ( jsonDoc.isNull() )
        {
            qWarning() << "JsonHelper::readJsonFile() json doc of file <" << strFile << "> is null";
        }
        else
        {
            bSuccess = true;
        }
    }
    else
    {
        qWarning() << "JsonHelper::readJsonFile() can't open Json file" << strFile;
    }

    return bSuccess;
}

/////////////////////////////////////////////////////////////////
//
//
/////////////////////////////////////////////////////////////////

// static
bool JsonHelper::readJsonFile( const QString & strFile,
                               QJsonObject &   jsonObj )
{
    bool bSuccess = false;
    jsonObj = QJsonObject();

    QJsonDocument jsonDoc;
    if ( readJsonFile( strFile, jsonDoc ) )
    {
        jsonObj  = jsonDoc.object();
        bSuccess = true;
    }

    return bSuccess;
}

/////////////////////////////////////////////////////////////////
//
//
/////////////////////////////////////////////////////////////////

// static
bool JsonHelper::writeJsonFile( const QString &       strFile,
                                const QJsonDocument & jsonDoc )
{
    bool  bSuccess = false;
    QFile file( strFile );

    if ( file.open( QIODevice::WriteOnly ) )
    {
        file.write( jsonDoc.toJson() );
        file.close();

        bSuccess = true;
    }
    else
    {
        qWarning() << "JsonHelper::writeJsonFile() can't open Json file" << strFile;
    }

    return bSuccess;
}

/////////////////////////////////////////////////////////////////
//
//
/////////////////////////////////////////////////////////////////

// static
bool JsonHelper::writeJsonFile( const QString &     strFile,
                                const QJsonObject & jsonObj )
{
    QJsonDocument doc( jsonObj );

    return writeJsonFile( strFile, doc );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

double JsonHelper::read( const QJsonObject & obj,
                         const QString &     strKey,
                         const double        dDefault,
                         bool *              pError )
{
    double dValue   = dDefault;
    bool   bSuccess = false;

    if ( obj.contains( strKey )
         && obj[strKey].isDouble() )
    {
        dValue   = obj[strKey].toDouble();
        bSuccess = true;
    }

    if ( pError )
    {
        *pError = !bSuccess;
    }

    return dValue;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

int JsonHelper::read( const QJsonObject & obj,
                      const QString &     strKey,
                      const int           nDefault,
                      bool *              pError )
{
    int  nValue   = nDefault;
    bool bSuccess = false;

    if ( obj.contains( strKey )
         && obj[strKey].isDouble() )
    {
        nValue   = obj[strKey].toInt();
        bSuccess = true;
    }

    if ( pError )
    {
        *pError = !bSuccess;
    }

    return nValue;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool JsonHelper::read( const QJsonObject & obj,
                       const QString &     strKey,
                       const bool          bDefault,
                       bool *              pError )
{
    bool bValue   = bDefault;
    bool bSuccess = false;

    if ( obj.contains( strKey )
         && obj[strKey].isBool() )
    {
        bValue   = obj[strKey].toBool();
        bSuccess = true;
    }

    if ( pError )
    {
        *pError = !bSuccess;
    }

    return bValue;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

QString JsonHelper::read( const QJsonObject & obj,
                          const QString &     strKey,
                          const QString &     strDefault,
                          bool *              pError )
{
    QString strValue;
    bool    bSuccess = false;

    if ( obj.contains( strKey )
         && obj[strKey].isString() )
    {
        strValue = obj[strKey].toString();
        bSuccess = true;
    }
    else
    {
        strValue = strDefault;
    }

    if ( pError )
    {
        *pError = !bSuccess;
    }

    return strValue;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

QString JsonHelper::read( const QJsonObject & obj,
                          const QString &     strKey,
                          const char *        pszDefault,
                          bool *              pError )
{
    QString strValue;
    bool    bSuccess = false;

    if ( obj.contains( strKey )
         && obj[strKey].isString() )
    {
        strValue = obj[strKey].toString();
        bSuccess = true;
    }
    else if ( pszDefault )
    {
        // nothing to be done
    }
    else
    {
        strValue = pszDefault;
    }

    if ( pError )
    {
        *pError = !bSuccess;
    }

    return strValue;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////
