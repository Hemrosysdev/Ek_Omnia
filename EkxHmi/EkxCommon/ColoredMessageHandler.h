///////////////////////////////////////////////////////////////////////////////
///
/// @file ColoredMessageHandler.h
///
/// @brief Header file of class ColoredMessageHandler.
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

#ifndef ColoredMessageHandler_h
#define ColoredMessageHandler_h

#include <QtGlobal>

class QCommandLineParser;

namespace UxEco
{

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

class ColoredMessageHandler
{

public:

    ColoredMessageHandler();

    void setDebugLevel( const int nDebugLevel );

    void setUseDebugTimestamps( const bool bUse );

    void addCommandLineOptions( QCommandLineParser * pCommandLineParser );

    void processCommandLine();

private:

    static void messageHandler( QtMsgType                  type,
                                const QMessageLogContext & context,
                                const QString &            msg );

private:

    static int           m_nDebugLevel;
    static bool          m_bUseDebugTimestamps;

    QCommandLineParser * m_pCommandLineParser = nullptr;

};

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

} // namespace UxEco

#endif // ColoredMessageHandler_h
