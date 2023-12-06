/*!*************************************************************************************************************************************************************
 *
 * @file SocketTask.cpp
 * @brief Source file of component unit Socket.
 *
 * This file was developed as part of SocketTask.
 *
 * @author Ultratronik GmbH
 *         Dornierstr. 9
 *         D-82205 Gilching
 *         http://www.ultratronik.de
 * @author Srdjan Nikolic, Entwicklung
 *
 * @copyright Copyright 2020 by Ultratronik GmbH.
 *            All rights reserved.
 *            None of this file or parts of it may be
 *            copied, redistributed or used in any other way
 *            without written approval of Ultratronik GmbH.
 *
 **************************************************************************************************************************************************************/

/***************************************************************************************************************************************************************
 *      Includes
 **************************************************************************************************************************************************************/

#include "SocketTask.h"

#include <string.h>
#include "ApplicationGlobals.h"
#include "EkxApplication.h"

#include "lwip/sockets.h"
#include "lwip/sys.h"


//#define DUMP_SOCKET_INPUT

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

SocketTask::SocketTask( UxEspCppLibrary::EspApplication*    pApplication,
                        const uint16_t                      speaker_port,
                        const uint16_t                      listener_port,
                        const std::string &                 strName )
: UxEspCppLibrary::FreeRtosTask( 4096,
                                 SOCKET_TASK_PRIORITY,
                                 strName )
{
    logInfo( "constructor" );

    m_currentSocketConfig.hostIp4 = inet_addr( SLIP_REMOTE_IP );
    m_currentSocketConfig.speaker_port = speaker_port;
    m_currentSocketConfig.listener_port = listener_port;

    m_state.state = SocketTaskTypes::state::UNINITIALIZED;
    m_state.lastError = 0;

    m_readCallback = nullptr;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

SocketTask::~SocketTask()
{
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void SocketTask::execute( void )
{
    vlogInfo( "execute()" );

    uint8_t tmpBuf[1024];

    while ( true )
    {
        setState( SocketTaskTypes::state::READING );

        int len = recvfrom( m_currentSocketConfig.listener_handler,
                            tmpBuf,
                            sizeof( tmpBuf ) - 1,
                            0, //MSG_DONTWAIT,
                            NULL,
                            NULL );

        //vlogInfo( "doStartReading: recvfrom %d ", len );
        if ( len != -1 )
        {
#ifdef DUMP_SOCKET_INPUT
            // skip alive message
            if ( len != 19 )
            {
                //vlogInfo( "doStartReading: recvfrom %d ", len );

                m_rxBuffer[len] = 0;
                printf( "%s\n", m_rxBuffer );
            }
#endif

            setState( SocketTaskTypes::state::CREATED );

            if ( m_readCallback )
            {
                m_readCallback( len,
                                tmpBuf );
            }
            else
            {
                vlogError( "execute: read call back not set" );
            }
        }
        else
        {
            vlogError( "execute: recvfrom return -1" );
        }
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

bool SocketTask::setState( const SocketTaskTypes::state state )
{
    bool bTransition = false;

    switch ( m_state.state )
    {
        case SocketTaskTypes::state::UNINITIALIZED:
        {
            if ( ( state == SocketTaskTypes::state::CREATED ) ||
                            ( state == SocketTaskTypes::state::ERROR ) )
            {
                bTransition = true;
            }
        }
        break;

        case SocketTaskTypes::state::CREATED:
        {
            if ( ( state == SocketTaskTypes::state::READING ) ||
                            ( state == SocketTaskTypes::state::ERROR ) )
            {
                bTransition = true;
            }
        }
        break;

        case SocketTaskTypes::state::READING:
        {
            if ( ( state == SocketTaskTypes::state::CREATED ) ||
                            ( state == SocketTaskTypes::state::ERROR ) )
            {
                bTransition = true;
            }
        }
        break;

        case SocketTaskTypes::state::ERROR:
        {
            if ( state == SocketTaskTypes::state::ERROR )
            {
                bTransition = true;
            }
        }
        break;

        default:
        {
            // do nothing
        }
        break;
    }

    if ( bTransition )
    {
        m_state.state = state;
    }
    else
    {
        logError( "State Transition Failed" );
    }

    return bTransition;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

SocketTaskTypes::state SocketTask::state( void ) const
{
    return m_state.state;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

SocketTask::SOC_ERR SocketTask::addReadHandler( std::function<void(int,void*)> callback )
{
    m_readCallback = callback;

    return SocketTask::SOC_ERR::OK;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

SocketTaskTypes::result SocketTask::sendData( const void*    pbuf,
                                              uint32_t size,
                                              int*     pBytesSent )
{
    //vlogInfo( "sendData - %d/%d", size, strlen( (char*)pbuf ) );
    SocketTaskTypes::result result = SocketTaskTypes::result::OK;

    if ( ( state() == SocketTaskTypes::state::CREATED )
                    || ( state() == SocketTaskTypes::state::READING ) )
    {
        struct sockaddr_in addr;

        memset( &addr,
                0,
                sizeof(addr) );

        addr.sin_len         = sizeof(addr);
        addr.sin_family      = AF_INET;
        addr.sin_port        = PP_HTONS( m_currentSocketConfig.speaker_port );
        addr.sin_addr.s_addr = m_currentSocketConfig.hostIp4;

        int err = sendto( m_currentSocketConfig.speaker_handler,
                          pbuf,
                          size,
                          0,
                          (struct sockaddr *)&addr,
                          sizeof(addr) );

        if ( err < 0 )
        {
            vlogError( "sendData failed" );
            result = SocketTaskTypes::result::SEND_DATA_FAILED;
        }
        else
        {
            //vlogInfo( "sendData Ok port %d", m_currentSocketConfig.speaker_port  );
        }

        if ( pBytesSent != NULL )
        {
            *pBytesSent = err;
        }
    }
    else
    {
        vlogError( "sendData failed, invalid state" );
        result = SocketTaskTypes::result::INVALID_STATE;
    }

    return result;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

uint16_t SocketTask::listenerPort( void ) const
{
    return m_currentSocketConfig.listener_port;
}
/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

uint16_t SocketTask::speakerPort( void ) const
{
    return m_currentSocketConfig.speaker_port;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

bool SocketTask::isSocketCreationFinished( void ) const
{
    bool bCreationFinished;

    switch ( m_state.state )
    {
        case SocketTaskTypes::state::CREATED: /* fall trough */
        case SocketTaskTypes::state::ERROR: /* fall trough */
        case SocketTaskTypes::state::READING: /* fall trough */
        {
            bCreationFinished = true;
        }
        break;

        case SocketTaskTypes::state::UNINITIALIZED: /* fall trough */
        default:
        {
            bCreationFinished = false;
        }
        break;
    }
    return bCreationFinished;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

SocketTask::SOC_ERR SocketTask::createSocket(  void )
{
    SocketTask::SOC_ERR result = SOC_ERR::FAIL;

    if ( state() == SocketTaskTypes::state::UNINITIALIZED )
    {
        m_currentSocketConfig.speaker_handler = socket( AF_INET,
                                                        SOCK_DGRAM,
                                                        IPPROTO_IP );
        {
            int reuse = 1;
            int ret = setsockopt( m_currentSocketConfig.speaker_handler,
                                  SOL_SOCKET,
                                  SO_REUSEADDR,
                                  &reuse,
                                  sizeof(reuse));
            if ( ret < 0 )
            {
                vlogError( "createSocket::setsockopt speaker_handler error %d", ret );
            }
        }

        m_currentSocketConfig.listener_handler = socket( AF_INET,
                                                         SOCK_DGRAM,
                                                         IPPROTO_IP );
        {
            int reuse = 1;
            int ret = setsockopt( m_currentSocketConfig.listener_handler,
                                  SOL_SOCKET,
                                  SO_REUSEADDR,
                                  &reuse,
                                  sizeof(reuse));
            if ( ret < 0 )
            {
                vlogError( "createSocket::setsockopt listener_handler error %d", ret );
            }
        }

        if ( m_currentSocketConfig.speaker_handler > 0 )
        {
            struct sockaddr_in addr;

            memset( &addr,
                    0,
                    sizeof(addr) );

            addr.sin_len         = sizeof(addr);
            addr.sin_family      = AF_INET;
            addr.sin_port        = PP_HTONS( m_currentSocketConfig.speaker_port );
            addr.sin_addr.s_addr = m_currentSocketConfig.hostIp4;

            int res = connect( m_currentSocketConfig.speaker_handler,
                               (struct sockaddr *) &addr,
                               sizeof(addr) );

            vlogInfo( "Socket speaker connect to %d (%d)", m_currentSocketConfig.speaker_port, res );

            if ( m_currentSocketConfig.listener_handler > 0 )
            {
                memset( &addr,
                        0,
                        sizeof(addr) );

                addr.sin_len         = sizeof(addr);
                addr.sin_family      = AF_INET;
                addr.sin_port        = PP_HTONS( m_currentSocketConfig.listener_port );
                addr.sin_addr.s_addr = INADDR_ANY;

                //int res = connect(m_currentSocketConfig.listener_handler, (struct sockaddr *)&addr, sizeof(addr));
                int res = bind( m_currentSocketConfig.listener_handler,
                                (struct sockaddr *) &addr,
                                sizeof(addr) );

                vlogInfo( "Socket listener bind to %d (%d)", m_currentSocketConfig.listener_port, res );

                result = SocketTask::SOC_ERR::OK;
                setState( SocketTaskTypes::state::CREATED );
                logInfo( "Socket Created" );

                createTask();
            }
            else
            {
                result = SocketTask::SOC_ERR::FAIL;
                setState( SocketTaskTypes::state::ERROR );
                logError( "Unable to create socket" );
            }
        }
        else
        {
            result = SocketTask::SOC_ERR::FAIL;
            setState( SocketTaskTypes::state::ERROR );
            logError( "Unable to create socket" );
        }
    }
    else
    {
        result = SocketTask::SOC_ERR::INVALID_STATE;
    }

    return result;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

