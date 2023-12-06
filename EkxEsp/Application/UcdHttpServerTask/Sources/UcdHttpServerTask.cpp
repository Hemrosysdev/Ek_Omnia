/*
 * UcdHttpServerTask.cpp
 *
 *  Created on: 21.10.2019
 *      Author: gesser
 */

#include "UcdHttpServerTask.h"

#include <sstream>
#include <cstdarg>
#include <cstring>
#include <regex>
#include <string>
#include <algorithm>
#include <ostream>

#include "ApplicationGlobals.h"
#include "ApplicationHelper.h"

#include "UcdHttpServerQueue.h"
#include "UcdHttpServerQueueTypes.h"
#include "EkxProtocol.h"
#include "UartControlledDevice.h"

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#define HTTP_SEND_CHUNK_SIZE        500
#define HEX_DIGIT_10_OFFSET         10
#define HEX_DIGIT_1                 1
#define HEX_DIGIT_2                 2
#define HEX_DIGIT_SPAN              2
#define HTML_STRING_DATA_OVERSIZE   1000
#define FAKE_PASSWORD               "fakepw"

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

UcdHttpServerTask::UcdHttpServerTask( UxEspCppLibrary::EspApplication * const pApplication )
    : UartControlledDevice( "HttpServer",
                            "HttpServer",
                            EkxProtocol::DriverId::HttpServerDriver,
                            pApplication,
                            ApplicationGlobals::c_nHttpServerTaskStackSize,
                            ApplicationGlobals::c_nHttpServerTaskPriority,
                            "UcdHttpServerTask",
                            new UcdHttpServerQueue() )
    , m_timerList( taskQueue(),
                   static_cast<uint32_t>( UcdHttpServerQueueTypes::messageType::TimeoutListClean ) )


