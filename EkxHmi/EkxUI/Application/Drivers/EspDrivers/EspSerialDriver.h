///////////////////////////////////////////////////////////////////////////////
///
/// @file EspSerialDriver.h
///
/// @brief Header file of class EspSerialDriver.
///
/// @author Ultratronik GmbH \n
///         Dornierstr. 9 \n
///         D-82205 Gilching \n
///         Germany
///         http://www.ultratronik.de
///
/// @author written by Gerd Esser, Forschung & Entwicklung, gesser@ultratronik.de
///
/// @date 03.05.2023
///
/// @copyright Copyright 2023 by Hemro International AG \n
///            Hemro International AG \n
///            Länggenstrasse 34 \n
///            CH 8184 Bachenbülach \n
///            Switzerland \n
///            Homepage: www.hemrogroup.com
///
///////////////////////////////////////////////////////////////////////////////

#ifndef EspSerialDriver_H
#define EspSerialDriver_H

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#include <QSerialPort>
#include <QTimer>
#include "CcittCrc16.h"
#include "EkxProtocol.h"

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

class EspSerialDriver
    : public QSerialPort
{
    Q_OBJECT

public:

    EspSerialDriver( QObject * parent = nullptr );

    virtual ~EspSerialDriver() override;

    bool openPort( const QString & strPort );

    void closePort();

    virtual void processMessageFrame( const EkxProtocol::MessageFrame & frame,
                                      const EkxProtocol::Deserializer::ParseResult nResult );

    virtual bool sendMessageFrame( const EkxProtocol::DriverId                  u8DriverId,
                                   const quint32                                u32MsgCounter,
                                   const quint8                                 u8RepeatCounter,
                                   EkxProtocol::Payload::MessagePayload * const pPayload );

    virtual bool sendMessageFrame( EkxProtocol::MessageFrame * const pFrame );

    bool isFailed() const;

    void dumpHexData( const QString &     strPrompt,
                      const std::string & strData ) const;

signals:

    void failedChanged( const bool bFailed );

public slots:

    void processReadyRead( void );

    void processDataTimeout( void );

private:

    bool processInputData();

    void messageParser( const quint8 u8Byte );

    void setFailed( const bool bFailed );

private:

    QTimer                      m_timerMsgTimeout;

    CcittCrc16                  m_crcIn;

    EkxProtocol::MessageFrame   m_messageFrame;

    EkxProtocol::Serializer *   m_pSerializer = nullptr;

    EkxProtocol::Deserializer * m_pDeserializer = nullptr;

    bool                        m_bFailed = true;

};

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#endif // EspSerialDriver_H
