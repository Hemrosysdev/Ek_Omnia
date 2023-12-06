///////////////////////////////////////////////////////////////////////////////
///
/// @file EspProtocolStack.h
///
/// @brief Header file of class EspProtocolStack.
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
/// @copyright Copyright 2023 by Hemro International AG \n
///            Hemro International AG \n
///            Länggenstrasse 34 \n
///            CH 8184 Bachenbülach \n
///            Switzerland \n
///            Homepage: www.hemrogroup.com
///
///////////////////////////////////////////////////////////////////////////////

#ifndef EspProtocolStack_h
#define EspProtocolStack_h

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#include <QObject>
#include <QTimer>
#include <QMap>
#include <QQueue>
#include <QPair>

#include "EspSerialDriver.h"
#include "EkxProtocol.h"

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

class EspDeviceDriver;

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

class EspProtocolStack : public EspSerialDriver
{
    Q_OBJECT

private:

    using driverMap = QMap<EkxProtocol::DriverId, EspDeviceDriver *>;

    static const quint8 c_u8MaxRepeatCounter = 3;

    static const int    c_nMaxQueueTicks = 2;

    using queuePair = QPair<int, EkxProtocol::MessageFrame *>;

public:

    EspProtocolStack();
    ~EspProtocolStack() override;

    bool isAlive() const;

    bool queueMessageFrame( const EkxProtocol::DriverId                  u8DriverId,
                            const quint32                                u32MsgCounter,
                            const quint8                                 u8RepeatCounter,
                            EkxProtocol::Payload::MessagePayload * const pPayload );

    void registerDriver( EspDeviceDriver * pDriver );

    quint32 incMsgCounter();

signals:

    void aliveChanged( const bool bAlive );

private:

    void setAlive( const bool bAlive );

    void delayedAliveChanged();

    void processTimeoutAlive();

    void processTimeoutSendPing();

    void processAliveChanged( const bool bAlive );

    void processMessageFrame( const EkxProtocol::MessageFrame & frame,
                              const EkxProtocol::Deserializer::ParseResult nResult ) override;

    void processTimeoutQueueObserver();

    void processAckedFrame( const EkxProtocol::DriverId u8DriverId,
                            const quint32 u32MsgCounter );

    void processNackedFrame( const EkxProtocol::MessageFrame & frame );

    virtual void frameAcked( const quint32 u32MsgCounter );

    virtual void frameNacked( const quint32                                u32MsgCounter,
                              const EkxProtocol::Payload::Nack::NackReason u8NackReason );

private:

    QTimer            m_timerSendPing;

    QTimer            m_timerAlive;

    QTimer            m_timerQueueObserver;

    bool              m_bAlive = false;

    quint32           m_u32MsgCounter = 0;

    driverMap         m_driverMap;

    QQueue<queuePair> m_theMsgQueue;

};

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#endif // EspProtocolStack_h
