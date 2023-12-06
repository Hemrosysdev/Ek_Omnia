///////////////////////////////////////////////////////////////////////////////
///
/// @file EspSwUpdateOutDriver.h
///
/// @brief Header file of class EspSwUpdateOutDriver.
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

#ifndef EspSwUpdateOutDriver_h
#define EspSwUpdateOutDriver_h

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#include <QObject>

class EspSwUpdateOut;

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

class EspSwUpdateOutDriver
        : public QObject
{
    Q_OBJECT

public:

    explicit EspSwUpdateOutDriver( QObject *parent = 0 );
    virtual ~EspSwUpdateOutDriver( ) override;

    void connectInterface( EspSwUpdateOut* pEspSwUpdateOut );
    void disconnectInterface();

    int transactionId( ) const;
    int incTransactionId( void );

    void setCommand( const int nCommand );
    int command( ) const;

    void setData( const int nChunkNo,
                  const int nChunkSize,
                  const QByteArray & data );

signals:

    void transactionIdChanged();
    void commandChanged();

private:

    void setTransactionId( const int nTransactionId );

private:

    EspSwUpdateOut*     m_pSwUpdateOut { nullptr };
    int                 m_nTransactionId { 0 };
    int                 m_nCommand { 0 };

};

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#endif // EspSwUpdateOutDriver_h
