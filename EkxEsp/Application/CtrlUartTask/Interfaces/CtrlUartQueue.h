/*!*****************************************************************************
*
* @file CtrlUartQueue.h
*
* @brief Header file for class CtrlUartQueue.
*
* @author Ultratronik GmbH
*         Dornierstr. 9
*         D-82205 Gilching
*         www.ultratronik.de
*
* @author Gerd Esser, Forschung & Entwicklung
*
* @created 27.05.2021
*
* @copyright Copyright (C) Bircher Smart Access, CH-8222 Beringen, 2021
*            All rights reserved.
*            None of this file or parts of it may be
*            copied, redistributed or used in any other way
*            without written approval of Bircher BSA.
*
* Information from SVN:
* revision of last commit: $Rev:  $
* date of last commit: $Date: $
* author of last commit: $Author: $
*
*******************************************************************************/

#ifndef CtrlUartQueue_h
#define CtrlUartQueue_h

/*##***********************************************************************************************************************************************************
 *      Includes
 *************************************************************************************************************************************************************/

#include "FreeRtosQueue.h"
#include "EkxProtocol.h"

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class UartControlledDevice;

namespace CtrlUart
{

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class CtrlUartQueue : public UxEspCppLibrary::FreeRtosQueue
{
private:

    friend class CtrlUartTask;

    static const int c_nCtrlUartQueueLen { 100 };

public:

    /*!*****************************************************************************************************************************************
     * @brief Constructor
     ******************************************************************************************************************************************/
    CtrlUartQueue();

    /*!*****************************************************************************************************************************************
     * @brief Destructor
     ******************************************************************************************************************************************/
    ~CtrlUartQueue() override;

    BaseType_t sendMessagePayload( const EkxProtocol::DriverId                  u8DriverId,
                                   const uint32_t                               u32MsgCounter,
                                   const uint8_t                                u8RepeatCounter,
                                   EkxProtocol::Payload::MessagePayload * const pMessagePayload );

    BaseType_t sendRegisterDriver( UartControlledDevice * const pDriver );

};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

} /* namespace CtrlUart */

#endif /* CtrlUartQueue_h */
