///////////////////////////////////////////////////////////////////////////////
///
/// @file DeviceInfoCollector.cpp
///
/// @brief Implementation file of class DeviceInfoCollector.
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

#include "DeviceInfoCollector.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QQmlEngine>
#include <QtQml>

#include "McuDriver.h"
#include "UartDriver.h"
#include "EspDriver.h"
#include "MainStatemachine.h"

#define VERSION_PATH       "/usr/local/bin/"
#define HWVERSION_PATH     "/proc/device-tree/hde/"

#define PACKAGE_FILENAME   "HEM-EKX-S01-Version.json"
#define EKXUI_FILENAME     "EkxUIVersion.json"
#define ESPHOST_FILENAME   "EspHostConnectorVersion.json"
#define HW_VERSION_BNAME   "bname"
#define HW_VERSION_BREV    "brev"
#define HW_VERSION_SERNR   "sernr"

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

DeviceInfoCollector::DeviceInfoCollector( QQmlEngine*  pEngine,
                                          MainStatemachine * pMainStatemachine )
    : QObject( pMainStatemachine )
    , m_pMainStatemachine( pMainStatemachine)
{
    MainStatemachine::ensureExistingDirectory( MainStatemachine::rootPath() + VERSION_PATH + "/dummy" );
    MainStatemachine::ensureExistingDirectory( MainStatemachine::rootPath() + HWVERSION_PATH + "/dummy" );

    pEngine->rootContext()->setContextProperty("deviceInfoCollector" , this);

    connect( m_pMainStatemachine->espDriver()->uartDriver()->mcuDriver(), &McuDriver::productIdValidChanged, this, &DeviceInfoCollector::collectMcuVersion );
    connect( m_pMainStatemachine->espDriver()->uartDriver()->mcuDriver(), &McuDriver::productIdRequestTimeout, this, &DeviceInfoCollector::mcuVersionTimeout );

    connect( m_pMainStatemachine->espDriver()->at24c16Driver(), &At24c16Driver::productIdChanged, this, &DeviceInfoCollector::collectEspVersion );
    connect( m_pMainStatemachine->espDriver()->at24c16Driver(), &At24c16Driver::serialNumberChanged, this, &DeviceInfoCollector::collectEspVersion );
    connect( m_pMainStatemachine->espDriver()->at24c16Driver(), &At24c16Driver::validChanged, this, &DeviceInfoCollector::collectEspVersion );

    connect( m_pMainStatemachine->espDriver()->swUpdateInDriver(), &EspSwUpdateInDriver::swVersionChanged, this, &DeviceInfoCollector::collectEspVersion );
    connect( m_pMainStatemachine->espDriver()->swUpdateInDriver(), &EspSwUpdateInDriver::validChanged, this, &DeviceInfoCollector::collectEspVersion );

    collectPackageInfo();
    collectMcuVersion();
    collectEkxUiVersion();
    collectEkxUiHWVersion();
    collectEspHostConnectorVersion();
    collectEspVersion();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

DeviceInfoCollector::~DeviceInfoCollector()
{
    m_pMainStatemachine = nullptr;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void DeviceInfoCollector::collectMcuVersion( )
{
    if( m_pMainStatemachine->espDriver()->uartDriver()->mcuDriver()->isProductIdValid() )
    {
        m_strMcuSwVersion = m_pMainStatemachine->espDriver()->uartDriver()->mcuDriver()->swVersion();
        m_strMcuHwVersion = m_pMainStatemachine->espDriver()->uartDriver()->mcuDriver()->hwVersion();
        m_strMcuSerialNo  = m_pMainStatemachine->espDriver()->uartDriver()->mcuDriver()->serialNo();
        m_strMcuProdId    = m_pMainStatemachine->espDriver()->uartDriver()->mcuDriver()->productId();

        setMcuInfosCollected( true );
    }
    else
    {
        m_strMcuSwVersion = NOT_VALID;
        m_strMcuHwVersion = NOT_VALID;
        m_strMcuSerialNo  = NOT_VALID;
        m_strMcuProdId    = NOT_VALID;

        setMcuInfosCollected( false );
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void DeviceInfoCollector::mcuVersionTimeout()
{
    m_strMcuSwVersion = NOT_VALID;
    m_strMcuHwVersion = NOT_VALID;
    m_strMcuSerialNo  = NOT_VALID;
    m_strMcuProdId    = NOT_VALID;

    // force update!
    m_bMcuInfosCollected = true;
    emit mcuInfosChanged();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void DeviceInfoCollector::collectEkxUiVersion()
{
    QFile file( MainStatemachine::rootPath() + QString(VERSION_PATH) + QString(EKXUI_FILENAME) );
    if ( file.open(QIODevice::ReadOnly) )
    {
        QByteArray ba = file.readAll();
        file.close();

        QJsonDocument doc = QJsonDocument::fromJson(ba);

        QJsonObject obj = doc.object();

        QString version_no   =  obj["VERSION_NO"].toString();

        m_strEkxUiSwVersion = version_no;
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void DeviceInfoCollector::collectEkxUiHWVersion()
{
    QString boardname     = "";
    QString boardRevision = "";
    QString serialNumber  = "";

    bool success = true;

    {
        QFile bnameFile( MainStatemachine::rootPath() + QString(HWVERSION_PATH) + QString(HW_VERSION_BNAME) );
        if ( bnameFile.open(QIODevice::ReadOnly) )
        {
            boardname = bnameFile.readAll();
            bnameFile.close();
            boardname.chop(1);
        }
        else
        {
            qCritical() << "collectEkxUiHWVersion(): can't read board name file from" << bnameFile.fileName();
            success = false;
        }
    }

    {
        QFile brevFile( MainStatemachine::rootPath() + QString(HWVERSION_PATH) + QString(HW_VERSION_BREV) );
        if ( brevFile.open(QIODevice::ReadOnly) && success )
        {
            boardRevision = brevFile.readAll();
            brevFile.close();
            boardRevision.chop(1);
        }
        else
        {
            qCritical() << "collectEkxUiHWVersion(): can't read board revision file from" << brevFile.fileName();
            success = false;
        }
    }

    {
        QFile sernrFile( MainStatemachine::rootPath() + QString(HWVERSION_PATH) + QString(HW_VERSION_SERNR) );
        if ( sernrFile.open(QIODevice::ReadOnly) )
        {
            serialNumber = sernrFile.readAll();
            sernrFile.close();
            serialNumber.chop(1);

            m_strEkxUiSerialNo = QString(serialNumber).replace(":","");
        }
        else
        {
            qCritical() << "collectEkxUiHWVersion(): can't read serial number file from" << sernrFile.fileName();
            success = false;
        }
    }

    if ( success )
    {
        m_strEkxUiHwVersion = QString( "UXG-WAL-" + boardRevision );
        qInfo() << "collectEkxUiHWVersion(): read board:" << m_strEkxUiHwVersion << " serial#:" << m_strEkxUiSerialNo;
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void DeviceInfoCollector::collectEspHostConnectorVersion()
{
    QFile file( MainStatemachine::rootPath() + QString(VERSION_PATH) + QString(ESPHOST_FILENAME) );
    if ( file.open(QIODevice::ReadOnly) )
    {
        QByteArray ba = file.readAll();
        file.close();

        QJsonDocument doc = QJsonDocument::fromJson(ba);

        QJsonObject obj = doc.object();

        QString version_no   =  obj["VERSION_NO"].toString();

        m_strEspHostConnectorSwVersion = version_no;
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void DeviceInfoCollector::collectEspVersion()
{
    if( m_pMainStatemachine->espDriver()->at24c16Driver()->isValid() )
    {
        m_strEspProdId   = m_pMainStatemachine->espDriver()->at24c16Driver()->productId();
        m_strEspSerialNo = m_pMainStatemachine->espDriver()->at24c16Driver()->serialNumber();
    }
    else
    {
        m_strEspProdId   = NOT_VALID;
        m_strEspSerialNo = NOT_VALID;
    }

    //qInfo() << "collectEspVersion " << m_pMainStatemachine->espDriver()->swUpdateInDriver()->getEspSwVersion();

    if( m_pMainStatemachine->espDriver()->swUpdateInDriver()->isValid() )
    {
        m_strEspSwVersion = m_pMainStatemachine->espDriver()->swUpdateInDriver()->swVersion();
        setEspInfosCollected( true );
    }
    else
    {
        m_strEspSwVersion = NOT_VALID;
        setEspInfosCollected( false );
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void DeviceInfoCollector::collectPackageInfo()
{
    QFile file( MainStatemachine::rootPath() + QString( VERSION_PATH ) + QString( PACKAGE_FILENAME ) );
    if ( file.open( QIODevice::ReadOnly ) )
    {
        QByteArray ba = file.readAll();
        file.close();

        QJsonDocument jsonDoc( QJsonDocument::fromJson( ba ) );

        QJsonObject jsonObj = jsonDoc.object();

        m_strPackageSwVersion = jsonObj["VERSION_NO"].toString();
        m_strPackageBuildNo   = jsonObj["BUILD_NO"].toString();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool DeviceInfoCollector::mcuInfosCollected() const
{
    if ( m_pMainStatemachine->isMcuIgnored() )
        return true;

    return m_bMcuInfosCollected;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void DeviceInfoCollector::setMcuInfosCollected( bool newMcuInfo )
{
    if( m_bMcuInfosCollected != newMcuInfo )
    {
        //qInfo() << "setMcuInfosCollected" << newMcuInfo << "/" << m_strMcuProdId << "/" << m_strMcuSerialNo << "/" << m_strMcuSwVersion;
        m_bMcuInfosCollected = newMcuInfo;
    }
    // each set emits a change!
    emit mcuInfosChanged();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool DeviceInfoCollector::espInfosCollected() const
{
#ifndef SIMULATION_BUILD
    return m_bEspInfosCollected;
#else
    return true;
#endif
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void DeviceInfoCollector::setEspInfosCollected( bool newEspInfo )
{
    if( m_bEspInfosCollected != newEspInfo )
    {
        m_bEspInfosCollected = newEspInfo;
        emit espInfosChanged();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

const QString & DeviceInfoCollector::mcuSwVersion() const
{
    return m_strMcuSwVersion;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

const QString & DeviceInfoCollector::mcuHwVersion() const
{
    return m_strMcuHwVersion;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

const QString & DeviceInfoCollector::mcuProdId() const
{
    return m_strMcuProdId;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

const QString & DeviceInfoCollector::mcuSerialNo() const
{
    return m_strMcuSerialNo;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

const QString & DeviceInfoCollector::espSwVersion() const
{
    return m_strEspSwVersion;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

const QString & DeviceInfoCollector::espProdId() const
{
    return m_strEspProdId;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

const QString & DeviceInfoCollector::espSerialNo() const
{
    return m_strEspSerialNo;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

const QString & DeviceInfoCollector::espHostConnectorSwVersion() const
{
    return m_strEspHostConnectorSwVersion;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

const QString & DeviceInfoCollector::ekxUiSwVersion() const
{
    return m_strEkxUiSwVersion;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

const QString & DeviceInfoCollector::ekxUiHwVersion() const
{
    return m_strEkxUiHwVersion;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

const QString & DeviceInfoCollector::ekxUiSerialNo() const
{
    return m_strEkxUiSerialNo;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void DeviceInfoCollector::resetMcuInfosCollected()
{
    m_strMcuSwVersion = NOT_VALID;
    m_strMcuHwVersion = NOT_VALID;
    m_strMcuSerialNo  = NOT_VALID;
    m_strMcuProdId    = NOT_VALID;

    setMcuInfosCollected( false );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void DeviceInfoCollector::resetEspInfosCollected()
{
    m_strEspProdId    = NOT_VALID;
    m_strEspSerialNo  = NOT_VALID;
    m_strEspSwVersion = NOT_VALID;

    setEspInfosCollected( false );
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

const QString & DeviceInfoCollector::packageSwVersion() const
{
    return m_strPackageSwVersion;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

const QString & DeviceInfoCollector::packageBuildNo() const
{
    return m_strPackageBuildNo;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

