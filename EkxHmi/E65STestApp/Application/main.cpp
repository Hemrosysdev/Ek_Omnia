///////////////////////////////////////////////////////////////////////////////
///
/// @file main.cpp
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
/// @date 31.03.2021
///
/// @copyright Copyright 2021 by Hemro International AG
///            Hemro International AG
///            Länggenstrasse 34
///            CH 8184 Bachenbülach
///            Switzerland
///            Homepage: www.hemrogroup.com
///
///////////////////////////////////////////////////////////////////////////////

#include <QGuiApplication>
#include <QApplication>
#include <QQuickView>
#include <QFont>
#include <QFontDatabase>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QQmlApplicationEngine>
#include <QTime>

#include "version.h"
#include "EkxQuickView.h"

#include <iostream>
#include <signal.h>

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

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

static int _nGlobalDebugLevel = 0;
static bool _bGlobalUseDebugTimestamps = false;

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void messageHandler( QtMsgType type,
                     const QMessageLogContext &context,
                     const QString &msg )
{
    QByteArray localMsg = msg.toLocal8Bit();

    const char *file     = context.file ? context.file : "";
    const char *function = context.function ? context.function : "";

    char szTimestamp[100];
    if ( _bGlobalUseDebugTimestamps )
    {
        sprintf( szTimestamp, "%s ", QTime::currentTime().toString( "hh:mm:ss.zzz" ).toLocal8Bit().constData() );
    }
    else
    {
        szTimestamp[0] = '\0';
    }

    switch (type)
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
                fprintf( stderr, DEBUG_COL_FATAL "(F) %s%s (%s:%u, %s)\n" DEBUG_COL_OFF, szTimestamp, localMsg.constData(), file, context.line, function);
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
    auto handler = [](int sig) -> void
    {
        // blocking and not aysnc-signal-safe func are valid
        qWarning() << "quit the application by signal " << sig;
        QCoreApplication::quit();
    };

    sigset_t blocking_mask;
    sigemptyset( &blocking_mask );

    for (auto sig : quitSignals)
        sigaddset( &blocking_mask, sig );

    struct sigaction sa;
    sa.sa_handler = handler;
    sa.sa_mask    = blocking_mask;
    sa.sa_flags   = 0;

    for (auto sig : quitSignals)
        sigaction(sig, &sa, nullptr);
}
#endif

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

int main( int argc,
          char *argv[] )
{
    qInstallMessageHandler( messageHandler );

    //
    // define command line parser
    //
    QCommandLineParser cliParser;

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
        QApplication app(argc, argv);

#ifdef __linux__
        catchUnixSignals( {SIGQUIT, SIGINT, SIGTERM, SIGHUP} );
#endif

        {
            cliParser.process( app );
            const QStringList args = cliParser.positionalArguments();

            _nGlobalDebugLevel = cliParser.value( optDebugLevel ).toInt();
            _bGlobalUseDebugTimestamps = cliParser.isSet( optUseTimestamps );

            QTextStream stream( stdout );
            stream << "E65STestApp (c) 2021 by Hemro International AG" << endl;
            stream << "================================================" << endl;
            stream << "Version " << VERSION_NO << " from " << VERSION_DATE << endl;
            stream << "Build " << BUILD_NO << " from " << BUILD_DATE << endl;
            stream << "Branch " << BRANCH << " SHA " << SHA << endl;
            stream << "================================================" << endl;
            stream << "Press CTRL-C to quit application" << endl;

            // no qrc:// due to QtBug-53378
            QFontDatabase::addApplicationFont( ":/Fonts/D-DINCondensedHEMRO-Bold.otf");
            QFontDatabase::addApplicationFont( ":/Fonts/D-DINCondensedHEMRO-Bold_Mono.otf");
            QFontDatabase::addApplicationFont( ":/Fonts/D-DINCondensedHEMRO-Bold_Mono_Numbers.otf");
            QFontDatabase::addApplicationFont( ":/Fonts/D-DINCondensedHEMRO-Light_Numbers.otf");
            QFontDatabase::addApplicationFont( ":/Fonts/D-DINCondensedHEMRO-Light_Mono_Numbers.otf");
            QFontDatabase::addApplicationFont( ":/Fonts/D-DINCondensedHEMRO-Regular.otf");
            QFontDatabase::addApplicationFont( ":/Fonts/D-DINCondensedHEMRO-Regular_Mono.otf");
            QFontDatabase::addApplicationFont( ":/Fonts/D-DINCondensedHEMRO-Regular_Mono_Numbers.otf" );

            QFontDatabase database;
            for( QString family : database.families( ) )
            {
                if ( family.startsWith( "D-DIN" ) )
                    qInfo() << family;
            }

            EkxQuickView *view = new EkxQuickView;

            //MainStatemachine mainStatemachine( view->engine() );

            //view->setMainStatemachine( &mainStatemachine );
            view->setSource(QUrl(QStringLiteral( "qrc:/QmlComponents/E65STestApp.qml" ) ) );

            view->show();

            nReturn = app.exec();

            delete view;
        }
    }
    catch ( ... )
    {
        qCritical() << "Caught unspecified exception!";
        nReturn = 0;
    }

    qInfo() << "Exit UI!";

    return nReturn;
}
