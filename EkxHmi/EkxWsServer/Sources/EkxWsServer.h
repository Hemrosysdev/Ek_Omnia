///////////////////////////////////////////////////////////////////////////////
///
/// @file EkxWsServer.h
///
/// @brief Header file of class EkxWsServer.
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

#ifndef EkxWsServer_H
#define EkxWsServer_H

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#include <QObject>
#include <QList>
#include <QTimer>
#include <QDateTime>

class QWebSocketServer;
class QWebSocket;

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

class EkxWsServer : public QObject
{
    Q_OBJECT

private:

    enum class DownloadState
    {
        Idle = 0,
        Downloading
    };

    struct PacketHistory
    {
        QDateTime m_dtTime;
        int       m_nReceivedDataSize;
    };

public:

    explicit EkxWsServer( void );

    ~EkxWsServer() override;

    bool create( const quint16   u16Port,
                 const QString & strDownloadDir,
                 const int       nBlockSize,
                 const int       nMaxErrorRepeats );

signals:

    void closed();

private slots:

    void onNewConnection();

    void processTextMessage( const QString strMessage );

    void processBinaryMessage( const QByteArray baMessage );

    void socketDisconnected();

    void processRequestChunkTimeout();

    void processSessionTimeout();

private:

    void startDownload( QWebSocket * const pClient,
                        const QString &    strDownloadFile,
                        const int          nFileSize );

    void stopDownload( QWebSocket * const pClient,
                       const bool         bSuccess,
                       const QString &    strReason,
                       const QString &    strFileState );

    void processChunk( QWebSocket * const pClient,
                       const int          nPos,
                       const QByteArray & baTransferData,
                       const bool         bDecode );

    void processChunk( QWebSocket * const pClient,
                       const QByteArray & baData );

    void processAbort( QWebSocket * const pClient,
                       const bool         bAbort );

    void processStatus( QWebSocket * const pClient,
                        const bool         bError,
                        const QString &    strReason );

    void processClearFile( QWebSocket * const pClient,
                           const QString &    strUploadFile );

    void requestNextChunk();

    void sendStatistic();

    bool writeSwUpdateStateFile( const QString & strProcess,
                                 const QString & strState,
                                 const QString & strErrorReason,
                                 const bool      bProgressReq,
                                 const int       nProgValue );

    static QString rootPath();

    void updateProgress();

private:

    QWebSocketServer *   m_pWebSocketServer { nullptr };

    QList<QWebSocket *>  m_clients;

    quint16              m_u16Port { 8899 };

    QString              m_strDownloadDir { "/storage" };

    DownloadState        m_nDownloadState { DownloadState::Idle };

    QWebSocket *         m_pDownloadClient { nullptr };

    QString              m_strDownloadFile;

    int                  m_nFileSize { 0 };

    QTimer               m_timerRequestChunkTimeout;

    QTimer               m_timerSessionTimeout;

    int                  m_nRequestChunkErrorCnt { 0 };

    QDateTime            m_dtStartOfDownload;

    int                  m_nTotalDownloadChunks { 0 };

    int                  m_nRepeatedDownloadChunks { 0 };

    int                  m_nLastFilePos { 0 };

    int                  m_nBlockSize { 1000 };

    int                  m_nMaxErrorRepeats { 100 };

    int                  m_nReceivedDataSize { 0 };

    int                  m_nReceivedTransferSize { 0 };

    QList<PacketHistory> m_packetHistory;

    int                  m_nProgress { 0 };
};

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#endif // EkxWsServer_H
