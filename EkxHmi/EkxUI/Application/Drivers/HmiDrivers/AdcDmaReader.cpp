///////////////////////////////////////////////////////////////////////////////
///
/// @file AdcDmaReader.cpp
///
/// @brief main application entry point of AdcDmaReader.
///
/// @author Ultratronik GmbH
///         Dornierstr. 9
///         D-82205 Gilching
///         Germany
///         http://www.ultratronik.de
///
/// @author written by Gerd Esser, Forschung & Entwicklung, gesser@ultratronik.de
///
/// @date 16.05.2022
///
/// @copyright Copyright 2022 by Hemro International AG
///            Hemro International AG
///            Länggenstrasse 34
///            CH 8184 Bachenbülach
///            Switzerland
///            Homepage: www.hemrogroup.com
///
///////////////////////////////////////////////////////////////////////////////

#include "AdcDmaReader.h"

#include <QFile>
#include <QDebug>
#include <QDateTime>
#include <QElapsedTimer>

namespace SystemIo
{

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

AdcDmaReader::AdcDmaReader( QObject * const pParent )
    : QThread( pParent )
    , m_semStopThread( 1 )
{
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

AdcDmaReader::~AdcDmaReader()
{
    disable();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void AdcDmaReader::create( const QString &             strDmaDeviceFile,
                           std::function<void()>       startDmaRead,
                           std::function<void ( int )> processAdcValue )
{
    m_strDmaFile      = strDmaDeviceFile;
    m_processAdcValue = processAdcValue;
    m_startDmaRead    = startDmaRead;

    if ( isRunning() )
    {
        qInfo() << "AdcDmaReader::create() try stopping thread" << objectName();
        m_semStopThread.acquire();
        wait( 5000 );
        terminate();
    }

    if ( !strDmaDeviceFile.isEmpty() )
    {
        // priority not really relevant for Linux
        start( QThread::TimeCriticalPriority );
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void AdcDmaReader::run()
{
    qInfo() << "AdcDmaReader::run() thread started" << objectName();

    QFile dmaFile( m_strDmaFile );
    if ( !dmaFile.open( QIODevice::ReadOnly | QIODevice::Unbuffered ) )
    {
        qCritical() << "AdcDmaReader::run() can't open file" << m_strDmaFile;
        emit openDmaDeviceFileFailed();
    }
    else
    {
        uint64_t u64SampleCnt = 0;
        int      nLoops       = 0;

        QElapsedTimer timer;
        timer.start();

        char pBuffer[c_nBufferSize];

        qint64 n64LastElapsed = 0;
        qint64 n64Min         = 999999;
        qint64 n64Max         = 0;

        while ( m_semStopThread.available() )
        {
            qint64 n64ReadData = dmaFile.read( pBuffer, c_nBufferSize );

            m_startDmaRead();

            nLoops++;

            quint64 u64BufferItems = 0;

            if ( n64ReadData > 0 )
            {
                u64BufferItems = n64ReadData / 2;
            }

            const quint16 * pItemBuffer = reinterpret_cast<quint16 *>( pBuffer );

            for ( quint64 i = 0; i < u64BufferItems; i++ )
            {
                m_processAdcValue( static_cast<int>( pItemBuffer[i] ) );
            }

            u64SampleCnt += u64BufferItems;

            qint64 n64Elapsed = timer.elapsed();
            qint64 n64Diff    = n64Elapsed - n64LastElapsed;

            n64Min = std::min( n64Min, n64Diff );
            n64Max = std::max( n64Max, n64Diff );

            if ( n64Elapsed >= 1000 ) //&& objectName() == "DmaReaderMotorPower" )
            {
                qDebug() << "SampleRate:"
                         << objectName()
                         << n64Elapsed << "ms"
                         << n64Min << "ms min"
                         << n64Max << "ms max"
                         << nLoops << "loops"
                         << u64SampleCnt << "cnt"
                         << ( u64SampleCnt * 1000 / n64Elapsed ) << "bytes/sec"
                         << static_cast<uint16_t>( pItemBuffer[0] ) << "ADC"
                         << ( ( double ) pItemBuffer[0] * 2.5 / 0xffffU ) << "V";
                timer.restart();
                u64SampleCnt = 0;
                nLoops       = 0;
                n64Min       = 9999999;
                n64Max       = 0;
                n64Elapsed   = 0;
            }

            n64LastElapsed = n64Elapsed;
        }

        dmaFile.close();
    }

    m_semStopThread.release();

    qInfo() << "AdcDmaReader::run() thread finished" << objectName();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void SystemIo::AdcDmaReader::disable()
{
    auto fakeAdcProcessFunc = []( const int nAdcValue )
                              {
                                  Q_UNUSED( nAdcValue );
                              };
    auto fakeStartDmaReadFunc = []()
                                {
                                };
    create( "", fakeStartDmaReadFunc, fakeAdcProcessFunc );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

} // namespace SystemIo
