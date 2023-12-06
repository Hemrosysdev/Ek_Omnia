///////////////////////////////////////////////////////////////////////////////
///
/// @file FreeRtosQueueTask.h
///
/// This file was developed as part of UX Extended Eco System Testframework (UX-EES-TSFW)
///
/// @brief Header file of class FreeRtosQueueTask.
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

#ifndef FreeRtosQueueTask_h
#define FreeRtosQueueTask_h

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#include <cstdint>

#include "FreeRtosTask.h"

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

namespace UxEspCppLibrary
{

class EspApplication;
class FreeRtosQueue;

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class FreeRtosQueueTask : public FreeRtosTask
{
public:

    ////////////////////////////////////////
    // public enum definitions
    ////////////////////////////////////////

    enum class startupStep_t : uint8_t
    {
        startupStepInit = 0,
        startupStep1,
        startupStep2,
        startupStep3,
        startupStep4,
        startupStep5,
        startupStep6,
        startupStep7,
        startupStep8,
        startupStep9,
        startupStepFinal,
        startupStepMemoryCheck
    };

    ////////////////////////////////////////
    // public methods
    ////////////////////////////////////////

    FreeRtosQueueTask( EspApplication * const pApplication,
                       const uint32_t         u32StackSize,
                       const UBaseType_t      uTaskPriority,
                       const std::string &    strName,
                       const UBaseType_t      uQueueLength,
                       const UBaseType_t      uQueueItemSize,
                       const TickType_t       xReceiveTicksToWait = portMAX_DELAY );

    FreeRtosQueueTask( EspApplication * const pApplication,
                       const uint32_t         u32StackSize,
                       const UBaseType_t      uTaskPriority,
                       const std::string &    strName,
                       FreeRtosQueue * const  pTaskQueue,
                       const TickType_t       xReceiveTicksToWait = portMAX_DELAY );

    ~FreeRtosQueueTask() override;

    FreeRtosQueue * taskQueue();

    const FreeRtosQueue * taskQueue() const;

    const uint8_t * receiveMsg( void ) const;

    void continueStartupStep( const startupStep_t nStartupStep );

    void sendStartupStepFromCtrl( const startupStep_t nStartupStep );

    EspApplication * app( void );

    const EspApplication * app( void ) const;

    static size_t minTargetQueueSize( const size_t nNativeQueueSize );

protected:

    virtual void executeInit( void );

    virtual void processTaskMessage( void ) = 0;

    virtual bool startupStep( const startupStep_t nStartupStep );

private:

    ////////////////////////////////////////
    // private methods
    ////////////////////////////////////////

    // standard constructor hidden
    FreeRtosQueueTask() = delete;

    void execute( void ) override;

    bool processStartupStep( void );

private:

    typedef enum
    {
        StartupStep = 0xaaaaaaaaUL,
    } MessageType;

    typedef struct
    {
        MessageType   m_nType;
        startupStep_t m_nStartupStep;
    } Message;

    ////////////////////////////////////////
    // private member variables
    ////////////////////////////////////////

    EspApplication * m_pApplication { nullptr };

    FreeRtosQueue *  m_pTaskQueue { nullptr };

    uint8_t *        m_pu8ReceiveMsg { nullptr };

    TickType_t       m_xReceiveTicksToWait;

};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

} // namespace UxEspCppLibrary

#endif /* FreeRtosQueueTask_h */
