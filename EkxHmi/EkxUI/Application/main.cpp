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
#include <QQuickView>
#include <QFont>
#include <QFontDatabase>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QQmlApplicationEngine>
#include <QTime>

#include "MainStatemachine.h"
#include "QmlTypes.h"
#include "version.h"
#include "EkxGlobals.h"
#include "EkxQuickView.h"
#include "ColoredMessageHandler.h"
#include "UnixSignalCatcher.h"

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

    QCommandLineOption optSerialPort( QStringList() << "p" << "port",
                                      QCoreApplication::translate( "main", "serial port of ESP (default: /dev/ttySTM2)" ),
                                      QCoreApplication::translate( "main", "serial port" ),
                                      QCoreApplication::translate( "main", "/dev/ttySTM2" ) );
    cliParser.addOption( optSerialPort );

    int nReturn = -1;

    try
    {
        QGuiApplication app( argc, argv );

#ifdef __linux__
        UxEco::UnixSignalCatcher::install( {SIGQUIT, SIGINT, SIGTERM, SIGHUP} );
#endif

        {
            cliParser.process( app );
            const QStringList args = cliParser.positionalArguments();

            messageHandler.processCommandLine();

            QTextStream stream( stdout );
            stream << "EkxUI (c) 2021 by Hemro International AG" << QT_ENDL;
            stream << "================================================" << QT_ENDL;
            stream << "Version " << VERSION_NO << " from " << VERSION_DATE << QT_ENDL;
            stream << "Build " << BUILD_NO << " from " << BUILD_DATE << QT_ENDL;
            stream << "Branch " << BRANCH << " SHA " << SHA << QT_ENDL;
            stream << "================================================" << QT_ENDL;
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

            QmlTypes::registerTypes();

            EkxQuickView * view = new EkxQuickView;

            MainStatemachine mainStatemachine( view->engine() );

            mainStatemachine.setSerialPort( cliParser.value( "port" ) );

            view->setMainStatemachine( &mainStatemachine );
            view->setSource( QUrl( QStringLiteral( "qrc:/QmlComponents/EkxUi.qml" ) ) );

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
