///////////////////////////////////////////////////////////////////////////////
///
/// @file EspLog.h
///
/// This file was developed as part of UX Extended Eco System Testframework (UX-EES-TSFW)
///
/// @brief Header file of class EspLog.
///
/// @author Ultratronik GmbH
///         Dornierstr. 9
///         D-82205 Gilching
///         http://www.ultratronik.de
///
/// @author written by Gerd Esser, Research & Development, gesser@ultratronik.de
///
/// @date 24.10.2019
///
/// @copyright Copyright 2021 by Ultratronik GmbH.
///
/// This file and/or parts of it are subject to Ultratronik´s software license terms (SoLiT, Version 1.16.2).
/// With the use of this software you accept the SoLiT. Without written approval of Ultratronik GmbH this
/// software may not be copied, redistributed or used in any other way than stated in the conditions of the
/// SoLiT.
///
///////////////////////////////////////////////////////////////////////////////

#ifndef EspLog_h
#define EspLog_h

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

#include <string>

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

namespace UxEspCppLibrary
{

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

class EspLog
{
public:

    EspLog();
    explicit EspLog( const std::string & strLogTag );
    explicit EspLog( const EspLog & theRight );

    virtual ~EspLog();

    void setLogTag( const std::string & strLogTag );
    const std::string & logTag( void ) const;

    void vlogError( const char * pszFormat, ... ) const;
    void logError( const char * pszLog ) const;

    void vlogWarning( const char * pszFormat, ... ) const;
    void logWarning( const char * pszLog ) const;

    void vlogInfo( const char * pszFormat, ... ) const;
    void logInfo( const char * pszLog ) const;

private:

    std::string m_strLogTag;

};

/*!************************************************************************************************************************************************************
 *
 *************************************************************************************************************************************************************/

} // namespace UxEspCppLibrary

#endif /* EspLog_h */
