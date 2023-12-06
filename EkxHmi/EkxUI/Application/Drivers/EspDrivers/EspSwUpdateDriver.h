///////////////////////////////////////////////////////////////////////////////
///
/// @file EspSwUpdateDriver.h
///
/// @brief Header file of class EspSwUpdateDriver.
///
/// @author Ultratronik GmbH
///         Dornierstr. 9
///         D-82205 Gilching
///         Germany
///         http://www.ultratronik.de
///
/// @author written by Florian Sonntag, Forschung & Entwicklung, gesser@ultratronik.de
///
/// @date 31.03.2021
///
/// @copyright Copyright 2021 by Hemro International AG
///            Hemro International AG
///            Länggenstrasse 34
///            CH 8184 Bachenbülach
///            Switzerland
///            Homepage: www.hemrogroup.com
///
///////////////////////////////////////////////////////////////////////////////

#ifndef EspSwUpdateDriver_h
#define EspSwUpdateDriver_h

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#include "EspDeviceDriver.h"
#include "EkxProtocol.h"

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

class EspSwUpdateDriver : public EspDeviceDriver
{
    Q_OBJECT

public:

    explicit EspSwUpdateDriver( const EkxProtocol::DriverId u8DriverId );
    ~EspSwUpdateDriver() override;

    int sendTransactionId() const;

    int incSendTransactionId( void );

    void sendCmd( const EkxProtocol::Payload::SwUpdateCmd nCommand );

    EkxProtocol::Payload::SwUpdateCmd sendCmd() const;

    void sendChunk( const int          nChunkNo,
                    const QByteArray & data );

    bool isValid() const;

    EkxProtocol::Payload::SwUpdateCmd statusCmd() const;

    int statusTransactionId() const;

    bool statusSuccess() const;

    const QString & statusSwVersion() const;

    void processMessageFrame( const EkxProtocol::MessageFrame & frame ) override;

    void processAlive( const bool bAlive ) override;

signals:

    void sendTransactionIdChanged();

    void sendCmdChanged();

    void statusChanged();

private:

    void setTransactionId( const int nTransactionId );

    void setStatusTransactionId( const int nTransactionId );

    void setValid( const bool bValid );

    void setStatusSuccess( const bool bSuccess );

    void setStatusCmd( const EkxProtocol::Payload::SwUpdateCmd nCmd );

    void setStatusSwVersion( const QString & strSwVersion );

private:

    int                               m_nSendTransactionId { 0 };
    EkxProtocol::Payload::SwUpdateCmd m_nSendCmd { EkxProtocol::Payload::SwUpdateCmd::Idle };

    int                               m_nStatusTransactionId { 0 };
    EkxProtocol::Payload::SwUpdateCmd m_nStatusCmd { EkxProtocol::Payload::SwUpdateCmd::Idle };
    bool                              m_bStatusSuccess { false };
    bool                              m_bValid { false };
    QString                           m_strStatusSwVersion;

};

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#endif // EspSwUpdateDriver_h
