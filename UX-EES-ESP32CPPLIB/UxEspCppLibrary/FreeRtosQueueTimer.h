///////////////////////////////////////////////////////////////////////////////
///
/// @file FreeRtosQueueTimer.h
///
/// This file was developed as part of UX Extended Eco System Testframework (UX-EES-TSFW)
///
/// @brief Header file of class FreeRtosQueueTimer.
///
/// @author Ultratronik GmbH
///         Dornierstr. 9
///         D-82205 Gilching
///         http://www.ultratronik.de
///
/// @author written by Gerd Esser, Research & Development, gesser@ultratronik.de
///
/// @date 22.10.2019
///
/// @copyright Copyright 2021 by Ultratronik GmbH.
///
/// This file and/or parts of it are subject to Ultratronik´s software license terms (SoLiT, Version 1.16.2).
/// With the use of this software you accept the SoLiT. Without written approval of Ultratronik GmbH this
/// software may not be copied, redistributed or used in any other way than stated in the conditions of the
/// SoLiT.
///
///////////////////////////////////////////////////////////////////////////////

#ifndef FreeRtosQueueTimer_h
#define FreeRtosQueueTimer_h

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#include "FreeRtosTimer.h"

#include <cstdint>

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

namespace UxEspCppLibrary
{

class FreeRtosQueue;

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class FreeRtosQueueTimer : public FreeRtosTimer
{   
public:

    FreeRtosQueueTimer( FreeRtosQueue * pQueue,
                        const uint32_t u32MessageType );

    ~FreeRtosQueueTimer() override;

    uint32_t messageType() const;

private:

    ////////////////////////////////////////
    // private methods
    ////////////////////////////////////////

    FreeRtosQueueTimer() = delete;

    void processTimeout( void ) override;

    ////////////////////////////////////////
    // private member variables
    ////////////////////////////////////////

    FreeRtosQueue * m_pQueue;

    uint32_t m_u32MessageType;

};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

} // namespace UxEspCppLibrary

#endif /* FreeRtosQueueTimer_h */
