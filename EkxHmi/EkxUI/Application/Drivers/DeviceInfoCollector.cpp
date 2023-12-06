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

#include <QJsonObject>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QDebug>

#include "McuDriver.h"
#include "EspMcuDriver.h"
#include "EspDriver.h"
#include "MainStatemachine.h"
#include "JsonHelper.h"
#include "At24c16Driver.h"
#include "EspSwUpdateDriver.h"

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

DeviceInfoCollector::DeviceInfoCollector( QObject * pParent )
    : QObject( pParent )
{
    MainStatemachine::ensureExistingDirectory( MainStatemachine::rootPath() + VERSION_PATH + "/dummy" );
    MainStatemachine::ensureExistingDirectory( MainStatemachine::rootPath() + HWVERSION_PATH + "/dummy" );
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

void DeviceInfoCollector::create( MainStatemachine * pMainStatemachine )
{
    m_pMainStatemachine = pMainStatemachine;

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

void DeviceInfoCollector::collectMcuVersion()
{
    if ( m_pMainStatemachine->espDriver()->espMcuDriver()->mcuDriver()->isProductIdValid() )
    {
        m_strMcuSwVersion = m_pMainStatemachine->espDriver()->espMcuDriver()->mcuDriver()->swVersion();
        m_strMcuHwVersion = m_pMainStatemachine->espDriver()->espMcuDriver()->mcuDriver()->hwVersion();
        m_strMcuSerialNo  = m_pMainStatemachine->espDriver()->espMcuDriver()->mcuDriver()->serialNo();
        m_strMcuProdId    = m_pMainStatemachine->espDriver()->espMcuDriver()->mcuDriver()->productId();

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
    QJsonObject jsonFile;
    if ( JsonHelper::readJsonFile( MainStatemachine::rootPath() + QString( VERSION_PATH ) + QString( EKXUI_FILENAME ), jsonFile ) )
    {
        m_strEkxUiSwVersion = JsonHelper::read( jsonFile, "VERSION_NO", "Invalid" );
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

#ifdef SIMULATION_BUILD
    boardname          = "HEM-EKX-HMI-P01";
    boardRevision      = "c400";
    serialNumber       = "1C:BC:CD";
    m_strEkxUiSerialNo = QString( serialNumber ).replace( ":", "" );
#else
    {
        QFile bnameFile( MainStatemachine::rootPath() + QString( HWVERSION_PATH ) + QString( HW_VERSION_BNAME ) );
        if ( bnameFile.open( QIODevice::ReadOnly ) )
        {
            boardname = bnameFile.readAll().trimmed();
            bnameFile.close();
        }
        else
        {
            qCritical() << "collectEkxUiHWVersion(): can't read board name file from" << bnameFile.fileName();
            success = false;
        }
    }

    {
        QFile brevFile( MainStatemachine::rootPath() + QString( HWVERSION_PATH ) + QString( HW_VERSION_BREV ) );
        if ( brevFile.open( QIODevice::ReadOnly )
             && success )
        {
            boardRevision = brevFile.readAll().trimmed();
            brevFile.close();
        }
        else
        {
            qCritical() << "collectEkxUiHWVersion(): can't read board revision file from" << brevFile.fileName();
            success = false;
        }
    }

    {
        QFile sernrFile( MainStatemachine::rootPath() + QString( HWVERSION_PATH ) + QString( HW_VERSION_SERNR ) );
        if ( sernrFile.open( QIODevice::ReadOnly ) )
        {
            serialNumber = sernrFile.readAll().trimmed();
            sernrFile.close();

            m_strEkxUiSerialNo = QString( serialNumber ).replace( ":", "" );
        }
        else
        {
            qCritical() << "collectEkxUiHWVersion(): can't read serial number file from" << sernrFile.fileName();
            success = false;
        }
    }
#endif

    if ( success )
    {
        m_strEkxUiHwVersion = QString( boardname + boardRevision );
        qInfo() << "collectEkxUiHWVersion(): read board:" << m_strEkxUiHwVersion << " serial#:" << m_strEkxUiSerialNo;
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void DeviceInfoCollector::collectEspHostConnectorVersion()
{
    QJsonObject jsonFile;
    if ( JsonHelper::readJsonFile( MainStatemachine::rootPath() + QString( VERSION_PATH ) + QString( ESPHOST_FILENAME ), jsonFile ) )
    {
        m_strEspHostConnectorSwVersion = JsonHelper::read( jsonFile, "VERSION_NO", "Invalid" );
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void DeviceInfoCollector::collectEspVersion()
{
    if ( m_pMainStatemachine->espDriver()->at24c16Driver()->isValid() )
    {
        m_strEspProdId   = m_pMainStatemachine->espDriver()->at24c16Driver()->productId();
        m_strEspSerialNo = m_pMainStatemachine->espDriver()->at24c16Driver()->serialNumber();
    }
    else
    {
        m_strEspProdId   = NOT_VALID;
        m_strEspSerialNo = NOT_VALID;
    }

    if ( m_pMainStatemachine->espDriver()->swUpdateDriver()->isValid() )
    {
        m_strEspSwVersion = m_pMainStatemachine->espDriver()->swUpdateDriver()->statusSwVersion();
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
    QJsonObject jsonFile;
    if ( JsonHelper::readJsonFile( MainStatemachine::rootPath() + QString( VERSION_PATH ) + QString( PACKAGE_FILENAME ), jsonFile ) )
    {
        m_strPackageSwVersion = JsonHelper::read( jsonFile, "VERSION_NO", "Invalid" );
        m_strPackageBuildNo   = JsonHelper::read( jsonFile, "BUILD_NO", "Invalid" );
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

bool DeviceInfoCollector::mcuInfosCollected() const
{
    if ( m_pMainStatemachine->isMcuIgnored() )
    {
        return true;
    }

    return m_bMcuInfosCollected;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void DeviceInfoCollector::setMcuInfosCollected( bool newMcuInfo )
{
    if ( m_bMcuInfosCollected != newMcuInfo )
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
    if ( m_bEspInfosCollected != newEspInfo )
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

bool DeviceInfoCollector::isDddPwmVariant() const
{
    bool bIsDddPwm = false;

    if ( ekxUiHwVersion().contains( "P01Q" ) )
    {
        qInfo() << "DeviceInfoCollector::isDddPwmVariant(): detect P01Q -> DDD PWM variant";
        bIsDddPwm = true;
    }
    else if ( ekxUiHwVersion().contains( "P01R" ) )
    {
        qInfo() << "DeviceInfoCollector::isDddPwmVariant(): detect P01R -> DDD ADC variant";
        bIsDddPwm = false;
    }
    else if ( ekxUiHwVersion().contains( "P01A" ) )
    {
        qInfo() << "DeviceInfoCollector::isDddPwmVariant(): detect P01A -> DDD ADC variant";
        bIsDddPwm = false;
    }
    else if ( ekxUiHwVersion().contains( "P01" ) )
    {
        qInfo() << "DeviceInfoCollector::isDddPwmVariant(): detect P01 -> DDD PWM variant";
        bIsDddPwm = true;
    }
    else
    {
        qCritical() << "DddCouple::create(): detect unknown variant (not P01)";
        throw 0;
    }

    return bIsDddPwm;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

