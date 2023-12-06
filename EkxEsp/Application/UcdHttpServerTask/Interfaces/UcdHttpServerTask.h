/*
 * HttpServerTask.h
 *
 *  Created on: 24.10.2019
 *      Author: gesser
 */

#ifndef UcdHttpServerTask_h
#define UcdHttpServerTask_h

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#include <string>
#include <mutex>
#include <chrono>

#include "UartControlledDevice.h"
#include "CtrlUartDispatcher.h"

#include "FreeRtosQueueTask.h"

#include <esp_http_server.h>
#include <esp_partition.h>
#include <map>

#include "UcdHttpServerQueue.h"

#define MAX_POST_DATA_BUF_LEN   2000

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class UcdHttpServerTask : public UartControlledDevice
{
private:

    struct AckInfo
    {
        std::chrono::steady_clock::time_point m_tlastReceived;
        bool                                  m_bSuccess;
    };

    struct PayloadInfo
    {
        std::chrono::steady_clock::time_point  m_tlastReceived;
        EkxProtocol::Payload::MessagePayload * m_pPayload;
        uint32_t                               m_u32MsgCounter;
        uint8_t                                m_u8RepeatCounter;
    };

    static const uint8_t  c_u8MaxFileUploadTimeoutErrorCount    = 10;
    static const int      c_nFileUploadChunkSize                = 2000;
    static const uint32_t c_u32MaxDataLen                       = 5000;
    static const uint32_t c_u32ListTimeoutUs                    = 2000000;
    static const uint32_t c_u32StandardTimeoutWaitMessageMs     = 2000;
    static const uint32_t c_u32FileDownloadTimeoutWaitMessageMs = 5000;
    static const uint32_t c_u32StandardTimeoutWaitAckMs         = 500;

public:

    using keyMap_t = std::map<std::string, std::string>;

    using AckMap = std::map<uint32_t, AckInfo>;

    using PayloadList = std::vector<PayloadInfo>;

public:

    UcdHttpServerTask( UxEspCppLibrary::EspApplication * const pApplication );

    ~UcdHttpServerTask() override;

    bool startupStep( const UxEspCppLibrary::FreeRtosQueueTask::startupStep_t nStartupStep ) override;

    void processTaskMessage( void ) override;

    UcdHttpServerQueue * httpServerQueue( void );

    const UcdHttpServerQueue * httpServerQueue( void ) const;

    bool queueCtrlUartCommand( const EkxProtocol::DriverId                  u8DriverId,
                               const uint32_t                               u32MsgCounter,
                               const uint8_t                                u8RepeatCounter,
                               EkxProtocol::Payload::MessagePayload * const pMessagePayload ) override;

    bool queueAckNackReceived( const uint32_t u32MsgCounter,
                               const bool     bSuccess ) override;

    bool sendFileUploadStart( const std::string & strFileName,
                              const uint32_t      u32FileSize );

    bool sendFileUploadData( const uint32_t     u32No,
                             const uint32_t     u32Offset,
                             const uint32_t     u32Len,
                             const char * const pData );

    bool sendFileUploadStop( EkxProtocol::Payload::FileUploadStop::StopReason reason,
                             uint32_t                                         u32TransferdSize );


    int sendFileDownloadStartRequest( const std::string & strFileName );

    int sendFileDownloadDataRequest( const std::string & strFileName,
                                     const uint32_t      u32Offset,
                                     const uint32_t      u32Size );

    void performTimeout();

protected:

    void processCtrlUartCommand( const EkxProtocol::DriverId                        u8DriverId,
                                 const uint32_t                                     u32MsgCounter,
                                 const uint8_t                                      u8RepeatCounter,
                                 const EkxProtocol::Payload::MessagePayload * const pMessagePayload ) override;

    void processAckNackReceived( const uint32_t u32MsgCounter,
                                 const bool     bSuccess ) override;

    static esp_err_t restRestApiHandler( httpd_req_t * pReq );

    static esp_err_t restRedirect( httpd_req_t *       pReq,
                                   const std::string & strUrl /* ,
                                                                 const SessionManager::sessionId_t u32SessionId*/);

    static esp_err_t restIndexHandler( httpd_req_t * pReq );

    static esp_err_t restCopyrightHandler( httpd_req_t * pReq );

    static esp_err_t restStatisticsHtmlHandler( httpd_req_t * pReq );

    static esp_err_t restIncludeHeaderHtmlHandler( httpd_req_t * pReq );

    static esp_err_t restIncludeFooterHtmlHandler( httpd_req_t * pReq );

    static esp_err_t restIncludeDevOptionsHtmlHandler( httpd_req_t * pReq );

    static esp_err_t restIncludePopupHtmlHandler( httpd_req_t * pReq );

    static esp_err_t restIncludeNoJavascriptHtmlHandler( httpd_req_t * pReq );

    static esp_err_t restStyleCssHandler( httpd_req_t * const pReq );

    static esp_err_t restZeptoMinJsHandler( httpd_req_t * const pReq );

    static esp_err_t restAliveCheckJsHandler( httpd_req_t * pReq );

    static esp_err_t restPopupManagementJsHandler( httpd_req_t * pReq );

    static esp_err_t restFirmwareUpdateObserverJsHandler( httpd_req_t * pReq );

    static esp_err_t restCommonHelpersJsHandler( httpd_req_t * pReq );

    static esp_err_t restCustomSplashScreenHandler( httpd_req_t * const pReq );

    static esp_err_t restFaviconIcoHandler( httpd_req_t * const pReq );

    static esp_err_t restQrCodePngHandler( httpd_req_t * const pReq );

    static esp_err_t restLogoHandler( httpd_req_t * const pReq );

    static esp_err_t restEkxJpgHandler( httpd_req_t * pReq );

    static esp_err_t restWifiPngHandler( httpd_req_t * pReq );

    static esp_err_t restHomePngHandler( httpd_req_t * pReq );

    static bool existsUriKey( httpd_req_t * const pReq,
                              const std::string & strUriKey );

    static std::string getUriKeyValue( httpd_req_t * const pReq,
                                       const std::string & strUriKey );

    static std::string getUriKeyValue( const std::string & strParamString,
                                       const std::string & strUriKey );

    esp_err_t collectHttpPostData( httpd_req_t * const pReq,
                                   std::string &       strHttpPostData );

    esp_err_t sendChunkData( httpd_req_t * const pReq,
                             const char * const  pszHttpType,
                             const char * const  pszHttpFilename,
                             const bool          bGzipped,
                             const char * const  pData,
                             const int           nDataLen );

    void parseUriParams( httpd_req_t * const pReq );

    bool waitForAck( const uint32_t u32MsgCounter,
                     const uint32_t u32TimeoutMs );

    EkxProtocol::Payload::JsonMessage * waitForJsonMessage( const uint32_t u32MsgCounter,
                                                            const uint32_t u32TimeoutMs );

    EkxProtocol::Payload::FileDownloadStartResponse * waitForFileDownloadStartResponse( const uint32_t u32MsgCounter,
                                                                                        const uint32_t u32TimeoutMs );

    EkxProtocol::Payload::FileDownloadDataResponse * waitForFileDownloadDataResponse( const uint32_t u32MsgCounter,
                                                                                      const uint32_t u32TimeoutMs );

private:

    UcdHttpServerTask() = delete;

    void startWebserver( void );

    void stopWebserver( void );

    void openServerRequest( httpd_req_t * pRequest );

    void closeServerRequest( void );

    esp_err_t espDownloadHandler( httpd_req_t *       pReq,
                                  const std::string & strFileName );

    esp_err_t espUploadHandler( httpd_req_t * pReq );

    void serverBothPrintf( const esp_err_t nLogType,
                           const char *    pszFormat,
                           ... );

    void serverRemotePrintf( const char * pszFormat,
                             ... );

    void replaceString( std::string &       subject,
                        const std::string & search,
                        const std::string & replace ) const;

    void replaceDynamicCommonContents( std::string & strHtmlContent ) const;

    void replaceDynamicWifiContents( std::string & strHtmlContent ) const;

    void replaceFormInputTextValue( std::string &       strSubject,
                                    const std::string & strVariable,
                                    const int           nValue ) const;

    uint8_t fromHex( const int ch ) const;

    std::string urlDecode( const std::string & strText ) const;

private:

    httpd_handle_t                      m_handleWebserver { nullptr };

    httpd_req_t *                       m_pServerRequest { nullptr };

    char                                m_szPostDataBuffer[MAX_POST_DATA_BUF_LEN];

    std::string                         m_strCurrentGetSessionId;

    std::string                         m_strLoginErrorMsg;

    keyMap_t                            m_mapUriParams;

    AckMap                              m_ackMap;

    PayloadList                         m_listLinuxReplies;

    std::mutex                          m_mutexPayloadList;

    std::mutex                          m_mutexAckMap;

    UxEspCppLibrary::FreeRtosQueueTimer m_timerList;

    bool                                m_bDevOption { false };

};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#endif /* UcdHttpServerTask_h */
