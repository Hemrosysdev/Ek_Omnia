///////////////////////////////////////////////////////////////////////////////
///
/// @file EkxWsServer.cpp
///
/// @brief Implementation file of class EkxWsServer.
///
/// @author Ultratronik GmbH \n
///         Dornierstr. 9 \n
///         D-82205 Gilching \n
///         Germany
///         http://www.ultratronik.de
///
/// @author written by Gerd Esser, Forschung & Entwicklung, gesser@ultratronik.de
///
/// @date 27.02.2022
///
/// @copyright Copyright 2022 by Hemro International AG \n
///            Hemro International AG \n
///            Länggenstrasse 34 \n
///            CH 8184 Bachenbülach \n
///            Switzerland \n
///            Homepage: www.hemrogroup.com
///
///////////////////////////////////////////////////////////////////////////////

#include "EkxWsServer.h"

#include <QDebug>
#include <QCoreApplication>
#include <QWebSocketServer>
#include <QWebSocket>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QFileInfo>
#include <algorithm>
#include <QtEndian>

#include "ConsoleReader.h"
#include "LockableFile.h"

#define MAX_PACKET_HISTORY_SIZE  30
#define SWSTATE_PATH                    "/storage/SoftwareUpdateState.json"

#define SWUPDATE_JSON_PROCESS           "Process"
#define SWUPDATE_JSON_STATE             "State"
#define SWUPDATE_JSON_ERROR             "ErrorReason"
#define SWUPDATE_JSON_PROGREQ           "ProgressRequired"
#define SWUPDATE_JSON_PROGVAL           "ProgressValue"

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

