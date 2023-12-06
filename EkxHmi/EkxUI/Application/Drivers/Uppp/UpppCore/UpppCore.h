///////////////////////////////////////////////////////////////////////////////
///
/// @file UpppCore.h
///
/// @brief Header file of class UpppCore.
///
/// @author Ultratronik GmbH \n
///         Dornierstr. 9 \n
///         D-82205 Gilching \n
///         Germany
///         http://www.ultratronik.de
///
/// @author written by Gerd Esser, Forschung & Entwicklung, gesser@ultratronik.de
///
/// @date 13.08.2020
///
/// @copyright Copyright 2020 by Hemro International AG \n
///            Hemro International AG \n
///            Länggenstrasse 34 \n
///            CH 8184 Bachenbülach \n
///            Switzerland \n
///            Homepage: www.hemrogroup.com
///
///////////////////////////////////////////////////////////////////////////////

#ifndef UpppCore_H
#define UpppCore_H

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#include <QSerialPort>
#include <QTimer>
#include "CcittCrc16.h"
#include "UpppCoreMsg.h"

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

class UpppCore
    : public QSerialPort
{
    Q_OBJECT

private:

    enum tParserState
    {
        Idle = 0,
        Preamble2,
        MessageType,
        TransactionId,
        Length1,
        Length2,
        Payload,
        Crc1,
        Crc2
    };

public:

    UpppCore( QObject * parent = nullptr );

    virtual ~UpppCore() override;

    void openStreamInterface();

    bool isOpen() const;

    virtual void close() override;

    virtual void processMessage( const UpppCoreMsg & coreMsg );

    bool sendCoreMessage( const UpppCoreMsg & coreMsg );

    bool outputToStream( const QByteArray & data );

    void inputFromStream( const QByteArray & data );

signals:

    void streamOutput( const QByteArray & data );

public slots:

    void processReadyRead( void );

    void processMsgTimeout( void );

private:

    void messageParser( const quint8 u8Byte );

private:

    tParserState m_nParserState;

    QTimer       m_timerMsgTimeout;

    int          m_nMsgLength;

    int          m_nMsgCrc;

    int          m_nCurrentPayloadLength;

    int          m_nCurrentMsgCrc;

    CcittCrc16   m_crcIn;

    UpppCoreMsg  m_coreMsg;

    bool         m_bStreamInterfaceOpen { false };

};

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#endif // UpppCore_H
