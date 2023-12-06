///////////////////////////////////////////////////////////////////////////////
///
/// @file FreeRtosQueueTask.cpp
///
/// This file was developed as part of UX Extended Eco System Testframework (UX-EES-TSFW)
///
/// @brief Implementation file of class FreeRtosQueueTask.
///
/// @author Ultratronik GmbH
///         Dornierstr. 9
///         D-82205 Gilching
///         http://www.ultratronik.de
///
/// @author written by Gerd Esser, Research & Development, gesser@ultratronik.de
///
/// @date 21.10.2019
///
/// @copyright Copyright 2021 by Ultratronik GmbH.
///
/// This file and/or parts of it are subject to Ultratronik´s software license terms (SoLiT, Version 1.16.2).
/// With the use of this software you accept the SoLiT. Without written approval of Ultratronik GmbH this
/// software may not be copied, redistributed or used in any other way than stated in the conditions of the
/// SoLiT.
///
///////////////////////////////////////////////////////////////////////////////

#include "FreeRtosQueueTask.h"

#include <esp_log.h>

#include "FreeRtosQueue.h"
#include "EspApplication.h"
#include "FreeRtosStartupCtrlTask.h"
#include "FreeRtosStartupCtrlQueue.h"

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

namespace UxEspCppLibrary
{

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

FreeRtosQueueTask::FreeRtosQueueTask( EspApplication * const pApplication,
                                      const uint32_t         u32StackSize,
                                      const UBaseType_t      uTaskPriority,
                                      const std::string &    strName,
                                      const UBaseType_t      uQueueLength,
                                      const UBaseType_t      uQueueItemSize,
                                      const TickType_t       xReceiveTicksToWait )
    : FreeRtosTask( u32StackSize, uTaskPriority, strName )
    , m_pApplication( pApplication )
    , m_pTaskQueue( new FreeRtosQueue( uQueueLength, uQueueItemSize, strName ) )
    , m_pu8ReceiveMsg( new uint8_t[uQueueItemSize] )
    , m_xReceiveTicksToWait( xReceiveTicksToWait )
{
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

FreeRtosQueueTask::FreeRtosQueueTask( EspApplication * const pApplication,
                                      const uint32_t         u32StackSize,
                                      const UBaseType_t      uTaskPriority,
                                      const std::string &    strName,
                                      FreeRtosQueue *        pTaskQueue,
                                      const TickType_t       xReceiveTicksToWait )
    : FreeRtosTask( u32StackSize, uTaskPriority, strName )
    , m_pApplication( pApplication )
    , m_pTaskQueue( pTaskQueue )
    , m_pu8ReceiveMsg( new uint8_t[pTaskQueue->queueItemSize()] )
    , m_xReceiveTicksToWait( xReceiveTicksToWait )
{
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

FreeRtosQueueTask::~FreeRtosQueueTask()
{
    delete m_pTaskQueue;
    m_pTaskQueue = nullptr;

    delete [] m_pu8ReceiveMsg;
    m_pu8ReceiveMsg = nullptr;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

FreeRtosQueue * FreeRtosQueueTask::taskQueue()
{
    return m_pTaskQueue;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

const FreeRtosQueue * FreeRtosQueueTask::taskQueue() const
{
    return m_pTaskQueue;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void FreeRtosQueueTask::executeInit( void )
{
    // do nothing, to be done in overloaded function
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

const uint8_t * FreeRtosQueueTask::receiveMsg( void ) const
{
    return m_pu8ReceiveMsg;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void FreeRtosQueueTask::execute( void )
{
    executeInit();

    while ( true )
    {
        BaseType_t xReceiveReturn = m_pTaskQueue->receive( m_pu8ReceiveMsg,
                                                           m_xReceiveTicksToWait );

        if ( xReceiveReturn == pdTRUE )
        {
            if ( !processStartupStep() )
            {
                processTaskMessage();
            }
        }
        else
        {
            vlogError( taskNameCstr(),
                       "execute: xQueueReceive receive failed %d",
                       xReceiveReturn );
        }
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

EspApplication * FreeRtosQueueTask::app( void )
{
    return m_pApplication;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

const EspApplication * FreeRtosQueueTask::app( void ) const
{
    return m_pApplication;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

bool FreeRtosQueueTask::processStartupStep( void )
{
    bool bProcessed = false;

    if ( m_pApplication != nullptr )
    {
        const Message * pMessage = reinterpret_cast<const Message *>( receiveMsg() );

        if ( pMessage != nullptr )
        {
            switch ( pMessage->m_nType )
            {
                case StartupStep:
                {
                    if ( startupStep( pMessage->m_nStartupStep ) )
                    {
                        continueStartupStep( pMessage->m_nStartupStep );
                    }

                    bProcessed = true;
                }
                break;

                default:
                {
                    // this is any other message for this task
                }
                break;
            }
        }
    }

    return bProcessed;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

bool FreeRtosQueueTask::startupStep( const FreeRtosQueueTask::startupStep_t nStartupStep )
{
    ( void ) nStartupStep;    // use unused parameter

    // this is default behavior -> do nothing

    return true;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void FreeRtosQueueTask::continueStartupStep( const FreeRtosQueueTask::startupStep_t nStartupStep )
{
    const BaseType_t xReturn = m_pApplication->startupCtrl().freeRtosStartupCtrlQueue()->sendContinueStartup( this, nStartupStep );

    if ( xReturn != pdTRUE )
    {
        vlogError( "continueStartupStep: can't send msg, error %d", xReturn );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void FreeRtosQueueTask::sendStartupStepFromCtrl( const FreeRtosQueueTask::startupStep_t nStartupStep )
{
    Message message;

    message.m_nType        = StartupStep;
    message.m_nStartupStep = nStartupStep;

    BaseType_t xReturn = taskQueue()->send( &message );

    if ( xReturn != pdTRUE )
    {
        vlogError( "sendStartupStepFromCtrl: can't send msg, error %d", xReturn );
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

// static
size_t FreeRtosQueueTask::minTargetQueueSize( const size_t nNativeQueueSize )
{
    size_t u32Size = nNativeQueueSize;

    if ( nNativeQueueSize < sizeof( Message ) )
    {
        u32Size = sizeof( Message );
    }

    return u32Size;
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

} // namespace UxEspCppLibrary

