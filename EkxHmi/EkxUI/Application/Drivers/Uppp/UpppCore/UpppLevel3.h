///////////////////////////////////////////////////////////////////////////////
///
/// @file UpppLevel3.h
///
/// @brief Header file of class UpppLevel3.
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

#ifndef UpppLevel3_H
#define UpppLevel3_H

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#include "UpppCore.h"
#include "UpppCoreMsg.h"
#include "UpppLevel3Msg.h"

#include <QMutex>

class UpppLevel3Msg;

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

class UpppLevel3
    : public UpppCore
{
    Q_OBJECT

public:

    enum tLevel3MsgType
    {
        Level3MsgTypeStatus = 0,
        Level3MsgTypeAck = 1,
        Level3MsgTypeNack = 2,
        Level3MsgTypeRequest = 3
    };

    enum tLevel3Status
    {
        Level3StatusSendOk = 0,
        Level3StatusSendFailed,
        Level3StatusReplyTimeout,
        Level3StatusMsgInProgress,
        Level3StatusReplyNack,
        Level3StatusNotOpen,
    };

    enum tLevel3MsgLength
    {
        Level3MsgLengthRequest = 1,
    };

    enum tLevel3NackReason
    {
        Level3NackReasonUnknown = 0,
        Level3NackReasonPayloadDataOutOfRange,
        Level3NackReasonInvalidCrc,
        Level3NackReasonInvalidMessageType,
        Level3NackReasonInvalidPayloadLength,
        Level3NackReasonMessageTypeNotSupported,
    };

public:

    UpppLevel3( QObject * parent = nullptr );

    virtual ~UpppLevel3() override;

    virtual void close() override;

    tLevel3Status sendRequest( const int nRequestMsgType );

    tLevel3Status sendMessage( const UpppLevel3Msg & level3Msg );

    bool sendAck( const quint8 u8TransactionId );

    bool sendNack( const int nTransactionId,
                   const int nReason );

    bool isServerMsgInProgress( void ) const;

    bool replyGranted( void ) const;

    void setReplyGranted( const bool bGranted );

    void queueMessage( const UpppLevel3Msg & msg );

protected:

    virtual void processLevel3Message( const UpppCoreMsg & coreMsg );

private:

    void processMessage( const UpppCoreMsg & coreMsg ) override;

    void processRequest( const UpppCoreMsg & coreMsg );

signals:

    void requestReceived( const int nTransactionId,
                          const int nRequestMsgType );

    void messageInProgressFinished( const int    nMsgType,
                                    const quint8 u8TransactionId,
                                    const bool   bSuccess );

    void statusMsgAcknowledged();

public slots:

    void processReplyTimeout( void );

    void processAckNack( void );

    void retryMessageInProgress( void );

    void processQueuedMessage();

private:

    quint8                 m_u8ServerTransactionId;

    bool                   m_bServerMsgInProgress;

    UpppCoreMsg            m_replyMessage;

    QTimer                 m_timerReplyMsg;

    UpppLevel3Msg          m_msgInProgress;

    bool                   m_bReplyGranted;

    QList<UpppLevel3Msg *> m_theMsgQueue;

};

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#endif // UpppLevel3_H