{
    logInfo( "constructor" );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

UcdHttpServerTask::~UcdHttpServerTask()
{
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

bool UcdHttpServerTask::startupStep( const UxEspCppLibrary::FreeRtosQueueTask::startupStep_t nStartupStep )
{
    bool bContinueStartup = true;

    switch ( nStartupStep )
    {
        case FreeRtosQueueTask::startupStep_t::startupStep1:
        {
            logInfo( "startupStep1" );
            startWebserver();
            registerDriver();

            m_timerList.startPeriodic( 2000000 );
        }
        break;

        default:
        {
            // do nothing
        }
        break;
    }

    return bContinueStartup;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void UcdHttpServerTask::processTaskMessage( void )
{
    const UcdHttpServerQueueTypes::message * pMessage = reinterpret_cast<const UcdHttpServerQueueTypes::message *>( receiveMsg() );

    if ( pMessage )
    {
        switch ( pMessage->u32Type )
        {
            case UcdHttpServerQueueTypes::messageType::CtrlUartCommand:
            {
                processBaseCtrlUartCommand( pMessage->payload.ctrlUartCommand.m_u8DriverId,
                                            pMessage->payload.ctrlUartCommand.m_u32MsgCounter,
                                            pMessage->payload.ctrlUartCommand.m_u8RepeatCounter,
                                            pMessage->payload.ctrlUartCommand.m_pMessagePayload );
            }
            break;

            case UcdHttpServerQueueTypes::messageType::AckNackReceived:
            {
                processAckNackReceived( pMessage->payload.ackNackReceived.m_u32MsgCounter,
                                        pMessage->payload.ackNackReceived.m_bSuccess );
            }
            break;

            case UcdHttpServerQueueTypes::messageType::Start:
            {
                startWebserver();
            }
            break;

            case UcdHttpServerQueueTypes::messageType::Stop:
            {
                stopWebserver();
            }
            break;

            case UcdHttpServerQueueTypes::messageType::TimeoutListClean:
            {
                performTimeout();
            }
            break;

            default:
            {
                vlogError( "processTaskMessage: receive illegal message type %d",
                           pMessage->u32Type );
            }
            break;
        }
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

UcdHttpServerQueue * UcdHttpServerTask::httpServerQueue( void )
{
    return dynamic_cast<UcdHttpServerQueue *>( taskQueue() );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

const UcdHttpServerQueue * UcdHttpServerTask::httpServerQueue( void ) const
{
    return dynamic_cast<const UcdHttpServerQueue *>( taskQueue() );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void UcdHttpServerTask::startWebserver( void )
{
    if ( m_handleWebserver != nullptr )
    {
        stopWebserver();
    }

    const httpd_uri_t uris[] =
    {
        { .uri = "/", .method = HTTP_GET, .handler = restIndexHandler, .user_ctx = this },
        { .uri = "/index", .method = HTTP_GET, .handler = restIndexHandler, .user_ctx = this },
        { .uri = "/index.html", .method = HTTP_GET, .handler = restIndexHandler, .user_ctx = this },
        { .uri = "/copyright.html", .method = HTTP_GET, .handler = restCopyrightHandler, .user_ctx = this },
        { .uri = "/statistics.html", .method = HTTP_GET, .handler = restStatisticsHtmlHandler, .user_ctx = this },
        { .uri = "/includePopup.html", .method = HTTP_GET, .handler = restIncludePopupHtmlHandler, .user_ctx = this },
        { .uri = "/includeHeader.html", .method = HTTP_GET, .handler = restIncludeHeaderHtmlHandler, .user_ctx = this },
        { .uri = "/includeFooter.html", .method = HTTP_GET, .handler = restIncludeFooterHtmlHandler, .user_ctx = this },
        { .uri = "/includeDevOptions.html", .method = HTTP_GET, .handler = restIncludeDevOptionsHtmlHandler, .user_ctx = this },
        { .uri = "/includeNoJavascript.html", .method = HTTP_GET, .handler = restIncludeNoJavascriptHtmlHandler, .user_ctx = this },

        { .uri = "/restApi", .method = HTTP_POST, .handler = restRestApiHandler, .user_ctx = this },
        { .uri = "/restApi", .method = HTTP_GET, .handler = restRestApiHandler, .user_ctx = this },

        { .uri = "/js/zepto.min.js", .method = HTTP_GET, .handler = restZeptoMinJsHandler, .user_ctx = this },
        { .uri = "/js/commonHelpers.js", .method = HTTP_GET, .handler = restCommonHelpersJsHandler, .user_ctx = this },
        { .uri = "/js/aliveCheck.js", .method = HTTP_GET, .handler = restAliveCheckJsHandler, .user_ctx = this },
        { .uri = "/js/popupManagement.js", .method = HTTP_GET, .handler = restPopupManagementJsHandler, .user_ctx = this },
        { .uri = "/js/firmwareUpdateObserver.js", .method = HTTP_GET, .handler = restFirmwareUpdateObserverJsHandler, .user_ctx = this },

        { .uri = "/css/style.css", .method = HTTP_GET, .handler = restStyleCssHandler, .user_ctx = this },

        { .uri = "/custom-splash-screen", .method = HTTP_GET, .handler = restCustomSplashScreenHandler, .user_ctx = this },
        { .uri = "/images/qrcode.png", .method = HTTP_GET, .handler = restQrCodePngHandler, .user_ctx = this },
        { .uri = "/images/mahlkoenig_logo_210x120.png", .method = HTTP_GET, .handler = restLogoHandler, .user_ctx = this },
        { .uri = "/favicon.ico", .method = HTTP_GET, .handler = restFaviconIcoHandler, .user_ctx = this },
        { .uri = "/images/favicon.ico", .method = HTTP_GET, .handler = restFaviconIcoHandler, .user_ctx = this },
        { .uri = "/images/ekx.jpg", .method = HTTP_GET, .handler = restEkxJpgHandler, .user_ctx = this },
        { .uri = "/images/wifi.png", .method = HTTP_GET, .handler = restWifiPngHandler, .user_ctx = this },
        { .uri = "/images/home.png", .method = HTTP_GET, .handler = restHomePngHandler, .user_ctx = this },

    };

    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.stack_size        = 8192;
    config.lru_purge_enable  = true;
    config.server_port       = ApplicationGlobals::c_httpServerDefaultPort;
    config.max_uri_handlers  = sizeof( uris ) / sizeof( httpd_uri_t );
    config.recv_wait_timeout = 2;

    // Start the httpd server
    vlogInfo( "startWebserver() Starting web server on port: '%d'", config.server_port );

    if ( httpd_start( &m_handleWebserver, &config ) == ESP_OK )
    {
        // Set URI handlers
        vlogInfo( "startWebserver() Registering URI handlers" );
        for ( int i = 0; i < config.max_uri_handlers; i++ )
        {
            httpd_register_uri_handler( m_handleWebserver, &uris[i] );
        }
    }
    else
    {
        logError( "startWebserver() Error starting server!" );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void UcdHttpServerTask::stopWebserver( void )
{
    vlogInfo( "stopWebserver()" );

    // Stop the httpd server
    httpd_stop( m_handleWebserver );
    m_handleWebserver = nullptr;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

// static
esp_err_t UcdHttpServerTask::restRestApiHandler( httpd_req_t * pReq )
{
    esp_err_t nEspError = ESP_FAIL;

    if ( pReq )
    {
        UcdHttpServerTask * pThis = reinterpret_cast<UcdHttpServerTask *>( pReq->user_ctx );

        if ( pThis )
        {
            pThis->parseUriParams( pReq );
            pThis->openServerRequest( pReq );

            httpd_err_code_t nHttpdErrorCode = HTTPD_500_INTERNAL_SERVER_ERROR;

            if ( pThis->existsUriKey( pReq, "uploadSwu" ) )
            {
                nEspError = pThis->espUploadHandler( pReq );

                nHttpdErrorCode = HTTPD_408_REQ_TIMEOUT;
            }
            else if ( pThis->existsUriKey( pReq, "uploadSplashScreenImage" ) )
            {
                nEspError = pThis->espUploadHandler( pReq );

                nHttpdErrorCode = HTTPD_408_REQ_TIMEOUT;
            }
            else if ( pThis->existsUriKey( pReq, "downloadDatabase" ) )
            {
                pThis->vlogInfo( "restRestApiHandler() downloadDatabase" );

                nEspError = pThis->espDownloadHandler( pReq, "HemroEkOmniaDatabase" );

                nHttpdErrorCode = HTTPD_408_REQ_TIMEOUT;
            }
            else if ( pThis->existsUriKey( pReq, "downloadAgsaLdtLogs" ) )
            {
                pThis->vlogInfo( "restRestApiHandler() downloadAgsaLdtLogs" );

                nEspError = pThis->espDownloadHandler( pReq, "HemroEkOmniaAgsaLdtLogs" );

                nHttpdErrorCode = HTTPD_408_REQ_TIMEOUT;
            }
            else if ( pThis->existsUriKey( pReq, "downloadCounterStatistics" ) )
            {
                pThis->vlogInfo( "restRestApiHandler() downloadCounterStatistics" );

                nEspError = pThis->espDownloadHandler( pReq, "HemroEkOmniaCounterStatistics" );

                nHttpdErrorCode = HTTPD_408_REQ_TIMEOUT;
            }
            else if ( pThis->existsUriKey( pReq, "downloadEventStatistics" ) )
            {
                pThis->vlogInfo( "restRestApiHandler() downloadEventStatistics" );

                nEspError = pThis->espDownloadHandler( pReq, "HemroEkOmniaEventStatistics" );

                nHttpdErrorCode = HTTPD_408_REQ_TIMEOUT;
            }
            else if ( pThis->existsUriKey( pReq, "downloadGrindStatistics" ) )
            {
                pThis->vlogInfo( "restRestApiHandler() downloadGrindStatistics" );

                nEspError = pThis->espDownloadHandler( pReq, "HemroEkOmniaGrindStatistics" );

                nHttpdErrorCode = HTTPD_408_REQ_TIMEOUT;
            }
            else if ( pThis->existsUriKey( pReq, "jsonCmd" ) )
            {
                std::string strHttpPostData;
                pThis->collectHttpPostData( pReq, strHttpPostData );

                //printf( "jsonCmd: %s\n", strHttpPostData.c_str() );

                EkxProtocol::Payload::JsonMessage * pPayload = new EkxProtocol::Payload::JsonMessage();
                pPayload->json().setValue( strHttpPostData );

                uint32_t u32MsgCounter = pThis->sendCtrlUartReply( pPayload, true );

                if ( pThis->waitForAck( u32MsgCounter, c_u32StandardTimeoutWaitAckMs ) )
                {
                    EkxProtocol::Payload::JsonMessage * pPayload = pThis->waitForJsonMessage( u32MsgCounter, c_u32StandardTimeoutWaitMessageMs );

                    if ( pPayload )
                    {
                        //pThis->vlogInfo( "Reply: %s", pPayload->json().value().c_str() );
                        pThis->sendChunkData( pReq, "text/json", nullptr, false,
                                              pPayload->json().value().data(),
                                              pPayload->json().value().size() );
                        delete pPayload;

                        nEspError = ESP_OK;
                    }
                    else
                    {
                        pThis->vlogError( "restRestApiHandler() no JSON reply in timeout received, cnt %u", u32MsgCounter );
                    }
                }
                else
                {
                    pThis->vlogError( "restRestApiHandler() no ack/nak in timeout received, cnt %u, %s", u32MsgCounter, strHttpPostData.c_str() );
                }
            }
            else
            {
                pThis->vlogError( "restRestApiHandler() unknown command request %s", pReq->uri );
            }

            if ( nEspError != ESP_OK )
            {
                httpd_resp_send_err( pReq,
                                     nHttpdErrorCode,
                                     "Internal error, please repeat request" );
            }

            pThis->closeServerRequest();
        }
    }

    return nEspError;

}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

// static
esp_err_t UcdHttpServerTask::restRedirect( httpd_req_t *       pReq,
                                           const std::string & strUrl /*,
                                                                         const SessionManager::sessionId_t u32SessionId */)
{
    esp_err_t nEspError = ESP_FAIL;

    if ( pReq )
    {
        UcdHttpServerTask * pThis = reinterpret_cast<UcdHttpServerTask *>( pReq->user_ctx );

        if ( pThis )
        {
            std::string strSessionUrl = strUrl;

            std::string strHtmlContent = "<!DOCTYPE html><html><head><meta http-equiv=\"refresh\" content=\"0; URL=" + strSessionUrl + "\"></head></html>";

            pThis->vlogInfo( "restRedirect: %s", strSessionUrl.c_str() );

            nEspError = pThis->sendChunkData( pReq,
                                              nullptr,
                                              nullptr,
                                              false,
                                              strHtmlContent.c_str(),
                                              strHtmlContent.size() );
        }
    }

    return nEspError;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

// static
esp_err_t UcdHttpServerTask::restIndexHandler( httpd_req_t * pReq )
{
    esp_err_t nEspError = ESP_FAIL;

    if ( pReq != nullptr )
    {
        UcdHttpServerTask * const pThis = reinterpret_cast<UcdHttpServerTask *>( pReq->user_ctx );

        if ( pThis != nullptr )
        {
            pThis->vlogInfo( "restIndexHandler(): (heap %d)", esp_get_free_heap_size() );

            pThis->parseUriParams( pReq );

            {
                try
                {
                    // build up null terminated std::string from blob data
                    extern const unsigned char pucBlobDataStartIndexHtml[] asm ( "_binary_index_html_stripped_start" );  // NOSONAR this valid pattern is not understood by Sonar
                    extern const unsigned char pucBlobDataEndIndexHtml[]   asm ( "_binary_index_html_stripped_end" );    // NOSONAR this valid pattern is not understood by Sonar
                    const size_t               u32BlobDataSizeIndexHtml = ( pucBlobDataEndIndexHtml - pucBlobDataStartIndexHtml );

                    std::string strHtmlContent( u32BlobDataSizeIndexHtml + HTML_STRING_DATA_OVERSIZE, '\0' );
                    memcpy( const_cast<char *>( strHtmlContent.data() ),                         // NOSONAR     intended: const_cast the only way to access the internal string::data
                            pucBlobDataStartIndexHtml,
                            u32BlobDataSizeIndexHtml );

                    if ( pThis->existsUriKey( pReq, "dev" ) )
                    {
                        pThis->m_bDevOption = true;
                    }
                    else if ( pThis->existsUriKey( pReq, "nodev" ) )
                    {
                        pThis->m_bDevOption = false;
                    }

                    pThis->replaceDynamicCommonContents( strHtmlContent );

                    nEspError = pThis->sendChunkData( pReq,
                                                      nullptr,
                                                      nullptr,
                                                      false,
                                                      strHtmlContent.c_str(),
                                                      std::strlen( strHtmlContent.data() ) );
                }
                catch ( const std::exception & e )   // NOSONAR no, I don't want to catch a more specific one!
                {
                    pThis->vlogError( "restIndexHandler(): std::exception %s", e.what() );
                    /* Respond with 500 Internal Server Error */
                    httpd_resp_send_err( pReq,
                                         HTTPD_500_INTERNAL_SERVER_ERROR,
                                         "Internal error, please repeat request (caught std::exception)" );
                }
            }
        }
    }

    return nEspError;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

esp_err_t UcdHttpServerTask::sendChunkData( httpd_req_t * const pReq,
                                            const char * const  pszHttpType,
                                            const char * const  pszHttpFilename,
                                            const bool          bGzipped,
                                            const char * const  pData,
                                            const int           nDataLen )
{
    esp_err_t nEspError = ESP_FAIL;

    if ( pReq != nullptr )
    {
        UcdHttpServerTask * const pThis = reinterpret_cast<UcdHttpServerTask *>( pReq->user_ctx );

        pThis->openServerRequest( pReq );

        if ( pszHttpType != nullptr )
        {
            httpd_resp_set_type( pReq, pszHttpType );
        }

        if ( bGzipped )
        {
            httpd_resp_set_hdr( pReq, "Content-Encoding", "gzip" );
        }

        if ( pszHttpFilename != nullptr )
        {
            httpd_resp_set_hdr( pReq, "Content-Disposition", ( std::string( "inline; filename=" ) + pszHttpFilename ).c_str() );
            httpd_resp_set_hdr( pReq, "Access-Control-Allow-Origin", "*" );
        }

        const int nChunkSize = HTTP_SEND_CHUNK_SIZE;
        int       j          = 0;
        for ( int i = 0; i < nDataLen; i += nChunkSize )
        {
            int nMaxChunkSize = nChunkSize;
            if ( nDataLen - i < nMaxChunkSize )
            {
                nMaxChunkSize = nDataLen - i;
            }

            nEspError = httpd_resp_send_chunk( pThis->m_pServerRequest,
                                               pData + i,
                                               nMaxChunkSize );

            if ( nEspError != ESP_OK )
            {
                pThis->vlogError( "Error while sending data (%s), chunk %d, data %d, cur data %d - abort", esp_err_to_name( nEspError ), j, nDataLen, i );
                break;
            }

            j++;
        }

        pThis->closeServerRequest();

        nEspError = ESP_OK;
    }

    return nEspError;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

// static
esp_err_t UcdHttpServerTask::restCopyrightHandler( httpd_req_t * const pReq )
{
    esp_err_t nEspError = ESP_FAIL;

    if ( pReq != nullptr )
    {
        UcdHttpServerTask * const pThis = reinterpret_cast<UcdHttpServerTask *>( pReq->user_ctx );

        if ( pThis != nullptr )
        {
            pThis->vlogInfo( "restCopyrightHandler()" );

            pThis->parseUriParams( pReq );

            {
                try
                {
                    extern const unsigned char pucBlobDataStartCopyrightHtml[] asm ( "_binary_copyright_html_stripped_start" );
                    extern const unsigned char pucBlobDataEndCopyrightHtml[]   asm ( "_binary_copyright_html_stripped_end" );
                    const size_t               u32BlobDataSize = ( pucBlobDataEndCopyrightHtml - pucBlobDataStartCopyrightHtml );

                    std::string strHtmlContent( u32BlobDataSize + HTML_STRING_DATA_OVERSIZE, '\0' );
                    memcpy( const_cast<char *>( strHtmlContent.data() ),                         // NOSONAR     intended: const_cast the only way to access the internal string::data
                            pucBlobDataStartCopyrightHtml,
                            u32BlobDataSize );

                    pThis->replaceDynamicCommonContents( strHtmlContent );

                    nEspError = pThis->sendChunkData( pReq,
                                                      nullptr,
                                                      nullptr,
                                                      false,
                                                      strHtmlContent.c_str(),
                                                      std::strlen( strHtmlContent.c_str() ) );
                }
                catch ( const std::exception & e )   // NOSONAR no, I don't want to catch a more specific one!
                {
                    pThis->vlogError( "restCopyrightHandler: std::exception %s", e.what() );
                    /* Respond with 500 Internal Server Error */
                    httpd_resp_send_err( pReq,
                                         HTTPD_500_INTERNAL_SERVER_ERROR,
                                         "Internal error, please repeat request (caught std::exception)" );
                }
            }
        }
    }

    return nEspError;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

esp_err_t UcdHttpServerTask::restStatisticsHtmlHandler( httpd_req_t * pReq )
{
    esp_err_t nEspError = ESP_FAIL;

    if ( pReq != nullptr )
    {
        UcdHttpServerTask * const pThis = reinterpret_cast<UcdHttpServerTask *>( pReq->user_ctx );

        if ( pThis != nullptr )
        {
            pThis->vlogInfo( "restStatisticsHtmlHandler()" );

            pThis->parseUriParams( pReq );

            {
                try
                {
                    extern const unsigned char pucBlobDataStartStatisticsHtml[] asm ( "_binary_statistics_html_stripped_start" );
                    extern const unsigned char pucBlobDataEndStatisticsHtml[]   asm ( "_binary_statistics_html_stripped_end" );
                    const size_t               u32BlobDataSize = ( pucBlobDataEndStatisticsHtml - pucBlobDataStartStatisticsHtml );

                    std::string strHtmlContent( u32BlobDataSize + HTML_STRING_DATA_OVERSIZE, '\0' );
                    memcpy( const_cast<char *>( strHtmlContent.data() ),
                            pucBlobDataStartStatisticsHtml,
                            u32BlobDataSize );

                    pThis->replaceDynamicCommonContents( strHtmlContent );

                    nEspError = pThis->sendChunkData( pReq,
                                                      nullptr,
                                                      nullptr,
                                                      false,
                                                      strHtmlContent.c_str(),
                                                      std::strlen( strHtmlContent.c_str() ) );
                }
                catch ( const std::exception & e )   // NOSONAR no, I don't want to catch a more specific one!
                {
                    pThis->vlogError( "restStatisticsHtmlHandler: std::exception %s", e.what() );
                    /* Respond with 500 Internal Server Error */
                    httpd_resp_send_err( pReq,
                                         HTTPD_500_INTERNAL_SERVER_ERROR,
                                         "Internal error, please repeat request (caught std::exception)" );
                }
            }
        }
    }

    return nEspError;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

// static
esp_err_t UcdHttpServerTask::restZeptoMinJsHandler( httpd_req_t * const pReq )
{
    esp_err_t nEspError = ESP_FAIL;

    if ( pReq != nullptr )
    {
        UcdHttpServerTask * const pThis = reinterpret_cast<UcdHttpServerTask *>( pReq->user_ctx );

        if ( pThis != nullptr )
        {
            pThis->vlogInfo( "restZeptoMinJsHandler()" );

            extern const unsigned char pucBlobDataStartZeptoMinJs[] asm ( "_binary_zepto_min_js_gz_start" );
            extern const unsigned char pucBlobDataEndZeptoMinJs[]   asm ( "_binary_zepto_min_js_gz_end" );
            const size_t               u32BlobDataSize = ( pucBlobDataEndZeptoMinJs - pucBlobDataStartZeptoMinJs );

            nEspError = pThis->sendChunkData( pReq,
                                              "text/javascript",
                                              "zepto.min.js",
                                              true,
                                              reinterpret_cast<const char *>( pucBlobDataStartZeptoMinJs ),
                                              static_cast<int>( u32BlobDataSize ) );
        }
    }

    return nEspError;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

// static
esp_err_t UcdHttpServerTask::restStyleCssHandler( httpd_req_t * const pReq )
{
    esp_err_t nEspError = ESP_FAIL;

    if ( pReq != nullptr )
    {
        UcdHttpServerTask * const pThis = reinterpret_cast<UcdHttpServerTask *>( pReq->user_ctx );

        if ( pThis != nullptr )
        {
            pThis->vlogInfo( "restStyleCssHandler()" );

            extern const unsigned char pucBlobDataStartStyleCss[] asm ( "_binary_style_css_stripped_gz_start" );
            extern const unsigned char pucBlobDataEndStyleCss[]   asm ( "_binary_style_css_stripped_gz_end" );
            const size_t               u32BlobDataSize = ( pucBlobDataEndStyleCss - pucBlobDataStartStyleCss );

            nEspError = pThis->sendChunkData( pReq,
                                              "text/css",
                                              "style.css",
                                              true,
                                              reinterpret_cast<const char *>( pucBlobDataStartStyleCss ),
                                              static_cast<int>( u32BlobDataSize ) );
        }
    }

    return nEspError;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

// static
esp_err_t UcdHttpServerTask::restFaviconIcoHandler( httpd_req_t * const pReq )
{
    esp_err_t nEspError = ESP_FAIL;

    if ( pReq != nullptr )
    {
        UcdHttpServerTask * const pThis = reinterpret_cast<UcdHttpServerTask *>( pReq->user_ctx );

        if ( pThis != nullptr )
        {
            pThis->vlogInfo( "restFaviconIcoHandler()" );

            extern const unsigned char pucBlobDataStartFaviconIco[] asm ( "_binary_favicon_ico_gz_start" );
            extern const unsigned char pucBlobDataEndFaviconIco[]   asm ( "_binary_favicon_ico_gz_end" );
            const size_t               u32BlobDataSize = ( pucBlobDataEndFaviconIco - pucBlobDataStartFaviconIco );

            nEspError = pThis->sendChunkData( pReq,
                                              "image/x-icon",
                                              "favicon.ico",
                                              true,
                                              reinterpret_cast<const char *>( pucBlobDataStartFaviconIco ),
                                              static_cast<int>( u32BlobDataSize ) );
        }
    }

    return nEspError;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

// static
esp_err_t UcdHttpServerTask::restCustomSplashScreenHandler( httpd_req_t * const pReq )
{
    esp_err_t nEspError = ESP_FAIL;

    if ( pReq != nullptr )
    {
        UcdHttpServerTask * const pThis = reinterpret_cast<UcdHttpServerTask *>( pReq->user_ctx );

        if ( pThis != nullptr )
        {
            pThis->vlogInfo( "restCustomSplashScreenHandler()" );

            nEspError = pThis->espDownloadHandler( pReq, "custom-splash-screen" );
        }
    }

    return nEspError;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

// static
esp_err_t UcdHttpServerTask::restQrCodePngHandler( httpd_req_t * const pReq )
{
    esp_err_t nEspError = ESP_FAIL;

    if ( pReq != nullptr )
    {
        UcdHttpServerTask * const pThis = reinterpret_cast<UcdHttpServerTask *>( pReq->user_ctx );

        if ( pThis != nullptr )
        {
            pThis->vlogInfo( "restQrCodePngHandler()" );

            extern const unsigned char pucBlobDataStartQrcodePng[] asm ( "_binary_qrcode_png_start" );
            extern const unsigned char pucBlobDataEndQrcodePng[]   asm ( "_binary_qrcode_png_end" );
            const size_t               u32BlobDataSize = ( pucBlobDataEndQrcodePng - pucBlobDataStartQrcodePng );

            nEspError = pThis->sendChunkData( pReq,
                                              "image/png",
                                              "qrcode.png",
                                              false,
                                              reinterpret_cast<const char *>( pucBlobDataStartQrcodePng ),
                                              static_cast<int>( u32BlobDataSize ) );
        }
    }

    return nEspError;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

// static
esp_err_t UcdHttpServerTask::restLogoHandler( httpd_req_t * const pReq )
{
    esp_err_t nEspError = ESP_FAIL;

    if ( pReq != nullptr )
    {
        UcdHttpServerTask * const pThis = reinterpret_cast<UcdHttpServerTask *>( pReq->user_ctx );

        if ( pThis != nullptr )
        {
            pThis->vlogInfo( "restLogoHandler()" );

            extern const unsigned char pucBlobDataStartMahlkoenigLogoPng[] asm ( "_binary_mahlkoenig_logo_210x120_png_start" );
            extern const unsigned char pucBlobDataEndMahlkoenigLogoPng[]   asm ( "_binary_mahlkoenig_logo_210x120_png_end" );
            const size_t               u32BlobDataSize = ( pucBlobDataEndMahlkoenigLogoPng - pucBlobDataStartMahlkoenigLogoPng );

            nEspError = pThis->sendChunkData( pReq,
                                              "image/png",
                                              "mahlkoenig_logo_210x120.png",
                                              false,
                                              reinterpret_cast<const char *>( pucBlobDataStartMahlkoenigLogoPng ),
                                              static_cast<int>( u32BlobDataSize ) );
        }
    }

    return nEspError;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

// static
esp_err_t UcdHttpServerTask::restEkxJpgHandler( httpd_req_t * pReq )
{
    esp_err_t nEspError = ESP_FAIL;

    if ( pReq )
    {
        UcdHttpServerTask * pThis = reinterpret_cast<UcdHttpServerTask *>( pReq->user_ctx );

        if ( pThis )
        {
            pThis->vlogInfo( "restEkxJpgHandler()" );

            extern const unsigned char pucBlobDataStartEkxJpg[] asm ( "_binary_ekx_jpg_start" );
            extern const unsigned char pucBlobDataEndEkxJpg[]   asm ( "_binary_ekx_jpg_end" );
            const size_t               u32BlobDataSize = ( pucBlobDataEndEkxJpg - pucBlobDataStartEkxJpg );

            nEspError = pThis->sendChunkData( pReq,
                                              "image/jpeg",
                                              "ekx.jpg",
                                              false,
                                              reinterpret_cast<const char *>( pucBlobDataStartEkxJpg ),
                                              static_cast<int>( u32BlobDataSize ) );
        }
    }

    return nEspError;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

// static
esp_err_t UcdHttpServerTask::restWifiPngHandler( httpd_req_t * pReq )
{
    esp_err_t nEspError = ESP_FAIL;

    if ( pReq )
    {
        UcdHttpServerTask * pThis = reinterpret_cast<UcdHttpServerTask *>( pReq->user_ctx );

        if ( pThis )
        {
            pThis->vlogInfo( "restWifiPngHandler()" );

            extern const unsigned char pucBlobDataStartWifiPng[] asm ( "_binary_wifi_png_start" );
            extern const unsigned char pucBlobDataEndWifiPng[]   asm ( "_binary_wifi_png_end" );
            const size_t               u32BlobDataSize = ( pucBlobDataEndWifiPng - pucBlobDataStartWifiPng );

            nEspError = pThis->sendChunkData( pReq,
                                              "image/png",
                                              "wifi.png",
                                              false,
                                              reinterpret_cast<const char *>( pucBlobDataStartWifiPng ),
                                              static_cast<int>( u32BlobDataSize ) );
        }
    }

    return nEspError;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

// static
esp_err_t UcdHttpServerTask::restHomePngHandler( httpd_req_t * pReq )
{
    esp_err_t nEspError = ESP_FAIL;

    if ( pReq )
    {
        UcdHttpServerTask * pThis = reinterpret_cast<UcdHttpServerTask *>( pReq->user_ctx );

        if ( pThis )
        {
            pThis->vlogInfo( "restHomePngHandler()" );

            extern const unsigned char pucBlobDataStartHomePng[] asm ( "_binary_home_png_start" );
            extern const unsigned char pucBlobDataEndHomePng[]   asm ( "_binary_home_png_end" );
            const size_t               u32BlobDataSize = ( pucBlobDataEndHomePng - pucBlobDataStartHomePng );

            nEspError = pThis->sendChunkData( pReq,
                                              "image/png",
                                              "home.png",
                                              false,
                                              reinterpret_cast<const char *>( pucBlobDataStartHomePng ),
                                              static_cast<int>( u32BlobDataSize ) );
        }
    }

    return nEspError;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

// static
bool UcdHttpServerTask::existsUriKey( httpd_req_t * const pReq,
                                      const std::string & strUriKey )
{
    bool bExists = false;

    if ( pReq
         && ( ( strstr( pReq->uri, ( "?" + strUriKey ).c_str() ) != nullptr )
              || ( strstr( pReq->uri, ( "&" + strUriKey ).c_str() ) != nullptr ) ) )
    {
        bExists = true;
    }

    return bExists;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

std::string UcdHttpServerTask::getUriKeyValue( httpd_req_t * const pReq,
                                               const std::string & strUriKey )
{
    std::string strValue;

    if ( pReq )
    {
        ApplicationGlobals::stringlist paramList;

        paramList = ApplicationGlobals::ApplicationHelper::splitString( pReq->uri, "?" );

        if ( paramList.size() > 1 )
        {
            strValue = getUriKeyValue( paramList[1], strUriKey );
        }
    }

    return strValue;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

// static
std::string UcdHttpServerTask::getUriKeyValue( const std::string & strParamString,
                                               const std::string & strUriKey )
{
    std::string strValue;

    ApplicationGlobals::stringlist paramList;

    paramList = ApplicationGlobals::ApplicationHelper::splitString( strParamString, "&" );

    for ( int i = 0; i < paramList.size(); i++ )
    {
        std::string                    strKeyValuePair = paramList[i];
        ApplicationGlobals::stringlist keyValuePair    = ApplicationGlobals::ApplicationHelper::splitString( strKeyValuePair, "=" );

        if ( keyValuePair.size() > 0
             && keyValuePair[0] == strUriKey )
        {
            if ( keyValuePair.size() > 1 )
            {
                strValue = keyValuePair[1];
            }

            break;
        }
    }

    return strValue;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

esp_err_t UcdHttpServerTask::collectHttpPostData( httpd_req_t * const pReq,
                                                  std::string &       strHttpPostData )
{
    esp_err_t nEspError = ESP_FAIL;

    const int nPostDataLen = pReq->content_len;

    if ( nPostDataLen == 0 )
    {
        nEspError = ESP_OK;
        vlogInfo( "collectHttpPostData(): no POST data" );
    }
    else if ( nPostDataLen >= sizeof( m_szPostDataBuffer ) )
    {
        vlogError( "collectHttpPostData(): POST data size too big (%d), skip request", nPostDataLen );
    }
    else
    {
        nEspError = ESP_OK;
        int          nReadDataLen = 0;
        char * const pBuffer      = m_szPostDataBuffer;

        while ( nReadDataLen < nPostDataLen )
        {
            const int nReceived = httpd_req_recv( pReq, pBuffer + nReadDataLen, nPostDataLen );

            if ( nReceived <= 0 )
            {
                logError( "collectHttpPostData(): httpd_req_recv failed within post data handling" );
                nEspError = ESP_FAIL;
                break;
            }
            nReadDataLen += nReceived;
        }
        pBuffer[nPostDataLen] = '\0';

        strHttpPostData.clear();
        if ( nEspError == ESP_OK )
        {
            strHttpPostData = pBuffer;
        }
    }

    return nEspError;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void UcdHttpServerTask::openServerRequest( httpd_req_t * pRequest )
{
    if ( pRequest )
    {
        m_pServerRequest = pRequest;
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void UcdHttpServerTask::closeServerRequest( void )
{
    if ( m_pServerRequest )
    {
        httpd_resp_set_hdr( m_pServerRequest, "Access-Control-Allow-Origin", "*" );
        httpd_resp_send_chunk( m_pServerRequest, nullptr, 0 );
        m_pServerRequest = nullptr;
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

esp_err_t UcdHttpServerTask::espDownloadHandler( httpd_req_t *       pReq,
                                                 const std::string & strFilename )
{
    esp_err_t nEspError = ESP_FAIL;

    if ( pReq )
    {
        int nMsgCounter = sendFileDownloadStartRequest( strFilename );

        if ( nMsgCounter >= 0 )
        {
            EkxProtocol::Payload::FileDownloadStartResponse * pStartResponse = nullptr;

            pStartResponse = waitForFileDownloadStartResponse( nMsgCounter, c_u32FileDownloadTimeoutWaitMessageMs );

            if ( !pStartResponse )
            {
                vlogError( "espDownloadHandler() Error Start File  Download, no response:%u", nMsgCounter );
            }
            else if ( pStartResponse->status().value() !=
                      EkxProtocol::Payload::FileDownloadStatus::OkOrContinue )
            {
                vlogError( "Error Start File  Download, err:%u", pStartResponse->status().value() );
            }
            else
            {
                vlogInfo( "espDownloadHandler() Download file %s", pStartResponse->fileName().value().c_str() );

                uint32_t           u32FileSize = pStartResponse->fileSize().value();
                std::ostringstream str;
                str << u32FileSize;

                std::string strDispositionArg = ( std::string( "attachment; filename=\"" )
                                                  + pStartResponse->fileName().value() + "\"" );
                /* Sending header ********************************************************/
                httpd_resp_set_hdr( pReq, "Content-Disposition", strDispositionArg.c_str() );
                httpd_resp_set_type( pReq, "application/octet-stream" );
                httpd_resp_set_hdr( pReq, "Content-Description", "File Transfer" );
                httpd_resp_set_hdr( pReq, "Cache-Control", "public" );
                httpd_resp_set_hdr( pReq, "Content-Transfer-Encoding", "binary" );
                httpd_resp_set_hdr( pReq, "Content-Length", str.str().c_str() );

                /* Request chunks and send ***********************************************/
                uint32_t u32Offset = 0;

                nEspError = ESP_OK;
                while ( ( u32Offset < u32FileSize )
                        && ( nEspError == ESP_OK ) )
                {
                    char szBuffer[1000];

                    uint32_t u32ChunkSize = std::min( ( u32FileSize - u32Offset ), static_cast<uint32_t>( sizeof( szBuffer ) ) );

                    nMsgCounter = sendFileDownloadDataRequest( pStartResponse->fileName().value(),
                                                               u32Offset,
                                                               u32ChunkSize );

                    if ( nMsgCounter < 0 )
                    {
                        nEspError = ESP_FAIL;
                    }
                    else
                    {
                        EkxProtocol::Payload::FileDownloadDataResponse * pDataResponse = nullptr;

                        pDataResponse = waitForFileDownloadDataResponse( nMsgCounter, c_u32StandardTimeoutWaitMessageMs );

                        if ( !pDataResponse )
                        {
                            vlogError( "espDownloadHandler() Error Reading File Data, err:%u", pDataResponse->status() );
                            nEspError = ESP_FAIL;
                        }
                        else
                        {
                            /* Send the buffer contents as HTTP response chunk */
                            nEspError = httpd_resp_send_chunk( pReq,
                                                               pDataResponse->chunkData().value().data(),
                                                               pDataResponse->chunkData().value().size() );

                            if ( pDataResponse->status().value() ==
                                 EkxProtocol::Payload::FileDownloadStatus::ReadEndOfFile )
                            {
                                vlogInfo( "espDownloadHandler() End of File" );
                            }
                            else if ( pDataResponse->status().value() ==
                                      EkxProtocol::Payload::FileDownloadStatus::OkOrContinue )
                            {
                                if ( pDataResponse->chunkOffset().value() == u32Offset )
                                {
                                    //vlogError(	"espDownloadHandler() Next Data, %s %u",
                                    //            pDataResponse->fileName().value().c_str(),
                                    //            pDataResponse->chunkOffset().value());
                                }
                                else
                                {
                                    vlogError( "espDownloadHandler() Error Reading File Data, offset mismatch" );
                                    nEspError = ESP_FAIL;
                                }
                            }
                            else
                            {
                                nEspError = ESP_FAIL;
                            }
                        }
                        u32Offset += u32ChunkSize;

                        delete pDataResponse;
                    }
                    /* Keep looping till the whole file is sent */
                }

                httpd_resp_send_chunk( pReq, nullptr, 0 );
                m_pServerRequest = nullptr;
            }

            delete pStartResponse;
        }
    }

    return nEspError;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

esp_err_t UcdHttpServerTask::espUploadHandler( httpd_req_t * pReq )
{
    esp_err_t nEspError = ESP_OK;

    if ( pReq )
    {
        char   buf[c_nFileUploadChunkSize];
        size_t remaining = pReq->content_len;
        int    chunkNo   = 0;

        vlogInfo( "espUploadHandler() file upload requested, file size %u\n", pReq->content_len );

        /* Sending Update Start Message */
        if ( sendFileUploadStart( "upload.file", pReq->content_len ) == false )
        {
            nEspError = ESP_FAIL;
        }
        else
        {
            uint8_t u8TimeoutErrorCount = 0;
            while ( remaining > 0
                    && nEspError == ESP_OK )
            {
                /* Read the data for the request */
                int nDataRead = httpd_req_recv( pReq, buf, std::min( remaining, sizeof( buf ) ) );

                if ( nDataRead <= 0 )
                {
                    /* Retry receiving if timeout occurred */
                    logWarning( "espUploadHandler(): timeout receive, retry" );
                    u8TimeoutErrorCount++;

                    if ( u8TimeoutErrorCount < c_u8MaxFileUploadTimeoutErrorCount )
                    {
                        continue;
                    }
                    else
                    {
                        vlogError( "espUploadHandler() abort upload, too much timeouts" );

                        sendFileUploadStop(
                            EkxProtocol::Payload::FileUploadStop::StopReason::DataTimeout,
                            pReq->content_len - remaining );

                        nEspError = ESP_FAIL;
                        break;
                    }
                }
                else
                {
                    u8TimeoutErrorCount = 0;

                    /* Sending Chunk */
                    if ( sendFileUploadData( chunkNo,
                                             pReq->content_len - remaining,
                                             nDataRead,
                                             buf ) == false )
                    {
                        vlogError( "espUploadHandler() Error while writing data (failed), %u %u %d",
                                   pReq->content_len,
                                   pReq->content_len - remaining,
                                   nDataRead );

                        sendFileUploadStop(
                            EkxProtocol::Payload::FileUploadStop::StopReason::Unknown,
                            pReq->content_len - remaining );

                        nEspError = ESP_FAIL;
                        break;
                    }
                    else
                    {
                        chunkNo++;
                        remaining -= nDataRead;
                    }
                }
            }

            if ( nEspError == ESP_OK )
            {
                vlogInfo( "espUploadHandler() File upload successfully finished" );

                sendFileUploadStop(
                    EkxProtocol::Payload::FileUploadStop::StopReason::Finished,
                    pReq->content_len - remaining );
            }
        }
    }

    return nEspError;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

esp_err_t UcdHttpServerTask::restAliveCheckJsHandler( httpd_req_t * pReq )
{
    esp_err_t nEspError = ESP_FAIL;

    if ( pReq != nullptr )
    {
        UcdHttpServerTask * const pThis = reinterpret_cast<UcdHttpServerTask *>( pReq->user_ctx );

        if ( pThis != nullptr )
        {
            pThis->vlogInfo( "restAliveCheckJsHandler()" );

            extern const unsigned char pucBlobDataStartAliveCheckJs[] asm ( "_binary_aliveCheck_js_stripped_gz_start" );
            extern const unsigned char pucBlobDataEndAliveCheckJs[]   asm ( "_binary_aliveCheck_js_stripped_gz_end" );
            const size_t               u32BlobDataSize = ( pucBlobDataEndAliveCheckJs - pucBlobDataStartAliveCheckJs );
            pThis->vlogInfo( "restAliveCheckJsHandler() size %u", u32BlobDataSize );
            nEspError = pThis->sendChunkData( pReq,
                                              "text/javascript",
                                              "aliveCheck.js",
                                              true,
                                              reinterpret_cast<const char *>( pucBlobDataStartAliveCheckJs ),
                                              static_cast<int>( u32BlobDataSize ) );
        }
    }

    return nEspError;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

esp_err_t UcdHttpServerTask::restPopupManagementJsHandler( httpd_req_t * pReq )
{
    esp_err_t nEspError = ESP_FAIL;

    if ( pReq != nullptr )
    {
        UcdHttpServerTask * const pThis = reinterpret_cast<UcdHttpServerTask *>( pReq->user_ctx );

        if ( pThis != nullptr )
        {
            pThis->vlogInfo( "restPopupManagementJsHandler()" );

            extern const unsigned char pucBlobDataStartPopupManagementJs[] asm ( "_binary_popupManagement_js_stripped_gz_start" );
            extern const unsigned char pucBlobDataEndPopupManagementJs[]   asm ( "_binary_popupManagement_js_stripped_gz_end" );
            const size_t               u32BlobDataSize = ( pucBlobDataEndPopupManagementJs - pucBlobDataStartPopupManagementJs );

            nEspError = pThis->sendChunkData( pReq,
                                              "text/javascript",
                                              "popupManagement.js",
                                              true,
                                              reinterpret_cast<const char *>( pucBlobDataStartPopupManagementJs ),
                                              static_cast<int>( u32BlobDataSize ) );
        }
    }

    return nEspError;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

esp_err_t UcdHttpServerTask::restFirmwareUpdateObserverJsHandler( httpd_req_t * pReq )
{
    esp_err_t nEspError = ESP_FAIL;

    if ( pReq != nullptr )
    {
        UcdHttpServerTask * const pThis = reinterpret_cast<UcdHttpServerTask *>( pReq->user_ctx );

        if ( pThis != nullptr )
        {
            pThis->vlogInfo( "restFirmwareUpdateObserverJsHandler()" );

            extern const unsigned char pucBlobDataStartFirmwareUpdateObserverJs[] asm ( "_binary_firmwareUpdateObserver_js_stripped_gz_start" );
            extern const unsigned char pucBlobDataEndFirmwareUpdateObserverJs[]   asm ( "_binary_firmwareUpdateObserver_js_stripped_gz_end" );
            const size_t               u32BlobDataSize = ( pucBlobDataEndFirmwareUpdateObserverJs - pucBlobDataStartFirmwareUpdateObserverJs );

            nEspError = pThis->sendChunkData( pReq,
                                              "text/javascript",
                                              "firmwareUpdateObserver.js",
                                              true,
                                              reinterpret_cast<const char *>( pucBlobDataStartFirmwareUpdateObserverJs ),
                                              static_cast<int>( u32BlobDataSize ) );
        }
    }

    return nEspError;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

esp_err_t UcdHttpServerTask::restCommonHelpersJsHandler( httpd_req_t * pReq )
{
    esp_err_t nEspError = ESP_FAIL;

    if ( pReq != nullptr )
    {
        UcdHttpServerTask * const pThis = reinterpret_cast<UcdHttpServerTask *>( pReq->user_ctx );

        if ( pThis != nullptr )
        {
            pThis->vlogInfo( "restCommonHelpersJsHandler()" );

            extern const unsigned char pucBlobDataStartCommonHelpersJs[] asm ( "_binary_commonHelpers_js_stripped_gz_start" );
            extern const unsigned char pucBlobDataEndCommonHelpersJs[]   asm ( "_binary_commonHelpers_js_stripped_gz_end" );
            const size_t               u32BlobDataSize = ( pucBlobDataEndCommonHelpersJs - pucBlobDataStartCommonHelpersJs );

            nEspError = pThis->sendChunkData( pReq,
                                              "text/javascript",
                                              "commonHelpers.js",
                                              true,
                                              reinterpret_cast<const char *>( pucBlobDataStartCommonHelpersJs ),
                                              static_cast<int>( u32BlobDataSize ) );
        }
    }

    return nEspError;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

esp_err_t UcdHttpServerTask::restIncludeHeaderHtmlHandler( httpd_req_t * pReq )
{
    esp_err_t nEspError = ESP_FAIL;

    if ( pReq != nullptr )
    {
        UcdHttpServerTask * const pThis = reinterpret_cast<UcdHttpServerTask *>( pReq->user_ctx );

        if ( pThis != nullptr )
        {
            pThis->vlogInfo( "restIncludeHeaderHtmlHandler()" );

            extern const unsigned char pucBlobDataStartIncludeHeaderHtml[] asm ( "_binary_includeHeader_html_stripped_gz_start" );
            extern const unsigned char pucBlobDataEndIncludeHeaderHtml[]   asm ( "_binary_includeHeader_html_stripped_gz_end" );
            const size_t               u32BlobDataSize = ( pucBlobDataEndIncludeHeaderHtml - pucBlobDataStartIncludeHeaderHtml );

            nEspError = pThis->sendChunkData( pReq,
                                              "text/html",
                                              "includeHeader.html",
                                              true,
                                              reinterpret_cast<const char *>( pucBlobDataStartIncludeHeaderHtml ),
                                              static_cast<int>( u32BlobDataSize ) );
        }
    }

    return nEspError;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

esp_err_t UcdHttpServerTask::restIncludeFooterHtmlHandler( httpd_req_t * pReq )
{
    esp_err_t nEspError = ESP_FAIL;

    if ( pReq != nullptr )
    {
        UcdHttpServerTask * const pThis = reinterpret_cast<UcdHttpServerTask *>( pReq->user_ctx );

        if ( pThis != nullptr )
        {
            pThis->vlogInfo( "restIncludeFooterHtmlHandler()" );

            extern const unsigned char pucBlobDataStartIncludeFooterHtml[] asm ( "_binary_includeFooter_html_stripped_gz_start" );
            extern const unsigned char pucBlobDataEndIncludeFooterHtml[]   asm ( "_binary_includeFooter_html_stripped_gz_end" );
            const size_t               u32BlobDataSize = ( pucBlobDataEndIncludeFooterHtml - pucBlobDataStartIncludeFooterHtml );

            nEspError = pThis->sendChunkData( pReq,
                                              "text/html",
                                              "includeFooter.html",
                                              true,
                                              reinterpret_cast<const char *>( pucBlobDataStartIncludeFooterHtml ),
                                              static_cast<int>( u32BlobDataSize ) );
        }
    }

    return nEspError;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

esp_err_t UcdHttpServerTask::restIncludeDevOptionsHtmlHandler( httpd_req_t * pReq )
{
    esp_err_t nEspError = ESP_FAIL;

    if ( pReq != nullptr )
    {
        UcdHttpServerTask * const pThis = reinterpret_cast<UcdHttpServerTask *>( pReq->user_ctx );

        if ( pThis != nullptr )
        {
            pThis->vlogInfo( "restIncludeDevOptionsHtmlHandler()" );

            if ( pThis->m_bDevOption )
            {
                extern const unsigned char pucBlobDataStartIncludeDevOptionsHtml[] asm ( "_binary_includeDevOptions_html_stripped_gz_start" );
                extern const unsigned char pucBlobDataEndIncludeDevOptionsHtml[]   asm ( "_binary_includeDevOptions_html_stripped_gz_end" );
                const size_t               u32BlobDataSize = ( pucBlobDataEndIncludeDevOptionsHtml - pucBlobDataStartIncludeDevOptionsHtml );

                nEspError = pThis->sendChunkData( pReq,
                                                  "text/html",
                                                  "includeDevOptions.html",
                                                  true,
                                                  reinterpret_cast<const char *>( pucBlobDataStartIncludeDevOptionsHtml ),
                                                  static_cast<int>( u32BlobDataSize ) );
            }
            else
            {
                nEspError = pThis->sendChunkData( pReq,
                                                  "text/html",
                                                  "includeDevOptions.html",
                                                  false,
                                                  "",
                                                  0 );
            }
        }
    }

    return nEspError;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

esp_err_t UcdHttpServerTask::restIncludePopupHtmlHandler( httpd_req_t * pReq )
{
    esp_err_t nEspError = ESP_FAIL;

    if ( pReq != nullptr )
    {
        UcdHttpServerTask * const pThis = reinterpret_cast<UcdHttpServerTask *>( pReq->user_ctx );

        if ( pThis != nullptr )
        {
            pThis->vlogInfo( "restIncludePopupHtmlHandler()" );

            extern const unsigned char pucBlobDataStartIncludePopupHtml[] asm ( "_binary_includePopup_html_stripped_gz_start" );
            extern const unsigned char pucBlobDataEndIncludePopupHtml[]   asm ( "_binary_includePopup_html_stripped_gz_end" );
            const size_t               u32BlobDataSize = ( pucBlobDataEndIncludePopupHtml - pucBlobDataStartIncludePopupHtml );

            nEspError = pThis->sendChunkData( pReq,
                                              "text/html",
                                              "includePopup.html",
                                              true,
                                              reinterpret_cast<const char *>( pucBlobDataStartIncludePopupHtml ),
                                              static_cast<int>( u32BlobDataSize ) );
        }
    }

    return nEspError;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

esp_err_t UcdHttpServerTask::restIncludeNoJavascriptHtmlHandler( httpd_req_t * pReq )
{
    esp_err_t nEspError = ESP_FAIL;

    if ( pReq != nullptr )
    {
        UcdHttpServerTask * const pThis = reinterpret_cast<UcdHttpServerTask *>( pReq->user_ctx );

        if ( pThis != nullptr )
        {
            pThis->vlogInfo( "restIncludeNoJavascriptHtmlHandler()" );

            extern const unsigned char pucBlobDataStartIncludeNoJavascriptHtml[] asm ( "_binary_includeNoJavascript_html_stripped_gz_start" );
            extern const unsigned char pucBlobDataEndIncludeNoJavascriptHtml[]   asm ( "_binary_includeNoJavascript_html_stripped_gz_end" );
            const size_t               u32BlobDataSize = ( pucBlobDataEndIncludeNoJavascriptHtml - pucBlobDataStartIncludeNoJavascriptHtml );

            nEspError = pThis->sendChunkData( pReq,
                                              "text/html",
                                              "includeNoJavascript.html",
                                              true,
                                              reinterpret_cast<const char *>( pucBlobDataStartIncludeNoJavascriptHtml ),
                                              static_cast<int>( u32BlobDataSize ) );
        }
    }

    return nEspError;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void UcdHttpServerTask::serverBothPrintf( const esp_err_t nLogType,
                                          const char *    pszFormat,
                                          ... )
{
    va_list argp;
    va_start( argp, pszFormat );

    static char szMsg[1000];
    szMsg[0] = '\0';

    vsprintf( szMsg, pszFormat, argp );

    va_end( argp );

    if ( nLogType == ESP_OK )
    {
        vlogInfo( "%s", szMsg );
    }
    else
    {
        vlogError( "%s", szMsg );
    }

    if ( m_pServerRequest )
    {
        httpd_resp_set_hdr( m_pServerRequest, "Access-Control-Allow-Origin", "*" );
        httpd_resp_send_chunk( m_pServerRequest, szMsg, strlen( szMsg ) );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void UcdHttpServerTask::serverRemotePrintf( const char * pszFormat,
                                            ... )
{
    if ( m_pServerRequest )
    {
        va_list argp;
        va_start( argp, pszFormat );

        static char szMsg[200];
        szMsg[0] = '\0';

        vsprintf( szMsg, pszFormat, argp );

        va_end( argp );

        httpd_resp_set_hdr( m_pServerRequest, "Access-Control-Allow-Origin", "*" );
        httpd_resp_send_chunk( m_pServerRequest, szMsg, strlen( szMsg ) );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void UcdHttpServerTask::replaceDynamicCommonContents( std::string & strHtmlContent ) const
{
    //    replaceString( strHtmlContent, "var_currentSn", x54App.systemStateCtrlTask().configStorage().grinderSerialNo() );
    replaceString( strHtmlContent, "var_sessionId", m_strCurrentGetSessionId );
    replaceString( strHtmlContent, "var_loginErrorMsg", m_strLoginErrorMsg );
    //    replaceString( strHtmlContent, "var_currentApIp", x54App.systemStateCtrlTask().configStorage().currentApIpv4() );
    //    replaceString( strHtmlContent, "var_currentStaIp", x54App.systemStateCtrlTask().configStorage().currentStaIpv4() );
    //    replaceString( strHtmlContent, "var_hostName", x54App.systemStateCtrlTask().configStorage().hostname() );
#if RELEASE_BRANCH == 0
    replaceString( strHtmlContent, "var_debugHint", "block" );
#else
    replaceString( strHtmlContent, "var_debugHint", "none" );
#endif
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void UcdHttpServerTask::replaceDynamicWifiContents( std::string & strHtmlContent ) const
{
    //    replaceString( strHtmlContent, "var_currentApIp", x54App.systemStateCtrlTask().configStorage().currentApIpv4() );
    //    replaceString( strHtmlContent, "var_currentStaIp", x54App.systemStateCtrlTask().configStorage().currentStaIpv4() );
    //    replaceString( strHtmlContent, "var_hostName", x54App.systemStateCtrlTask().configStorage().hostname() );
    //    replaceString( strHtmlContent, "var_apMacAddress", x54App.systemStateCtrlTask().configStorage().apMacAddress() );
    //    replaceString( strHtmlContent, "var_staMacAddress", x54App.systemStateCtrlTask().configStorage().staMacAddress() );
    //    replaceString( strHtmlContent, "var_wifiStaSsid", x54App.systemStateCtrlTask().configStorage().wifiStaSsid() );

    //    if ( x54App.systemStateCtrlTask().configStorage().wifiStaPassword().empty() )
    //    {
    //        replaceString( strHtmlContent, "var_wifiStaPassword", "" );
    //    }
    //    else
    {
        replaceString( strHtmlContent, "var_wifiStaPassword", FAKE_PASSWORD );
    }

    //    replaceString( strHtmlContent, "var_staStaticIp", x54App.systemStateCtrlTask().configStorage().wifiStaStaticIpv4() );
    //    replaceString( strHtmlContent, "var_staStaticNetmask", x54App.systemStateCtrlTask().configStorage().wifiStaStaticNetmask() );
    //    replaceString( strHtmlContent, "var_staStaticGateway", x54App.systemStateCtrlTask().configStorage().wifiStaStaticGateway() );
    //
    //    replaceString( strHtmlContent, "var_staWifiConfigDhcpChecked",
    //                   ( x54App.systemStateCtrlTask().configStorage().isWifiStaDhcp() ) ? "checked" : "" );
    //    replaceString( strHtmlContent, "var_staWifiConfigStaticChecked",
    //                   ( x54App.systemStateCtrlTask().configStorage().isWifiStaDhcp() ) ? "" : "checked" );

    replaceString( strHtmlContent, "var_saveWifiDisabled", ( true ? "" : "disabled" ) );
    replaceString( strHtmlContent, "var_sessionId", m_strCurrentGetSessionId );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void UcdHttpServerTask::replaceString( std::string &       subject,
                                       const std::string & search,
                                       const std::string & replace ) const
{
    const size_t u32SearchLen  = search.length();
    const size_t u32ReplaceLen = replace.length();
    const size_t u32SubjectLen = strlen( subject.c_str() );

    if ( u32SearchLen == 0 )
    {
        vlogWarning( "replaceString() searchLen==0" );
    }
    else if ( u32SubjectLen == 0 )
    {
        vlogWarning( "replaceString() u32SubjectLen==0" );
    }
    else if ( u32SubjectLen + u32ReplaceLen > subject.size() )
    {
        vlogWarning( "replaceString() insufficient subject size" );
    }
    else
    {
        size_t pos = 0;

        while ( ( pos = subject.find( search, pos ) ) != std::string::npos )
        {
            const size_t u32SubjectLen = strlen( subject.c_str() );   // we need the real strlen within the oversized std::string

            // subject is the HTML document, which is oversized
            // check here, if core C-string operations fit within the buffer size
            if ( ( u32SubjectLen - u32SearchLen + u32ReplaceLen ) > subject.size() )
            {
                // we won't resize the buffer because its occasionally runs into bad_alloc exception

                vlogError( "replaceString: heap %d", esp_get_free_heap_size() );
                vlogError( "replaceString: pos %d", pos );
                vlogError( "replaceString: search %s - size: %d", search.c_str(), u32SearchLen );
                vlogError( "replaceString: replace %s - size: %d", replace.c_str(), u32ReplaceLen );
                vlogError( "replaceString: subject size: %d strlen %d", subject.size(), u32SubjectLen );

                throw std::bad_alloc();
            }
            else
            {
                char * const pContent = const_cast<char *>( subject.data() );  // NOSONAR common pattern to access constant string content data
                std::memmove( pContent + pos + u32ReplaceLen,
                              pContent + pos + u32SearchLen,
                              u32SubjectLen - pos - u32SearchLen );
                std::memcpy( pContent + pos,
                             replace.data(),
                             u32ReplaceLen );
                *( pContent + u32SubjectLen + u32ReplaceLen - u32SearchLen ) = '\0';
                pos                                                         += u32ReplaceLen;
            }
        }
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void UcdHttpServerTask::replaceFormInputTextValue( std::string &       strSubject,
                                                   const std::string & strVariable,
                                                   const int           nValue ) const
{
    std::ostringstream str;
    str << nValue;
    replaceString( strSubject, strVariable, str.str() );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

uint8_t UcdHttpServerTask::fromHex( const int ch ) const
{
    return static_cast<uint8_t>( ( isdigit( ch ) > 0 ) ? ( ch - '0' ) : ( tolower( ch ) - 'a' + HEX_DIGIT_10_OFFSET ) );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void UcdHttpServerTask::parseUriParams( httpd_req_t * const pReq )
{
    m_mapUriParams.clear();

    std::regex regParam( "[\\?\\&](\\w+)=(\\w+[^$\\&])" );

    std::string strUri            = pReq->uri;
    auto        matchesParamBegin = std::sregex_iterator( strUri.begin(), strUri.end(), regParam );
    auto        matchesParamEnd   = std::sregex_iterator();

    for ( auto itParam = matchesParamBegin; itParam != matchesParamEnd; itParam++ )
    {
        std::string strParam = itParam->str();
        std::string strKey   = itParam->str( 1 );
        std::string strValue = itParam->str( 2 );

        m_mapUriParams[strKey] = strValue;
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void UcdHttpServerTask::processAckNackReceived( const uint32_t u32MsgCounter,
                                                const bool     bSuccess )
{
    std::chrono::steady_clock::time_point tNow = std::chrono::steady_clock::now();

    const std::lock_guard<std::mutex> lock( m_mutexAckMap );

    m_ackMap[u32MsgCounter].m_bSuccess      = bSuccess;
    m_ackMap[u32MsgCounter].m_tlastReceived = tNow;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

bool UcdHttpServerTask::waitForAck( const uint32_t u32MsgCounter,
                                    const uint32_t u32TimeoutMs )
{
    bool bSuccess = false;

    const uint32_t u32TimeIncMs = 5;

    for ( uint32_t u32TimeCounter = 0; u32TimeCounter < u32TimeoutMs; u32TimeCounter += u32TimeIncMs )
    {
        {
            const std::lock_guard<std::mutex> lock( m_mutexAckMap );

            AckMap::iterator it = m_ackMap.find( u32MsgCounter );

            if ( it != m_ackMap.end() )
            {
                bSuccess = it->second.m_bSuccess;
                m_ackMap.erase( u32MsgCounter );
                break;
            }
        }
        portYIELD();
        vTaskDelay( u32TimeIncMs / portTICK_PERIOD_MS );
    }

    return bSuccess;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

EkxProtocol::Payload::JsonMessage * UcdHttpServerTask::waitForJsonMessage( const uint32_t u32MsgCounter,
                                                                           const uint32_t u32TimeoutMs )
{
    EkxProtocol::Payload::JsonMessage * pJsonMessage = nullptr;

    const uint32_t u32TimeIncMs = 5;

    for ( uint32_t u32TimeCounter = 0; u32TimeCounter < u32TimeoutMs; u32TimeCounter += u32TimeIncMs )
    {
        {
            const std::lock_guard<std::mutex> lock( m_mutexPayloadList );

            for ( PayloadList::iterator it = m_listLinuxReplies.begin(); it != m_listLinuxReplies.end(); it++ )
            {
                //                pResult = dynamic_cast<EkxProtocol::Payload::JsonMessage*>( *it );
                pJsonMessage = dynamic_cast<EkxProtocol::Payload::JsonMessage *>( ( it->m_pPayload ) );

                if ( pJsonMessage
                     && pJsonMessage->refMsgCounter().value() == u32MsgCounter )
                {
                    m_listLinuxReplies.erase( it );
                    break;
                }
                pJsonMessage = nullptr;
            }

            if ( pJsonMessage )
            {
                break;
            }
        }
        portYIELD();
        vTaskDelay( u32TimeIncMs / portTICK_PERIOD_MS );
    }

    if ( !pJsonMessage )
    {
        vlogWarning( "waitForJsonMessage() timeout for msg %u", u32MsgCounter );
    }

    return pJsonMessage;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

EkxProtocol::Payload::FileDownloadStartResponse * UcdHttpServerTask::waitForFileDownloadStartResponse( const uint32_t u32MsgCounter,
                                                                                                       const uint32_t u32TimeoutMs )
{
    EkxProtocol::Payload::FileDownloadStartResponse * pResult = nullptr;

    const uint32_t u32TimeIncMs = 5;

    for ( uint32_t u32TimeCounter = 0; u32TimeCounter < u32TimeoutMs; u32TimeCounter += u32TimeIncMs )
    {
        {
            const std::lock_guard<std::mutex> lock( m_mutexPayloadList );

            for ( PayloadList::iterator it = m_listLinuxReplies.begin(); it != m_listLinuxReplies.end(); it++ )
            {
                //                pResult = dynamic_cast<EkxProtocol::Payload::FileDownloadStartResponse*>( *it );
                pResult = dynamic_cast<EkxProtocol::Payload::FileDownloadStartResponse *>( it->m_pPayload );

                if ( pResult )
                {
                    //logInfo( "find response" );
                    if ( pResult->refMsgCounter().value() == u32MsgCounter )
                    {
                        m_listLinuxReplies.erase( it );
                        break;
                    }
                }
                pResult = nullptr;
            }

            if ( pResult )
            {
                break;
            }
        }
        portYIELD();
        vTaskDelay( u32TimeIncMs / portTICK_PERIOD_MS );
    }

    return pResult;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

EkxProtocol::Payload::FileDownloadDataResponse * UcdHttpServerTask::waitForFileDownloadDataResponse( const uint32_t u32MsgCounter,
                                                                                                     const uint32_t u32TimeoutMs )
{
    EkxProtocol::Payload::FileDownloadDataResponse * pResult = nullptr;

    const uint32_t u32TimeIncMs = 5;

    for ( uint32_t u32TimeCounter = 0; u32TimeCounter < u32TimeoutMs; u32TimeCounter += u32TimeIncMs )
    {
        {
            const std::lock_guard<std::mutex> lock( m_mutexPayloadList );

            for ( PayloadList::iterator it = m_listLinuxReplies.begin(); it != m_listLinuxReplies.end(); it++ )
            {
                //                pResult = dynamic_cast<EkxProtocol::Payload::FileDownloadDataResponse*>( *it );
                pResult = dynamic_cast<EkxProtocol::Payload::FileDownloadDataResponse *>( it->m_pPayload );

                if ( pResult
                     && pResult->refMsgCounter().value() == u32MsgCounter )
                {
                    m_listLinuxReplies.erase( it );
                    break;
                }
                pResult = nullptr;
            }

            if ( pResult )
            {
                break;
            }
        }
        portYIELD();
        vTaskDelay( u32TimeIncMs / portTICK_PERIOD_MS );
    }

    return pResult;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

bool UcdHttpServerTask::queueAckNackReceived( const uint32_t u32MsgCounter,
                                              const bool     bSuccess )
{
    return httpServerQueue()->sendAckNackReceived( u32MsgCounter, bSuccess ) == pdPASS;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

std::string UcdHttpServerTask::urlDecode( const std::string & strText ) const
{
    int                h;
    std::ostringstream escaped;
    escaped.fill( '0' );

    const auto n = strText.end();
    for ( auto i = strText.begin(); i != n; ++i )
    {
        const std::string::value_type c = ( *i );

        if ( c == '%' )
        {
            if ( ( i[HEX_DIGIT_1] != '\0' )
                 && ( i[HEX_DIGIT_2] != '\0' ) )
            {
                h = static_cast<int>( static_cast<uint8_t>( fromHex( i[HEX_DIGIT_1] ) << 4 ) | fromHex( i[HEX_DIGIT_2] ) );
                escaped << static_cast<char>( h );   // NOSONAR must be char to be correctly inserted into ostringstream
                i += HEX_DIGIT_SPAN;
            }
        }
        else if ( c == '+' )
        {
            escaped << ' ';
        }
        else
        {
            escaped << c;
        }
    }

    return escaped.str();
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

bool UcdHttpServerTask::queueCtrlUartCommand( const EkxProtocol::DriverId                  u8DriverId,
                                              const uint32_t                               u32MsgCounter,
                                              const uint8_t                                u8RepeatCounter,
                                              EkxProtocol::Payload::MessagePayload * const pMessagePayload )
{
    return httpServerQueue()->sendCtrlUartCommand( u8DriverId, u32MsgCounter, u8RepeatCounter, pMessagePayload ) == pdPASS;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void UcdHttpServerTask::processCtrlUartCommand( const EkxProtocol::DriverId                        u8DriverId,
                                                const uint32_t                                     u32MsgCounter,
                                                const uint8_t                                      u8RepeatCounter,
                                                const EkxProtocol::Payload::MessagePayload * const pMessagePayload )
{
    PayloadInfo payload;

    std::chrono::steady_clock::time_point tNow = std::chrono::steady_clock::now();

    payload.m_tlastReceived   = tNow;//std::chrono::steady_clock::now();
    payload.m_u32MsgCounter   = u32MsgCounter;
    payload.m_u8RepeatCounter = u8RepeatCounter;

    //	vlogInfo("Time Now=%u", tNow);
    //	vlogInfo("Time m_tlastReceived=%u", payload.m_tlastReceived);

    switch ( pMessagePayload->payloadType() )
    {
        case EkxProtocol::Payload::PayloadType::FileDownloadDataResponse:
        case EkxProtocol::Payload::PayloadType::FileDownloadStartResponse:
        case EkxProtocol::Payload::PayloadType::JsonMessage:
        {
            const std::lock_guard<std::mutex> lock( m_mutexPayloadList );
            payload.m_pPayload = pMessagePayload->clone();

            m_listLinuxReplies.push_back( payload );

            sendCtrlUartReplyAck( u32MsgCounter, u8RepeatCounter );
        }
        break;

        default:
        {
            vlogError( "processCtrlUartCommand: cant't process data; unknown command" );
            sendCtrlUartReplyNack( u32MsgCounter, u8RepeatCounter, EkxProtocol::Payload::Nack::NackReason::UnknownDriverCommand );
        }
        break;
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void UcdHttpServerTask::performTimeout()
{
    std::chrono::steady_clock::time_point tNow = std::chrono::steady_clock::now();

    /* Check AckNack List ----------------------------------------------------------------------------------------*/
    //	vlogInfo("Run timeout, m_ackMap.size=%u", m_ackMap.size());
    // Collect the keys of items to be removed
    std::vector<int> keysToRemove;
    for ( const auto & pair : m_ackMap )
    {
        //		vlogWarning("Check Time, msgCnt=%u, Time=%u", pair.first,  tNow - pair.second.m_tlastReceived);
        if ( std::chrono::duration_cast<std::chrono::microseconds>( tNow - pair.second.m_tlastReceived ).count() > c_u32ListTimeoutUs )
        {
            vlogError( "performTimeout() Remove expired AckNack from List, msgCnt=%u", pair.first );
            keysToRemove.push_back( pair.first );
        }
    }
    // Remove the items using the collected keys
    for ( const int key : keysToRemove )
    {
        m_ackMap.erase( key );
    }

    /* Check Payload List ----------------------------------------------------------------------------------------*/
    //	vlogInfo("Run timeout, m_listLinuxReplies.size=%u", m_listLinuxReplies.size());
    // Collect the indices of items to be removed
    std::vector<int> indicesToRemove;
    for ( size_t i = 0; i < m_listLinuxReplies.size(); ++i )
    {
        const PayloadInfo & info = m_listLinuxReplies[i];

        //		vlogWarning("Check Time, LastTime=%lu",  std::chrono::duration_cast<std::chrono::microseconds>( tNow - m_listLinuxReplies[i].m_tlastReceived ).count());
        if ( std::chrono::duration_cast<std::chrono::microseconds>( tNow - info.m_tlastReceived ).count() > c_u32ListTimeoutUs )
        {
            vlogError( "performTimeout() Remove expired payload from list %d %u %d",
                       static_cast<int>( info.m_pPayload->payloadType() ),
                       info.m_u32MsgCounter,
                       static_cast<int>( info.m_u8RepeatCounter ) );
            indicesToRemove.push_back( i );
        }
    }
    // Remove the items using the collected indices
    for ( auto it = indicesToRemove.rbegin(); it != indicesToRemove.rend(); ++it )
    {
        m_listLinuxReplies.erase( m_listLinuxReplies.begin() + *it );
    }
}

/*************************************************************************************************************************************************************
*
*************************************************************************************************************************************************************/

bool UcdHttpServerTask::sendFileUploadStart( const std::string & strFileName,
                                             const uint32_t      u32FileSize )
{
    EkxProtocol::Payload::FileUploadStart * pPayload = new EkxProtocol::Payload::FileUploadStart();
    pPayload->fileName().setValue( strFileName );
    pPayload->fileSize().setValue( u32FileSize );

    vlogInfo( "sendFileUploadStart() Send Start, name:%s, size:%u ", pPayload->fileName().value().c_str(), pPayload->fileSize().value() );

    uint32_t u32MsgCounter = sendCtrlUartReply( pPayload, true );

    bool ack = waitForAck( u32MsgCounter, c_u32StandardTimeoutWaitAckMs );

    ack ? vlogInfo( "Start ACK %u", u32MsgCounter ) : vlogError( "Start NACK %u", u32MsgCounter );

    return ack;
}

/*************************************************************************************************************************************************************
*
*************************************************************************************************************************************************************/

bool UcdHttpServerTask::sendFileUploadData( const uint32_t     u32No,
                                            const uint32_t     u32Offset,
                                            const uint32_t     u32Len,
                                            const char * const pData )
{
    bool bSuccess = false;

    if ( u32Len > c_u32MaxDataLen )
    {
        vlogError( "sendFileUploadData() data size out of range: %u", u32Len );
    }
    else
    {
        EkxProtocol::Payload::FileUploadData * pPayload = new EkxProtocol::Payload::FileUploadData();
        pPayload->chunkNo().setValue( u32No );
        pPayload->chunkOffset().setValue( u32Offset );

        std::string strData( u32Len, 0 );
        std::memcpy( const_cast<char *>( strData.data() ), pData, u32Len );
        pPayload->chunkData().setValue( strData );

        static int nDebugCounter = 0;
        if ( ++nDebugCounter % 10 == 0 )
        {
            vlogInfo( "sendFileUploadData() Send Chunk, no: %u, offset: %u, len: %u, heap %d ", pPayload->chunkNo().value(), pPayload->chunkOffset().value(), strData.size(), esp_get_free_heap_size() );
        }

        uint32_t u32MsgCounter = sendCtrlUartReply( pPayload, true );

        bSuccess = waitForAck( u32MsgCounter, c_u32StandardTimeoutWaitAckMs );

        if ( bSuccess )
        {
            if ( nDebugCounter % 10 == 0 )
            {
                vlogInfo( "sendFileUploadData() Data ACK %u", u32MsgCounter );
            }
        }
        else
        {
            vlogError( "sendFileUploadData() Data NACK %u", u32MsgCounter );
        }
    }

    return bSuccess;
}

/*************************************************************************************************************************************************************
*
*************************************************************************************************************************************************************/

bool UcdHttpServerTask::sendFileUploadStop( EkxProtocol::Payload::FileUploadStop::StopReason reason,
                                            uint32_t                                         u32TransferdSize )
{
    EkxProtocol::Payload::FileUploadStop * pPayload = new EkxProtocol::Payload::FileUploadStop();
    pPayload->stopReason().setValue( reason );
    pPayload->transferedSize().setValue( u32TransferdSize );

    vlogInfo( "sendFileUploadStop()" );

    uint32_t u32MsgCounter = sendCtrlUartReply( pPayload, true );

    bool ack = waitForAck( u32MsgCounter, c_u32StandardTimeoutWaitAckMs );

    ack ? logInfo( "Stop ACK" ) : logError( "Stop NACK" );

    return ack;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

int UcdHttpServerTask::sendFileDownloadStartRequest( const std::string & strFileName )
{
    EkxProtocol::Payload::FileDownloadStartRequest * pPayload = new EkxProtocol::Payload::FileDownloadStartRequest();
    pPayload->fileName().setValue( strFileName );

    vlogInfo( "sendFileDownloadStartRequest() Send Request Start, name:%s", pPayload->fileName().value().c_str() );

    uint32_t u32MsgCounter = sendCtrlUartReply( pPayload, true );

    bool ack = waitForAck( u32MsgCounter, c_u32StandardTimeoutWaitAckMs );

    if ( ack == true )
    {
        //vlogInfo("Start ACK %u", u32MsgCounter );
        return u32MsgCounter;
    }

    //vlogError("Start NACK %u", u32MsgCounter);
    return -1;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

int UcdHttpServerTask::sendFileDownloadDataRequest( const std::string & strFileName,
                                                    const uint32_t      u32Offset,
                                                    const uint32_t      u32Size )
{
    EkxProtocol::Payload::FileDownloadDataRequest * pPayload = new EkxProtocol::Payload::FileDownloadDataRequest();
    pPayload->fileName().setValue( strFileName );
    pPayload->chunkOffset().setValue( u32Offset );
    pPayload->chunkSize().setValue( u32Size );

    //vlogInfo("sendFileDownloadDataRequest() Send Data Request, name:%s, size:%u ", pPayload->fileName().value().c_str(), pPayload->chunkSize().value() );

    uint32_t u32MsgCounter = sendCtrlUartReply( pPayload, true );

    bool ack = waitForAck( u32MsgCounter, c_u32StandardTimeoutWaitAckMs );

    if ( ack == true )
    {
        //vlogInfo("Data ACK %u", u32MsgCounter );
        return u32MsgCounter;
    }

    //vlogError("Data NACK %u", u32MsgCounter);
    return -1;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/
