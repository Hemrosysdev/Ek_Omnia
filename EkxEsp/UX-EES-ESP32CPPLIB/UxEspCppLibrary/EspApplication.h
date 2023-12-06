///////////////////////////////////////////////////////////////////////////////
///
/// @file EspApplication.h
///
/// This file was developed as part of UX Extended Eco System Testframework (UX-EES-TSFW)
///
/// @brief Header file of class EspApplication.
///
/// @author Ultratronik GmbH
///         Dornierstr. 9
///         D-82205 Gilching
///         http://www.ultratronik.de
///
/// @author written by Gerd Esser, Research & Development, gesser@ultratronik.de
///
/// @date 13.12.2019
///
/// @copyright Copyright 2021 by Ultratronik GmbH.
///
/// This file and/or parts of it are subject to Ultratronik´s software license terms (SoLiT, Version 1.16.2).
/// With the use of this software you accept the SoLiT. Without written approval of Ultratronik GmbH this
/// software may not be copied, redistributed or used in any other way than stated in the conditions of the
/// SoLiT.
///
///////////////////////////////////////////////////////////////////////////////

#ifndef EspApplication_h
#define EspApplication_h

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#include "EspLog.h"

#include <string>
#include <vector>
#include <esp_err.h>

#include "FreeRtosStartupCtrlTask.h"

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

namespace UxEspCppLibrary
{

class FreeRtosQueueTask;

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class EspApplication : public EspLog
{

public:

    explicit EspApplication( const std::string & strName );

    ~EspApplication() override;

    void appMain( void );

    void printChipInfo( void );

    void registerTask( FreeRtosQueueTask * const pTask );

    FreeRtosStartupCtrlTask & startupCtrl( void );

    void processOtaRollback();

protected:

    virtual void createApp( void );

    void printSha256( const uint8_t * const pu8ImageHash,
                      const char * const    pszLabel );

    esp_err_t mountFatFs( const std::string & strFatFsBasePath,
                          const std::string & strFatFsLabel );

    void unmountFatFs( const std::string & strFatFsBasePath );

    esp_err_t mountSpiFfs( const std::string & strBasePath,
                           const std::string & strPartitionLabel );

    void unmountSpiFfs( const std::string & strPartitionLabel );

    esp_err_t initNvs( void );

    void initTcpIp( void );

private:

    ////////////////////////////////////////
    // private methods
    ////////////////////////////////////////

    EspApplication() = delete;

    void createTasks( void );

    void startInitSteps( void );

    ////////////////////////////////////////
    // private member variables
    ////////////////////////////////////////

    std::vector<FreeRtosQueueTask *> m_theTasks;

    FreeRtosStartupCtrlTask          m_theStartupCtrl;

};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

} // namespace UxEspCppLibrary

#endif /* EspApplication_h */
