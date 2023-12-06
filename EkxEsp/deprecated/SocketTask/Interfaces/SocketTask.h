/*!*************************************************************************************************************************************************************
 *
 * @file SocketTask.h
 * @brief Source file of component unit <title>.
 *
 * This file was developed as part of <component>.
 *
 * @author Ultratronik GmbH
 *         Dornierstr. 9
 *         D-82205 Gilching
 *         http://www.ultratronik.de
 * @author Srdjan Nikolic, Entwicklung - <optional according to the authors needs>
 *
 * @copyright Copyright 2020 by Ultratronik GmbH.
 *            All rights reserved.
 *            None of this file or parts of it may be
 *            copied, redistributed or used in any other way
 *            without written approval of Ultratronik GmbH.
 *
 **************************************************************************************************************************************************************/
#ifndef __COMPSOCKETTASK_SOCKETTASK_H__
#define __COMPSOCKETTASK_SOCKETTASK_H__

/***************************************************************************************************************************************************************
 *      Includes
 **************************************************************************************************************************************************************/
#include <stdbool.h>
#include <string>
#include <functional>

#include <esp_log.h>

#include "lwip/ip4_addr.h"

#include "FreeRtosQueueTask.h"
#include "EspLog.h"

/***************************************************************************************************************************************************************
 *
 **************************************************************************************************************************************************************/

namespace SocketQueueTypes
{
struct startReadingPayload;
}

namespace SocketTaskTypes
{
enum state
{
    UNINITIALIZED,
    CREATED,
    READING,
    ERROR
};

enum result
{
    OK,
    INVALID_STATE,
    SENDQ_FAILED,
    SEND_DATA_FAILED,
    FAIL
};
}

/***************************************************************************************************************************************************************
 *
 **************************************************************************************************************************************************************/

class SocketTask : public UxEspCppLibrary::FreeRtosTask
{

private:

    struct socketConfig_t
    {
        in_addr_t     hostIp4; // IP of the HOST with which communication is intended to be done
        uint16_t      speaker_port;  // ESP -> Linux
        uint16_t      listener_port; // Linux -> ESP
        int           speaker_handler;
        int           listener_handler;
    };

    struct state_t
    {
        SocketTaskTypes::state state;
        int                    lastError;
    };

public:

    enum SOC_ERR
    {
        OK,
        INVALID_STATE,
        SENDQ_FAILED,
        SEND_DATA_FAILED,
        FAIL
    };

public:

    SocketTask( UxEspCppLibrary::EspApplication *   pApplication,
                const uint16_t                      speaker_port,  // ESP -> Linux
                const uint16_t                      listener_port, // Linux -> ESP
                const std::string &                 strName = "SocketTask" );

    virtual ~SocketTask() override;

    SOC_ERR addReadHandler( std::function<void(int,void*)> callback );

    SOC_ERR createSocket( void );

    SocketTaskTypes::result sendData( const void*    pbuf,
                                      uint32_t size,
                                      int*     pBytesSent );

    uint16_t listenerPort( void ) const;
    uint16_t speakerPort( void ) const;

    bool isSocketCreationFinished( void ) const;

private:

    SocketTask();

    void doStartReading( const SocketQueueTypes::startReadingPayload * startReadingRef );

    void execute( void ) override;

    bool setState( const SocketTaskTypes::state state );

    SocketTaskTypes::state state( void ) const;

private:

    socketConfig_t                  m_currentSocketConfig;

    state_t                         m_state;

    std::function<void(int,void*)>  m_readCallback;

};

/***************************************************************************************************************************************************************
 *
 **************************************************************************************************************************************************************/

#endif /* __COMPSOCKETTASK_SOCKETTASK_H__ */
