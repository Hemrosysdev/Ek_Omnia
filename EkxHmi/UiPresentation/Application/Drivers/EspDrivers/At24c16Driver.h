///////////////////////////////////////////////////////////////////////////////
///
/// @file At24c16Driver.h
///
/// @brief Header file of class At24c16Driver.
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

#ifndef At24c16Driver_h
#define At24c16Driver_h

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#include <QObject>

class QQmlEngine;
class EspAt24c16In;
class EspDataInterface;

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

class At24c16Driver : public QObject
{
    Q_OBJECT

    Q_PROPERTY( QString   productId     READ productId     NOTIFY productIdChanged )
    Q_PROPERTY( QString   serialNumber  READ serialNumber  NOTIFY serialNumberChanged )
    Q_PROPERTY( bool      valid         READ isValid       NOTIFY validChanged )

public:

    explicit At24c16Driver( QQmlEngine * pEngine,
                            QObject *parent = nullptr );

    virtual ~At24c16Driver( ) override;

    const QString & productId() const;
    const QString & serialNumber() const;
    bool isValid() const;

    void connectInterface( EspAt24c16In * pAt24c16In );
    void disconnectInterface();

public slots:

    void processDataChanged( EspDataInterface * pData );
    void processValidChanged( EspDataInterface * pData );

signals:

    void productIdChanged();
    void serialNumberChanged();
    void validChanged();

private:

    void setProductId( const QString & strProductId );
    void setSerialNumber( const QString & strSerialNumber );
    void setDataValid( const bool bValid );

private:

    EspAt24c16In * m_pAt24c16In { nullptr };
    QString        m_strProductId;
    QString        m_strSerialNumber;
    bool           m_bValid { false };

};

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#endif // At24c16Driver_h
