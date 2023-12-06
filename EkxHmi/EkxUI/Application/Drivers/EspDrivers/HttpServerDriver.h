///////////////////////////////////////////////////////////////////////////////
///
/// @file HttpServerDriver.h
///
/// @brief Header file of class HttpServerDriver.
///
/// @author Ultratronik GmbH
///         Dornierstr. 9
///         D-82205 Gilching
///         Germany
///         http://www.ultratronik.de
///
/// @author written by Gerd Esser, Forschung & Entwicklung, gesser@ultratronik.de
///
/// @date 06.05.2023
///
/// @copyright Copyright 2023 by Hemro International AG
///            Hemro International AG
///            Länggenstrasse 34
///            CH 8184 Bachenbülach
///            Switzerland
///            Homepage: www.hemrogroup.com
///
///////////////////////////////////////////////////////////////////////////////

#ifndef HttpServerDriver_h
#define HttpServerDriver_h

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#include "EspDeviceDriver.h"

#include <QTimer>
#include <QDateTime>

class JsonApi;
class DeviceInfoCollector;

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

class HttpServerDriver : public EspDeviceDriver
{
    Q_OBJECT

private:

    static const QString c_strStoragePath;
    static const QString c_strConfigPath;

private:

    enum class FileUploadState
    {
        Idle = 0,
        Start,
        Data,
        Stop,
        Timeout
    };

public:

    explicit HttpServerDriver( const EkxProtocol::DriverId u8DriverId );

    void create( EspProtocolStack * const    pProtocolStack,
                 JsonApi * const             pJsonApi,
                 DeviceInfoCollector * const pDeviceInfoCollector );

    void processMessageFrame( const EkxProtocol::MessageFrame & frame ) override;

    void processAlive( const bool bAlive ) override;

    FileUploadState fileUploadState() const;

    void cleanStorage();

signals:

    void fileUploadStateChanged( const FileUploadState nFileUploadState );

private:

    void processFileDownloadStartRequest( const EkxProtocol::MessageFrame & frame );

    void processFileDownloadDataRequest( const EkxProtocol::MessageFrame & frame );

    void triggerFileUploadStart( const EkxProtocol::MessageFrame & frame );

    void triggerFileUploadData( const EkxProtocol::MessageFrame & frame );

    void triggerFileUploadStop( const EkxProtocol::MessageFrame & frame );

    void processJsonMessage( const EkxProtocol::MessageFrame & frame );

    void setFileUploadState( const FileUploadState nFileUploadState );

    void processTimeoutFileUpload();

    bool buildUploadFile( const std::string & strData );

    void abortFileUpload();

    void dumpFileUploadStatistics();

    QString createFileSuffix() const;

    bool zipFile( const QString & strWorkingDir,
                  const QString & strFromFile,
                  const QString & strToFile ) const;

    bool prepareDatabaseDownload( const EkxProtocol::Payload::FileDownloadStartRequest * const pRequest,
                                  EkxProtocol::Payload::FileDownloadStartResponse * const      pResponse );

    bool prepareAgsaLdtLogsDownload( const EkxProtocol::Payload::FileDownloadStartRequest * const pRequest,
                                     EkxProtocol::Payload::FileDownloadStartResponse * const      pResponse );

    bool prepareEventStatisticsDownload( const EkxProtocol::Payload::FileDownloadStartRequest * const pRequest,
                                         EkxProtocol::Payload::FileDownloadStartResponse * const      pResponse );

    bool prepareGrindStatisticsDownload( const EkxProtocol::Payload::FileDownloadStartRequest * const pRequest,
                                         EkxProtocol::Payload::FileDownloadStartResponse * const      pResponse );

    bool prepareCounterStatisticsDownload( const EkxProtocol::Payload::FileDownloadStartRequest * const pRequest,
                                           EkxProtocol::Payload::FileDownloadStartResponse * const      pResponse );

    bool prepareCustomSplashScreenDownload( const EkxProtocol::Payload::FileDownloadStartRequest * const pRequest,
                                            EkxProtocol::Payload::FileDownloadStartResponse * const      pResponse );

private:

    JsonApi *             m_pJsonApi { nullptr };

    DeviceInfoCollector * m_pDeviceInfoCollector { nullptr };

    QString               m_strUploadFile;

    FileUploadState       m_nFileUploadState { FileUploadState::Idle };

    quint32               m_u32FileUploadNextOffset { 0 };

    quint32               m_u32FileUploadSize { 0 };

    QTimer                m_timerFileUpload;

    QDateTime             m_dtStartTime;

    QString               m_strFileDownloadLastGeneratedFile;

    quint32               m_u32FileDownloadLastStartMsgCounter;

};

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#endif // HttpServerDriver_h
