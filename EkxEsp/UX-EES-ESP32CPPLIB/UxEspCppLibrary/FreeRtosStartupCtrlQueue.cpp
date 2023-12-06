///////////////////////////////////////////////////////////////////////////////
///
/// @file FreeRtosStartupCtrlQueue.cpp
///
/// This file was developed as part of UX Extended Eco System Testframework (UX-EES-TSFW)
///
/// @brief Implementation file of class FreeRtosStartupCtrlQueue.
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

#include "FreeRtosStartupCtrlQueue.h"

#define FREE_RTOS_STARTUP_CTRL_QUEUE_LEN  10

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

namespace UxEspCppLibrary
{

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

FreeRtosStartupCtrlQueue::FreeRtosStartupCtrlQueue()
    : FreeRtosQueue( FREE_RTOS_STARTUP_CTRL_QUEUE_LEN,
                     sizeof( Message ),
                     "FreeRtosStartupCtrlQueue" )
{
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

FreeRtosStartupCtrlQueue::~FreeRtosStartupCtrlQueue()
{
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

BaseType_t FreeRtosStartupCtrlQueue::sendBeginStartup( void )
{
    return sendSimpleEvent( BeginStartup );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

BaseType_t FreeRtosStartupCtrlQueue::sendContinueStartup( FreeRtosQueueTask * const              pStartupTask,
                                                          const FreeRtosQueueTask::startupStep_t nStartupStep )
{
    Message message;

    message.type = ContinueStartup;

    message.payload.continueStartupPayload.m_pStartupTask = pStartupTask;
    message.payload.continueStartupPayload.m_nStartupStep = nStartupStep;

    return send( &message );
}

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

} // namespace UxEspCppLibrary

