///////////////////////////////////////////////////////////////////////////////
///
/// @file HttpServerDriver.cpp
///
/// @brief main application entry point of HttpServerDriver.
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

#include "HttpServerDriver.h"

#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QDateTime>
#include <QCoreApplication>

#include "JsonApi.h"
#include "DeviceInfoCollector.h"

const QString HttpServerDriver::c_strStoragePath = "/storage/";
const QString HttpServerDriver::c_strConfigPath  = "/config/";

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

HttpServerDriver::HttpServerDriver( const EkxProtocol::DriverId u8DriverId )
    : EspDeviceDriver( u8DriverId )
{
    m_timerFileUpload.setInterval( 1000 );
    m_timerFileUpload.setSingleShot( true );

    connect( &m_timerFileUpload, &QTimer::timeout, this, &HttpServerDriver::processTimeoutFileUpload );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void HttpServerDriver::create( EspProtocolStack * const    pProtocolStack,
                               JsonApi * const             pJsonApi,
                               DeviceInfoCollector * const pDeviceInfoCollector )
{
    m_pJsonApi             = pJsonApi;
    m_pDeviceInfoCollector = pDeviceInfoCollector;

    EspDeviceDriver::create( pProtocolStack );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void HttpServerDriver::processMessageFrame( const EkxProtocol::MessageFrame & frame )
{
    const EkxProtocol::Payload::MessagePayload * pPayload = frame.payloadRef().value();

    switch ( pPayload->payloadType() )
    {
        case EkxProtocol::Payload::PayloadType::FileDownloadStartRequest:
            processFileDownloadStartRequest( frame );
            break;
        case EkxProtocol::Payload::PayloadType::FileDownloadDataRequest:
            processFileDownloadDataRequest( frame );
            break;
        case EkxProtocol::Payload::PayloadType::FileUploadStart:
            triggerFileUploadStart( frame );
            break;
        case EkxProtocol::Payload::PayloadType::FileUploadData:
            triggerFileUploadData( frame );
            break;
        case EkxProtocol::Payload::PayloadType::FileUploadStop:
            triggerFileUploadStop( frame );
            break;
        case EkxProtocol::Payload::PayloadType::JsonMessage:
            processJsonMessage( frame );
            break;
        default:
            replyNack( frame.driverId().value(),
                       frame.msgCounter().value(),
                       frame.repeatCounter().value(),
                       EkxProtocol::Payload::Nack::NackReason::PayloadBroken );
            break;
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void HttpServerDriver::processAlive( const bool bAlive )
{
    Q_UNUSED( bAlive );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

HttpServerDriver::FileUploadState HttpServerDriver::fileUploadState() const
{
    return m_nFileUploadState;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void HttpServerDriver::cleanStorage()
{
#ifdef TARGETBUILD
    QProcess process;
    process.start( "sh", QStringList() << "-c" << QString( "rm -rf %1*.bin %1*.swu %1*.zip upload.file" ).arg( c_strStoragePath ) );

    if ( !process.waitForFinished( 5000 ) )
    {
        qCritical() << "cleanStorage() error rm";
    }
#endif
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void HttpServerDriver::processFileDownloadStartRequest( const EkxProtocol::MessageFrame & frame )
{
    const EkxProtocol::Payload::FileDownloadStartRequest * const pRequest = dynamic_cast<const EkxProtocol::Payload::FileDownloadStartRequest *>( frame.payloadRef().value() );

    if ( !pRequest )
    {
        replyNack( frame.driverId().value(),
                   frame.msgCounter().value(),
                   frame.repeatCounter().value(),
                   EkxProtocol::Payload::Nack::NackReason::PayloadBroken );
    }
    else
    {
        replyAck( frame.driverId().value(),
                  frame.msgCounter().value(),
                  frame.repeatCounter().value() );

        EkxProtocol::Payload::FileDownloadStartResponse * const pResponse = new EkxProtocol::Payload::FileDownloadStartResponse();
        pResponse->refMsgCounter().setValue( frame.msgCounter().value() );

        // avoid recreating a file upon repeat start requests,
        // where the repeat cause was a problem of the requestor (in fact HMI has
        // already create the requested file with any previous request)
        if ( m_u32FileDownloadLastStartMsgCounter != frame.msgCounter().value() )
        {
            bool bSuccess = false;

            if ( pRequest->fileName().value() == "HemroEkOmniaDatabase" )
            {
                bSuccess = prepareDatabaseDownload( pRequest, pResponse );
            }
            else if ( pRequest->fileName().value() == "HemroEkOmniaAgsaLdtLogs" )
            {
                bSuccess = prepareAgsaLdtLogsDownload( pRequest, pResponse );
            }
            else if ( pRequest->fileName().value() == "HemroEkOmniaEventStatistics" )
            {
                bSuccess = prepareEventStatisticsDownload( pRequest, pResponse );
            }
            else if ( pRequest->fileName().value() == "HemroEkOmniaGrindStatistics" )
            {
                bSuccess = prepareGrindStatisticsDownload( pRequest, pResponse );
            }
            else if ( pRequest->fileName().value() == "HemroEkOmniaCounterStatistics" )
            {
                bSuccess = prepareCounterStatisticsDownload( pRequest, pResponse );
            }
            else if ( pRequest->fileName().value() == "custom-splash-screen" )
            {
                bSuccess = prepareCustomSplashScreenDownload( pRequest, pResponse );
            }
            else
            {
                pResponse->fileName().setValue( pRequest->fileName().value() );
                pResponse->fileSize().setValue( 0 );
                pResponse->status().setValue( EkxProtocol::Payload::FileDownloadStatus::FileNotExisting );

                qCritical() << "HttpServerDriver::processFileDownloadStartRequest() error, unknown file download requested," << pRequest->fileName().value().c_str();
            }

            if ( bSuccess )
            {
                m_u32FileDownloadLastStartMsgCounter = frame.msgCounter().value();
            }
        }
        else
        {
            QString strFullName =  c_strStoragePath + m_strFileDownloadLastGeneratedFile;

            pResponse->fileName().setValue( m_strFileDownloadLastGeneratedFile.toStdString() );
            pResponse->fileSize().setValue( QFileInfo( strFullName ).size() );
            pResponse->status().setValue( EkxProtocol::Payload::FileDownloadStatus::OkOrContinue );
        }

        send( pResponse );
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void HttpServerDriver::processFileDownloadDataRequest( const EkxProtocol::MessageFrame & frame )
{
    const EkxProtocol::Payload::FileDownloadDataRequest * const pPayload = dynamic_cast<const EkxProtocol::Payload::FileDownloadDataRequest *>( frame.payloadRef().value() );

    if ( !pPayload )
    {
        replyNack( frame.driverId().value(),
                   frame.msgCounter().value(),
                   frame.repeatCounter().value(),
                   EkxProtocol::Payload::Nack::NackReason::PayloadBroken );
    }
    else
    {
        replyAck( frame.driverId().value(),
                  frame.msgCounter().value(),
                  frame.repeatCounter().value() );

        QString strFullName;

        if ( pPayload->fileName().value() == "custom-splash-screen" )
        {
            strFullName = QString( "%1%2" ).arg( c_strConfigPath, pPayload->fileName().value().c_str() );
        }
        else
        {
            strFullName = QString( "%1%2" ).arg( c_strStoragePath, pPayload->fileName().value().c_str() );
        }

        QFile file( strFullName );

        EkxProtocol::Payload::FileDownloadDataResponse * const pResponse = new EkxProtocol::Payload::FileDownloadDataResponse();
        pResponse->fileName().setValue( pPayload->fileName().value() );
        pResponse->refMsgCounter().setValue( frame.msgCounter().value() );
        pResponse->chunkOffset().setValue( pPayload->chunkOffset().value() );

        if ( !file.open( QIODevice::ReadOnly ) )
        {
            pResponse->status().setValue( EkxProtocol::Payload::FileDownloadStatus::FileNotExisting );
            qCritical() << "HttpServerDriver::processFileDownloadDataRequest() error, file not existing," << strFullName;
        }
        else if ( file.size() < static_cast<int>( pPayload->chunkOffset().value() + pPayload->chunkSize().size() ) )
        {
            pResponse->status().setValue( EkxProtocol::Payload::FileDownloadStatus::ReadErrorSizeOvershoot );
            qCritical() << "HttpServerDriver::processFileDownloadDataRequest() error, file size overshoot";
        }
        else if ( !file.seek( static_cast<quint64>( pPayload->chunkOffset().value() ) ) )
        {
            pResponse->status().setValue( EkxProtocol::Payload::FileDownloadStatus::ReadErrorSetOffset );
            qCritical() << "HttpServerDriver::processFileDownloadDataRequest() error file seek";
        }
        else
        {
            std::string data( pPayload->chunkSize().value(), 0 );

            file.read( const_cast<char *>( data.data() ), data.size() );

            pResponse->chunkData().setValue( data );

            if ( file.error() )
            {
                pResponse->status().setValue( EkxProtocol::Payload::FileDownloadStatus::ReadError );
                qCritical() << "HttpServerDriver::processFileDownloadDataRequest() error file read";
            }
            else if ( file.atEnd()
                      || ( file.size() == static_cast<int>( pPayload->chunkOffset().value() + pPayload->chunkSize().value() ) ) )
            {
                pResponse->status().setValue( EkxProtocol::Payload::FileDownloadStatus::ReadEndOfFile );
                qInfo() << "HttpServerDriver::processFileDownloadDataRequest() read end of file";
            }
            else
            {
                pResponse->status().setValue( EkxProtocol::Payload::FileDownloadStatus::OkOrContinue );
            }
        }

        send( pResponse );
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void HttpServerDriver::triggerFileUploadStart( const EkxProtocol::MessageFrame & frame )
{
    const EkxProtocol::Payload::FileUploadStart * const pPayload = dynamic_cast<const EkxProtocol::Payload::FileUploadStart *>( frame.payloadRef().value() );

    if ( !pPayload )
    {
        replyNack( frame.driverId().value(),
                   frame.msgCounter().value(),
                   frame.repeatCounter().value(),
                   EkxProtocol::Payload::Nack::NackReason::PayloadBroken );
    }
    else
    {
        switch ( fileUploadState() )
        {
            case FileUploadState::Stop:
            case FileUploadState::Idle:
                m_strUploadFile     = c_strStoragePath + pPayload->fileName().value().c_str();
                m_u32FileUploadSize = pPayload->fileSize().value();
                m_dtStartTime       = QDateTime::currentDateTime();

                m_timerFileUpload.start();
                m_u32FileUploadNextOffset = 0;

                qInfo() << "triggerFileUploadStart() start file upload of file" << m_strUploadFile << "size" << m_u32FileUploadSize;

                cleanStorage();

                setFileUploadState( FileUploadState::Data );
                replyAck( frame.driverId().value(),
                          frame.msgCounter().value(),
                          frame.repeatCounter().value() );
                break;

            default:
                replyNack( frame.driverId().value(),
                           frame.msgCounter().value(),
                           frame.repeatCounter().value(),
                           EkxProtocol::Payload::Nack::NackReason::FileUploadBusy );
                qCritical() << "triggerFileUploadStart() FileUploadBusy, state" << static_cast<int>( fileUploadState() ) << " / " << frame.msgCounter().value() << frame.repeatCounter().value();
                break;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void HttpServerDriver::triggerFileUploadData( const EkxProtocol::MessageFrame & frame )
{
    const EkxProtocol::Payload::FileUploadData * const pPayload = dynamic_cast<const EkxProtocol::Payload::FileUploadData *>( frame.payloadRef().value() );

    if ( !pPayload )
    {
        replyNack( frame.driverId().value(),
                   frame.msgCounter().value(),
                   frame.repeatCounter().value(),
                   EkxProtocol::Payload::Nack::NackReason::PayloadBroken );
    }
    else
    {
        switch ( fileUploadState() )
        {
            case FileUploadState::Idle:
                replyNack( frame.driverId().value(),
                           frame.msgCounter().value(),
                           frame.repeatCounter().value(),
                           EkxProtocol::Payload::Nack::NackReason::FileUploadNotStarted );
                qCritical() << "triggerFileUploadData() FileUploadNotStarted" << frame.msgCounter().value() << frame.repeatCounter().value();
                break;

            case FileUploadState::Data:
                setFileUploadState( FileUploadState::Data );
                m_timerFileUpload.start();

                if ( pPayload->chunkOffset().value() + pPayload->chunkData().value().size() == m_u32FileUploadNextOffset )
                {
                    qWarning() << "triggerFileUploadData() FileUploadOffset, skip already processed chunk, offset" << pPayload->chunkOffset().value() << ", expected" << m_u32FileUploadNextOffset << " / " << frame.msgCounter().value() << frame.repeatCounter().value();
                    // nothing more to be done
                    replyAck( frame.driverId().value(),
                              frame.msgCounter().value(),
                              frame.repeatCounter().value() );
                }
                else if ( pPayload->chunkOffset().value() != m_u32FileUploadNextOffset )
                {
                    qCritical() << "triggerFileUploadData() FileUploadOffset, offset" << pPayload->chunkOffset().value() << ", expected" << m_u32FileUploadNextOffset << " / " << frame.msgCounter().value() << frame.repeatCounter().value();
                    abortFileUpload();
                    replyNack( frame.driverId().value(),
                               frame.msgCounter().value(),
                               frame.repeatCounter().value(),
                               EkxProtocol::Payload::Nack::NackReason::FileUploadOffset );
                }
                else if ( m_u32FileUploadNextOffset + pPayload->chunkData().value().size() > m_u32FileUploadSize )
                {
                    qCritical() << "triggerFileUploadData() FileUploadOverflow" << m_u32FileUploadNextOffset << pPayload->chunkData().value().size() << m_u32FileUploadSize;
                    abortFileUpload();
                    replyNack( frame.driverId().value(),
                               frame.msgCounter().value(),
                               frame.repeatCounter().value(),
                               EkxProtocol::Payload::Nack::NackReason::FileUploadOverflow );
                }
                else if ( !buildUploadFile( pPayload->chunkData().value() ) )
                {
                    qCritical() << "triggerFileUploadData() FileUploadStorage" << frame.msgCounter().value() << frame.repeatCounter().value();
                    abortFileUpload();
                    replyNack( frame.driverId().value(),
                               frame.msgCounter().value(),
                               frame.repeatCounter().value(),
                               EkxProtocol::Payload::Nack::NackReason::FileUploadStorage );
                }
                else
                {
                    //qInfo() << "triggerFileUploadData() data ok" << frame.msgCounter().value() << frame.repeatCounter().value();
                    replyAck( frame.driverId().value(),
                              frame.msgCounter().value(),
                              frame.repeatCounter().value() );
                }
                break;

            case FileUploadState::Timeout:
                qCritical() << "triggerFileUploadData() FileUploadTimeout";
                abortFileUpload();
                replyNack( frame.driverId().value(),
                           frame.msgCounter().value(),
                           frame.repeatCounter().value(),
                           EkxProtocol::Payload::Nack::NackReason::FileUploadTimeout );
                break;

            default:
                qCritical() << "triggerFileUploadData() FileUploadIllegalState, state" << static_cast<int>( fileUploadState() ) << " / " << frame.msgCounter().value() << frame.repeatCounter().value();
                abortFileUpload();
                replyNack( frame.driverId().value(),
                           frame.msgCounter().value(),
                           frame.repeatCounter().value(),
                           EkxProtocol::Payload::Nack::NackReason::FileUploadIllegalState );
                break;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void HttpServerDriver::triggerFileUploadStop( const EkxProtocol::MessageFrame & frame )
{
    const EkxProtocol::Payload::FileUploadStop * const pPayload = dynamic_cast<const EkxProtocol::Payload::FileUploadStop *>( frame.payloadRef().value() );

    if ( !pPayload )
    {
        replyNack( frame.driverId().value(),
                   frame.msgCounter().value(),
                   frame.repeatCounter().value(),
                   EkxProtocol::Payload::Nack::NackReason::PayloadBroken );
    }
    else
    {
        switch ( fileUploadState() )
        {
            case FileUploadState::Idle:
                replyNack( frame.driverId().value(),
                           frame.msgCounter().value(),
                           frame.repeatCounter().value(),
                           EkxProtocol::Payload::Nack::NackReason::FileUploadNotStarted );
                qCritical() << "triggerFileUploadStop() FileUploadNotStarted";
                break;

            case FileUploadState::Data:
                if ( pPayload->stopReason().value() != EkxProtocol::Payload::FileUploadStop::StopReason::Finished )
                {
                    qWarning() << "triggerFileUploadStop() stopped with reason != 'Finished', received size";
                    abortFileUpload();
                    replyAck( frame.driverId().value(),
                              frame.msgCounter().value(),
                              frame.repeatCounter().value() );
                }
                else if ( m_u32FileUploadNextOffset != m_u32FileUploadSize )
                {
                    qCritical() << "triggerFileUploadStop() FileUploadIllegalSize";
                    abortFileUpload();
                    replyNack( frame.driverId().value(),
                               frame.msgCounter().value(),
                               frame.repeatCounter().value(),
                               EkxProtocol::Payload::Nack::NackReason::FileUploadIllegalSize );
                }
                else
                {
                    qInfo() << "triggerFileUploadStop() successfully uploaded";
                    dumpFileUploadStatistics();
                    m_timerFileUpload.stop();
                    setFileUploadState( FileUploadState::Idle );
                    replyAck( frame.driverId().value(),
                              frame.msgCounter().value(),
                              frame.repeatCounter().value() );
                }
                break;

            case FileUploadState::Timeout:
                qCritical() << "triggerFileUploadStop() Timeout";
                abortFileUpload();
                replyNack( frame.driverId().value(),
                           frame.msgCounter().value(),
                           frame.repeatCounter().value(),
                           EkxProtocol::Payload::Nack::NackReason::FileUploadTimeout );
                break;

            default:
                qCritical() << "triggerFileUploadStop() FileUploadIllegalState";
                abortFileUpload();
                replyNack( frame.driverId().value(),
                           frame.msgCounter().value(),
                           frame.repeatCounter().value(),
                           EkxProtocol::Payload::Nack::NackReason::FileUploadIllegalState );
                break;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void HttpServerDriver::processJsonMessage( const EkxProtocol::MessageFrame & frame )
{
    const EkxProtocol::Payload::JsonMessage * const pPayload = dynamic_cast<const EkxProtocol::Payload::JsonMessage *>( frame.payloadRef().value() );

    if ( !pPayload )
    {
        replyNack( frame.driverId().value(),
                   frame.msgCounter().value(),
                   frame.repeatCounter().value(),
                   EkxProtocol::Payload::Nack::NackReason::PayloadBroken );
    }
    else
    {
        replyAck( frame.driverId().value(),
                  frame.msgCounter().value(),
                  frame.repeatCounter().value() );

        QCoreApplication::processEvents();

        if ( m_pJsonApi->processJsonMessage( frame.msgCounter().value(),
                                             QByteArray( pPayload->json().value().data(),
                                                         pPayload->json().value().size() ) ) )
        {
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void HttpServerDriver::setFileUploadState( const FileUploadState nFileUploadState )
{
    if ( m_nFileUploadState != nFileUploadState )
    {
        qWarning() << "setFileUploadState" << static_cast<int>( nFileUploadState );
        m_nFileUploadState = nFileUploadState;
        emit fileUploadStateChanged( m_nFileUploadState );
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void HttpServerDriver::processTimeoutFileUpload()
{
    qCritical() << "processTimeoutFileUpload()";
    setFileUploadState( FileUploadState::Timeout );
    abortFileUpload();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool HttpServerDriver::buildUploadFile( const std::string & strData )
{
    bool bSuccess = false;

    QFile file( m_strUploadFile );

    if ( file.open( QIODevice::Append ) )
    {
        file.write( strData.data(), strData.size() );

        if ( file.error() == QFile::FileError::NoError )
        {
            bSuccess = true;
        }
        file.flush();

        m_u32FileUploadNextOffset += strData.size();
    }

    return bSuccess;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void HttpServerDriver::abortFileUpload()
{
    dumpFileUploadStatistics();
    QFile::remove( m_strUploadFile );
    m_timerFileUpload.stop();
    setFileUploadState( FileUploadState::Idle );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void HttpServerDriver::dumpFileUploadStatistics()
{
    quint64 u64Size = QFileInfo( m_strUploadFile ).size();

    QDateTime now = QDateTime::currentDateTime();

    quint64 u64DiffTime = m_dtStartTime.msecsTo( now );

    qInfo() << "dumpFileUploadStatistics() msecs used" << u64DiffTime << "file size" << u64Size << "data rate" << ( u64Size * 1000 / u64DiffTime ) << "bytes/sec";
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

QString HttpServerDriver::createFileSuffix() const
{
    return QString( "_%1_%2" ).arg( m_pDeviceInfoCollector->ekxUiSerialNo(),
                                    QDateTime::currentDateTime().toString( "yyyyMMdd_hhmmss" ) );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool HttpServerDriver::zipFile( const QString & strWorkingDir,
                                const QString & strFromFile,
                                const QString & strToFile ) const
{
    QProcess process;

    process.setWorkingDirectory( strWorkingDir );
    process.start( "sh", QStringList() << "-c" << QString( "zip %1 %2" ).arg( strToFile, strFromFile ) );

    bool bSuccess = process.waitForFinished();

    if ( !bSuccess
         || !QFile::exists( strToFile ) )
    {
        qCritical() << "HttpServerDriver::zipFile() error creating file" << strToFile;
        bSuccess = false;
    }

    return bSuccess;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool HttpServerDriver::prepareDatabaseDownload( const EkxProtocol::Payload::FileDownloadStartRequest * const pRequest,
                                                EkxProtocol::Payload::FileDownloadStartResponse * const      pResponse )
{
    bool bSuccess = false;

    qInfo() << "HttpServerDriver::prepareDatabaseDownload() request download" << pRequest->fileName().value().c_str();

    cleanStorage();

    QString strTargetFilename = QString( "%1%2.zip" )
                                .arg( pRequest->fileName().value().c_str(),
                                      createFileSuffix() );
    QString strFullName =  c_strStoragePath + strTargetFilename;
    if ( zipFile( "/db_storage", "EkxSqlite.db", strFullName ) )
    {
        pResponse->fileName().setValue( strTargetFilename.toStdString() );
        pResponse->fileSize().setValue( QFileInfo( strFullName ).size() );
        pResponse->status().setValue( EkxProtocol::Payload::FileDownloadStatus::OkOrContinue );

        qInfo() << "HttpServerDriver::prepareDatabaseDownload() prepare file" << strFullName << "for download";
        bSuccess = true;

        m_strFileDownloadLastGeneratedFile = strTargetFilename;
    }
    else
    {
        pResponse->fileName().setValue( pRequest->fileName().value() );
        pResponse->fileSize().setValue( 0 );
        pResponse->status().setValue( EkxProtocol::Payload::FileDownloadStatus::FilePreparationError );

        qCritical() << "HttpServerDriver::prepareDatabaseDownload() error, file preparation" << strFullName;
    }

    return bSuccess;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool HttpServerDriver::prepareAgsaLdtLogsDownload( const EkxProtocol::Payload::FileDownloadStartRequest * const pRequest,
                                                   EkxProtocol::Payload::FileDownloadStartResponse * const      pResponse )
{
    bool bSuccess = false;

    qInfo() << "HttpServerDriver::prepareAgsaLdtLogsDownload() request download" << pRequest->fileName().value().c_str();

    cleanStorage();

    QString strTargetFilename = QString( "%1%2.zip" )
                                .arg( pRequest->fileName().value().c_str(),
                                      createFileSuffix() );
    QString strFullName = c_strStoragePath + strTargetFilename;
    if ( zipFile( "/storage/HemroEkxAgsaLdtLogs", "*", strFullName ) )
    {
        pResponse->fileName().setValue( strTargetFilename.toStdString() );
        pResponse->fileSize().setValue( QFileInfo( strFullName ).size() );
        pResponse->status().setValue( EkxProtocol::Payload::FileDownloadStatus::OkOrContinue );

        qInfo() << "HttpServerDriver::prepareAgsaLdtLogsDownload() prepare file" << strFullName << "for download";
        bSuccess = true;

        m_strFileDownloadLastGeneratedFile = strTargetFilename;
    }
    else
    {
        pResponse->fileName().setValue( pRequest->fileName().value() );
        pResponse->fileSize().setValue( 0 );
        pResponse->status().setValue( EkxProtocol::Payload::FileDownloadStatus::FilePreparationError );

        qCritical() << "HttpServerDriver::prepareAgsaLdtLogsDownload() error, file preparation" << strFullName;
    }

    return bSuccess;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool HttpServerDriver::prepareEventStatisticsDownload( const EkxProtocol::Payload::FileDownloadStartRequest * const pRequest,
                                                       EkxProtocol::Payload::FileDownloadStartResponse * const      pResponse )
{
    bool bSuccess = false;

    qInfo() << "HttpServerDriver::prepareEventStatisticsDownload() request download" << pRequest->fileName().value().c_str();

    cleanStorage();

    QString strTargetFilename = QString( "%1%2.zip" )
                                .arg( pRequest->fileName().value().c_str(),
                                      createFileSuffix() );
    QString strFullName =  c_strStoragePath + strTargetFilename;
    if ( zipFile( "/storage", "HemroEkOmniaEventStatistics.csv", strFullName ) )
    {
        pResponse->fileName().setValue( strTargetFilename.toStdString() );
        pResponse->fileSize().setValue( QFileInfo( strFullName ).size() );
        pResponse->status().setValue( EkxProtocol::Payload::FileDownloadStatus::OkOrContinue );

        qInfo() << "HttpServerDriver::prepareEventStatisticsDownload() prepare file" << strFullName << "for download";
        bSuccess = true;

        m_strFileDownloadLastGeneratedFile = strTargetFilename;
    }
    else
    {
        pResponse->fileName().setValue( pRequest->fileName().value() );
        pResponse->fileSize().setValue( 0 );
        pResponse->status().setValue( EkxProtocol::Payload::FileDownloadStatus::FilePreparationError );

        qCritical() << "HttpServerDriver::prepareEventStatisticsDownload() error, file preparation" << strFullName;
    }

    return bSuccess;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool HttpServerDriver::prepareGrindStatisticsDownload( const EkxProtocol::Payload::FileDownloadStartRequest * const pRequest,
                                                       EkxProtocol::Payload::FileDownloadStartResponse * const      pResponse )
{
    bool bSuccess = false;

    qInfo() << "HttpServerDriver::prepareGrindStatisticsDownload() request download" << pRequest->fileName().value().c_str();

    cleanStorage();

    QString strTargetFilename = QString( "%1%2.zip" )
                                .arg( pRequest->fileName().value().c_str(),
                                      createFileSuffix() );
    QString strFullName =  c_strStoragePath + strTargetFilename;
    if ( zipFile( "/storage", "HemroEkOmniaGrindStatistics.csv", strFullName ) )
    {
        pResponse->fileName().setValue( strTargetFilename.toStdString() );
        pResponse->fileSize().setValue( QFileInfo( strFullName ).size() );
        pResponse->status().setValue( EkxProtocol::Payload::FileDownloadStatus::OkOrContinue );

        qInfo() << "HttpServerDriver::prepareGrindStatisticsDownload() prepare file" << strFullName << "for download";
        bSuccess = true;

        m_strFileDownloadLastGeneratedFile = strTargetFilename;
    }
    else
    {
        pResponse->fileName().setValue( pRequest->fileName().value() );
        pResponse->fileSize().setValue( 0 );
        pResponse->status().setValue( EkxProtocol::Payload::FileDownloadStatus::FilePreparationError );

        qCritical() << "HttpServerDriver::prepareGrindStatisticsDownload() error, file preparation" << strFullName;
    }

    return bSuccess;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool HttpServerDriver::prepareCounterStatisticsDownload( const EkxProtocol::Payload::FileDownloadStartRequest * const pRequest,
                                                         EkxProtocol::Payload::FileDownloadStartResponse * const      pResponse )
{
    bool bSuccess = false;

    qInfo() << "HttpServerDriver::prepareCounterStatisticsDownload() request download" << pRequest->fileName().value().c_str();

    cleanStorage();

    QString strTargetFilename = QString( "%1%2.zip" )
                                .arg( pRequest->fileName().value().c_str(),
                                      createFileSuffix() );
    QString strFullName =  c_strStoragePath + strTargetFilename;
    if ( zipFile( "/storage", "HemroEkOmniaCounterStatistics.csv", strFullName ) )
    {
        pResponse->fileName().setValue( strTargetFilename.toStdString() );
        pResponse->fileSize().setValue( QFileInfo( strFullName ).size() );
        pResponse->status().setValue( EkxProtocol::Payload::FileDownloadStatus::OkOrContinue );

        qInfo() << "HttpServerDriver::prepareCounterStatisticsDownload() prepare file" << strFullName << "for download";
        bSuccess = true;

        m_strFileDownloadLastGeneratedFile = strTargetFilename;
    }
    else
    {
        pResponse->fileName().setValue( pRequest->fileName().value() );
        pResponse->fileSize().setValue( 0 );
        pResponse->status().setValue( EkxProtocol::Payload::FileDownloadStatus::FilePreparationError );

        qCritical() << "HttpServerDriver::prepareCounterStatisticsDownload() error, file preparation" << strFullName;
    }

    return bSuccess;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool HttpServerDriver::prepareCustomSplashScreenDownload( const EkxProtocol::Payload::FileDownloadStartRequest * const pRequest,
                                                          EkxProtocol::Payload::FileDownloadStartResponse * const      pResponse )
{
    bool bSuccess = false;

    qInfo() << "HttpServerDriver::prepareCustomSplashScreenDownload() request download" << pRequest->fileName().value().c_str();

    QString strTargetFilename = "custom-splash-screen";
    QString strFullName       =  c_strConfigPath + strTargetFilename;
    if ( QFile::exists( strFullName ) )
    {
        qInfo() << "HttpServerDriver::prepareCustomSplashScreenDownload() prepare file" << strFullName << "for download";

        pResponse->fileName().setValue( strTargetFilename.toStdString() );
        pResponse->fileSize().setValue( QFileInfo( strFullName ).size() );
        pResponse->status().setValue( EkxProtocol::Payload::FileDownloadStatus::OkOrContinue );

        bSuccess = true;

        m_strFileDownloadLastGeneratedFile = strTargetFilename;
    }
    else
    {
        qCritical() << "HttpServerDriver::prepareCustomSplashScreenDownload() error, file preparation" << strFullName;

        pResponse->fileName().setValue( pRequest->fileName().value() );
        pResponse->fileSize().setValue( 0 );
        pResponse->status().setValue( EkxProtocol::Payload::FileDownloadStatus::FileNotExisting );
    }

    return bSuccess;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////
