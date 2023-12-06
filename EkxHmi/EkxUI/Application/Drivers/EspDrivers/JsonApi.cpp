///////////////////////////////////////////////////////////////////////////////
///
/// @file JsonApi.cpp
///
/// @brief main application entry point of JsonApi.
///
/// @author Ultratronik GmbH
///         Dornierstr. 9
///         D-82205 Gilching
///         Germany
///         http://www.ultratronik.de
///
/// @author written by Florian Sonntag, Forschung & Entwicklung, gesser@ultratronik.de
///
/// @date 07.05.2023
///
/// @copyright Copyright 2023 by Hemro International AG
///            Hemro International AG
///            Länggenstrasse 34
///            CH 8184 Bachenbülach
///            Switzerland
///            Homepage: www.hemrogroup.com
///
///////////////////////////////////////////////////////////////////////////////

#include "JsonApi.h"

#include <QDebug>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QFileInfo>
#include <QProcess>
#include <QThread>

#include "MainStatemachine.h"
#include "LockableFile.h"
#include "EspDriver.h"
#include "LockableFile.h"
#include "HttpServerDriver.h"
#include "DeviceInfoCollector.h"
#include "SqliteInterface.h"
#include "SwUpdateController.h"

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

const QString JsonApi::c_strStoragePath            = "/storage/";
const QString JsonApi::c_strSoftwareUpdateJsonFile = c_strStoragePath + "SoftwareUpdateState.json";
const QString JsonApi::c_strStartUpdateShFile      = "/tmp/start_update.sh";

const QString JsonApi::c_strJsonStartFirmwareUpdate         = "StartFirmwareUpdate";
const QString JsonApi::c_strJsonExecuteSysCommands          = "ExecuteSysCommands";
const QString JsonApi::c_strJsonRequestSoftwareUpdateStatus = "RequestSoftwareUpdateStatus";
const QString JsonApi::c_strJsonSetSoftwareUpdateStatus     = "SetSoftwareUpdateStatus";
const QString JsonApi::c_strJsonPing                        = "Ping";
const QString JsonApi::c_strJsonError                       = "Error";
const QString JsonApi::c_strJsonRequestSystemStatus         = "RequestSystemStatus";
const QString JsonApi::c_strJsonRequestRecipeNames          = "RequestRecipeNames";
const QString JsonApi::c_strJsonRequestRecipeModes          = "RequestRecipeModes";
const QString JsonApi::c_strJsonRequestEventTypes           = "RequestEventTypes";
const QString JsonApi::c_strJsonRequestNotificationTypes    = "RequestNotificationTypes";

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

