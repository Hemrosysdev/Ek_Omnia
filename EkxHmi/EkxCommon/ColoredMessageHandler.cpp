///////////////////////////////////////////////////////////////////////////////
///
/// @file ColoredMessageHandler.cpp
///
/// @brief main application entry point of ColoredMessageHandler.
///
/// @author Ultratronik GmbH
///         Dornierstr. 9
///         D-82205 Gilching
///         Germany
///         http://www.ultratronik.de
///
/// @author written by Gerd Esser, Forschung & Entwicklung, gesser@ultratronik.de
///
/// @date 03.04.2023
///
///////////////////////////////////////////////////////////////////////////////

#include "ColoredMessageHandler.h"

#include <QByteArray>
#include <QString>
#include <QTime>
#include <QCommandLineOption>
#include <QCommandLineParser>

namespace UxEco
{

int  ColoredMessageHandler::m_nDebugLevel         = 0;
bool ColoredMessageHandler::m_bUseDebugTimestamps = false;

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

ColoredMessageHandler::ColoredMessageHandler()
{
    qInstallMessageHandler( messageHandler );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void ColoredMessageHandler::setDebugLevel( const int nDebugLevel )
{
    m_nDebugLevel = nDebugLevel;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void ColoredMessageHandler::setUseDebugTimestamps( const bool bUse )
{
    m_bUseDebugTimestamps = bUse;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void ColoredMessageHandler::addCommandLineOptions( QCommandLineParser * pCommandLineParser )
{
    m_pCommandLineParser = pCommandLineParser;

    if ( m_pCommandLineParser )
    {
        QCommandLineOption optDebugLevel( QStringList() << "D" << "debuglevel",
                                          "debug level (default: 0)",
                                          "debuglevel",
                                          "0" );
        m_pCommandLineParser->addOption( optDebugLevel );

        QCommandLineOption optUseTimestamps( QStringList() << "t" << "timestamps",
                                             "use timestamps" );
        m_pCommandLineParser->addOption( optUseTimestamps );
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void ColoredMessageHandler::processCommandLine()
{
    if ( m_pCommandLineParser )
    {
        setDebugLevel( m_pCommandLineParser->value( "debuglevel" ).toInt() );
        setUseDebugTimestamps( m_pCommandLineParser->isSet( "timestamps" ) );
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void ColoredMessageHandler::messageHandler( QtMsgType                  type,
                                            const QMessageLogContext & context,
                                            const QString &            msg )
{
    QByteArray localMsg = msg.toLocal8Bit();

    const char * file     = context.file ? context.file : "";
    const char * function = context.function ? context.function : "";

    char szTimestamp[100];
    if ( m_bUseDebugTimestamps )
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
            if ( m_nDebugLevel >= 3 )
            {
                fprintf( stdout, DEBUG_COL_DEBUG "(D) %s%s\n" DEBUG_COL_OFF, szTimestamp, localMsg.constData() );
            }
            break;
        case QtInfoMsg:
            if ( m_nDebugLevel >= 2 )
            {
                fprintf( stdout, DEBUG_COL_INFO "(I) %s%s\n" DEBUG_COL_OFF, szTimestamp, localMsg.constData() );
            }
            break;
        case QtWarningMsg:
            if ( m_nDebugLevel >= 1 )
            {
                fprintf( stdout, DEBUG_COL_WARNING "(W) %s%s\n" DEBUG_COL_OFF, szTimestamp, localMsg.constData() );
            }
            break;
        case QtCriticalMsg:
            if ( m_nDebugLevel >= 0 )
            {
                fprintf( stderr, DEBUG_COL_CRITICAL "(C) %s%s\n" DEBUG_COL_OFF, szTimestamp, localMsg.constData() );
            }
            break;
        case QtFatalMsg:
            if ( m_nDebugLevel >= 0 )
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

} // namespace UxEco

