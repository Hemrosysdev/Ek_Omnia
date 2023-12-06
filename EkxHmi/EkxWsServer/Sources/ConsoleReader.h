///////////////////////////////////////////////////////////////////////////////
///
/// @file EspConnector.h
///
/// @brief Header file of class EspConnector.
///
/// @author Ultratronik GmbH \n
///         Dornierstr. 9 \n
///         D-82205 Gilching \n
///         Germany
///         http://www.ultratronik.de
///
/// @author written by Gerd Esser, Forschung & Entwicklung, gesser@ultratronik.de
///
/// @date 31.03.2020
///
/// @copyright Copyright 2020 by Hemro International AG \n
///            Hemro International AG \n
///            Länggenstrasse 34 \n
///            CH 8184 Bachenbülach \n
///            Switzerland \n
///            Homepage: www.hemrogroup.com
///
///////////////////////////////////////////////////////////////////////////////

#ifndef ConsoleReader_H
#define ConsoleReader_H

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#include <QThread>
#include <termios.h>

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

class ConsoleReader
    : public QThread
{
    Q_OBJECT

signals:

    void keyPressed( int ch );

public:

    ConsoleReader();

    virtual ~ConsoleReader() override;

private:

    void initTermios( const int echo );

    void resetTermios( void );

    int getch( void );

    void run() override;

private:

    struct termios m_oldSettings;
    struct termios m_newSettings;

};

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#endif  /* ConsoleReader_H */