JsonApi::JsonApi()
{
    m_jsonCommands[c_strJsonStartFirmwareUpdate]         = &JsonApi::jacStartFirmwareUpdate;
    m_jsonCommands[c_strJsonExecuteSysCommands]          = &JsonApi::jacExecuteSysCommands;
    m_jsonCommands[c_strJsonRequestSoftwareUpdateStatus] = &JsonApi::jacRequestSoftwareUpdateStatus;
    m_jsonCommands[c_strJsonRequestSystemStatus]         = &JsonApi::jacRequestSystemStatus;
    m_jsonCommands[c_strJsonSetSoftwareUpdateStatus]     = &JsonApi::jacSetSoftwareUpdateStatus;
    m_jsonCommands[c_strJsonPing]                        = &JsonApi::jacPing;
    m_jsonCommands[c_strJsonRequestRecipeNames]          = &JsonApi::jacRequestRecipeNames;
    m_jsonCommands[c_strJsonRequestRecipeModes]          = &JsonApi::jacRequestRecipeModes;
    m_jsonCommands[c_strJsonRequestEventTypes]           = &JsonApi::jacRequestEventTypes;
    m_jsonCommands[c_strJsonRequestNotificationTypes]    = &JsonApi::jacRequestNotificationTypes;

    {
        ::system( "df | grep /usr/local | cut -f 1 -d ' ' | cut -f 3 -d / > /tmp/df.txt" );
        QFile file( "/tmp/df.txt" );
        if ( file.open( QIODevice::ReadOnly ) )
        {
            m_strActiveAppPartition = file.readAll();
        }
    }
    {
        ::system( "rdev | cut -f 1 -d ' ' | cut -f 3 -d / > /tmp/df.txt" );
        QFile file( "/tmp/df.txt" );
        if ( file.open( QIODevice::ReadOnly ) )
        {
            m_strActiveRfsPartition = file.readAll();
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void JsonApi::create( MainStatemachine * const pMainStatemachine )
{
    m_pMainStatemachine = pMainStatemachine;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool JsonApi::processJsonMessage( const quint32      u32MsgCounter,
                                  const QByteArray & strJson )
{
    bool bSuccess = false;
    m_u32RefMsgCounter = u32MsgCounter;

    QJsonParseError error;

    QJsonObject jsonObj = QJsonDocument::fromJson( strJson, &error ).object();

    if ( jsonObj.isEmpty()
         || error.error != QJsonParseError::NoError )
    {
        qCritical() << "JsonApi::processJsonMessage() JSON object empty or wrong formatted" << strJson;
        replyJson( c_strJsonError, false, "JSON object empty!" );
    }
    else
    {
        bSuccess = true;

        if ( !jsonObj["msgType"].isString() )
        {
            qCritical() << "JsonApi::processJsonMessage() msgType not existing or not of type string, type" << jsonObj["msgType"];
            replyJson( c_strJsonError, false, "msgType not existing or not of type string!" );
        }
        else if ( !m_jsonCommands.contains( jsonObj["msgType"].toString() ) )
        {
            qCritical() << "JsonApi::processJsonMessage() unknown msgType" << jsonObj["msgType"];
            replyJson( c_strJsonError, false, "unknown msgType!" );
        }
        else if ( !( this->*m_jsonCommands[jsonObj["msgType"].toString()] )( jsonObj ) )
        {
            qCritical() << "JsonApi::processJsonMessage() message type processing failed, msgType" << jsonObj;
        }
        else if ( jsonObj["msgType"].toString() != "Ping" )
        {
            // successful, nothing to do
            //qInfo() << "JsonApi::processJsonMessage() JSON command successfully executed, msgType" << jsonObj;
        }
    }

    return bSuccess;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool JsonApi::jacStartFirmwareUpdate( const QJsonObject & jsonObject )
{
    bool    bSuccess   = false;
    QString strSwuFile = c_strStoragePath + jsonObject["swuFile"].toString();

    if ( !QFileInfo::exists( strSwuFile ) )
    {
        qCritical() << "JsonApi::jacStartFirmwareUpdate() file" << strSwuFile << "doesn't exist";
        replyJson( c_strJsonStartFirmwareUpdate, false, "SWU file doesn't exist!" );
    }
    else
    {
        m_pMainStatemachine->swUpdateController()->setSwUploadCompletedState();

        QFile::remove( c_strStartUpdateShFile );

        QProcess process;

        process.start( "swupdate", QStringList() << "-i" << strSwuFile << "-e" << "tmp,startupdate" );

        if ( !process.waitForFinished( 5000 ) )
        {
            replyJson( c_strJsonStartFirmwareUpdate, false,
                       QString( "Cannot unpack extract start script, reason" ).arg( process.errorString() ) );
            qCritical() << "JsonApi::jacStartFirmwareUpdate() Cannot extract start script, reason" << process.errorString();
        }
        else if ( !QFileInfo::exists( c_strStartUpdateShFile ) )
        {
            replyJson( c_strJsonStartFirmwareUpdate, false,
                       "Start script not found after extract!" );
            qCritical() << "JsonApi::jacStartFirmwareUpdate() Start script not found after extract!";
        }
        else
        {
            replyJson( c_strJsonStartFirmwareUpdate, true, "Firmware update successfully started!" );
            bSuccess = true;

            //QThread::msleep( 1000 );

            if ( !process.startDetached( c_strStartUpdateShFile, QStringList() << "-p" << strSwuFile ) )
            {
                replyJson( c_strJsonStartFirmwareUpdate, false,
                           QString( "Cannot start update script, reason" ).arg( process.errorString() ) );
                qCritical() << "JsonApi::jacStartFirmwareUpdate() Cannot start update script, reason" << process.errorString();
            }
        }
    }

    return bSuccess;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool JsonApi::jacExecuteSysCommands( const QJsonObject & jsonObject )
{
    bool bSuccess = false;

    QJsonArray strSysCommands = jsonObject["sysCommands"].toArray();
    int        nTimeout       = jsonObject["timeout"].toInt( 5000 );

    if ( strSysCommands.isEmpty() )
    {
        qCritical() << "JsonApi::jacExecuteSysCommands() tag sysCommands doesn't exist or is empty";
    }
    else
    {
        QString strResult;
        for ( int i = 0; i < strSysCommands.size(); i++ )
        {
            //            QProcess process;
            //            process.start( strSysCommands[i].toString() );
            //            bSuccess = process.waitForFinished( nTimeout );
            //            strResult += process.readAll();

            bSuccess = !system( strSysCommands[i].toString().toStdString().c_str() );

            if ( bSuccess )
            {
                qInfo() << "JsonApi::jacExecuteSysCommands() successful," << strSysCommands[i].toString();
            }
            else
            {
                qCritical() << "JsonApi::jacExecuteSysCommands() timeout after" << nTimeout << "msec";
                break;
            }
        }

        replyJson( c_strJsonExecuteSysCommands, bSuccess, strResult );
    }

    return bSuccess;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool JsonApi::jacRequestSoftwareUpdateStatus( const QJsonObject & jsonObject )
{
    Q_UNUSED( jsonObject );

    bool bSuccess = false;

    LockableFile file( c_strSoftwareUpdateJsonFile );

    if ( file.open( QIODevice::ReadOnly ) )
    {
        QByteArray strContent = file.readAll();
        file.close();

        QJsonObject jsonStatus = QJsonDocument::fromJson( strContent ).object();

        replyJson( c_strJsonRequestSoftwareUpdateStatus, true, jsonStatus );
        bSuccess = true;
    }
    else
    {
        qCritical() << "JsonApi::jacRequestSoftwareUpdateStatus() can't open file" << c_strSoftwareUpdateJsonFile;
        replyJson( c_strJsonRequestSoftwareUpdateStatus, false, "Can't open status file!" );
    }

    return bSuccess;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool JsonApi::jacRequestSystemStatus( const QJsonObject & jsonObject )
{
    Q_UNUSED( jsonObject );

    bool bSuccess = false;

    LockableFile file( c_strSoftwareUpdateJsonFile );

    QJsonObject jsonSwUpdateStatus;

    if ( file.open( QIODevice::ReadOnly ) )
    {
        QByteArray strContent = file.readAll();
        file.close();

        jsonSwUpdateStatus = QJsonDocument::fromJson( strContent ).object();
    }
    else
    {
        qCritical() << "JsonApi::jacRequestSystemStatus() can't open file" << c_strSoftwareUpdateJsonFile;
    }

    QJsonObject jsonStatus;
    jsonStatus["softwareUpdateStatus"]       = jsonSwUpdateStatus;
    jsonStatus["time"]                       = QDateTime::currentDateTime().toString( "dd.MM.yyyy hh:mm.ss" );
    jsonStatus["mcuIgnored"]                 = QFileInfo::exists( "/config/mcu_ignored" );
    jsonStatus["customSplashScreenExisting"] = QFileInfo::exists( "/config/custom-splash-screen" );
    jsonStatus["activeRfsPartition"]         = m_strActiveRfsPartition;
    jsonStatus["activeAppPartition"]         = m_strActiveAppPartition;
    jsonStatus["hmiSerialNo"]                = m_pMainStatemachine->deviceInfoCollector()->ekxUiSerialNo();
    jsonStatus["packageSwVersionNo"]         = m_pMainStatemachine->deviceInfoCollector()->packageSwVersion();
    jsonStatus["packageBuildNo"]             = m_pMainStatemachine->deviceInfoCollector()->packageBuildNo();

    replyJson( c_strJsonRequestSystemStatus, true, jsonStatus );
    bSuccess = true;

    return bSuccess;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool JsonApi::jacSetSoftwareUpdateStatus( const QJsonObject & jsonObject )
{
    bool bSuccess = false;

    QJsonObject jsonData = jsonObject["data"].toObject();

    if ( jsonData.isEmpty() )
    {
        qCritical() << "JsonApi::jacSetSwUpdateState(() tag data() tag data) tag data doesn't exist or is empty";
        replyJson( c_strJsonSetSoftwareUpdateStatus, false, "JSON data empty!" );
    }
    else
    {
        LockableFile file( c_strSoftwareUpdateJsonFile );

        QJsonDocument jsonDoc( jsonData );

        if ( file.open( QIODevice::WriteOnly ) )
        {
            file.write( jsonDoc.toJson() );
            file.close();

            bSuccess = true;

            replyJson( c_strJsonSetSoftwareUpdateStatus, true, "Ok!" );
        }
        else
        {
            replyJson( c_strJsonSetSoftwareUpdateStatus, false, "Can't write status file!" );
        }

    }

    return bSuccess;
}

/*
   RequestRecipeModes

                                    <?php
                                        $query = "select recipe_mode_id, recipe_mode_name from recipe_modes order by recipe_mode_name;";
                                        exec( "sudo /usr/bin/sqlite3 /db_storage/EkxSqlite.db -csv -separator \";\" \"$query\" > /tmp/recipe_modes;sync", $output, $returnCode );

                                        $result = file_get_contents( "/tmp/recipe_modes" );
                                        $result_list = explode( "\n", $result );

                                        for ( $i = 0; $i < count( $result_list ); $i++ )
                                        {
                                            if ( !empty( $result_list[$i] ) )
                                            {
                                                $line_result = explode( ";", $result_list[$i] );
                                                echo "<option value=\"$line_result[0]\">$line_result[1]</option>\n";
                                            }
                                        }
                                    ?>

 */

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool JsonApi::jacRequestRecipeNames( const QJsonObject & jsonObject )
{
    Q_UNUSED( jsonObject );

    bool bSuccess = false;

    SqliteInterface::StringMap list = m_pMainStatemachine->sqliteInterface()->getRecipeNames();

    if ( !list.isEmpty() )
    {
        bSuccess = true;

        QJsonArray jsonArray;
        for ( SqliteInterface::StringMap::const_iterator it = list.constBegin(); it != list.constEnd(); it++ )
        {
            QJsonObject jsonPair;
            jsonPair["key"]   = it.key();
            jsonPair["value"] = it.value();

            jsonArray << jsonPair;
        }

        replyJson( c_strJsonRequestRecipeNames, true, jsonArray );
    }
    else
    {
        replyJson( c_strJsonRequestRecipeNames, false, "Error request recipe names!" );
    }

    return bSuccess;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool JsonApi::jacRequestRecipeModes( const QJsonObject & jsonObject )
{
    Q_UNUSED( jsonObject );

    bool bSuccess = false;

    SqliteInterface::StringMap list = m_pMainStatemachine->sqliteInterface()->getRecipeModes();

    if ( !list.isEmpty() )
    {
        bSuccess = true;

        QJsonArray jsonArray;
        for ( SqliteInterface::StringMap::const_iterator it = list.constBegin(); it != list.constEnd(); it++ )
        {
            QJsonObject jsonPair;
            jsonPair["key"]   = it.key();
            jsonPair["value"] = it.value();

            jsonArray << jsonPair;
        }

        replyJson( c_strJsonRequestRecipeNames, true, jsonArray );
    }
    else
    {
        replyJson( c_strJsonRequestRecipeNames, false, "Error request recipe names!" );
    }

    return bSuccess;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool JsonApi::jacRequestEventTypes( const QJsonObject & jsonObject )
{
    Q_UNUSED( jsonObject );

    bool bSuccess = false;

    SqliteInterface::StringMap list = m_pMainStatemachine->sqliteInterface()->getEventTypes();

    if ( !list.isEmpty() )
    {
        bSuccess = true;

        QJsonArray jsonArray;
        for ( SqliteInterface::StringMap::const_iterator it = list.constBegin(); it != list.constEnd(); it++ )
        {
            QJsonObject jsonPair;
            jsonPair["key"]   = it.key();
            jsonPair["value"] = it.value();

            jsonArray << jsonPair;
        }

        replyJson( c_strJsonRequestEventTypes, true, jsonArray );
    }
    else
    {
        replyJson( c_strJsonRequestEventTypes, false, "Error request recipe names!" );
    }

    return bSuccess;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool JsonApi::jacRequestNotificationTypes( const QJsonObject & jsonObject )
{
    Q_UNUSED( jsonObject );

    bool bSuccess = false;

    SqliteInterface::StringMap list = m_pMainStatemachine->sqliteInterface()->getNotificationTypes();

    if ( !list.isEmpty() )
    {
        bSuccess = true;

        QJsonArray jsonArray;
        for ( SqliteInterface::StringMap::const_iterator it = list.constBegin(); it != list.constEnd(); it++ )
        {
            QJsonObject jsonPair;
            jsonPair["key"]   = it.key();
            jsonPair["value"] = it.value();

            jsonArray << jsonPair;
        }

        replyJson( c_strJsonRequestNotificationTypes, true, jsonArray );
    }
    else
    {
        replyJson( c_strJsonRequestNotificationTypes, false, "Error request recipe names!" );
    }

    return bSuccess;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool JsonApi::jacPing( const QJsonObject & jsonObject )
{
    Q_UNUSED( jsonObject );

    QJsonObject json;

    json["msgType"] = "Pong";

    QByteArray strReply = QJsonDocument( json ).toJson();

    EkxProtocol::Payload::JsonMessage * pPayload = new EkxProtocol::Payload::JsonMessage();
    pPayload->json().setValue( strReply.toStdString() );
    pPayload->refMsgCounter().setValue( m_u32RefMsgCounter );

    m_pMainStatemachine->espDriver()->httpServerDriver()->send( pPayload );

    return true;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void JsonApi::replyJson( const QString & strRefMsgType,
                         const bool      bResult,
                         const QString & strData )
{
    QJsonObject json;

    json["msgType"]    = "Reply";
    json["refMsgType"] = strRefMsgType;
    json["success"]    = bResult;
    json["data"]       = strData;

    QByteArray strReply = QJsonDocument( json ).toJson();

    EkxProtocol::Payload::JsonMessage * pPayload = new EkxProtocol::Payload::JsonMessage();
    pPayload->json().setValue( strReply.toStdString() );
    pPayload->refMsgCounter().setValue( m_u32RefMsgCounter );

    m_pMainStatemachine->espDriver()->httpServerDriver()->send( pPayload );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void JsonApi::replyJson( const QString &     strRefMsgType,
                         const bool          bResult,
                         const QJsonObject & jsonData )
{
    QJsonObject json;

    json["msgType"]    = "Reply";
    json["refMsgType"] = strRefMsgType;
    json["success"]    = bResult;
    json["data"]       = jsonData;

    QByteArray strReply = QJsonDocument( json ).toJson();

    EkxProtocol::Payload::JsonMessage * pPayload = new EkxProtocol::Payload::JsonMessage();
    pPayload->json().setValue( strReply.toStdString() );
    pPayload->refMsgCounter().setValue( m_u32RefMsgCounter );

    m_pMainStatemachine->espDriver()->httpServerDriver()->send( pPayload );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void JsonApi::replyJson( const QString &    strRefMsgType,
                         const bool         bResult,
                         const QJsonArray & jsonData )
{
    QJsonObject json;

    json["msgType"]    = "Reply";
    json["refMsgType"] = strRefMsgType;
    json["success"]    = bResult;
    json["data"]       = jsonData;

    QByteArray strReply = QJsonDocument( json ).toJson();

    EkxProtocol::Payload::JsonMessage * pPayload = new EkxProtocol::Payload::JsonMessage();
    pPayload->json().setValue( strReply.toStdString() );
    pPayload->refMsgCounter().setValue( m_u32RefMsgCounter );

    m_pMainStatemachine->espDriver()->httpServerDriver()->send( pPayload );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////
