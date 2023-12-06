///////////////////////////////////////////////////////////////////////////////
///
/// @file ElapsedTimer.h
///
/// @brief Header file of class ElapsedTimer.
///
/// @author Ultratronik GmbH
///         Dornierstr. 9
///         D-82205 Gilching
///         Germany
///         http://www.ultratronik.de
///
/// @author written by Kai Uwe Broulik, kai.uwe.broulik@basyskom.com
///
/// @date 2023-02-24
///
/// @copyright Copyright 2021 by Hemro International AG
///            Hemro International AG
///            Länggenstrasse 34
///            CH 8184 Bachenbülach
///            Switzerland
///            Homepage: www.hemrogroup.com
///
///////////////////////////////////////////////////////////////////////////////

#ifndef ELAPSEDTIMER_H
#define ELAPSEDTIMER_H

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#include <QElapsedTimer>
#include <QObject>

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

class QTimer;

class ElapsedTimer
    : public QObject
{
    Q_OBJECT

    /** How many milliseconds have elapsed since the timer was started. */
    Q_PROPERTY( int elapsed READ elapsed NOTIFY elapsedChanged )
    /** Interval at which to signal a change of the "elapsed" property. */
    Q_PROPERTY( int reportInterval READ reportInterval WRITE setReportInterval NOTIFY reportIntervalChanged )
    /** Whether the timer is running. */
    Q_PROPERTY( bool running READ running WRITE setRunning NOTIFY runningChanged )

public:

    explicit ElapsedTimer( QObject * parent = nullptr );
    ~ElapsedTimer() override;

    int elapsed() const;

    Q_SIGNAL void elapsedChanged( int elapsed );

    int reportInterval() const;

    void setReportInterval( int nReportInterval );

    Q_SIGNAL void reportIntervalChanged( int reportInterval );

    bool running() const;

    void setRunning( bool bRunning );

    Q_SIGNAL void runningChanged( bool running );

    /** Starts the timer. Does nothing if it is already running. */
    Q_INVOKABLE void start();

    /** Restarts the timer. Starts it if it is not already running. */
    Q_INVOKABLE void restart();

    /** Stops the timer and resets "elapsed". */
    Q_INVOKABLE void stop();

private:

    void report();

    QTimer *      m_pReportTimer;
    QElapsedTimer m_elapsedTimer;

    int           m_nElapsed        = 0;
    int           m_nReportInterval = 0;

};

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#endif // ELAPSEDTIMER_H
