/*!*************************************************************************************************************************************************************
 *
 * @file Ethernet.h
 * @brief Source file of component unit <title>.
 *
 * This file was developed as part of <component>.
 *
 * @author Ultratronik GmbH
 *         Dornierstr. 9
 *         D-82205 Gilching
 *         http://www.ultratronik.de
 * @author Srdjan Nikolic, Entwicklung - <optional according to the authors needs>
 *
 * @copyright Copyright 2020 by Ultratronik GmbH.
 *            All rights reserved.
 *            None of this file or parts of it may be
 *            copied, redistributed or used in any other way
 *            without written approval of Ultratronik GmbH.
 *
 **************************************************************************************************************************************************************/
#ifndef __COMPAPPLICATION_ETHERNET_CONTROLLER_H__
#define __COMPAPPLICATION_ETHERNET_CONTROLLER_H__

/***************************************************************************************************************************************************************
 *      Includes
 **************************************************************************************************************************************************************/
#include <stdint.h>
#include <string>

#include "esp_event_base.h"

#include "EspLog.h"
/***************************************************************************************************************************************************************
 *      Declarations
 **************************************************************************************************************************************************************/

class EthernetController : public UxEspCppLibrary::EspLog
{
public:

    EthernetController( const std::string & strName = "EthernetController");

    ~EthernetController() override;

	void init( void );

	enum LINK_STATE
	{
		UNINITIALIZED,
		STARTED,
		STOPPED,
		CONNECTED,
		DISCONNECTED,
		GOT_IP,
		ERROR
	};

	LINK_STATE getLinkState();

private:
	static void gotIpEventCallback( void*            arg,
								    esp_event_base_t event_base,
								    int32_t          event_id,
							  	    void*            event_data );

    static void ethEventCallback( void*            arg,
							 	  esp_event_base_t event_base,
								  int32_t          event_id,
								  void*            event_data );

private:

    static LINK_STATE m_linkState;

};

#endif /* __COMPAPPLICATION_ETHERNET_CONTROLLER_H__ */
