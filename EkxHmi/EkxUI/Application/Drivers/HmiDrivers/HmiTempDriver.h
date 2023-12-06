///////////////////////////////////////////////////////////////////////////////
///
/// @file HmiTempDriver.h
///
/// @brief Header file of class HmiTempDriver.
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
/// @copyright Copyright 2021 by Hemro International AG
///            Hemro International AG
///            Länggenstrasse 34
///            CH 8184 Bachenbülach
///            Switzerland
///            Homepage: www.hemrogroup.com
///
///////////////////////////////////////////////////////////////////////////////

#ifndef HmiTempDriver_h
#define HmiTempDriver_h

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#include <QObject>
#include <QTimer>

class MainStatemachine;

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

class HmiTempDriver
    : public QObject
{
    Q_OBJECT

public:

    HmiTempDriver();
    virtual ~HmiTempDriver() override;

    void create( MainStatemachine * pMainStatemachine );

private:

    QString logFilename();

    void removeOldLogFiles();

private slots:

    void processLogTimeout();

private:

    MainStatemachine * m_pMainStatemachine { nullptr };
    QTimer             m_timerLog;
    QString            m_strHdc2010DeviceFile;

};

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

#endif // HmiTempDriver_h
