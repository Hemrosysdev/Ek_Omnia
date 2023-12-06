///////////////////////////////////////////////////////////////////////////////
///
/// @file EspDeviceDriver.h
///
/// @brief Header file of class EspDeviceDriver.
///
/// @author Ultratronik GmbH \n
///         Dornierstr. 9 \n
///         D-82205 Gilching \n
///         Germany
///         http://www.ultratronik.de
///
/// @author written by Gerd Esser, Forschung & Entwicklung, gesser@ultratronik.de
///
/// @date 05.05.2023
///
/// @copyright Copyright 2023 by Hemro International AG \n
///            Hemro International AG \n
///            Länggenstrasse 34 \n
///            CH 8184 Bachenbülach \n
///            Switzerland \n
///            Homepage: www.hemrogroup.com
///
///////////////////////////////////////////////////////////////////////////////

#ifndef EspDeviceDriver_h
#define EspDeviceDriver_h

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#include <QObject>
#include "EkxProtocol.h"

class EspProtocolStack;

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

class EspDeviceDriver : public QObject
{

    Q_OBJECT

public:

    explicit EspDeviceDriver( const EkxProtocol::DriverId u8DriverId,
                              QObject *                   parent = nullptr );

    void create( EspProtocolStack * const pProtocolStack );

    EkxProtocol::DriverId driverId() const;

    virtual void processMessageFrame( const EkxProtocol::MessageFrame & frame );

    void replyNack( const EkxProtocol::DriverId                  u8DriverId,
                    const quint32                                u32MsgCounter,
                    const quint32                                u8RepeatCounter,
                    const EkxProtocol::Payload::Nack::NackReason u8NackReason );

    void replyAck( const EkxProtocol::DriverId u8DriverId,
                   const quint32               u32MsgCounter,
                   const quint32               u8RepeatCounter );

    void send( const EkxProtocol::DriverId                  u8DriverId,
               const quint32                                u32MsgCounter,
               const quint32                                u8RepeatCounter,
               EkxProtocol::Payload::MessagePayload * const pPayload );

    void send( EkxProtocol::Payload::MessagePayload * const pPayload );

    virtual void processAlive( const bool bAlive );

signals:

private:

    EkxProtocol::DriverId m_u8DriverId = EkxProtocol::DriverId::Last;

    EspProtocolStack *    m_pProtocolStack = nullptr;

};

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#endif // EspDeviceDriver_h
