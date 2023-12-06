///////////////////////////////////////////////////////////////////////////////
///
/// @file FreeRtosStartupCtrlQueue.h
///
/// This file was developed as part of UX Extended Eco System Testframework (UX-EES-TSFW)
///
/// @brief Header file of class FreeRtosStartupCtrlQueue.
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

#ifndef FreeRtosStartupCtrlQueue_h
#define FreeRtosStartupCtrlQueue_h

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#include "FreeRtosQueueTask.h"
#include "FreeRtosQueue.h"

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

namespace UxEspCppLibrary
{

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class FreeRtosStartupCtrlQueue : public FreeRtosQueue
{
    friend class FreeRtosStartupCtrlTask;

private:

    typedef enum
    {
        BeginStartup = 0,
        ContinueStartup,
    } MessageType;

    typedef struct
    {
        FreeRtosQueueTask *               m_pStartupTask;
        FreeRtosQueueTask::startupStep_t  m_nStartupStep;
    } continueStartupPayload_t;

    typedef union
    {
        continueStartupPayload_t    continueStartupPayload;
    } MessagePayload;

    typedef struct
    {
        MessageType    type; //!<  it has to make sure, that this attribute takes 32bit, as it is mapped to a general uint32 attribute.
        MessagePayload payload; //!< generic payload
    } Message;

public:

    FreeRtosStartupCtrlQueue();

    ~FreeRtosStartupCtrlQueue() override;

    BaseType_t sendBeginStartup( void );

    BaseType_t sendContinueStartup( FreeRtosQueueTask * const pStartupTask,
                                    const FreeRtosQueueTask::startupStep_t nStartupStep );

};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

} // namespace UxEspCppLibrary

#endif /* FreeRtosStartupCtrlQueue_h */
