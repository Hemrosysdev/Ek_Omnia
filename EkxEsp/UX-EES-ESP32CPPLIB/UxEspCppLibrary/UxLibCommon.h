///////////////////////////////////////////////////////////////////////////////
///
/// @file UxLibCommon.h
///
/// This file was developed as part of UX Extended Eco System Testframework (UX-EES-TSFW)
///
/// @brief Header file for common constants and types of package UxEspCppLibrary.
///
/// @author Ultratronik GmbH
///         Dornierstr. 9
///         D-82205 Gilching
///         http://www.ultratronik.de
///
/// @author written by Gerd Esser, Research & Development, gesser@ultratronik.de
///
/// @date 26.05.2021
///
/// @copyright Copyright 2021 by Ultratronik GmbH.
///
/// This file and/or parts of it are subject to Ultratronik´s software license terms (SoLiT, Version 1.16.2).
/// With the use of this software you accept the SoLiT. Without written approval of Ultratronik GmbH this
/// software may not be copied, redistributed or used in any other way than stated in the conditions of the
/// SoLiT.
///
///////////////////////////////////////////////////////////////////////////////

#ifndef UxLibCommon_h
#define UxLibCommon_h

#include <cstring>

#define ONE_MB_SIZE                     ( 1024 * 1024 )
#define STRTOL_DECIMAL_BASE             10
#define TASK_STACK_SIZE_STANDARD_4096   4096

#define TASK_PRIO_VERY_LOW              1

namespace UxEspCppLibrary
{

namespace UxLibCommon
{

inline void saveStringCopy( char * const       pDest,
                            const char * const pSrc,
                            const size_t       u32DestBufSize )
{
    *( pDest ) = '\0';
    strncat( pDest,
             pSrc,
             u32DestBufSize - 1 );
}

} // namespace UxLibCommon

} // namespace UxEspCppLibrary

#endif /* UxLibCommon_h */
