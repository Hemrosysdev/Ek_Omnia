///////////////////////////////////////////////////////////////////////////////
///
/// @file main.cpp
///
/// @brief main application entry point of EspConnector.
///
/// @author Ultratronik GmbH \n
///         Dornierstr. 9 \n
///         D-82205 Gilching \n
///         Germany
///         http://www.ultratronik.de
///
/// @author written by Gerd Esser, Forschung & Entwicklung, gesser@ultratronik.de
///
/// @date 27.02.2022
///
/// @copyright Copyright 2022 by Hemro International AG \n
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
#include <QTime>

#include <signal.h>

#include "EkxWsServer.h"
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
    cliParser.setApplicationDescription( "EkxWsServer - Web Socket server application for file download" );
    cliParser.addHelpOption();

    QCommandLineOption optDownloadDir( QStringList() << "d" << "directory",
                                       QCoreApplication::translate( "main", "target directory for downloads (default: /tmp)" ),
                                       QCoreApplication::translate( "main", "directory name" ) );
    cliParser.addOption( optDownloadDir );

    QCommandLineOption optPort( QStringList() << "p" << "port",
                                QCoreApplication::translate( "main", "WebSocket server port (default: 9988)" ),
                                QCoreApplication::translate( "main", "WebSocket server port" ),
                                QCoreApplication::translate( "main", "9988" ) );
    cliParser.addOption( optPort );

    QCommandLineOption optBlockSize( QStringList() << "b" << "block size",
                                     QCoreApplication::translate( "main", "TCP blockt size (default: 1000)" ),
                                     QCoreApplication::translate( "main", "TCP blockt size" ),
                                     QCoreApplication::translate( "main", "1000" ) );
    cliParser.addOption( optBlockSize );

    QCommandLineOption optMaxErrorRepeats( QStringList() << "r" << "repeats",
                                           QCoreApplication::translate( "main", "max. number of repeats per TCP packet (default: 100)" ),
                                           QCoreApplication::translate( "main", "number of repeats" ),
                                           QCoreApplication::translate( "main", "100" ) );
    cliParser.addOption( optMaxErrorRepeats );

    QCommandLineOption optDebugLevel( QStringList() << "D" << "debug",
                                      QCoreApplication::translate( "main", "debug level (default 0)" ),
                                      QCoreApplication::translate( "main", "level" ),
                                      "0" );
    cliParser.addOption( optDebugLevel );

    QCommandLineOption optUseTimestamps( QStringList() << "t" << "timestamps",
                                         QCoreApplication::translate( "main", "use timestamps for output" ) );
    cliParser.addOption( optUseTimestamps );

    int nReturn = -1;

    try
    {
        QCoreApplication theApp( argc, argv );

        catchUnixSignals( {SIGQUIT, SIGINT, SIGTERM, SIGHUP} );

        cliParser.process( theApp );
        const QStringList args = cliParser.positionalArguments();

        _nGlobalDebugLevel         = cliParser.value( optDebugLevel ).toInt();
        _bGlobalUseDebugTimestamps = cliParser.isSet( optUseTimestamps );

        QTextStream stream( stdout );
        stream << "EkxWsServer (c) 2022 by Hemro International AG" << endl;
        stream << "===================================================" << endl;
        stream << "Version " << VERSION_NO << " from " << VERSION_DATE << endl;
        stream << "Build " << BUILD_NO << " from " << BUILD_DATE << endl;
        stream << "Branch " << BRANCH << " SHA " << SHA << endl;
        stream << "===================================================" << endl;
        stream << "Press CTRL-C to quit application" << endl;

        EkxWsServer server;

        QString strDownloadDir   = cliParser.value( optDownloadDir );
        quint16 u16Port          = cliParser.value( optPort ).toUInt();
        int     nBlockSize       = cliParser.value( optBlockSize ).toInt();
        int     nMaxErrorRepeats = cliParser.value( optMaxErrorRepeats ).toInt();

        if ( server.create( u16Port, strDownloadDir, nBlockSize, nMaxErrorRepeats ) )
        {
            nReturn = theApp.exec();
        }
    }
    catch ( ... )
    {
        qCritical() << "Caught unspecified exception";
        nReturn = 0;
    }

    qInfo() << "Exit server!";

    return nReturn;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

