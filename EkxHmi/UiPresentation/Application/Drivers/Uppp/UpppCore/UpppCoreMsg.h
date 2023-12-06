///////////////////////////////////////////////////////////////////////////////
///
/// @file UpppCoreMsg.h
///
/// @brief Header file of class UpppCoreMsg.
///
/// @author Ultratronik GmbH \n
///         Dornierstr. 9 \n
///         D-82205 Gilching \n
///         Germany
///         http://www.ultratronik.de
///
/// @author written by Gerd Esser, Forschung & Entwicklung, gesser@ultratronik.de
///
/// @date 17.08.2020
///
/// @copyright Copyright 2020 by Hemro International AG \n
///            Hemro International AG \n
///            Länggenstrasse 34 \n
///            CH 8184 Bachenbülach \n
///            Switzerland \n
///            Homepage: www.hemrogroup.com
///
///////////////////////////////////////////////////////////////////////////////

#ifndef UpppCoreMsg_H
#define UpppCoreMsg_H

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#include <QObject>
#include <QByteArray>

class UpppCore;

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

class UpppCoreMsg : public QObject
{
    Q_OBJECT

public:

    enum tMessageByte
    {
        PreambleByte1 = 0x5a,
        PreambleByte2 = 0xa5
    };

    // little endian word
    union tLeWord
    {
        quint16 word;
        struct
        {
            quint8 lsb;
            quint8 msb;
        } byte;
    };

public:

    UpppCoreMsg( QObject * parent  = nullptr );
    UpppCoreMsg( const UpppCoreMsg & theRight );

    virtual ~UpppCoreMsg();

    UpppCoreMsg & operator=( const UpppCoreMsg & theRight );

    UpppCoreMsg & operator<<( const quint8 u8Data );
    UpppCoreMsg & operator<<( const quint16 u16Data );
    UpppCoreMsg & operator<<( const quint32 u32Data );
    UpppCoreMsg & operator<<( const QByteArray & data );

    void clear();

    int msgType( void ) const;
    void setMsgType( const int nMsgType );

    quint8 transactionId() const;
    void setTransactionId(const quint8 nTransactionId);

    const QByteArray & payload() const;
    QByteArray & payload();

    quint16  crc16() const;

    bool send( UpppCore & core ) const;

    void dump( const QString & strPrompt ) const;

signals:

public slots:

private:

    int m_nMsgType;

    quint8 m_u8TransactionId;

    QByteArray  m_payload;

};

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#endif // UpppCoreMsg_H