EkxWsServer::EkxWsServer( void )
    : QObject( nullptr )
    , m_pWebSocketServer( new QWebSocketServer( "EkxWsServer",
                                                QWebSocketServer::NonSecureMode,
                                                this ) )
{
    connect( m_pWebSocketServer,
             &QWebSocketServer::newConnection,
             this,
             &EkxWsServer::onNewConnection );
    connect( m_pWebSocketServer,
             &QWebSocketServer::closed,
             this,
             &EkxWsServer::closed );
    connect( &m_timerRequestChunkTimeout,
             &QTimer::timeout,
             this,
             &EkxWsServer::processRequestChunkTimeout );
    connect( &m_timerSessionTimeout,
             &QTimer::timeout,
             this,
             &EkxWsServer::processSessionTimeout );

    m_timerRequestChunkTimeout.setSingleShot( true );
    m_timerRequestChunkTimeout.setInterval( 500 );

    m_timerSessionTimeout.setSingleShot( true );
    m_timerSessionTimeout.setInterval( 60000 );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

EkxWsServer::~EkxWsServer()
{
    m_pWebSocketServer->close();
    qDeleteAll( m_clients.begin(), m_clients.end() );

    delete m_pWebSocketServer;
    m_pWebSocketServer = nullptr;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool EkxWsServer::create( const quint16   u16Port,
                          const QString & strDownloadDir,
                          const int       nBlockSize,
                          const int       nMaxErrorRepeats )
{
    bool bSuccess = m_pWebSocketServer->listen( QHostAddress::Any, u16Port );

    m_u16Port          = u16Port;
    m_strDownloadDir   = strDownloadDir;
    m_nBlockSize       = nBlockSize;
    m_nMaxErrorRepeats = nMaxErrorRepeats;

    if ( bSuccess )
    {
        qInfo() << "create(): Successful, port " << u16Port << ", dir " << strDownloadDir;
    }
    else
    {
        qCritical() << "create(): failed, port " << u16Port << ", dir " << strDownloadDir;
    }

    return bSuccess;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

// static
QString EkxWsServer::rootPath()
{
#ifdef TARGETBUILD
    return "";
#else
    return QDir::homePath() + "/EkxData";
#endif
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void EkxWsServer::updateProgress()
{
    int64_t n64Progress = 0;

    if ( m_nFileSize > 0
         && m_nLastFilePos > -1 )
    {
        n64Progress = static_cast<int64_t>( m_nLastFilePos ) * 100 / m_nFileSize;
    }

    if ( n64Progress != m_nProgress )
    {
        m_nProgress = n64Progress;

        writeSwUpdateStateFile( "Update",
                                "UPLOAD_IN_PROGRESS",
                                "",
                                true,
                                m_nProgress );
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void EkxWsServer::onNewConnection()
{
    QWebSocket * pClient = m_pWebSocketServer->nextPendingConnection();

    if ( pClient )
    {
        qInfo() << "onNewConnection(): From " << pClient->localAddress().toString();

        // currently only one connection allowed
        if ( m_clients.size() )
        {
            qInfo() << "onNewConnection(): close it, only one session allowed";
            pClient->deleteLater();
        }
        else
        {
            connect( pClient,
                     &QWebSocket::textMessageReceived,
                     this,
                     &EkxWsServer::processTextMessage );
            connect( pClient,
                     &QWebSocket::binaryMessageReceived,
                     this,
                     &EkxWsServer::processBinaryMessage );
            connect( pClient,
                     &QWebSocket::disconnected,
                     this,
                     &EkxWsServer::socketDisconnected );

            m_clients << pClient;
            m_pDownloadClient = pClient;

            m_timerSessionTimeout.start();
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void EkxWsServer::processTextMessage( const QString strMessage )
{
    QWebSocket * pClient = qobject_cast<QWebSocket *>( sender() );

    qDebug() << "processTextMessage(): Message received" << strMessage.size() << "/" << strMessage;

    if ( pClient )
    {
        m_timerSessionTimeout.start();

        QJsonDocument jsonDoc = QJsonDocument::fromJson( strMessage.toUtf8() );
        QJsonObject   jsonObj = jsonDoc.object();

        bool bUploadFile   = jsonObj.value( "UploadFile" ).isObject();
        bool bUploadStatus = jsonObj.value( "UploadStatus" ).isObject();
        bool bAbortUpload  = jsonObj.value( "AbortUpload" ).isBool();
        bool bClearFile    = jsonObj.value( "ClearFile" ).isString();
        bool bUploadChunk  = jsonObj.value( "UploadChunk" ).isObject();

        if ( bUploadFile )
        {
            QJsonObject jsonUploadFile = jsonObj.value( "UploadFile" ).toObject();

            QString strUploadFile = jsonUploadFile.value( "Name" ).toString();
            int     nSize         = jsonUploadFile.value( "Size" ).toInt();

            startDownload( pClient, strUploadFile, nSize );
        }
        else if ( bUploadStatus )
        {
            QJsonObject jsonUploadStatus = jsonObj.value( "UploadStatus" ).toObject();

            bool    bError    = jsonUploadStatus.value( "Error" ).toBool();
            QString strReason = jsonUploadStatus.value( "Reason" ).toString();

            processStatus( pClient, bError, strReason );
        }
        else if ( bUploadChunk )
        {
            QJsonObject jsonUploadChunk = jsonObj.value( "UploadChunk" ).toObject();

            int        nPos   = jsonUploadChunk.value( "Pos" ).toInt();
            QByteArray baData = jsonUploadChunk.value( "Data" ).toString().toUtf8();

            processChunk( pClient, nPos, baData, true );
        }
        else if ( bAbortUpload )
        {
            bool bAbort = jsonObj.value( "AbortUpload" ).toBool();

            processAbort( pClient, bAbort );
        }
        else if ( bClearFile )
        {
            QString strFile = jsonObj.value( "ClearFile" ).toString();

            processClearFile( pClient, strFile );
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void EkxWsServer::processBinaryMessage( const QByteArray baMessage )
{
    QWebSocket * pClient = qobject_cast<QWebSocket *>( sender() );

    qDebug() << "processBinaryMessage(): Message received size " << baMessage.size();

    if ( pClient )
    {
        m_timerSessionTimeout.start();

        QByteArray baPos = baMessage.left( 4 );
        int        nPos  = *reinterpret_cast<qint32 *>( baPos.data() );

        processChunk( pClient, nPos, baMessage.mid( sizeof( qint32 ) ), false );
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void EkxWsServer::socketDisconnected()
{
    QWebSocket * pClient = qobject_cast<QWebSocket *>( sender() );

    if ( pClient )
    {
        qInfo() << "socketDisconnected(): From " << pClient->localAddress().toString();

        m_clients.removeAll( pClient );
        pClient->deleteLater();

        if ( pClient == m_pDownloadClient )
        {
            m_pDownloadClient = nullptr;
            m_timerRequestChunkTimeout.stop();
            m_timerSessionTimeout.stop();
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void EkxWsServer::processRequestChunkTimeout()
{
    if ( m_nDownloadState == DownloadState::Downloading )
    {
        qWarning() << "processRequestChunkTimeout(): repeat counter" << m_nRequestChunkErrorCnt;

        m_nRequestChunkErrorCnt++;

        if ( m_nRequestChunkErrorCnt >= m_nMaxErrorRepeats )
        {
            stopDownload( m_pDownloadClient, false, "too much errors", "UPLOAD_FAILED" );
        }
        else
        {
            requestNextChunk();
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void EkxWsServer::processSessionTimeout()
{
    qWarning() << "processSessionTimeout()";

    if ( m_pDownloadClient )
    {
        m_clients.removeAll( m_pDownloadClient );
        stopDownload( m_pDownloadClient, false, "session timeout", "UPLOAD_FAILED" );
        m_pDownloadClient->deleteLater();
        m_pDownloadClient = nullptr;
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void EkxWsServer::startDownload( QWebSocket * const pClient,
                                 const QString &    strDownloadFile,
                                 const int          nFileSize )
{
    if ( m_nDownloadState != DownloadState::Idle )
    {
        stopDownload( pClient, false, "busy", "" );
    }
    else
    {
        qInfo() << "startDownload(): Download file " << strDownloadFile << ", size " << nFileSize;

        m_strDownloadFile         = m_strDownloadDir + "/" + strDownloadFile;
        m_nFileSize               = nFileSize;
        m_nRequestChunkErrorCnt   = 0;
        m_dtStartOfDownload       = QDateTime::currentDateTime();
        m_nTotalDownloadChunks    = 0;
        m_nRepeatedDownloadChunks = 0;
        m_nReceivedDataSize       = 0;
        m_nReceivedTransferSize   = 0;
        m_nLastFilePos            = -1;
        m_packetHistory.clear();

        m_nDownloadState = DownloadState::Downloading;

        requestNextChunk();

        writeSwUpdateStateFile( "Update",
                                "UPLOAD_IN_PROGRESS",
                                "",
                                true,
                                0 );
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void EkxWsServer::stopDownload( QWebSocket * const pClient,
                                const bool         bSuccess,
                                const QString &    strReason,
                                const QString &    strFileState )
{
    qInfo() << "stopDownload(): success" << bSuccess << ", reason" << strReason;

    if ( pClient )
    {
        if ( m_pDownloadClient == pClient )
        {
            sendStatistic();
        }

        QJsonObject jsonStop;
        jsonStop["Success"] = bSuccess;
        jsonStop["Reason"]  = strReason;

        QJsonObject json;
        json.insert( "StopDownload", jsonStop );

        QString strJson = QJsonDocument( json ).toJson();
        pClient->sendTextMessage( strJson );

        if ( ( m_pDownloadClient == pClient )
             && ( m_nDownloadState == DownloadState::Downloading ) )
        {
            m_timerRequestChunkTimeout.stop();
            m_timerSessionTimeout.stop();

            m_strDownloadFile = "";
            m_nFileSize       = 0;
            m_nDownloadState  = DownloadState::Idle;
        }

        writeSwUpdateStateFile( "Update",
                                strFileState,
                                strReason,
                                true,
                                0 );
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void EkxWsServer::processChunk( QWebSocket * const pClient,
                                const int          nPos,
                                const QByteArray & baTransferData,
                                const bool         bDecode )
{
    if ( m_nDownloadState == DownloadState::Downloading )
    {
        qDebug() << "processChunk(): pos" << nPos << ", size" << baTransferData.size();

        QByteArray baDecodedData;
        if ( bDecode )
        {
            baDecodedData = QByteArray::fromBase64( baTransferData );
        }
        else
        {
            baDecodedData = baTransferData;
        }

        m_timerRequestChunkTimeout.stop();

        QFile file( m_strDownloadFile );

        // receive a package out of sync
        if ( file.size() != nPos )
        {
            // do nothing, wait for timeout
            qWarning() << "processChunk(): package out of sync, pos" << nPos << ", size" << baDecodedData.size() << ", expected pos" << file.size();
        }
        // receive a package oversizes the expected file size
        else if ( file.size() + baDecodedData.size() > m_nFileSize )
        {
            // do nothing, wait for timeout
            qWarning() << "processChunk(): package overshoots EOF, pos" << nPos << ", size" << baDecodedData.size() << ", file size" << m_nFileSize;
        }
        else if ( file.open( QIODevice::Append ) )
        {
            file.write( baDecodedData );
            file.flush();
            file.close();

            m_nRequestChunkErrorCnt  = 0;
            m_nReceivedDataSize     += baDecodedData.size();
            m_nReceivedTransferSize += baTransferData.size();
            //m_packetHistory << PacketHistory { QDateTime::currentDateTime(), m_nReceivedDataSize };
            //if ( m_packetHistory.size() > MAX_PACKET_HISTORY_SIZE )
            //{
            //    m_packetHistory.takeFirst();
            //}

            requestNextChunk();
        }
        else
        {
            stopDownload( pClient, false, "error open dest file", "UPLOAD_FAILED" );
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void EkxWsServer::processChunk( QWebSocket * const pClient,
                                const QByteArray & baData )
{
    if ( m_nDownloadState == DownloadState::Downloading )
    {
        qDebug() << "processChunk(): size" << baData.size();

        QFile file( m_strDownloadFile );

        if ( file.open( QIODevice::Append ) )
        {
            file.write( baData );
            file.flush();
            file.close();

            requestNextChunk();
        }
        else
        {
            stopDownload( pClient, false, "error open dest file", "UPLOAD_FAILED" );
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void EkxWsServer::processAbort( QWebSocket * const pClient,
                                const bool         bAbort )
{
    qCritical() << "processAbort(): abort" << bAbort;

    if ( m_pDownloadClient == pClient
         && m_nDownloadState == DownloadState::Downloading
         && bAbort )
    {
        stopDownload( pClient, false, "client abort", "UPLOAD_ABORTED" );
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void EkxWsServer::processStatus( QWebSocket * const pClient,
                                 const bool         bError,
                                 const QString &    strReason )
{
    qInfo() << "processStatus(): error" << bError << ", reason:" << strReason;

    if ( pClient == m_pDownloadClient
         && m_nDownloadState == DownloadState::Downloading )
    {
        if ( bError )
        {
            stopDownload( pClient, false, "client status error", "UPLOAD_FAILED" );
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void EkxWsServer::processClearFile( QWebSocket * const pClient,
                                    const QString &    strUploadFile )
{
    qInfo() << "processClearFile(): file " << m_strDownloadDir + "/" + strUploadFile;

    if ( pClient )
    {
        if ( m_nDownloadState == DownloadState::Idle )
        {
            QFile::remove( m_strDownloadDir + "/" + strUploadFile );
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void EkxWsServer::requestNextChunk()
{
    if ( m_nDownloadState == DownloadState::Downloading )
    {
        QFileInfo info( m_strDownloadFile );

        if ( info.size() >= m_nFileSize )
        {
            stopDownload( m_pDownloadClient, true, "", "UPLOAD_COMPLETED" );
        }
        else
        {
            QJsonObject jsonChunk;
            jsonChunk["Pos"]      = info.size();
            jsonChunk["Size"]     = std::min( m_nBlockSize, m_nFileSize - static_cast<int>( info.size() ) );
            jsonChunk["Progress"] = QString( "%1%" ).arg( static_cast<double>( static_cast<int>( info.size() * 1000 / m_nFileSize ) ) / 10.0, 0, 'f', 1 );

            //if ( m_packetHistory.size() > 1 )
            //{
            //    PacketHistory first = m_packetHistory.first();
            //    PacketHistory last  = m_packetHistory.last();

            //int nDataTransferRate = ( last.m_nReceivedDataSize - first.m_nReceivedDataSize ) * 1000
            //                        / first.m_dtTime.msecsTo( QDateTime::currentDateTime() );

            quint64 u64TimeEnroute = m_dtStartOfDownload.msecsTo( QDateTime::currentDateTime() );

            int nEffectiveTransferRate = 0;
            int nRealTransferRate      = 0;

            if ( u64TimeEnroute )
            {
                nEffectiveTransferRate = static_cast<int>( static_cast<quint64>( m_nReceivedDataSize ) * 1000
                                                           / u64TimeEnroute );
                nRealTransferRate = static_cast<int>( static_cast<quint64>( m_nReceivedTransferSize ) * 1000
                                                      / u64TimeEnroute );
                jsonChunk["EffTransferRate"]  = nEffectiveTransferRate;
                jsonChunk["RealTransferRate"] = nRealTransferRate;
            }
            else
            {
                jsonChunk["EffTransferRate"]  = "n/a";
                jsonChunk["RealTransferRate"] = "n/a";
            }

            if ( nEffectiveTransferRate )
            {
                jsonChunk["EtaSec"] = ( m_nFileSize - static_cast<int>( info.size() ) ) / nEffectiveTransferRate;
            }
            else
            {
                jsonChunk["EtaSec"] = "n/a";
            }
            //}
            //else
            //{
            //    jsonChunk["EffTransferRate"] = "n/a";
            //    jsonChunk["RealTransferRate"] = "n/a";
            //    jsonChunk["EtaSec"] = "n/a";
            //}

            QJsonObject json;
            json.insert( "RequestChunk", jsonChunk );

            QString strJson = QJsonDocument( json ).toJson();
            m_pDownloadClient->sendTextMessage( strJson );

            qDebug() << "requestNextChunk(): pos" << info.size() << "(" << ( info.size() * 100 / m_nFileSize ) << "%)";

            m_timerRequestChunkTimeout.start();

            m_nTotalDownloadChunks++;
            if ( m_nLastFilePos == info.size() )
            {
                m_nRepeatedDownloadChunks++;
            }
            m_nLastFilePos = info.size();

            updateProgress();
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void EkxWsServer::sendStatistic()
{
    qInfo() << "sendStatistic()";

    if ( m_pDownloadClient )
    {
        QDateTime dtNow = QDateTime::currentDateTime();

        QJsonObject jsonStatistic;
        jsonStatistic["StartTime"]        = m_dtStartOfDownload.toString( "yyyy/MM/dd hh:mm:ss" );
        jsonStatistic["EndTime"]          = dtNow.toString( "yyyy/MM/dd hh:mm:ss" );
        jsonStatistic["DurationSec"]      = static_cast<int>( m_dtStartOfDownload.secsTo( dtNow ) );
        jsonStatistic["TotalChunks"]      = m_nTotalDownloadChunks;
        jsonStatistic["RepeatedChunks"]   = m_nRepeatedDownloadChunks;
        jsonStatistic["FileSize"]         = m_nFileSize;
        jsonStatistic["ReceivedDataSize"] = m_nReceivedDataSize;
        if ( m_dtStartOfDownload.secsTo( dtNow ) )
        {
            jsonStatistic["EffTranferRate"]  = m_nReceivedDataSize / static_cast<int>( m_dtStartOfDownload.secsTo( dtNow ) );
            jsonStatistic["RealTranferRate"] = m_nReceivedTransferSize / static_cast<int>( m_dtStartOfDownload.secsTo( dtNow ) );
        }
        else
        {
            jsonStatistic["EffTranferRate"]  = "n/a";
            jsonStatistic["RealTranferRate"] = "n/a";
        }

        QJsonObject json;
        json.insert( "Statistic", jsonStatistic );

        QString strJson = QJsonDocument( json ).toJson();
        m_pDownloadClient->sendTextMessage( strJson );
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool EkxWsServer::writeSwUpdateStateFile( const QString & strProcess,
                                          const QString & strState,
                                          const QString & strErrorReason,
                                          const bool      bProgressReq,
                                          const int       nProgValue )
{
    bool success = false;

    if ( !strState.isEmpty() )
    {
        LockableFile file( rootPath() + SWSTATE_PATH );

        if ( file.open( QIODevice::WriteOnly ) )
        {
            QJsonObject swUpdateObj;

            swUpdateObj[SWUPDATE_JSON_PROCESS] = strProcess;
            swUpdateObj[SWUPDATE_JSON_STATE]   = strState;
            swUpdateObj[SWUPDATE_JSON_ERROR]   = strErrorReason;
            swUpdateObj[SWUPDATE_JSON_PROGREQ] = bProgressReq;
            swUpdateObj[SWUPDATE_JSON_PROGVAL] = nProgValue;

            QJsonDocument jsonDoc( swUpdateObj );

            file.write( jsonDoc.toJson() );
            file.flush();
            file.close();

            success = true;
        }
    }

    return success;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////
