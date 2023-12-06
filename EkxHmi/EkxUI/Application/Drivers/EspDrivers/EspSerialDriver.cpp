///////////////////////////////////////////////////////////////////////////////
///
/// @file EspSerialDriver.cpp
///
/// @brief Implementation file of class EspSerialDriver.
///
/// @author Ultratronik GmbH \n
///         Dornierstr. 9 \n
///         D-82205 Gilching \n
///         Germany
///         http://www.ultratronik.de
///
/// @author written by Gerd Esser, Forschung & Entwicklung, gesser@ultratronik.de
///
/// @date 04.05.2023
///
/// @copyright Copyright 2020 by Hemro International AG \n
///            Hemro International AG \n
///            Länggenstrasse 34 \n
///            CH 8184 Bachenbülach \n
///            Switzerland \n
///            Homepage: www.hemrogroup.com
///
///////////////////////////////////////////////////////////////////////////////

#include "EspSerialDriver.h"

#include <QDebug>
#include <QTime>

#include "esp_rom_crc.h"

//#define DEBUG_ESP_UART

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

EspSerialDriver::EspSerialDriver( QObject * parent )
    : QSerialPort( parent )
    , m_timerMsgTimeout( parent )
{
    connect( this, &EspSerialDriver::readyRead, this, &EspSerialDriver::processReadyRead );

    m_timerMsgTimeout.setInterval( 200 );
    m_timerMsgTimeout.setSingleShot( true );
    connect( &m_timerMsgTimeout, &QTimer::timeout, this, &EspSerialDriver::processDataTimeout );

    auto lambdaCrc32 = []( const std::string & strData, EkxProtocol::MessageFrame & message )
    {
        uint32_t u32Crc32 = esp_rom_crc32_be( 0x0, reinterpret_cast<const uint8_t *>( strData.data() ), strData.size() );
        message.crc32().setValue( u32Crc32 );
    };

    m_pSerializer   = new EkxProtocol::Serializer( lambdaCrc32 );
    m_pDeserializer = new EkxProtocol::Deserializer( lambdaCrc32 );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

EspSerialDriver::~EspSerialDriver()
{
    closePort();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool EspSerialDriver::openPort( const QString & strPort )
{
    setBaudRate( 921600 );
    setPortName( strPort );

    qInfo() << "EspSerialDriver::openPort() try open" << strPort;

    bool bSuccess = open( QIODevice::ReadWrite );

    if ( !bSuccess )
    {
        qCritical() << "EspSerialDriver::openPort() can't open" << strPort;
    }
    else
    {
        flush();
        readAll();
    }

    return bSuccess;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void EspSerialDriver::closePort()
{
    m_timerMsgTimeout.stop();
    QSerialPort::close();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void EspSerialDriver::processMessageFrame( const EkxProtocol::MessageFrame & frame,
                                           const EkxProtocol::Deserializer::ParseResult nResult )
{
    Q_UNUSED( frame );
    Q_UNUSED( nResult );

    // null implementation, all stuff done in overloaded classes
    qCritical() << "not overloaded";
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool EspSerialDriver::sendMessageFrame( const EkxProtocol::DriverId                  u8DriverId,
                                        const quint32                                u32MsgCounter,
                                        const quint8                                 u8RepeatCounter,
                                        EkxProtocol::Payload::MessagePayload * const pPayload )
{
    EkxProtocol::MessageFrame frame;

    frame.driverId().setValue( u8DriverId );
    frame.msgCounter().setValue( u32MsgCounter );
    frame.repeatCounter().setValue( u8RepeatCounter );
    frame.payloadRef().setValue( pPayload );     // eats the pointer

    return sendMessageFrame( &frame );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool EspSerialDriver::sendMessageFrame( EkxProtocol::MessageFrame * const pFrame )
{

    bool bSuccess = false;

    if ( isOpen() )
    {
        setFailed( false );

        std::string strPreliminaryData = pFrame->serializedData();

        m_pSerializer->serialize( *pFrame );

#ifdef DEBUG_ESP_UART
        dumpHexData( "sendMessageFrame:", m_pSerializer->serializedData() );
#endif

        write( m_pSerializer->serializedData().data(),
               m_pSerializer->serializedData().size() );

        if ( error() == QSerialPort::NoError )
        {
            bSuccess = true;
        }
        else
        {
            qCritical() << "EspSerialDriver::sendMessageFrame() error while sending," << errorString();
        }
    }
    else
    {
        setFailed( true );
    }

    return bSuccess;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool EspSerialDriver::isFailed() const
{
    return m_bFailed;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void EspSerialDriver::dumpHexData( const QString &     strPrompt,
                                   const std::string & strData ) const
{
    QString strTimestamp = QTime::currentTime().toString( "hh:mm:ss.zzz" );

    printf( "%s : %s\n", strTimestamp.toStdString().c_str(), strPrompt.toStdString().c_str() );
    for ( size_t i = 0; i < strData.size(); )
    {
        printf( "    %05d: ", static_cast<int>( i ) );
        for ( size_t j = 0; j < 16
              && i < strData.size(); j++, i++ )
        {
            printf( "%02x ", static_cast<int>( strData[i] ) );
        }
        printf( "\n" );

        if ( i == 3 * 16 && strData.size() > 5 * 16 )
        {
            printf( "      ...\n" );
            i = ( ( strData.size() / 16 ) - 2 ) * 16;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void EspSerialDriver::processReadyRead()
{
    if ( bytesAvailable() )
    {
        QByteArray array = readAll();

        m_pDeserializer->addStreamData( reinterpret_cast<const uint8_t *>( array.data() ), array.size() );

#ifdef DEBUG_ESP_UART
        dumpHexData( "processReadyRead:", m_pDeserializer->streamData() );
#endif

        processInputData();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void EspSerialDriver::processDataTimeout()
{
    if ( !processInputData()
         && m_pDeserializer->hasData() )
    {
        qWarning() << "EspSerialDriver::processDataTimeout(), flush size" << m_pDeserializer->streamData().size();
        m_pDeserializer->flushStreamData();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool EspSerialDriver::processInputData()
{
    bool bProcessed = false;

    while ( true )
    {
        m_timerMsgTimeout.start();

        EkxProtocol::Deserializer::ParseResult result = m_pDeserializer->parseData();

        if ( ( result == EkxProtocol::Deserializer::ParseResult::Success )
             || ( result == EkxProtocol::Deserializer::ParseResult::WrongCrc ) )
        {
            processMessageFrame( m_pDeserializer->messageFrame(), result );
            bProcessed = true;
        }
        else
        {
            break;
        }
    }

    return bProcessed;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void EspSerialDriver::setFailed( const bool bFailed )
{
    if ( m_bFailed != bFailed )
    {
        m_bFailed = bFailed;
        emit failedChanged( m_bFailed );

        if ( m_bFailed )
        {
            qCritical() << "EspSerialDriver::setFailed() failed!";
        }
        else
        {
            qInfo() << "EspSerialDriver::setFailed() not failed!";
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////
