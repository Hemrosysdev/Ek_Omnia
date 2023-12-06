/*
 * EkxApplication.h
 *
 *  Created on: 24.10.2019
 *      Author: gesser
 */

#ifndef EkxApplication_H
#define EkxApplication_H

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#include "UcdDcHallMotorTask.h"
#include "UcdDiscreteInTask.h"
#include "UcdEepromTask.h"
#include "UcdStartStopBtnTask.h"
#include "UcdStepperMotorTask.h"
#include "UcdSwUpdateTask.h"
#include "UcdTempSensorTask.h"
#include "UcdMcuUartTask.h"
#include "UcdWifiConnectorTask.h"
#include "UcdSpotLightTask.h"
#include "UcdStartStopLightTask.h"

#include "UcdHttpServerTask.h"

#include "EspApplication.h"
#include "CtrlUartTask.h"

#include "PluginTaskAt24c16.h"
#include "PluginTaskWifiConnector.h"

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class EkxApplication : public UxEspCppLibrary::EspApplication
{
public:

    ~EkxApplication() override;

    static EkxApplication * createSingleton();

    static EkxApplication * singleton();

    static void deleteSingleton();

    UcdWifiConnectorTask & wifiConnectorTask();

    UxEspCppLibrary::PluginTaskAt24c16 & pluginTaskAt24c16();

    UxEspCppLibrary::PluginTaskWifiConnector & pluginTaskWifiConnector();

    UcdStartStopBtnTask & startStopBtnTask();

    UcdStepperMotorTask & agsaStepperMotorTask();

    UcdDcHallMotorTask & pduDcHallMotorTask();

    UcdStartStopLightTask & startStopLightTask();

    UcdMcuUartTask & mcuUartTask();

    UcdSpotLightTask & spotLightTask();

    UcdEepromTask & eepromTask();

    UcdTempSensorTask & tempSensorTask();

    CtrlUart::CtrlUartTask & ctrlUartTask();

    UcdHttpServerTask & httpServerTask();

private:

    EkxApplication();

    void createApp();

private:

    static EkxApplication *                  m_pSingleton;

    UcdWifiConnectorTask                     m_wifiConnectorTask;

    UxEspCppLibrary::PluginTaskAt24c16       m_pluginTaskAt24c16;

    UxEspCppLibrary::PluginTaskWifiConnector m_pluginTaskWifiConnector;

    UcdStartStopBtnTask                      m_ucdStartStopBtnTask;

    UcdStartStopLightTask                    m_scdStartStopLightTask;

    UcdMcuUartTask                           m_mcuUartTask;

    UcdSpotLightTask                         m_ucdSpotLightTask;

    UcdEepromTask                            m_ucdEepromTask;

    UcdTempSensorTask                        m_ucdTempSensorTask;

    UcdSwUpdateTask                          m_swUpdateTask;

    UcdStepperMotorTask                      m_agsaStepperMotorTask;

    UcdDcHallMotorTask                       m_pduDcHallMotorTask;

    CtrlUart::CtrlUartTask                   m_ctrlUartTask;

    UcdHttpServerTask                        m_ucdHttpServerTask;

};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

extern EkxApplication & ekxApp();

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#endif /* EkxApplication_H */
