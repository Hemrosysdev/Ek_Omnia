///////////////////////////////////////////////////////////////////////////////
///
/// @file UnixSignalCatcher.cpp
///
/// @brief Main entry point of the application
///
/// @author Ultratronik GmbH
///         Dornierstr. 9
///         D-82205 Gilching
///         Germany
///         http://www.ultratronik.de
///
/// @author written by Gerd Esser, Forschung & Entwicklung, gesser@ultratronik.de
///
/// @date 05.04.2023
///
///////////////////////////////////////////////////////////////////////////////

#include "UnixSignalCatcher.h"

#include <QDebug>
#include <QCoreApplication>

#include <iostream>
#include <signal.h>

namespace UxEco
{

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#ifdef __linux__
// static
void UnixSignalCatcher::install( std::initializer_list<int> quitSignals )
{
    auto handler = []( int sig ) -> void
                   {
                       // blocking and not aysnc-signal-safe func are valid
                       qWarning() << "quit the application by signal " << sig;
                       QCoreApplication::quit();
                   };

    sigset_t blocking_mask;
    sigemptyset( &blocking_mask );

    for ( auto sig : quitSignals ) {
        sigaddset( &blocking_mask, sig );
    }

    struct sigaction sa;
    sa.sa_handler = handler;
    sa.sa_mask    = blocking_mask;
    sa.sa_flags   = 0;

    for ( auto sig : quitSignals ) {
        sigaction( sig, &sa, nullptr );
    }
}

#endif

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

} // namespace UxEco
