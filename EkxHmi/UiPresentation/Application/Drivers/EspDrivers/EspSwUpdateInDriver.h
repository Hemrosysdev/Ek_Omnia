///////////////////////////////////////////////////////////////////////////////
///
/// @file EspSwUpdateInDriver.h
///
/// @brief Header file of class EspSwUpdateInDriver.
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

#ifndef EspSwUpdateInDriver_h
#define EspSwUpdateInDriver_h

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#include <QObject>

class EspSwUpdateIn;
class EspDataInterface;

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

class EspSwUpdateInDriver
        : public QObject
{
    Q_OBJECT

public:

    explicit EspSwUpdateInDriver( QObject *parent = nullptr );
    virtual ~EspSwUpdateInDriver( ) override;

    int transactionId() const;
    int command( ) const;
    bool isValid( ) const;
    bool success( ) const;
    QString swVersion( ) const;

    void connectInterface( EspSwUpdateIn * pEspSwUpdateIn );
    void disconnectInterface();

signals:

    void transactionIdChanged();
    void commandChanged();
    void successChanged();
    void validChanged();
    void swVersionChanged();

public slots:

    void processDataChanged( EspDataInterface * pData );
    void processValidChanged( EspDataInterface * pData );

private:

    void setTransactionId( const int nTransactionId );
    void setValid( const bool bValid );
    void setSuccess( const bool bSuccess );
    void setCommand( const int nCommand );
    void setSwVersion( const QString & strSwVersion );

private:

    EspSwUpdateIn*      m_pSwUpdateIn { nullptr };
    int                 m_nCurrentTransactionId { 0 };
    int                 m_nCommand { 0 };
    bool                m_bSuccess { false };
    bool                m_bValid { false };
    QString             m_strSwVersion;

};

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#endif // EspSwUpdateInDriver_h
