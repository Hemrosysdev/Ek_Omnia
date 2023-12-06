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
/// @date 05.04.2021
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
#include <QSplashScreen>
#include <QLabel>
#include <QThread>

#include "MainStatemachine.h"
#include "version.h"
#include "EkxQuickView.h"
#include "EkxGlobals.h"
#include "QmlTypes.h"
#include "ColoredMessageHandler.h"

#ifdef __linux__
#include "UnixSignalCatcher.h"
#endif

#include <iostream>
#include <signal.h>

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

int main( int    argc,
          char * argv[] )
{
    UxEco::ColoredMessageHandler messageHandler;

    //
    // define command line parser
    //
    QCommandLineParser cliParser;
    cliParser.addHelpOption();

    messageHandler.addCommandLineOptions( &cliParser );

    int nReturn = -1;

    try
    {
        QApplication app( argc, argv );

#ifdef __linux__
        UxEco::UnixSignalCatcher::install( {SIGQUIT, SIGINT, SIGTERM, SIGHUP} );
#endif

        {
            QTextStream stream( stdout );

            QPixmap pixmap;
            if ( !pixmap.load( ":/Images/EKX UI Demonstrator_SplashScreen_06.png" ) )
            {
                stream << "main(): pixmap not loaded" << QT_ENDL;
            }
            QSplashScreen splash( pixmap );

            splash.showMessage( QString( "Version %1 / %2" ).arg( VERSION_NO, VERSION_DATE ),
                                Qt::AlignRight, Qt::white );

            splash.show();

            for ( int i = 0; i < 1000; i++ ) {
                app.processEvents();
            }

            QThread::sleep( 3 );

            cliParser.process( app );
            const QStringList args = cliParser.positionalArguments();

            messageHandler.processCommandLine();

            stream << "EkxHmiDemonstrator (c) 2021 by Hemro International AG" << QT_ENDL;
            stream << "=====================================================" << QT_ENDL;
            stream << "Version " << VERSION_NO << " from " << VERSION_DATE << QT_ENDL;
            stream << "Build " << BUILD_NO << " from " << BUILD_DATE << QT_ENDL;
            stream << "Branch " << BRANCH << " SHA " << SHA << QT_ENDL;
            stream << "=====================================================" << QT_ENDL;
            stream << "Press CTRL-C to quit application" << QT_ENDL;

            // no qrc:// due to QtBug-53378
            QFontDatabase::addApplicationFont( ":/Fonts/D-DINCondensedHEMRO-Bold.otf" );
            QFontDatabase::addApplicationFont( ":/Fonts/D-DINCondensedHEMRO-Bold_Mono.otf" );
            QFontDatabase::addApplicationFont( ":/Fonts/D-DINCondensedHEMRO-Bold_Mono_Numbers.otf" );
            QFontDatabase::addApplicationFont( ":/Fonts/D-DINCondensedHEMRO-Light_Numbers.otf" );
            QFontDatabase::addApplicationFont( ":/Fonts/D-DINCondensedHEMRO-Light_Mono_Numbers.otf" );
            QFontDatabase::addApplicationFont( ":/Fonts/D-DINCondensedHEMRO-Regular.otf" );
            QFontDatabase::addApplicationFont( ":/Fonts/D-DINCondensedHEMRO-Regular_Mono.otf" );
            QFontDatabase::addApplicationFont( ":/Fonts/D-DINCondensedHEMRO-Regular_Mono_Numbers.otf" );

            //QFontDatabase database;
            //for( QString family : database.families( ) )
            //{
            //    if ( family.startsWith( "D-DIN" ) )
            //        qInfo() << family;
            //}

            QmlTypes::registerTypes();

            EkxQuickView * view = new EkxQuickView;
            view->setIcon( QIcon( ":/Images/fav_32x32.png" ) );

            MainStatemachine mainStatemachine( view->engine() );

            view->setMainStatemachine( &mainStatemachine );
            view->setSource( QUrl( "qrc:/main.qml" ) );

            view->show();

            splash.finish( nullptr );

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

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#ifdef __WIN32__
int WinMain( HINSTANCE hInstance,
             HINSTANCE hPrevInstance,
             PSTR      lpCmdLine,
             int       nCmdShow )
{
    Q_UNUSED( hInstance );
    Q_UNUSED( hPrevInstance );
    Q_UNUSED( lpCmdLine );
    Q_UNUSED( nCmdShow );

    return main( __argc, __argv );
}

#endif

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

