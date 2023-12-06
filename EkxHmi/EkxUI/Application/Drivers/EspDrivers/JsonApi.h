///////////////////////////////////////////////////////////////////////////////
///
/// @file JsonApi.h
///
/// @brief Header file of class JsonApi.
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

#ifndef JsonApi_h
#define JsonApi_h

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#include <QObject>
#include <QMap>

class MainStatemachine;

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

class JsonApi : public QObject
{
    Q_OBJECT

private:

    using callbackFunction = bool ( JsonApi::* )( const QJsonObject & jsonObject );

    using callbackMap = QMap<QString, callbackFunction>;

    static const QString c_strSoftwareUpdateJsonFile;
    static const QString c_strStartUpdateShFile;

    static const QString c_strJsonStartFirmwareUpdate;
    static const QString c_strJsonRequestSoftwareUpdateStatus;
    static const QString c_strJsonRequestSystemStatus;
    static const QString c_strJsonExecuteSysCommands;
    static const QString c_strJsonSetSoftwareUpdateStatus;
    static const QString c_strJsonRequestRecipeNames;
    static const QString c_strJsonRequestRecipeModes;
    static const QString c_strJsonRequestEventTypes;
    static const QString c_strJsonRequestNotificationTypes;
    static const QString c_strJsonPing;
    static const QString c_strJsonError;
    static const QString c_strStoragePath;

public:

    explicit JsonApi();

    void create( MainStatemachine * const pMainStatemachine );

    bool processJsonMessage( const quint32      u32MsgCounter,
                             const QByteArray & strJson );

private:

    bool jacStartFirmwareUpdate( const QJsonObject & jsonObject );

    bool jacExecuteSysCommands( const QJsonObject & jsonObject );

    bool jacRequestSoftwareUpdateStatus( const QJsonObject & jsonObject );

    bool jacRequestSystemStatus( const QJsonObject & jsonObject );

    bool jacRequestRecipeNames( const QJsonObject & jsonObject );

    bool jacRequestRecipeModes( const QJsonObject & jsonObject );

    bool jacRequestEventTypes( const QJsonObject & jsonObject );

    bool jacRequestNotificationTypes( const QJsonObject & jsonObject );

    bool jacSetSoftwareUpdateStatus( const QJsonObject & jsonObject );

    bool jacPing( const QJsonObject & jsonObject );

    void replyJson( const QString & strRefMsgType,
                    const bool      bResult,
                    const QString & strData );

    void replyJson( const QString &     strRefMsgType,
                    const bool          bResult,
                    const QJsonObject & jsonData );

    void replyJson( const QString &    strRefMsgType,
                    const bool         bResult,
                    const QJsonArray & jsonData );

private:

    MainStatemachine * m_pMainStatemachine { nullptr };

    callbackMap        m_jsonCommands;

    quint32            m_u32RefMsgCounter { 0 };

    QString            m_strActiveRfsPartition;

    QString            m_strActiveAppPartition;

};

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#endif // JsonApi_h
