///////////////////////////////////////////////////////////////////////////////
///
/// @file FreeRtosStartupCtrlTask.h
///
/// This file was developed as part of UX Extended Eco System Testframework (UX-EES-TSFW)
///
/// @brief Header file of class FreeRtosStartupCtrlTask.
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

#ifndef FreeRtosStartupCtrlTask_h
#define FreeRtosStartupCtrlTask_h

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#include "FreeRtosQueueTask.h"

#include <vector>
#include <map>

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

namespace UxEspCppLibrary
{

class FreeRtosStartupCtrlQueue;

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class FreeRtosStartupCtrlTask : public FreeRtosQueueTask
{

public:

    explicit FreeRtosStartupCtrlTask( EspApplication * const pApplication );

    ~FreeRtosStartupCtrlTask() override;

    void registerStartupTask( FreeRtosQueueTask * const pStartupTask );

    void processTaskMessage( void ) override;

    FreeRtosStartupCtrlQueue * freeRtosStartupCtrlQueue( void );

    const FreeRtosStartupCtrlQueue * freeRtosStartupCtrlQueue( void ) const;

private:

    ////////////////////////////////////////
    // private methods
    ////////////////////////////////////////

    FreeRtosStartupCtrlTask() = delete;

    void processContinueStartup( FreeRtosQueueTask * const              pStartupTask,
                                 const FreeRtosQueueTask::startupStep_t nStartupStep );

    void distributeStartupStep( void );

    ////////////////////////////////////////
    // private type definitions
    ////////////////////////////////////////

    FreeRtosQueueTask::startupStep_t m_nCurrentStartupStep;

    typedef std::map<FreeRtosQueueTask *, FreeRtosQueueTask::startupStep_t>  mapSteps_t;
    typedef std::pair<FreeRtosQueueTask *, FreeRtosQueueTask::startupStep_t>  pairSteps_t;

    mapSteps_t m_theStartupSteps;

};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

} // namespace UxEspCppLibrary

#endif /* FreeRtosStartupCtrlTask_h */
