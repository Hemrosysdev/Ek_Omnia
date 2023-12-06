/*!*************************************************************************************************************************************************************
 *
 * @file UcdStartStopBtnTask.h
 * @brief Source file of component unit UcdStartStopBtnTask.
 *
 * This file was developed as part of SCD Start-Stop Button
 *
 * @author Ultratronik GmbH
 *         Dornierstr. 9
 *         D-82205 Gilching
 *         http://www.ultratronik.de
 * @author Srdjan Nikolic, Entwicklung
 *
 * @copyright Copyright 2020 by Ultratronik GmbH.
 *            All rights reserved.
 *            None of this file or parts of it may be
 *            copied, redistributed or used in any other way
 *            without written approval of Ultratronik GmbH.
 *
 **************************************************************************************************************************************************************/

#ifndef UcdStartStopBtnTask_h
#define UcdStartStopBtnTask_h

/*!*************************************************************************************************************************************************************
 *
 **************************************************************************************************************************************************************/

#include "UcdDiscreteInTask.h"

/***************************************************************************************************************************************************************
 *      Declarations
 **************************************************************************************************************************************************************/

class UcdStartStopBtnTask : public UcdDiscreteInTask
{
public:

    UcdStartStopBtnTask( UxEspCppLibrary::EspApplication * pApplication );

    ~UcdStartStopBtnTask() override;

protected:

    int debounceCounter( void ) const override;

    uint64_t samplingPeriodUs( void ) const override;

};

#endif /* UcdStartStopBtnTask_h */
