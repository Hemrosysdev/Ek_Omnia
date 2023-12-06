///////////////////////////////////////////////////////////////////////////////
///
/// @file HmiTempDriver.cpp
///
/// @brief main application entry point of HmiTempDriver.
///
/// @author Ultratronik GmbH
///         Dornierstr. 9
///         D-82205 Gilching
///         Germany
///         http://www.ultratronik.de
///
/// @author written by Florian Sonntag, Forschung & Entwicklung, gesser@ultratronik.de
///
/// @date 25.01.2021
///
/// @copyright Copyright 2020 by Hemro International AG
///            Hemro International AG
///            Länggenstrasse 34
///            CH 8184 Bachenbülach
///            Switzerland
///            Homepage: www.hemrogroup.com
///
///////////////////////////////////////////////////////////////////////////////

#include "HmiTempDriver.h"

#include <QDebug>
#include <QDate>
#include <QDir>
#include <QtCore/qfiledevice.h>

#include "MainStatemachine.h"

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#define MAX_LOG_FILE_NUM   (10)

#define LOG_DIRECTORY       "/log/HemroEkxTempLogs"
#define DEVICE_DIR          "/sys/devices/platform/soc/40012000.i2c/i2c-0/0-0040/"
#define DEVICE_FILE         "in_temp_raw"

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

HmiTempDriver::HmiTempDriver( MainStatemachine * pMainStatemachine )
    : QObject( pMainStatemachine )
    , m_pMainStatemachine( pMainStatemachine )
{
    MainStatemachine::ensureExistingDirectory( MainStatemachine::rootPath() + DEVICE_DIR + DEVICE_FILE );
    MainStatemachine::ensureExistingDirectory( MainStatemachine::rootPath() + LOG_DIRECTORY + "/dummy" );

    QStringList filters;
    filters << "iio:device*";

    QDir dir( MainStatemachine::rootPath() + DEVICE_DIR );
    QStringList dirs = dir.entryList(filters);

    if ( dirs.size() > 0 )
    {
        m_strHdc2010DeviceFile = QString( MainStatemachine::rootPath() + DEVICE_DIR + dirs.at(0) );
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

HmiTempDriver::~HmiTempDriver()
{
    m_pMainStatemachine = nullptr;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void HmiTempDriver::create()
{
    m_timerLog.setInterval( 60000 );   // each one minute
    m_timerLog.setSingleShot( false );
    m_timerLog.start();

    connect( &m_timerLog, &QTimer::timeout, this, &HmiTempDriver::processLogTimeout );

    processLogTimeout();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

QString HmiTempDriver::logFilename()
{
    QDate currentDate = QDate::currentDate();

    QString strEkxSerialNumber = m_pMainStatemachine->deviceInfoCollector()->ekxUiSerialNo();
    return QString( "HemroEkxTempLog_" + strEkxSerialNumber + "_" + currentDate.toString("yyyyMMdd") + ".csv");
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void HmiTempDriver::removeOldLogFiles()
{
    QDir dir( MainStatemachine::rootPath() + LOG_DIRECTORY );
    QFileInfoList files = dir.entryInfoList( QDir::Files | QDir::NoDotAndDotDot, QDir::Time );

    for( int index = files.size(); index > MAX_LOG_FILE_NUM; --index )
    {
        const QFileInfo & info = files[index - 1];
        QFile::remove( info.absoluteFilePath() );
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void HmiTempDriver::processLogTimeout()
{
    QDateTime currentDateTime =  QDateTime::currentDateTime();

    QFile deviceFile( m_strHdc2010DeviceFile + "/" + DEVICE_FILE );

    QFile logFile( QString( MainStatemachine::rootPath() + LOG_DIRECTORY "/" + logFilename() ) );

    if ( deviceFile.open(QIODevice::ReadOnly) )
    {
        QString tempStr = deviceFile.readAll();
        deviceFile.close();

        // Algorithm from https://www.ti.com/lit/ds/symlink/hdc2010.pdf?ts=1603964145476&ref_url=https%253A%252F%252Fwww.google.com%252F
        int tmp = tempStr.toInt() * 165;
        int realTemp = (tmp >> 16) - 40;

        if ( logFile.open( QIODevice::WriteOnly | QIODevice::Append ) )
        {
            QTextStream stream( &logFile );
            stream << currentDateTime.toString("yyyy-MM-dd hh:mm:ss")<< ";" << QString::number(realTemp) << endl;
            logFile.flush();
            logFile.close();
        }
    }

    removeOldLogFiles();
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////
