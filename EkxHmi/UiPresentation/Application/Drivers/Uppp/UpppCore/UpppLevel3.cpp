///////////////////////////////////////////////////////////////////////////////
///
/// @file UpppLevel3.cpp
///
/// @brief Implementation file of class UpppLevel3.
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

#include "UpppLevel3.h"

#include <QCoreApplication>
#include <QThread>
#include <QDebug>
#include <QDateTime>

#include "UpppLevel3Msg.h"

#define MAX_RETRY_CNT  3

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

UpppLevel3::UpppLevel3( QObject * parent )
    : UpppCore( parent )
    , m_u8ServerTransactionId( 0 )
    , m_bServerMsgInProgress( false )
    , m_replyMessage( parent )
    , m_timerReplyMsg( parent )
    , m_bReplyGranted( false )
{
    m_timerReplyMsg.setSingleShot( true );

    connect( &m_timerReplyMsg, &QTimer::timeout, this, &UpppLevel3::processReplyTimeout );
    connect( this, &UpppLevel3::messageInProgressFinished, this, &UpppLevel3::processQueuedMessage );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

UpppLevel3::~UpppLevel3()
{
    m_timerReplyMsg.stop();

    close();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void UpppLevel3::close()
{
    m_timerReplyMsg.stop();
    UpppCore::close();
    m_bServerMsgInProgress = false;

    while ( m_theMsgQueue.size() )
    {
        delete m_theMsgQueue.takeFirst();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

UpppLevel3::tLevel3Status UpppLevel3::sendRequest( const int nRequestMsgType )
{
    UpppLevel3Msg msg( this );
    msg.setMsgType( Level3MsgTypeRequest );
    msg.payload().push_back( static_cast<char>( nRequestMsgType ) );

    return sendMessage( msg );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

UpppLevel3::tLevel3Status UpppLevel3::sendMessage( const UpppLevel3Msg & level3Msg )
{
    tLevel3Status nReply = Level3StatusSendOk;

    if ( !isOpen() )
    {
        nReply = Level3StatusNotOpen;
    }
    else if ( m_bServerMsgInProgress )
    {
        nReply = Level3StatusMsgInProgress;
    }
    else
    {
        m_bServerMsgInProgress = true;
        m_u8ServerTransactionId++;

        m_msgInProgress = level3Msg;
        m_msgInProgress.setRetryCnt( 0 );
        m_msgInProgress.setTransactionId( m_u8ServerTransactionId );

        retryMessageInProgress();
    }

    return nReply;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void UpppLevel3::retryMessageInProgress(void)
{
    if ( m_bServerMsgInProgress )
    {
        if ( m_msgInProgress.msgType() > Level3MsgTypeNack && !replyGranted() )
        {
            m_bServerMsgInProgress = false;
            m_msgInProgress.dump( "Try Send" );
            qDebug() << "message skipped - not granted to be sent";
            emit messageInProgressFinished( m_msgInProgress.msgType(),
                                            m_msgInProgress.transactionId(),
                                            false );
        }
        else if ( m_msgInProgress.retryCnt() < MAX_RETRY_CNT )
        {
            if ( m_msgInProgress.repeatSend() )
            {
                m_msgInProgress.incRetryCnt();
            }
            else
            {
                m_msgInProgress.setRetryCnt( MAX_RETRY_CNT );
            }

            m_timerReplyMsg.start( m_msgInProgress.timeout() );

            if ( !sendCoreMessage( m_msgInProgress ) )
            {
                qWarning() << "can't send message";
            }
        }
        else
        {
            m_bServerMsgInProgress = false;
            qWarning() << "message failed after max. retries";
            emit messageInProgressFinished( m_msgInProgress.msgType(),
                                            m_msgInProgress.transactionId(),
                                            false );
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void UpppLevel3::processAckNack()
{
    if ( m_bServerMsgInProgress )
    {
        if ( m_replyMessage.transactionId() == m_u8ServerTransactionId )
        {
            m_timerReplyMsg.stop();

            if ( m_replyMessage.msgType() == Level3MsgTypeAck )
            {
                m_bServerMsgInProgress = false;

                if ( m_msgInProgress.msgType() == Level3MsgTypeStatus )
                {
                    emit statusMsgAcknowledged();
                }

                emit messageInProgressFinished( m_msgInProgress.msgType(),
                                                m_msgInProgress.transactionId(),
                                                true );
            }
            else if ( m_replyMessage.msgType() == Level3MsgTypeNack )
            {
                qWarning() << "retry message after nack, reason " << static_cast<int>( m_replyMessage.payload()[0] );
                retryMessageInProgress();
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void UpppLevel3::processLevel3Message( const UpppCoreMsg & coreMsg )
{
    Q_UNUSED( coreMsg )

    // null implementation
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void UpppLevel3::processMessage( const UpppCoreMsg & coreMsg )
{
    switch ( coreMsg.msgType() )
    {
        case Level3MsgTypeNack:
        case Level3MsgTypeAck:
        {
            m_replyMessage = coreMsg;
            processAckNack();
        }
        break;

        case Level3MsgTypeRequest:
            processRequest( coreMsg );
        break;

        default:
            processLevel3Message( coreMsg );
        break;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void UpppLevel3::processRequest( const UpppCoreMsg &coreMsg )
{
    if ( m_bReplyGranted )
    {
        if ( coreMsg.payload().size() != Level3MsgLengthRequest )
        {
            sendNack( coreMsg.transactionId(),
                      Level3NackReasonInvalidPayloadLength );
        }
        else
        {
            int nRequestMsgType = static_cast<int>( coreMsg.payload()[0] );

            sendAck( coreMsg.transactionId() );

            emit requestReceived( coreMsg.transactionId(),
                                  nRequestMsgType );
        }
    }
}

//////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool UpppLevel3::sendAck( const quint8 u8TransactionId )
{
    UpppCoreMsg msg( this );
    msg.setMsgType( Level3MsgTypeAck );
    msg.setTransactionId( u8TransactionId );

    return sendCoreMessage( msg );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool UpppLevel3::sendNack( const int nTransactionId,
                           const int nReason )
{
    UpppCoreMsg msg( this );
    msg.setMsgType( Level3MsgTypeNack );
    msg.setTransactionId( nTransactionId );
    msg.payload().push_back( static_cast<char>( nReason ) );

    return sendCoreMessage( msg );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool UpppLevel3::isServerMsgInProgress() const
{
    return m_bServerMsgInProgress;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool UpppLevel3::replyGranted() const
{
    return m_bReplyGranted;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void UpppLevel3::setReplyGranted(const bool bGranted)
{
    m_bReplyGranted = bGranted;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void UpppLevel3::queueMessage(const UpppLevel3Msg &msg)
{
    m_theMsgQueue.push_back( new UpppLevel3Msg( msg ) );
    processQueuedMessage();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void UpppLevel3::processQueuedMessage()
{
    if ( !isServerMsgInProgress() )
    {
        if ( m_theMsgQueue.size() )
        {
            UpppLevel3Msg * pMsg = m_theMsgQueue.takeFirst();

            if ( pMsg )
            {
                sendMessage( *pMsg );
                delete pMsg;
            }
        }
    }

}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void UpppLevel3::processReplyTimeout()
{
    qWarning() << "message timeout";
    retryMessageInProgress();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

