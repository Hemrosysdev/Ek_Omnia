///////////////////////////////////////////////////////////////////////////////
///
/// @file DeviceInfoCollector.h
///
/// @brief Header file of class DeviceInfoCollector.
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

#ifndef DeviceInfoCollector_h
#define DeviceInfoCollector_h

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#include <QObject>

class MainStatemachine;

#define NOT_VALID     "Invalid"

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

class DeviceInfoCollector
    : public QObject
{
    Q_OBJECT

    Q_PROPERTY( QString mcuSwVersion READ mcuSwVersion NOTIFY mcuInfosChanged )
    Q_PROPERTY( QString mcuHwVersion READ mcuHwVersion NOTIFY mcuInfosChanged )
    Q_PROPERTY( QString mcuProdId READ mcuProdId NOTIFY mcuInfosChanged )
    Q_PROPERTY( QString mcuSerialNo READ mcuSerialNo NOTIFY mcuInfosChanged )

    Q_PROPERTY( QString espSwVersion READ espSwVersion NOTIFY espInfosChanged )
    Q_PROPERTY( QString espProdId READ espProdId NOTIFY espInfosChanged )
    Q_PROPERTY( QString espSerialNo READ espSerialNo NOTIFY espInfosChanged )

    Q_PROPERTY( QString ekxUiSwVersion READ ekxUiSwVersion CONSTANT )
    Q_PROPERTY( QString ekxUiHwVersion READ ekxUiHwVersion CONSTANT )
    Q_PROPERTY( QString ekxUiSerialNo READ ekxUiSerialNo CONSTANT )
    Q_PROPERTY( QString espHostConSwVersion READ espHostConnectorSwVersion CONSTANT )

    Q_PROPERTY( QString packageSwVersion READ packageSwVersion CONSTANT )
    Q_PROPERTY( QString packageBuildNo READ packageBuildNo CONSTANT )

public:

    DeviceInfoCollector( QObject * pParent = nullptr );
    ~DeviceInfoCollector() override;

    void create( MainStatemachine * pMainStatemachine );

    bool mcuInfosCollected() const;

    void resetMcuInfosCollected();

    const QString & mcuSwVersion() const;

    const QString & mcuHwVersion() const;

    const QString & mcuProdId() const;

    const QString & mcuSerialNo() const;

    bool espInfosCollected() const;

    void resetEspInfosCollected();

    const QString & espSwVersion() const;

    const QString & espProdId() const;

    const QString & espSerialNo() const;

    const QString & ekxUiSwVersion() const;

    const QString & ekxUiHwVersion() const;

    const QString & ekxUiSerialNo() const;

    const QString & espHostConnectorSwVersion() const;

    const QString & packageSwVersion() const;

    const QString & packageBuildNo() const;

    bool isDddPwmVariant() const;

public slots:

    void collectMcuVersion();

    void mcuVersionTimeout();

    void collectEspVersion();

signals:

    void mcuInfosChanged();

    void espInfosChanged();

private:

    void collectPackageInfo();

    void collectEkxUiVersion();

    void collectEkxUiHWVersion();

    void collectEspHostConnectorVersion();

    void setMcuInfosCollected( const bool newMcuInfo );

    void setEspInfosCollected( const bool newEspInfo );

private:

    MainStatemachine * m_pMainStatemachine { nullptr };

    QString            m_strEkxUiSwVersion { NOT_VALID };
    QString            m_strEspHostConnectorSwVersion { NOT_VALID };
    QString            m_strEkxUiHwVersion { NOT_VALID };
    QString            m_strEkxUiSerialNo { NOT_VALID };

    QString            m_strPackageSwVersion { NOT_VALID };
    QString            m_strPackageBuildNo { NOT_VALID };

    QString            m_strEspSwVersion { NOT_VALID };
    QString            m_strEspProdId { NOT_VALID };
    QString            m_strEspSerialNo { NOT_VALID };

    QString            m_strMcuProdId { NOT_VALID };
    QString            m_strMcuSerialNo { NOT_VALID };
    QString            m_strMcuSwVersion { NOT_VALID };
    QString            m_strMcuHwVersion { NOT_VALID };

    bool               m_bMcuInfosCollected { false };
    bool               m_bEspInfosCollected { false };

};

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#endif // DeviceInfoCollector_h
