///////////////////////////////////////////////////////////////////////////////
///
/// @file FreeRtosTask.h
///
/// This file was developed as part of UX Extended Eco System Testframework (UX-EES-TSFW)
///
/// @brief Header file of class FreeRtosTask.
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

#ifndef FreeRtosTask_h
#define FreeRtosTask_h

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#include "EspLog.h"

#include <cstdint>
#include <string>

#include <freertos/FreeRTOS.h>
#include <freertos/portmacro.h>

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

namespace UxEspCppLibrary
{

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class FreeRtosTask : public EspLog
{
public:

    FreeRtosTask( const uint32_t      u32StackSize,
                  const UBaseType_t   uTaskPriority,
                  const std::string & strName );

    ~FreeRtosTask() override;

    virtual void create( void );

    const std::string & taskName( void ) const;

    const char * taskNameCstr( void ) const;

    void run( void );

    void createTask( void );

    void createTaskPinnedToCore( const int nCore );

    virtual void systemUp( void );

private:

    ////////////////////////////////////////
    // private methods
    ////////////////////////////////////////

    // standard constructor hidden
    FreeRtosTask() = delete;

    virtual void execute( void ) = 0;

    static void taskCallback( void * const pArg );

    ////////////////////////////////////////
    // private member variables
    ////////////////////////////////////////

    uint32_t    m_u32StackSize;

    UBaseType_t m_uPriority;

    std::string m_strName;

};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

} // namespace UxEspCppLibrary

#endif /* FreeRtosTask_h */
