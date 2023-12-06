///////////////////////////////////////////////////////////////////////////////
///
/// @file McuSimulator.cpp
///
/// @brief Implementation file of class McuSimulator.
///
/// @author Ultratronik GmbH \n
///         Dornierstr. 9 \n
///         D-82205 Gilching \n
///         Germany
///         http://www.ultratronik.de
///
/// @author written by Gerd Esser, Forschung & Entwicklung, gesser@ultratronik.de
///
/// @date 31.03.2020
///
/// @copyright Copyright 2020 by Hemro International AG \n
///            Hemro International AG \n
///            Länggenstrasse 34 \n
///            CH 8184 Bachenbülach \n
///            Switzerland \n
///            Homepage: www.hemrogroup.com
///
///////////////////////////////////////////////////////////////////////////////

#include "McuSimulator.h"

#include <QDebug>
#include <QCoreApplication>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

McuSimulator::McuSimulator( void )
    : HmiDriver()
{
    readMcuFirmwareConfig();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

McuSimulator::~McuSimulator()
{
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void McuSimulator::readMcuFirmwareConfig()
{
    QFile file( "McuFirmwareConfig.json" );

    if ( file.open( QIODevice::ReadOnly ) )
    {
        QByteArray ba = file.readAll();
        file.close();

        QJsonDocument doc = QJsonDocument::fromJson( ba );

        QJsonObject obj = doc.object();

        setProductId( obj["ProductId"].toString() );
        setHwVersion( obj["HwVersion"].toString() );
        setSwVersion( obj["SwVersion"].toString() );
        setSerialNo( obj["SerialNo"].toString() );

        qInfo() << "readMcuFirmwareConfig():" << productId() << "/" << serialNo() << "/" << hwVersion() << "/" << swVersion();
    }
    else
    {
        qWarning() << "readMcuFirmwareConfig() no config file found, use default version information";
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

