///////////////////////////////////////////////////////////////////////////////
///
/// @file main.cpp
///
/// @brief main application entry point of McuSimulator.
///
/// @author Ultratronik GmbH \n
///         Dornierstr. 9 \n
///         D-82205 Gilching \n
///         Germany
///         http://www.ultratronik.de
///
/// @author written by Gerd Esser, Forschung & Entwicklung, gesser@ultratronik.de
///
/// @date 19.08.2020
///
/// @copyright Copyright 2020 by Hemro International AG \n
///            Hemro International AG \n
///            Länggenstrasse 34 \n
///            CH 8184 Bachenbülach \n
///            Switzerland \n
///            Homepage: www.hemrogroup.com
///
///////////////////////////////////////////////////////////////////////////////

#include <QCoreApplication>
#include <QDebug>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QHostAddress>
#include <QTime>

#ifdef __linux__
#include <signal.h>
#endif

#include "McuSimulator.h"
#include "version.h"

static int  _nGlobalDebugLevel         = 0;
static bool _bGlobalUseDebugTimestamps = false;

#define DEBUG_COL_FATAL         "\033[0;31m"
#define DEBUG_COL_CRITICAL      "\033[0;31m"
#define DEBUG_COL_WARNING       "\033[0;33m"
#define DEBUG_COL_INFO          "\033[0;32m"
#define DEBUG_COL_DEBUG         ""
#define DEBUG_COL_OFF           "\033[0m"

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void messageHandler( QtMsgType                  type,
                     const QMessageLogContext & context,
                     const QString &            msg )
{
    QByteArray localMsg = msg.toLocal8Bit();

    const char * file     = context.file ? context.file : "";
    const char * function = context.function ? context.function : "";

    char szTimestamp[100];
    if ( _bGlobalUseDebugTimestamps )
    {
        sprintf( szTimestamp, "%s ", QTime::currentTime().toString( "hh:mm:ss.zzz" ).toLocal8Bit().constData() );
    }
    else
    {
        szTimestamp[0] = '\0';
    }

    switch ( type )
    {
        case QtDebugMsg:
            if ( _nGlobalDebugLevel >= 3 )
            {
                fprintf( stdout, DEBUG_COL_DEBUG "(D) %s%s\n" DEBUG_COL_OFF, szTimestamp, localMsg.constData() );
            }
            break;
        case QtInfoMsg:
            if ( _nGlobalDebugLevel >= 2 )
            {
                fprintf( stdout, DEBUG_COL_INFO "(I) %s%s\n" DEBUG_COL_OFF, szTimestamp, localMsg.constData() );
            }
            break;
        case QtWarningMsg:
            if ( _nGlobalDebugLevel >= 1 )
            {
                fprintf( stdout, DEBUG_COL_WARNING "(W) %s%s\n" DEBUG_COL_OFF, szTimestamp, localMsg.constData() );
            }
            break;
        case QtCriticalMsg:
            if ( _nGlobalDebugLevel >= 0 )
            {
                fprintf( stderr, DEBUG_COL_CRITICAL "(C) %s%s\n" DEBUG_COL_OFF, szTimestamp, localMsg.constData() );
            }
            break;
        case QtFatalMsg:
            if ( _nGlobalDebugLevel >= 0 )
            {
                fprintf( stderr, DEBUG_COL_FATAL "(F) %s%s (%s:%u, %s)\n" DEBUG_COL_OFF, szTimestamp, localMsg.constData(), file, context.line, function );
            }
            break;
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#ifdef __linux__
void catchUnixSignals( std::initializer_list<int> quitSignals )
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

int main( int    argc,
          char * argv[] )
{
    qInstallMessageHandler( messageHandler );

    //
    // define command line parser
    //
    QCommandLineParser cliParser;

    //
    cliParser.setApplicationDescription( "McuSimulator - Motor control unit simulator for Hemro EKX" );
    cliParser.addHelpOption();

    QCommandLineOption optSerialDevice( QStringList() << "d" << "device",
                                        QCoreApplication::translate( "main", "serial device name" ),
                                        QCoreApplication::translate( "main", "serial device" ) );
    cliParser.addOption( optSerialDevice );

    QCommandLineOption optDebugLevel( QStringList() << "D" << "debug level",
                                      QCoreApplication::translate( "main", "debug level (default 0)" ),
                                      QCoreApplication::translate( "main", "debug level" ),
                                      "0" );
    cliParser.addOption( optDebugLevel );

    QCommandLineOption optUseTimestamps( QStringList() << "t" << "use timestamps for output",
                                         QCoreApplication::translate( "main", "use timestamps" ) );
    cliParser.addOption( optUseTimestamps );

    int nReturn = -1;

    try
    {
        QCoreApplication theApp( argc, argv );

#ifdef __linux__
        catchUnixSignals( {SIGQUIT, SIGINT, SIGTERM, SIGHUP} );
#endif

        cliParser.process( theApp );
        const QStringList args = cliParser.positionalArguments();

        _nGlobalDebugLevel         = cliParser.value( optDebugLevel ).toInt();
        _bGlobalUseDebugTimestamps = cliParser.isSet( optUseTimestamps );

        QTextStream stream( stdout );
        stream << "McuSimulator (c) 2020 by Hemro International AG" << endl;
        stream << "================================================" << endl;
        stream << "Version " << VERSION_NO << " from " << VERSION_DATE << endl;
        stream << "Build " << BUILD_NO << " from " << BUILD_DATE << endl;
        stream << "Branch " << BRANCH << " SHA " << SHA << endl;
        stream << "================================================" << endl;
        stream << "Press CTRL-C to quit application" << endl;

        if ( !cliParser.isSet( optSerialDevice ) )
        {
            qCritical() << "no serial device defined";
            nReturn = -4;
        }
        else
        {
            McuSimulator simulator;

            if ( simulator.open( cliParser.value( optSerialDevice ) ) )
            {
                nReturn = theApp.exec();
            }
        }
    }
    catch ( ... )
    {
        qCritical() << "Caught unspecified exception!";
        nReturn = 0;
    }

    qInfo() << "Exit simulator!";

    return nReturn;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

