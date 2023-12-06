/*
 * PluginTaskAt24c16.h
 *
 *  Created on: 27.04.2020
 *      Author: gesser
 */

#ifndef PluginTaskAt24c16_h
#define PluginTaskAt24c16_h

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#include "PluginTask.h"

#include <driver/i2c.h>

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

namespace UxEspCppLibrary
{

class EspApplication;
class I2cAt24c16;
class PluginTaskAt24c16Distributor;
class PluginTaskAt24c16Queue;

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class PluginTaskAt24c16 : public PluginTask
{
public:

    PluginTaskAt24c16( EspApplication * const pApplication,
                       const int              nTaskPriority,
                       const i2c_port_t       nI2cMasterPort );

    ~PluginTaskAt24c16() override;

    bool startupStep( const FreeRtosQueueTask::startupStep_t nStartupStep ) override;

    void processTaskMessage( void ) override;

    PluginTaskAt24c16Queue * pluginTaskAt24c16Queue( void );

    const PluginTaskAt24c16Queue * pluginTaskAt24c16Queue( void ) const;

    PluginTaskAt24c16Distributor * pluginTaskAt24c16Distributor( void );

    const PluginTaskAt24c16Distributor * pluginTaskAt24c16Distributor( void ) const;

private:

    PluginTaskAt24c16() = delete;

private:

    I2cAt24c16 * m_pI2cAt24c16 { nullptr };

};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

} // namespace UxEspCppLibrary

#endif /* PluginTaskAt24c16_h */
