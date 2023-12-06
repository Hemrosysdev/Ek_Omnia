///////////////////////////////////////////////////////////////////////////////
///
/// @file FreeRtosStartupCtrlTask.cpp
///
/// This file was developed as part of UX Extended Eco System Testframework (UX-EES-TSFW)
///
/// @brief Implementation file of class FreeRtosStartupCtrlTask.
///
/// @author Ultratronik GmbH
///         Dornierstr. 9
///         D-82205 Gilching
///         http://www.ultratronik.de
///
/// @author written by Gerd Esser, Research & Development, gesser@ultratronik.de
///
/// @date 29.12.2019
///
/// @copyright Copyright 2021 by Ultratronik GmbH.
///
/// This file and/or parts of it are subject to Ultratronik´s software license terms (SoLiT, Version 1.16.2).
/// With the use of this software you accept the SoLiT. Without written approval of Ultratronik GmbH this
/// software may not be copied, redistributed or used in any other way than stated in the conditions of the
/// SoLiT.
///
///////////////////////////////////////////////////////////////////////////////

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#include "FreeRtosStartupCtrlTask.h"
#include "FreeRtosStartupCtrlQueue.h"
#include "EspApplication.h"
#include "UxLibCommon.h"

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

namespace UxEspCppLibrary
{

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

FreeRtosStartupCtrlTask::FreeRtosStartupCtrlTask( EspApplication * const pApplication )
    : FreeRtosQueueTask( pApplication,
                         4096,
                         1, // lowest priority
                         "FreeRtosStartupCtrlTask",
                         new FreeRtosStartupCtrlQueue() )
    , m_nCurrentStartupStep( FreeRtosQueueTask::startupStep_t::startupStepInit )
{
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

FreeRtosStartupCtrlTask::~FreeRtosStartupCtrlTask()
{
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

FreeRtosStartupCtrlQueue * FreeRtosStartupCtrlTask::freeRtosStartupCtrlQueue( void )
{
    return dynamic_cast<FreeRtosStartupCtrlQueue *>( taskQueue() );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

const FreeRtosStartupCtrlQueue * FreeRtosStartupCtrlTask::freeRtosStartupCtrlQueue( void ) const
{
    return dynamic_cast<const FreeRtosStartupCtrlQueue *>( taskQueue() );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void FreeRtosStartupCtrlTask::registerStartupTask( FreeRtosQueueTask * const pStartupTask )
{
    m_theStartupSteps.insert( std::pair<FreeRtosQueueTask *, FreeRtosQueueTask::startupStep_t>( pStartupTask, FreeRtosQueueTask::startupStep_t::startupStepInit ) );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void FreeRtosStartupCtrlTask::processTaskMessage( void )
{
    const FreeRtosStartupCtrlQueue::Message * pMessage = reinterpret_cast<const FreeRtosStartupCtrlQueue::Message *>( receiveMsg() );

    if ( pMessage != nullptr )
    {
        switch ( pMessage->type )
        {
            case FreeRtosStartupCtrlQueue::BeginStartup:
            {
                m_nCurrentStartupStep = FreeRtosQueueTask::startupStep_t::startupStep1;

                distributeStartupStep();
            }
            break;

            case FreeRtosStartupCtrlQueue::ContinueStartup:
            {
                processContinueStartup( pMessage->payload.continueStartupPayload.m_pStartupTask,
                                        pMessage->payload.continueStartupPayload.m_nStartupStep );
            }
            break;

            default:
            {
                vlogError( "processTaskMessage: receive illegal message type %d",
                           pMessage->type );
            }
            break;
        }
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void FreeRtosStartupCtrlTask::processContinueStartup( FreeRtosQueueTask * const              pStartupTask,
                                                      const FreeRtosQueueTask::startupStep_t nStartupStep )
{
    if ( m_theStartupSteps.find( pStartupTask ) == m_theStartupSteps.end() )
    {
        vlogError( "processContinueStartup: task %s not known to startup ctrl", pStartupTask->taskName().c_str() );
    }
    else
    {
        // update given step in list
        if ( nStartupStep != m_nCurrentStartupStep )
        {
            vlogError( "processContinueStartup: task %s, step is out of order (1) %d", pStartupTask->taskName().c_str(), nStartupStep );
        }
        else if ( m_theStartupSteps[pStartupTask] >= m_nCurrentStartupStep )
        {
            vlogError( "processContinueStartup: task %s, step is out of order (2) %d", pStartupTask->taskName().c_str(), nStartupStep );
        }
        else
        {
            m_theStartupSteps[pStartupTask] = nStartupStep;
        }

        // search list, if all steps are on current level -> step next level
        bool                 bContinueStartup = true;
        mapSteps_t::iterator i;
        for ( i = m_theStartupSteps.begin(); i != m_theStartupSteps.end(); i++ )
        {
            if ( i->second != m_nCurrentStartupStep )
            {
                bContinueStartup = false;
                break;
            }
        }

        // increment startup step and distribute messages
        if ( bContinueStartup )
        {
            if ( m_nCurrentStartupStep < FreeRtosQueueTask::startupStep_t::startupStepMemoryCheck )
            {
                m_nCurrentStartupStep = static_cast<FreeRtosQueueTask::startupStep_t>( static_cast<int>( m_nCurrentStartupStep ) + 1 );

                distributeStartupStep();
            }
        }
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

void FreeRtosStartupCtrlTask::distributeStartupStep( void )
{
    vlogWarning( "distributeStartupStep %d", m_nCurrentStartupStep );

    mapSteps_t::iterator i;
    for ( i = m_theStartupSteps.begin(); i != m_theStartupSteps.end(); i++ )
    {
        i->first->sendStartupStepFromCtrl( m_nCurrentStartupStep );
    }

    if ( ( m_nCurrentStartupStep == FreeRtosQueueTask::startupStep_t::startupStepFinal )
         && app() )
    {
        app()->processOtaRollback();
    }
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

} // namespace UxEspCppLibrary

